#include "include/frame_resource_node.h"

ResourceNode::ResourceNode(const std::string_view name, uint32_t id,
    uint32_t resourceId, uint32_t version)
    : GraphNode{ name, id }, m_resourceId{ resourceId }, m_version{ version } {}