#include "include/profiler.h"

/*
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED

GFSDK_Aftermath_API(*GFSDK_Aftermath_EnableGpuCrashDumps)(GFSDK_Aftermath_Version apiVersion, uint32_t watchedApis, uint32_t flags, PFN_GFSDK_Aftermath_GpuCrashDumpCb gpuCrashDumpCb, PFN_GFSDK_Aftermath_ShaderDebugInfoCb shaderDebugInfoCb, PFN_GFSDK_Aftermath_GpuCrashDumpDescriptionCb descriptionCb, PFN_GFSDK_Aftermath_ResolveMarkerCb resolveMarkerCb, void* pUserData);
GFSDK_Aftermath_API(*GFSDK_Aftermath_DisableGpuCrashDumps)();
GFSDK_Aftermath_API(*GFSDK_Aftermath_GetCrashDumpStatus)(GFSDK_Aftermath_CrashDump_Status* pOutStatus);

GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_CreateDecoder)(GFSDK_Aftermath_Version apiVersion, const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, GFSDK_Aftermath_GpuCrashDump_Decoder* pDecoder);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_DestroyDecoder)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetBaseInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, GFSDK_Aftermath_GpuCrashDump_BaseInfo* pBaseInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t key, uint32_t* pValueSize);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetDescription)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t key, const uint32_t valueBufferSize, char* pValue);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetDeviceInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, GFSDK_Aftermath_GpuCrashDump_DeviceInfo* pDeviceInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetSystemInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, GFSDK_Aftermath_GpuCrashDump_SystemInfo* pSystemInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetGpuInfoCount)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, uint32_t* pGpuCount);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetGpuInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t gpuInfoBufferCount, GFSDK_Aftermath_GpuCrashDump_GpuInfo* pGpuInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetPageFaultInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, GFSDK_Aftermath_GpuCrashDump_PageFaultInfo* pPageFaultInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfoCount)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, uint32_t* pShaderCount);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t shaderInfoBufferCount, GFSDK_Aftermath_GpuCrashDump_ShaderInfo* pShaderInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfoCount)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, uint32_t* pMarkerCount);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t markerInfoBufferCount, GFSDK_Aftermath_GpuCrashDump_EventMarkerInfo* pMarkerInfo);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GenerateJSON)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, uint32_t decoderFlags, uint32_t formatFlags, PFN_GFSDK_Aftermath_ShaderDebugInfoLookupCb shaderDebugInfoLookupCb, PFN_GFSDK_Aftermath_ShaderLookupCb shaderLookupCb, PFN_GFSDK_Aftermath_ShaderSourceDebugInfoLookupCb shaderSourceDebugInfoLookupCb, void* pUserData, uint32_t* pJsonSize);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GpuCrashDump_GetJSON)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const uint32_t jsonBufferSize, char* pJson);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GetShaderDebugInfoIdentifier)(GFSDK_Aftermath_Version apiVersion, const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GetShaderHashForShaderInfo)(const GFSDK_Aftermath_GpuCrashDump_Decoder decoder, const GFSDK_Aftermath_GpuCrashDump_ShaderInfo* pShaderInfo, GFSDK_Aftermath_ShaderBinaryHash* pShaderHash);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GetShaderHashSpirv)(GFSDK_Aftermath_Version apiVersion, const GFSDK_Aftermath_SpirvCode* pShader, GFSDK_Aftermath_ShaderBinaryHash* pShaderHash);
GFSDK_Aftermath_API(*GFSDK_Aftermath_GetShaderDebugNameSpirv)(GFSDK_Aftermath_Version apiVersion, const GFSDK_Aftermath_SpirvCode* pShader, const GFSDK_Aftermath_SpirvCode* pStrippedShader, GFSDK_Aftermath_ShaderDebugName* pShaderDebugName);

#endif
*/

#ifdef NSIGHT_AFTERMATH_INC_PATH

#include NSIGHT_AFTERMATH_INC_PATH

#endif

namespace RaysterizerEngine
{
    Profiler::Profiler()
    {
    }

    Profiler::Profiler(Context* c_) : c(c_)
    {
    }

    void Profiler::Setup()
    {
#ifdef ENABLE_PROFILER
        auto AllocateContext = [&](QueueType queue_type)
        {
            auto command_buffer = AssignOrPanic(c->GetRenderFrame().GetCommandBuffer(queue_type));

            vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
            vk::Device device = c->GetDevice();
            vk::Queue queue = c->GetQueue(queue_type);

            tracy::VkCtx* tracy_context = TracyVkContext(physical_device, device, queue, **command_buffer);
            command_buffer->End();
            return tracy_context;
        };

        graphics_context = AllocateContext(QueueType::Graphics);
        compute_context = AllocateContext(QueueType::Graphics);
        transfer_context = AllocateContext(QueueType::Graphics);
        present_context = AllocateContext(QueueType::Graphics);

#define ContextName(ctx, name) TracyVkContextName(ctx, name.c_str(), name.length())

        std::string graphics_context_name = "Graphics Context";
        std::string compute_context_name = "Transfer Context";
        std::string transfer_context_name = "Transfer Context";
        std::string present_context_name = "Present Context";
        ContextName(graphics_context, graphics_context_name);
        ContextName(compute_context, compute_context_name);
        ContextName(transfer_context, transfer_context_name);
        ContextName(present_context, present_context_name);
#endif
    }

#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
    void Profiler::SetupNvidiaNSightAfterMath()
    {
#ifdef NSIGHT_AFTERMATH_INC_PATH
        
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_LOAD_FROM_MEMORY
        DWORD error_code{};
        static auto memory_module = (HMEMMODULE)MemModuleHelper(MHM_BOOL_LOAD, (void*)NVIDIA_NSIGHT_DLL, (LPVOID)TRUE, &error_code);
        if (error_code)
        {
            PANIC("Failed to load nsight from memory - {}", error_code);
        }

#define GFSDK_LOAD_FUNCTION(function) \
        function = decltype(function)(MemModuleHelper(MHM_FARPROC_GETPROC, memory_module, STRINGIFY(function), (VOID*)TRUE)); \

        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_EnableGpuCrashDumps);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_DisableGpuCrashDumps);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetCrashDumpStatus);

        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_CreateDecoder);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDescription);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDeviceInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetSystemInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetGpuInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetGpuInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetPageFaultInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GenerateJSON);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetJSON);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugInfoIdentifier);
        //GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHash);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHashSpirv);
        //GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugName);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugNameSpirv);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHashForShaderInfo);
