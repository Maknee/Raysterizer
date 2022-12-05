#pragma once

#include "pch.h"

namespace RaysterizerEngine
{

#define ReturnIfVkError(x) \
    do \
    { \
      vk::Result err = static_cast<vk::Result>(x); \
      if (err != vk::Result::eSuccess) \
      { \
        return StringError("Vulkan Error: {}", vk::to_string(err)); \
      } \
    } \
    while(0) \

#define AssignOrReturnVkError(v, x) \
    auto UNIQUIFY(vk_result) = x; \
    vk::Result UNIQUIFY(vk_err) = static_cast<vk::Result>(UNIQUIFY(vk_result).result); \
    ReturnIfVkError(UNIQUIFY(vk_err)); \
    v = UNIQUIFY(vk_result).value; \

#define VkCheck(x) \
    do \
    { \
      vk::Result err = static_cast<vk::Result>(x); \
      if (err != vk::Result::eSuccess) \
      { \
        PANIC("Vulkan Error: {}", vk::to_string(x)); \
      } \
    } \
    while(0) \

#define AssignOrPanicVkError(x) \
    [](auto&& UNIQUIFY(vk_result)) -> decltype(UNIQUIFY(vk_result).value) \
    { \
        VkCheck(UNIQUIFY(vk_result).result); \
        return std::move(UNIQUIFY(vk_result).value); \
    }(x) \

    class HasCompletionCallback
    {
    public:
        Error AddCompletionCallback(std::function<void()> completion_callback);
        Error PerformCompletionCallbacks();

    protected:
        std::vector<std::function<void()>> completion_callbacks;
    };

    class HasInUseOption
    {
    public:
        void SetInUse(bool in_use_ = true);
        bool HasInUse() const;
        bool IsInUse() const;
        void ResetInUse();
    protected:
        std::optional<bool> in_use = std::nullopt;
    };
}
