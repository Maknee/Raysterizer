#include "include/frame_graph.h"

//
// FrameGraph class:
//

template <typename Data, typename Setup, typename Execute>
inline const Data& FrameGraph::addCallbackPass(const std::string_view name,
    Setup&& setup, Execute&& exec) {
    static_assert(std::is_invocable<Setup, Builder&, Data&>::value,
        "Invalid setup callback");
    static_assert(std::is_invocable<Execute, const Data&,
        FrameGraphPassResources&, void*>::value,
        "Invalid exec callback");
    static_assert(sizeof(Execute) < 1024, "Execute captures too much");

    auto* pass = new FrameGraphPass<Data, Execute>(std::forward<Execute>(exec));
    auto& passNode =
        _createPassNode(name, std::unique_ptr<FrameGraphPass<Data, Execute>>(pass));
    Builder builder{ *this, passNode };
    std::invoke(setup, builder, pass->data);
    return pass->data;
}
template <typename Setup, typename Execute>
inline void FrameGraph::addCallbackPass(const std::string_view name,
    Setup&& setup, Execute&& exec) {
    struct NoData {};
    addCallbackPass<NoData>(name, setup, std::forward<Execute>(exec));
}

_VIRTUALIZABLE_CONCEPT_IMPL
inline typename const T::Desc&
FrameGraph::getDescriptor(FrameGraphResource id) {
    return _getResourceEntry(id)._getModel<T>()->descriptor;
}

_VIRTUALIZABLE_CONCEPT_IMPL
inline FrameGraphResource FrameGraph::import(const std::string_view name,
    typename T::Desc&& desc,
    T&& resource) {
    const auto resourceId = static_cast<uint32_t>(m_resourceRegistry.size());
    m_resourceRegistry.emplace_back(
        ResourceEntry{ resourceId, std::forward<T::Desc>(desc),
                      std::forward<T>(resource), kResourceInitialVersion, true });
    return _createResourceNode(name, resourceId).m_id;
}

_VIRTUALIZABLE_CONCEPT_IMPL
inline FrameGraphResource FrameGraph::_create(const std::string_view name,
    typename T::Desc&& desc) {
    const auto resourceId = static_cast<uint32_t>(m_resourceRegistry.size());
    m_resourceRegistry.emplace_back(ResourceEntry{
      resourceId, std::forward<T::Desc>(desc), T{}, kResourceInitialVersion });
    return _createResourceNode(name, resourceId).m_id;
}

//
// FrameGraph::Builder class:
//

_VIRTUALIZABLE_CONCEPT_IMPL
inline FrameGraphResource
FrameGraph::Builder::create(const std::string_view name,
    typename T::Desc&& desc) {
    const auto id = m_frameGraph._create<T>(name, std::move(desc));
    return m_passNode.m_creates.emplace_back(id);
}

//
// FrameGraphPassResources class:
//

_VIRTUALIZABLE_CONCEPT_IMPL
inline T& FrameGraphPassResources::get(FrameGraphResource id) {
    assert(m_passNode.reads(id) || m_passNode.creates(id) ||
        m_passNode.writes(id));
    return m_frameGraph._getResourceEntry(id).get<T>();
}

_VIRTUALIZABLE_CONCEPT_IMPL
inline typename const T::Desc&
FrameGraphPassResources::getDescriptor(FrameGraphResource id) const {
    assert(m_passNode.reads(id) || m_passNode.creates(id) ||
        m_passNode.writes(id));
    return m_frameGraph._getResourceEntry(id).getDescriptor<T>();
}

namespace {

    struct StyleSheet {
        bool useClusters{ true };
        const char* rankDir{ "TB" }; // TB, LR, BT, RL

        struct {
            const char* name{ "helvetica" };
            int32_t size{ 10 };
        } font;
        struct {
            // https://graphviz.org/doc/info/colors.html
            struct {
                const char* executed{ "orange" };
                const char* culled{ "lightgray" };
            } pass;
            struct {
                const char* imported{ "lightsteelblue" };
                const char* transient{ "skyblue" };
            } resource;
            struct {
                const char* read{ "olivedrab3" };
                const char* write{ "orangered" };
            } edge;
        } color;
    };

} // namespace

//
// FrameGraph class:
//

bool FrameGraph::isValid(FrameGraphResource id) const {
    const auto& node = _getResourceNode(id);
    auto& resource = m_resourceRegistry[node.m_resourceId];
    return node.m_version == resource.m_version;
}