#else
        auto write_gfsdk_path = fs::temp_directory_path() / "GFSDK_Aftermath_Lib.x64.dll";
        if (auto f = std::ofstream(write_gfsdk_path, std::ios::binary | std::ios::ate))
        {
            f.write((const char*)NVIDIA_NSIGHT_DLL, sizeof(NVIDIA_NSIGHT_DLL));
        }

        auto write_gfsdk_path_str = write_gfsdk_path.string();
        auto lib = LoadLibrary(write_gfsdk_path_str.c_str());

#define GFSDK_LOAD_FUNCTION(function) \
        function = decltype(function)(GetProcAddress(lib, STRINGIFY(function))); \

        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_EnableGpuCrashDumps);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_DisableGpuCrashDumps);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetCrashDumpStatus);

        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_CreateDecoder);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDescription);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetDeviceInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetSystemInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetGpuInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetGpuInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetPageFaultInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfoCount);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfo);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GenerateJSON);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GpuCrashDump_GetJSON);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugInfoIdentifier);
        //GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHash);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHashSpirv);
        //GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugName);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderDebugNameSpirv);
        GFSDK_LOAD_FUNCTION(GFSDK_Aftermath_GetShaderHashForShaderInfo);

#endif

#endif
        gpu_crash_tracker = std::make_shared<GpuCrashTracker>(marker_map);
        gpu_crash_tracker->Initialize();

        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#endif

#ifdef ENABLE_PROFILER
    Profiler::~Profiler()
    {
        if (graphics_context)
        {
            TracyVkDestroy(graphics_context);
        }
        if (compute_context)
        {
            TracyVkDestroy(compute_context);
        }
        if (transfer_context)
        {
            TracyVkDestroy(transfer_context);
        }
        if (present_context)
        {
            TracyVkDestroy(present_context);
        }
    }

    tracy::VkCtx* Profiler::Get(CommandBuffer* command_buffer2)
    {
        auto& command_buffer = *command_buffer2;
        if (!c)
        {
            PANIC("Context is not set!");
        }

        if (!*command_buffer)
        {
            PANIC("Command buffer is not set!");
        }

        if (!command_buffer.command_buffer_create_info.command_pool)
        {
            PANIC("Command pool is not set!");
        }
        command_buffer.Begin();

        auto queue_type = command_buffer.command_buffer_create_info.command_pool->command_pool_create_info.queue_type;
        switch (queue_type)
        {
        case QueueType::Graphics:
            return graphics_context;
        case QueueType::Compute:
            return compute_context;
        case QueueType::Transfer:
            return transfer_context;
        case QueueType::Present:
            return present_context;
        default:
            PANIC("Not possible option!");
        }
    }
#endif;

    void Profiler::Update(vk::CommandBuffer cb, std::string_view name)
    {
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
        unsigned int index = frame % GpuCrashTracker::c_markerFrameHistory;
        auto& current_frame_marker_map = marker_map[index];
        size_t marker_id = index * 10000 + current_frame_marker_map.size() + 1;
        current_frame_marker_map[marker_id] = name;
        cb.setCheckpointNV((const void*)marker_id);
#endif
    }

    void Profiler::EndFrame()
    {
#ifdef ENABLE_PROFILER
        FrameMark;
#endif
    }

    GPUSample::GPUSample(vk::CommandBuffer command_buffer_, std::string_view name) :
        command_buffer(command_buffer_)
    {
        const static bool enable_profiler = Config["profiler"]["enable"];
        if (enable_profiler)
        {
            vk::DebugUtilsLabelEXT debug_label;

            debug_label.pNext = nullptr;
            debug_label.pLabelName = name.data();
            debug_label.color[0] = 0.0f;
            debug_label.color[1] = 1.0f;
            debug_label.color[2] = 0.0f;
            debug_label.color[3] = 1.0f;

            command_buffer.beginDebugUtilsLabelEXT(debug_label);
        }
    }

    GPUSample::~GPUSample()
    {
        const static bool enable_profiler = Config["profiler"]["enable"];
        if (enable_profiler)
        {
            command_buffer.endDebugUtilsLabelEXT();
        }
    }
}