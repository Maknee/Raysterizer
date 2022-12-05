#include "include/acceleration_structure.h"

namespace RaysterizerEngine
{
    vk::AccelerationStructureGeometryKHR GeometryDescription::GetGeometryTrianglesData() const
    {
        auto max_vertex = std::max(vertex_buffer.count, 1u) - 1u;
        auto geometry_triangles_data = vk::AccelerationStructureGeometryTrianglesDataKHR{}
            .setVertexFormat(vertex_buffer.format)
            .setVertexData(vertex_buffer.buffer->GetAddress())
            .setVertexStride(vertex_buffer.stride)
            .setMaxVertex(max_vertex)
            .setIndexType(index_buffer.index_type)
            .setIndexData(index_buffer.buffer->GetAddress());
        
        if (transform)
        {
            const auto& transform_data = *transform;
            geometry_triangles_data.setTransformData(vk::DeviceOrHostAddressConstKHR().setHostAddress((void*)&transform_data));
        }

        auto geometry_data = vk::AccelerationStructureGeometryKHR{}
            .setGeometryType(vk::GeometryTypeKHR::eTriangles)
            .setGeometry(
                vk::AccelerationStructureGeometryDataKHR{}
                .setTriangles(geometry_triangles_data)
            )
            .setFlags(geometry_flags);

        return geometry_data;
    }

