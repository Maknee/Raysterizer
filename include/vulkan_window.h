#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
    using WindowResizeCallback = std::function<void(vk::Extent2D new_window_extent)>;
    using WindowShutdownCallback = std::function<void()>;

    class VulkanWindow
    {
    public:
        explicit VulkanWindow(std::string name_, uint32_t width, uint32_t height);
        virtual ~VulkanWindow();
        virtual std::vector<std::string> GetVulkanExtensions() = 0;
        virtual vk::SurfaceKHR CreateSurface(const vk::Instance& instance) const = 0;
        virtual bool Update() = 0;
        virtual void* GetUnderlyingWindow() { return nullptr; } 
        vk::Extent2D GetSize() const;
        void SetResizeCallback(WindowResizeCallback window_resize_callback_);
        Error CallResizeCallback(vk::Extent2D extent);
        void SetShutdownCallback(WindowShutdownCallback window_shutdown_callback_);
    protected:
        std::string name{};
        vk::Extent2D size{};
        WindowResizeCallback window_resize_callback{};
        WindowShutdownCallback window_shutdown_callback{};
    };

    class GLFWWindow : public VulkanWindow
    {
    public:
        explicit GLFWWindow(std::string name_, uint32_t width, uint32_t height);
        virtual ~GLFWWindow();
        std::vector<std::string> GetVulkanExtensions() final;
        vk::SurfaceKHR CreateSurface(const vk::Instance& instance) const final;
        bool Update() final;
        virtual void* GetUnderlyingWindow() { return static_cast<void*>(window); }
    private:
        GLFWwindow* window{};
    };
}
