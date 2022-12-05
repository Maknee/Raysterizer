#include "include/vulkan_window.h"

namespace RaysterizerEngine
{
    VulkanWindow::VulkanWindow(std::string name_, uint32_t width, uint32_t height) :
        name(std::move(name_)), size(vk::Extent2D{ width, height })
    {
    }

    VulkanWindow::~VulkanWindow()
    {

    }

    vk::Extent2D VulkanWindow::GetSize() const
    {
        return size;
    }

    void VulkanWindow::SetResizeCallback(WindowResizeCallback window_resize_callback_)
    {
        window_resize_callback = window_resize_callback_;
    }

    Error VulkanWindow::CallResizeCallback(vk::Extent2D extent)
    {
        if (!window_resize_callback)
        {
            return StringError("Resize callback is not valid");
        }

        window_resize_callback(extent);

        return NoError();
    }

    void VulkanWindow::SetShutdownCallback(WindowShutdownCallback window_shutdown_callback_)
    {
        window_shutdown_callback = window_shutdown_callback_;
    }

    GLFWWindow::GLFWWindow(std::string name_, uint32_t width, uint32_t height) :
        VulkanWindow(name_, width, height)
    {
        if (!glfwInit())
        {
            PANIC("Failed to init GLFW");
        }
        
        //not full screen (windowed)
        //glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        if (window)
        {
            DEBUG("Window initialized");
        }
        else
        {
            PANIC("Unable to initialize window");
        }

        if (!glfwVulkanSupported())
        {
            PANIC("Vulkan is not supported for glfw");
        }

        //glfwMakeContextCurrent(window);

        //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        //glfwSetCursorPosCallback(window, mouse_callback);
        //glfwSetScrollCallback(window, scroll_callback);

        // tell GLFW to capture our mouse
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //glfwSwapInterval(1);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            GLFWWindow* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
            if (!glfw_window)
            {
                PANIC("No glfw window");
            }

            vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

            PanicIfError(glfw_window->CallResizeCallback(extent));
        });
    }

    GLFWWindow::~GLFWWindow()
    {
        glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window));
        //glfwTerminate();
    }

    std::vector<std::string> GLFWWindow::GetVulkanExtensions()
    {
        uint32_t count;
        const char** exts = glfwGetRequiredInstanceExtensions(&count);

        std::vector<std::string> result(count);
        std::copy(exts, exts + count, std::begin(result));
        return result;
    }

    vk::SurfaceKHR GLFWWindow::CreateSurface(const vk::Instance& instance) const
    {
        VkSurfaceKHR surface{};

        const auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            PANIC("Unable to create window surface: {}", glfwGetError(nullptr));
        }

        return surface;
    }

    bool GLFWWindow::Update()
    {
        /*
        glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(window));
        while (!glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(window)))
        {
        }
        */
        //glfwMakeContextCurrent(nullptr);
        if (glfwWindowShouldClose(window))
        {
            if (window_shutdown_callback)
            {
                window_shutdown_callback();
            }
            return false;
        }

        //glfwSwapBuffers(window);
        glfwPollEvents();

        return true;
    }
}