void FrameGraph::compile() {
    for (auto& pass : m_passNodes) {
        pass.m_refCount = pass.m_writes.size();
        for (auto id : pass.m_reads) {
            auto& consumed = m_resourceNodes[id];
            consumed.m_refCount++;
        }
        for (auto id : pass.m_writes) {
            auto& written = m_resourceNodes[id];
            written.m_producer = &pass;
        }
    }

    // -- Culling:

    std::stack<ResourceNode*> unreferencedResources;
    for (auto& node : m_resourceNodes)
        if (node.m_refCount == 0) unreferencedResources.push(&node);

    while (!unreferencedResources.empty()) {
        auto* unreferencedResource = unreferencedResources.top();
        unreferencedResources.pop();
        PassNode* producer{ unreferencedResource->m_producer };
        if (producer == nullptr || producer->hasSideEffect()) continue;

        assert(producer->m_refCount >= 1);
        if (--producer->m_refCount == 0) {
            for (auto id : producer->m_reads) {
                auto& node = m_resourceNodes[id];
                if (--node.m_refCount == 0) unreferencedResources.push(&node);
            }
        }
    }

    // -- Calculate resources lifetime:

    for (auto& pass : m_passNodes) {
        if (pass.m_refCount == 0) continue;

        for (auto id : pass.m_creates)
            _getResourceEntry(id).m_producer = &pass;
        for (auto id : pass.m_writes)
            _getResourceEntry(id).m_last = &pass;
        for (auto id : pass.m_reads)
            _getResourceEntry(id).m_last = &pass;
    }
}
void FrameGraph::execute(void* context, void* allocator) {
    for (auto& pass : m_passNodes) {
        if (!pass.canExecute()) continue;

        for (auto id : pass.m_creates)
            _getResourceEntry(id).create(allocator);

        FrameGraphPassResources resources{ *this, pass };
        std::invoke(*pass.m_exec, resources, context);

        for (auto& entry : m_resourceRegistry)
            if (entry.m_last == &pass && entry.isTransient())
                entry.destroy(allocator);
    }
}

void FrameGraph::exportGraphviz(std::ostream& os) const {
    // https://www.graphviz.org/pdf/dotguide.pdf

    static StyleSheet style;

    os << "digraph FrameGraph {" << std::endl;
    os << "graph [style=invis, rankdir=\"" << style.rankDir
        << "\" ordering=out, splines=spline]" << std::endl;
    os << "node [shape=record, fontname=\"" << style.font.name
        << "\", fontsize=" << style.font.size << ", margin=\"0.2,0.03\"]"
        << std::endl
        << std::endl;

    // -- Define pass nodes

    for (const PassNode& node : m_passNodes) {
        os << "P" << node.m_id << " [label=<{ {<B>" << node.m_name << "</B>} | {"
            << (node.hasSideEffect() ? "&#x2605; " : "")
            << "Refs: " << node.m_refCount << "<BR/> Index: " << node.m_id
            << "} }> style=\"rounded,filled\", fillcolor="
            << ((node.m_refCount > 0 || node.hasSideEffect())
                ? style.color.pass.executed
                : style.color.pass.culled);

        os << "]" << std::endl;
    }
    os << std::endl;

    // -- Define resource nodes

    for (const ResourceNode& node : m_resourceNodes) {
        const auto& entry = m_resourceRegistry[node.m_resourceId];
        os << "R" << entry.m_id << "_" << node.m_version << " [label=<{ {<B>"
            << node.m_name << "</B>";
        if (node.m_version > kResourceInitialVersion) {
            // FIXME: Bold text overlaps regular text
            os << "   <FONT>v" + std::to_string(node.m_version) + "</FONT>";
        }
        os << "<BR/>" << entry.toString() << "} | {Index: " << entry.m_id << "<BR/>"
            << "Refs : " << node.m_refCount << "} }> style=filled, fillcolor="
            << (entry.isImported() ? style.color.resource.imported
                : style.color.resource.transient);

        os << "]" << std::endl;
    }
    os << std::endl;

    // -- Each pass node points to resource that it writes

    for (const PassNode& node : m_passNodes) {
        os << "P" << node.m_id << " -> { ";
        for (auto id : node.m_writes) {
            const auto& written = m_resourceNodes[id];
            os << "R" << written.m_resourceId << "_" << written.m_version << " ";
        }
        os << "} [color=" << style.color.edge.write << "]" << std::endl;
    }

    // -- Each resource node points to pass where it's consumed

    os << std::endl;
    for (const ResourceNode& node : m_resourceNodes) {
        os << "R" << node.m_resourceId << "_" << node.m_version << " -> { ";
        // find all readers of this resource node
        for (const PassNode& pass : m_passNodes) {
            for (const auto id : pass.m_reads)
                if (id == node.m_id) os << "P" << pass.m_id << " ";
        }
        os << "} [color=" << style.color.edge.read << "]" << std::endl;
    }
    os << std::endl;

    // -- Clusters:

    if (style.useClusters) {
        for (const PassNode& node : m_passNodes) {
            os << "subgraph cluster_" << node.m_id << " {" << std::endl;

            os << "P" << node.m_id << " ";
            for (auto id : node.m_creates) {
                const auto& r = m_resourceNodes[id];
                os << "R" << r.m_resourceId << "_" << r.m_version << " ";
            }
            os << std::endl << "}" << std::endl;
        }
        os << std::endl;

        os << "subgraph cluster_imported_resources {" << std::endl;
        os << "graph [style=dotted, fontname=\"helvetica\", label=< "
            "<B>Imported</B> >]"
            << std::endl;

        for (const ResourceEntry& entry : m_resourceRegistry) {
            if (entry.isImported()) os << "R" << entry.m_id << "_1 ";
        }
        os << std::endl << "}" << std::endl << std::endl;
    }

    os << "}";
}

