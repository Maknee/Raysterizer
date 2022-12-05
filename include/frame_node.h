#pragma once

#include "pch.h"

using FrameGraphResource = int32_t;

class GraphNode {
public:
    GraphNode() = delete;
    GraphNode(const GraphNode&) = delete;
    GraphNode(GraphNode&&) noexcept = default;
    virtual ~GraphNode() = default;

    GraphNode& operator=(const GraphNode&) = delete;
    GraphNode& operator=(GraphNode&&) noexcept = delete;

protected:
    GraphNode(const std::string_view name, uint32_t id)
        : m_name{ name }, m_id{ id } {}
protected:
    const std::string m_name;
    const uint32_t m_id; // Unique id, matches an array index in FrameGraph
    int32_t m_refCount{ 0 };
};

class PassNode final : public GraphNode {
    friend class FrameGraph;

public:
    [[nodiscard]] bool creates(FrameGraphResource id);
    [[nodiscard]] bool reads(FrameGraphResource id);
    [[nodiscard]] bool writes(FrameGraphResource id);

    [[nodiscard]] bool hasSideEffect() const;
    [[nodiscard]] bool canExecute() const;

private:
    PassNode(const std::string_view name, uint32_t id,
        std::unique_ptr<FrameGraphPassConcept>&&);

    FrameGraphResource _read(FrameGraphResource id);
    [[nodiscard]] FrameGraphResource _write(FrameGraphResource id);

private:
    std::unique_ptr<FrameGraphPassConcept> m_exec;

    std::vector<FrameGraphResource> m_creates;
    std::vector<FrameGraphResource> m_reads;
    std::vector<FrameGraphResource> m_writes;

    bool m_hasSideEffect{ false };
};