#include "include/queue_batch_manager.h"

namespace RaysterizerEngine
{
    QueueBatchManager::QueueBatchManager()
    {

    }

    QueueBatchManager::QueueBatchManager(Context* c_) : c(std::move(c_))
    {

    }

    QueueBatchManager::QueueBatchManager(Context& c_) : c(&c_)
    {

    }

    void QueueBatchManager::EnqueueCommandBuffer(CMShared<CommandBuffer> command_buffer)
    {
        if (command_buffer)
        {
            command_buffers.emplace_back(std::move(command_buffer));
        }
    }

    void QueueBatchManager::EnqueueWait(vk::PipelineStageFlags wait_dst_stage_mask, CMShared<Semaphore> wait_semaphore)
    {
        if (wait_semaphore)
        {
            QueueBatchResource queue_batch_resource{ wait_dst_stage_mask, wait_semaphore, {} };
            queue_batch_resources.emplace_back(std::move(queue_batch_resource));
        }
    }

    void QueueBatchManager::EnqueueSignal(CMShared<Semaphore> signal_semaphore)
    {
        if (signal_semaphore)
        {
            QueueBatchResource queue_batch_resource{ {}, {}, signal_semaphore };
            queue_batch_resources.emplace_back(std::move(queue_batch_resource));
        }
    }

    Error QueueBatchManager::Submit(QueueType queue_type, CMShared<Fence> fence)
    {
        std::vector<vk::PipelineStageFlags> wait_dst_stage_masks;
        std::vector<CMShared<Semaphore>> wait_semaphores;
        std::vector<CMShared<Semaphore>> signal_semaphores;

        for (const auto& queue_batch_resource : queue_batch_resources)
        {
            const auto& [wait_dst_stage_mask, wait_semaphore, signal_semaphore] = queue_batch_resource;

            if (wait_dst_stage_mask)
            {
                wait_dst_stage_masks.emplace_back(*wait_dst_stage_mask);
            }

            if (wait_semaphore)
            {
                wait_semaphores.emplace_back(wait_semaphore);
            }

            if (signal_semaphore)
            {
                signal_semaphores.emplace_back(signal_semaphore);
            }
        }

        /*
        if (command_buffers.empty() && wait_dst_stage_masks.empty() && wait_semaphores.empty() && signal_semaphores.empty())
        {
            return NoError();
        }
        */

        ReturnIfError(c->Submit(queue_type, command_buffers, fence, wait_dst_stage_masks, wait_semaphores, signal_semaphores));

        command_buffers.clear();
        queue_batch_resources.clear();

        return NoError();
    }
}
