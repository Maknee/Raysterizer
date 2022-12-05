#include "include/frame_resource_entry.h"

//
// ResourceEntry class:
//

template <typename T> inline T& ResourceEntry::get() {
    return _getModel<T>()->resource;
}
template <typename T>
inline typename const T::Desc& ResourceEntry::getDescriptor() const {
    return _getModel<T>()->descriptor;
}

template <typename T>
inline ResourceEntry::ResourceEntry(uint32_t id, typename T::Desc&& desc,
    T&& obj, uint32_t version, bool imported)
    : m_id{ id }, m_concept{ std::make_unique<Model<T>>(
                  std::forward<T::Desc>(desc), std::forward<T>(obj)) },
    m_version{ version }, m_imported{ imported } {}

template <typename T> inline auto* ResourceEntry::_getModel() const {
    auto* model = dynamic_cast<Model<T> *>(m_concept.get());
    assert(model && "Invalid type");
    return model;
}

//
// ResourceEntry::Model class:
//

template <typename T>
inline ResourceEntry::Model<T>::Model(typename T::Desc&& desc, T&& obj)
    : descriptor{ std::move(desc) }, resource{ std::move(obj) } {}

template <typename T>
inline void ResourceEntry::Model<T>::create(void* allocator) {
    resource.create(descriptor, allocator);
}
template <typename T>
inline void ResourceEntry::Model<T>::destroy(void* allocator) {
    resource.destroy(descriptor, allocator);
}

template <typename T>
inline std::string ResourceEntry::Model<T>::toString() const {
#if _HAS_CXX20
    if constexpr (has_toString<T>)
#else
    if constexpr (has_toString<T>::value)
#endif
        return T::toString(descriptor);
    else
        return "";
}

std::string ResourceEntry::toString() const { return m_concept->toString(); }

void ResourceEntry::create(void* allocator) {
    assert(isTransient());
    m_concept->create(allocator);
}
void ResourceEntry::destroy(void* allocator) {
    assert(isTransient());
    m_concept->destroy(allocator);
}

uint32_t ResourceEntry::getVersion() const { return m_version; }
bool ResourceEntry::isImported() const { return m_imported; }
bool ResourceEntry::isTransient() const { return !m_imported; }