    /*
    namespace
    {
        vk::AccelerationStructureInstanceKHR InstanceFromEntity(vk::Device device,
            const Raysterizer::Entity& entity,
            const Model& model,
            uint32_t instance_id)
        {
            vk::AccelerationStructureInstanceKHR instance = {};
            instance.setInstanceCustomIndex(instance_id);
            instance.setMask(0xff);
            //instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable);
            //instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable | vk::GeometryInstanceFlagBitsKHR::eForceNoOpaque);
            //vk::GeometryInstanceFlagsKHR flags = vk::GeometryInstanceFlagBitsKHR::eForceOpaque;
            vk::GeometryInstanceFlagsKHR flags = vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable;// | vk::GeometryInstanceFlagBitsKHR::eForceOpaque;
#ifdef ENABLE_ANY_HIT
            flags &= ~vk::GeometryInstanceFlagBitsKHR::eForceOpaque;
            flags |= vk::GeometryInstanceFlagBitsKHR::eForceNoOpaque;
#endif
            instance.setFlags(flags);

            instance.setInstanceShaderBindingTableRecordOffset(entity.hit_group);

            // 3x4 row-major affine transformation matrix.
#ifdef RAYSTERIZER_TLAS_INSTANCE_TRANSPOSE_TRANSFORM
            const auto transform = glm::transpose(entity.transform);
#else
            const auto transform = entity.transform;
#endif
            memcpy(&instance.transform, &transform, sizeof(instance.transform));
            //instance.transform.matrix = *reinterpret_cast<const vk::ArrayWrapper2D<float, 3, 4>*>(&transform);

            const auto blas_address = device.getAccelerationStructureAddressKHR({ vk::AccelerationStructureKHR(*model.GetBlas()) });
            instance.setAccelerationStructureReference(blas_address);

            return instance;
        }

        inline Raysterizer::Vulkan::Buffer CreateAccelerationStructureBuffer(vk::AccelerationStructureBuildSizesInfoKHR build_size_info)
        {
            auto buffer =
                Raysterizer::Vulkan::Buffer(build_size_info.accelerationStructureSize,
                    vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

            return buffer;
        }

        inline Raysterizer::Vulkan::Buffer CreateScratchBuffer(vk::DeviceSize size)
        {
            auto buffer =
                Raysterizer::Vulkan::Buffer(size,
                    vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

            return buffer;
        }
    }

    TopLevelAccelerationStructure::TopLevelAccelerationStructure(const vk::CommandBuffer& cmd,
        const Raysterizer::Scene& scene,
        vk::BuildAccelerationStructureFlagsKHR flags,
        bool update)
    {
        auto& vc = VulkanContext::Get();

        std::vector<VkAccelerationStructureInstanceKHR> geometry_instances;
        std::vector<InstanceOffsetTableEntry> instance_offset_table_entries;

        // Grab instances from scene.
        for (const auto& entity : scene.GetEntities())
        {
            const auto& model = AssignOrPanic(scene.GetModel(entity.GetModelHandle()));

            //const auto instance = InstanceFromEntity(*vc.GetDevice(), entity, model, (uint32_t)geometry_instances.size());
            const auto instance = InstanceFromEntity(*vc.GetDevice(), entity, model, entity.instance_id);
            geometry_instances.push_back(instance);

            const auto& vertex_buffer_view = model.GetVertexBufferView();
            const auto& index_buffer_view = model.GetIndexBufferView();
            const auto& material_buffer_view = entity.material_buffer_view;

            auto& entry = instance_offset_table_entries.emplace_back();

            entry.vertexBufferOffset = vertex_buffer_view.OffsetInElements();
            entry.indexBufferOffset = index_buffer_view.OffsetInElements();
            entry.materialBufferOffset = material_buffer_view.OffsetInElements();
        }

        // Make sure the copy of the instance buffer are copied before triggering the
        // acceleration structure build
        instances = Raysterizer::Vulkan::CreateBufferFromData(geometry_instances, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
        instance_offset_table = Raysterizer::Vulkan::CreateBufferFromData(instance_offset_table_entries, vk::BufferUsageFlagBits::eStorageBuffer);

        vk::DeviceOrHostAddressConstKHR instance_address{};
        instance_address.setDeviceAddress(instances.Address());

        vk::AccelerationStructureGeometryInstancesDataKHR instances_data{};
        instances_data.setArrayOfPointers(false);
        instances_data.setData(instance_address);

        vk::AccelerationStructureGeometryKHR geometry{};

        vk::GeometryFlagsKHR geometry_flags = vk::GeometryFlagBitsKHR::eOpaque;
#ifdef ENABLE_ANY_HIT
        geometry_flags &= ~vk::GeometryFlagBitsKHR::eOpaque;
        geometry_flags |= vk::GeometryFlagBitsKHR::eNoDuplicateAnyHitInvocation;
#endif

        geometry.setFlags(geometry_flags);
        geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
        geometry.setGeometry(instances_data);

        vk::AccelerationStructureBuildGeometryInfoKHR geometry_info{};
        geometry_info.setFlags(flags);
        geometry_info.setGeometryCount(1);
        geometry_info.setGeometries(geometry);
        geometry_info.setMode(update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild);
        geometry_info.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
        geometry_info.srcAccelerationStructure = vk::AccelerationStructureKHR{};
        geometry_info.dstAccelerationStructure = vk::AccelerationStructureKHR{};

        vk::AccelerationStructureBuildSizesInfoKHR size_info =
            vc.GetDevice()->getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                geometry_info,
                geometry_instances.size());

        acceleration_structure_buffer = CreateAccelerationStructureBuffer(size_info);

        if (!update)
        {
            vk::AccelerationStructureCreateInfoKHR create_info = {};
            create_info.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
            create_info.setSize(size_info.accelerationStructureSize);
            create_info.setBuffer(acceleration_structure_buffer);

            acceleration_structure = vc.GetDevice()->createAccelerationStructureKHRUnique(create_info);
            geometry_info.dstAccelerationStructure = *acceleration_structure;
        }

        scratch = CreateScratchBuffer(size_info.buildScratchSize);

        geometry_info.srcAccelerationStructure = update ? geometry_info.dstAccelerationStructure : vk::AccelerationStructureKHR{};
        geometry_info.setScratchData(scratch.Address());

        vk::AccelerationStructureBuildRangeInfoKHR build_range_info{};
        build_range_info.setFirstVertex(0);
        build_range_info.setPrimitiveCount(geometry_instances.size());
        build_range_info.setPrimitiveOffset(0);
        build_range_info.setTransformOffset(0);

        auto acceleration_structure_features = vc.GetAccelerationStructureFeatures();
        if (acceleration_structure_features.accelerationStructureHostCommands)
        {
            vk::DeferredOperationKHR deferred_operation{};
            vc.GetDevice()->buildAccelerationStructuresKHR(deferred_operation, geometry_info, &build_range_info);
        }
        else
        {
            auto cmd = vc.GetGraphicsQueue()->CreateCommandBuffer();
            vk::CommandBufferBeginInfo cmd_begin_info{};
            cmd->begin(cmd_begin_info);

            cmd->buildAccelerationStructuresKHR(geometry_info, &build_range_info);

            cmd->end();

            vk::FenceCreateInfo fence_create_info{};
            auto fence = vc.GetDevice()->createFenceUnique(fence_create_info);

            vc.GetGraphicsQueue()->Submit(*cmd, *fence);
            vc.GetDevice()->waitForFences(*fence, true, Raysterizer::DefaultFenceTimeout());
        }

        instances.SetName("TLAS instances");
        scratch.SetName("TLAS scratch");
        acceleration_structure_buffer.SetName("TLAS buffer");
        vc.SetObjectName(*acceleration_structure, "TLAS");
    }

    BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const vk::CommandBuffer& cmd, const Model& model, vk::BuildAccelerationStructureFlagsKHR flags)
    {
        auto& vc = VulkanContext::Get();

        const auto& vertex_buffer_view = model.GetVertexBufferView();
        const auto& index_buffer_view = model.GetIndexBufferView();

        vk::AccelerationStructureGeometryTrianglesDataKHR triangle{};
        triangle.setVertexData(vertex_buffer_view.buffer_address);
        triangle.setVertexFormat(vk::Format::eR32G32B32Sfloat);
        triangle.setVertexStride(vertex_buffer_view.element_size);
        //triangle.setMaxVertex(model.GetVertexView().GetNumElements());
        triangle.setMaxVertex(vertex_buffer_view.SizeInElements());
        triangle.setIndexData(index_buffer_view.buffer_address);

        // Set the index type
        vk::IndexType index_type = vk::IndexType::eNoneKHR;
        if (index_buffer_view.element_size == sizeof(uint32_t))
        {
            index_type = vk::IndexType::eUint32;
        }
        else if (index_buffer_view.element_size == sizeof(uint16_t))
        {
            index_type = vk::IndexType::eUint16;
        }
        else if (index_buffer_view.element_size == sizeof(uint8_t))
        {
            index_type = vk::IndexType::eUint8EXT;
        }
        else
        {
            PANIC("Index type is not avaliable for size {}", index_buffer_view.element_size);
        }

        triangle.setIndexType(index_type);
        triangle.setTransformData(nullptr);

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometry(triangle);
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        vk::AccelerationStructureBuildRangeInfoKHR build_range_info{};
        build_range_info.setFirstVertex(0);
        build_range_info.setPrimitiveCount(model.NumFaces());
        build_range_info.setPrimitiveOffset(0);
        build_range_info.setTransformOffset(0);

        vk::AccelerationStructureBuildGeometryInfoKHR geometry_info{};
        geometry_info.flags = flags;
        geometry_info.geometryCount = 1;
        geometry_info.pGeometries = &geometry;
        geometry_info.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
        geometry_info.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
        geometry_info.srcAccelerationStructure = vk::AccelerationStructureKHR{};
        geometry_info.dstAccelerationStructure = vk::AccelerationStructureKHR{};

        vk::AccelerationStructureBuildSizesInfoKHR size_info =
            vc.GetDevice()->getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                geometry_info,
                build_range_info.primitiveCount);

        acceleration_structure_buffer = CreateAccelerationStructureBuffer(size_info);

        vk::AccelerationStructureCreateInfoKHR create_info{};
        create_info.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
        create_info.setBuffer(acceleration_structure_buffer);
        create_info.setSize(size_info.accelerationStructureSize);

        acceleration_structure = vc.GetDevice()->createAccelerationStructureKHRUnique(create_info);
        geometry_info.dstAccelerationStructure = *acceleration_structure;

        auto scratch = CreateScratchBuffer(size_info.buildScratchSize);
        geometry_info.setScratchData(scratch.Address());

        auto acceleration_structure_features = vc.GetAccelerationStructureFeatures();
        if (acceleration_structure_features.accelerationStructureHostCommands)
        {
            vk::DeferredOperationKHR deferred_operation{};
            vc.GetDevice()->buildAccelerationStructuresKHR(deferred_operation, geometry_info, &build_range_info);
        }
        else
        {
            auto cmd = vc.GetGraphicsQueue()->CreateCommandBuffer();
            vk::CommandBufferBeginInfo cmd_begin_info{};
            cmd->begin(cmd_begin_info);

            cmd->buildAccelerationStructuresKHR(geometry_info, &build_range_info);

            cmd->end();

            vk::FenceCreateInfo fence_create_info{};
            auto fence = vc.GetDevice()->createFenceUnique(fence_create_info);

            vc.GetGraphicsQueue()->Submit(*cmd, *fence);
            vc.GetDevice()->waitForFences(*fence, true, Raysterizer::DefaultFenceTimeout());
        }

        scratch.SetName("BLAS scratch");
        acceleration_structure_buffer.SetName("BLAS buffer");
        vc.SetObjectName(*acceleration_structure, "BLAS");
    }

    void AccelerationStructureBarrier(const vk::CommandBuffer& cmd)
    {
        vk::MemoryBarrier memory_barrier = {};

        memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
        memory_barrier.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
            vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
            {},
            { memory_barrier },
            {},
            {}
        );
    }
    */
}
