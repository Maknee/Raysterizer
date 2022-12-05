#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
    // Compaction related
    struct QueryPoolCreateInfo
    {
        vk::QueryPoolCreateInfo query_pool_create_info;

    public:
        bool operator==(const QueryPoolCreateInfo& o) const noexcept {
            return query_pool_create_info == o.query_pool_create_info;
        }
    };

    struct QueryPool
    {
        vk::QueryPool query_pool;

    public:
        const vk::QueryPool& operator*() const noexcept
        {
            return query_pool;
        }

        operator vk::QueryPool() noexcept
        {
            return query_pool;
        }
    };

    struct GeometryBuffer
    {
        CMShared<Buffer> buffer;
        uint32_t count;
        uint32_t stride;
    };

    struct GeometryVertexBuffer : public GeometryBuffer
    {
        vk::Format format;
    };

    struct GeometryIndexBuffer : public GeometryBuffer
    {
        vk::IndexType index_type;
    };

    struct GeometryDescription
    {
        GeometryVertexBuffer vertex_buffer;
        GeometryIndexBuffer index_buffer;

        vk::GeometryFlagsKHR geometry_flags{};
        std::optional<glm::mat4x3> transform{};

        vk::AccelerationStructureGeometryKHR GetGeometryTrianglesData() const;
    };

    struct BottomLevelAccelerationStructureCreateInfo
    {
        std::vector<GeometryDescription> geometry_descriptions;
        vk::BuildAccelerationStructureFlagsKHR build_flags;
    };

    struct BottomLevelAccelerationStructure;
    struct BottomLevelAccelerationStructureUpdateInfo
    {
        std::vector<GeometryDescription> geometry_descriptions;
        vk::BuildAccelerationStructureFlagsKHR build_flags;
        CMShared<BottomLevelAccelerationStructure> blas;
    };

    struct AccelerationStructureWithBuffer
    {
        CMShared<Buffer> buffer;
        vk::AccelerationStructureKHR acceleration_structure{};
        vk::DeviceAddress acceleration_structure_address{};

    public:
        bool operator==(const AccelerationStructureWithBuffer& o) const noexcept {
            return CheckEquality(buffer, o.buffer) &&
                acceleration_structure == o.acceleration_structure &&
                acceleration_structure_address == o.acceleration_structure_address;
        }
    };

    struct BottomLevelAccelerationStructure
    {
        BottomLevelAccelerationStructureCreateInfo create_info;

        AccelerationStructureWithBuffer acceleration_structure_with_buffer;
        AccelerationStructureWithBuffer compact_acceleration_structure_with_buffer{};

        vk::AccelerationStructureBuildSizesInfoKHR build_sizes{};

        bool request_compaction = false;
        bool compacted = false;

        // associated with compaction
        //CMShared<QueryPool> query_pool{};

        uint32_t first_query = 0;
        uint32_t query_count = 1;

    public:
        vk::DeviceAddress GetAccelerationStructureAddress() const
        {
            return acceleration_structure_with_buffer.acceleration_structure_address;
        }

    public:
        const vk::AccelerationStructureKHR& operator*() const noexcept
        {
            return acceleration_structure_with_buffer.acceleration_structure;
        }

        operator vk::AccelerationStructureKHR() noexcept
        {
            return acceleration_structure_with_buffer.acceleration_structure;
        }
    };

    struct TopLevelAccelerationStructureCreateInfo
    {
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        vk::BuildAccelerationStructureFlagsKHR build_flags;

        std::vector<CMShared<BottomLevelAccelerationStructure>> blases;

        vk::GeometryFlagsKHR geometry_flags = vk::GeometryFlagBitsKHR::eOpaque;
        bool update = false;
        std::optional<vk::AccelerationStructureKHR> existing_acceleration_structure{};
        CMShared<Buffer> existing_instance_buffer{};
    };

    struct TopLevelAccelerationStructure
    {
        TopLevelAccelerationStructureCreateInfo tlas_create_info;
        AccelerationStructureWithBuffer acceleration_structure_with_buffer;

        // Holds memory for vk::AccelerationStructureInstanceKHR
        CMShared<Buffer> instance_buffer;
    public:
        const vk::AccelerationStructureKHR& operator*() const noexcept
        {
            return acceleration_structure_with_buffer.acceleration_structure;
        }

        operator vk::AccelerationStructureKHR() noexcept
        {
            return acceleration_structure_with_buffer.acceleration_structure;
        }

        bool operator==(const TopLevelAccelerationStructure& o) const noexcept {
            return acceleration_structure_with_buffer == o.acceleration_structure_with_buffer;
        }
    };
}

namespace std
{
    using namespace RaysterizerEngine;

    template<>
    struct hash<QueryPoolCreateInfo>
    {
        size_t operator()(const QueryPoolCreateInfo& o) const noexcept
        {
            size_t h{};
            HashCombine(h, Hash(o.query_pool_create_info));
            return h;
        }
    };

    template<>
    struct hash<AccelerationStructureWithBuffer>
    {
        size_t operator()(const AccelerationStructureWithBuffer& o) const noexcept
        {
            size_t h{};
            if (o.buffer)
            {
                HashCombine(h, StdHash(o.buffer));
            }
            HashCombine(h, Hash(o.acceleration_structure));
            return h;
        }
    };

    template<>
    struct hash<TopLevelAccelerationStructure>
    {
        size_t operator()(const TopLevelAccelerationStructure& o) const noexcept
        {
            size_t h{};
            HashCombine(h, StdHash(o.acceleration_structure_with_buffer));
            return h;
        }
    };
}