// ---

PassNode&
FrameGraph::_createPassNode(const std::string_view name,
    std::unique_ptr<FrameGraphPassConcept>&& base) {
    const auto id = static_cast<uint32_t>(m_passNodes.size());
    return m_passNodes.emplace_back(PassNode{ name, id, std::move(base) });
}

ResourceNode& FrameGraph::_createResourceNode(const std::string_view name,
    uint32_t resourceId) {
    const auto id = static_cast<uint32_t>(m_resourceNodes.size());
    return m_resourceNodes.emplace_back(
        ResourceNode{ name, id, resourceId, kResourceInitialVersion });
}
FrameGraphResource FrameGraph::_clone(FrameGraphResource id) {
    const auto& node = _getResourceNode(id);
    assert(node.m_resourceId < m_resourceRegistry.size());
    auto& entry = m_resourceRegistry[node.m_resourceId];
    entry.m_version++;

    const auto cloneId = static_cast<uint32_t>(m_resourceNodes.size());
    m_resourceNodes.emplace_back(
        ResourceNode{ node.m_name, cloneId, node.m_resourceId, entry.getVersion() });
    return cloneId;
}

const ResourceNode& FrameGraph::_getResourceNode(FrameGraphResource id) const {
    assert(id < m_resourceNodes.size());
    return m_resourceNodes[id];
}
ResourceEntry& FrameGraph::_getResourceEntry(FrameGraphResource id) {
    const auto& node = _getResourceNode(id);
    assert(node.m_resourceId < m_resourceRegistry.size());
    return m_resourceRegistry[node.m_resourceId];
}

// ---

std::ostream& operator<<(std::ostream& os, const FrameGraph& fg) {
    fg.exportGraphviz(os);
    return os;
}

//
// FrameGraph::Builder class:
//

FrameGraphResource FrameGraph::Builder::read(FrameGraphResource id) {
    assert(m_frameGraph.isValid(id));
    return m_passNode._read(id);
}
FrameGraphResource FrameGraph::Builder::write(FrameGraphResource id) {
    assert(m_frameGraph.isValid(id));
    if (m_frameGraph._getResourceEntry(id).isImported()) setSideEffect();

    if (m_passNode.creates(id)) {
        return m_passNode._write(id);
    }
    else {
        // Writing to a texture produces a renamed handle.
        // This allows us to catch errors when resources are modified in
        // undefined order (when same resource is written by different passes).
        // Renaming resources enforces a specific execution order of the render
        // passes.
        m_passNode._read(id);
        return m_passNode._write(m_frameGraph._clone(id));
    }
}

FrameGraph::Builder& FrameGraph::Builder::setSideEffect() {
    m_passNode.m_hasSideEffect = true;
    return *this;
}

FrameGraph::Builder::Builder(FrameGraph& fg, PassNode& node)
    : m_frameGraph{ fg }, m_passNode{ node } {}

//
// FrameGraphPassResources class:
//

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, PassNode& node)
    : m_frameGraph{ fg }, m_passNode{ node } {}