#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct QueueBatchResource
	{
		std::optional<vk::PipelineStageFlags> wait_dst_stage_mask = std::nullopt;
		CMShared<Semaphore> wait_semaphore = nullptr;
		CMShared<Semaphore> signal_semaphore = nullptr;
	};

	class QueueBatchManager
	{
	public:
		explicit QueueBatchManager();
		explicit QueueBatchManager(Context* c_);
		explicit QueueBatchManager(Context& c_);

        ~QueueBatchManager() {}

        QueueBatchManager(const QueueBatchManager& o) :
            c(o.c),
            command_buffers(o.command_buffers),
            queue_batch_resources(o.queue_batch_resources)
        {
        }

        QueueBatchManager(QueueBatchManager&& o) :
            c(std::move(o.c)),
            command_buffers(std::move(o.command_buffers)),
            queue_batch_resources(std::move(o.queue_batch_resources))
        {
        }

        QueueBatchManager& operator=(const QueueBatchManager& o)
        {
            if (&o != this)
            {
                c = o.c;
                command_buffers = o.command_buffers;
                queue_batch_resources = o.queue_batch_resources;
            }
            return *this;
        }

        QueueBatchManager& operator=(QueueBatchManager&& o)
        {
            if (&o != this)
            {
                std::swap(c, o.c);
                std::swap(command_buffers, o.command_buffers);
                std::swap(queue_batch_resources, o.queue_batch_resources);
            }
            return *this;
        }

		void EnqueueCommandBuffer(CMShared<CommandBuffer> command_buffer);
		void EnqueueWait(vk::PipelineStageFlags wait_dst_stage_mask, CMShared<Semaphore> wait_semaphore);
		void EnqueueSignal(CMShared<Semaphore> signal_semaphore);
		Error Submit(QueueType queue_type, CMShared<Fence> fence = nullptr);

	private:
		Context* c{};

		std::vector<CMShared<CommandBuffer>> command_buffers;
		std::vector<QueueBatchResource> queue_batch_resources;
	};
}