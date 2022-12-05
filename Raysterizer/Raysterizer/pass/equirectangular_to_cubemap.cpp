#include "equirectangular_to_cubemap.h"

#define SKY_CUBEMAP_SIZE 512

namespace
{
    static const unsigned int kCONVERT_VERT_SPIRV_size = 1280;
    static const unsigned int kCONVERT_VERT_SPIRV_data[1280 / 4] = {
        0x07230203,
        0x00010000,
        0x000d000a,
        0x00000026,
        0x00000000,
        0x00020011,
        0x00000001,
        0x0006000b,
        0x00000001,
        0x4c534c47,
        0x6474732e,
        0x3035342e,
        0x00000000,
        0x0003000e,
        0x00000000,
        0x00000001,
        0x0008000f,
        0x00000000,
        0x00000004,
        0x6e69616d,
        0x00000000,
        0x00000009,
        0x0000000b,
        0x00000013,
        0x00030003,
        0x00000002,
        0x000001c2,
        0x000a0004,
        0x475f4c47,
        0x4c474f4f,
        0x70635f45,
        0x74735f70,
        0x5f656c79,
        0x656e696c,
        0x7269645f,
        0x69746365,
        0x00006576,
        0x00080004,
        0x475f4c47,
        0x4c474f4f,
        0x6e695f45,
        0x64756c63,
        0x69645f65,
        0x74636572,
        0x00657669,
        0x00040005,
        0x00000004,
        0x6e69616d,
        0x00000000,
        0x00060005,
        0x00000009,
        0x495f5346,
        0x6f575f4e,
        0x50646c72,
        0x0000736f,
        0x00060005,
        0x0000000b,
        0x495f5356,
        0x6f505f4e,
        0x69746973,
        0x00006e6f,
        0x00060005,
        0x00000011,
        0x505f6c67,
        0x65567265,
        0x78657472,
        0x00000000,
        0x00060006,
        0x00000011,
        0x00000000,
        0x505f6c67,
        0x7469736f,
        0x006e6f69,
        0x00070006,
        0x00000011,
        0x00000001,
        0x505f6c67,
        0x746e696f,
        0x657a6953,
        0x00000000,
        0x00070006,
        0x00000011,
        0x00000002,
        0x435f6c67,
        0x4470696c,
        0x61747369,
        0x0065636e,
        0x00070006,
        0x00000011,
        0x00000003,
        0x435f6c67,
        0x446c6c75,
        0x61747369,
        0x0065636e,
        0x00030005,
        0x00000013,
        0x00000000,
        0x00060005,
        0x00000017,
        0x68737550,
        0x736e6f43,
        0x746e6174,
        0x00000073,
        0x00060006,
        0x00000017,
        0x00000000,
        0x77656976,
        0x6f72705f,
        0x0000006a,
        0x00060005,
        0x00000019,
        0x75505f75,
        0x6f436873,
        0x6174736e,
        0x0073746e,
        0x00040047,
        0x00000009,
        0x0000001e,
        0x00000000,
        0x00040047,
        0x0000000b,
        0x0000001e,
        0x00000000,
        0x00050048,
        0x00000011,
        0x00000000,
        0x0000000b,
        0x00000000,
        0x00050048,
        0x00000011,
        0x00000001,
        0x0000000b,
        0x00000001,
        0x00050048,
        0x00000011,
        0x00000002,
        0x0000000b,
        0x00000003,
        0x00050048,
        0x00000011,
        0x00000003,
        0x0000000b,
        0x00000004,
        0x00030047,
        0x00000011,
        0x00000002,
        0x00040048,
        0x00000017,
        0x00000000,
        0x00000005,
        0x00050048,
        0x00000017,
        0x00000000,
        0x00000023,
        0x00000000,
        0x00050048,
        0x00000017,
        0x00000000,
        0x00000007,
        0x00000010,
        0x00030047,
        0x00000017,
        0x00000002,
        0x00020013,
        0x00000002,
        0x00030021,
        0x00000003,
        0x00000002,
        0x00030016,
        0x00000006,
        0x00000020,
        0x00040017,
        0x00000007,
        0x00000006,
        0x00000003,
        0x00040020,
        0x00000008,
        0x00000003,
        0x00000007,
        0x0004003b,
        0x00000008,
        0x00000009,
        0x00000003,
        0x00040020,
        0x0000000a,
        0x00000001,
        0x00000007,
        0x0004003b,
        0x0000000a,
        0x0000000b,
        0x00000001,
        0x00040017,
        0x0000000d,
        0x00000006,
        0x00000004,
        0x00040015,
        0x0000000e,
        0x00000020,
        0x00000000,
        0x0004002b,
        0x0000000e,
        0x0000000f,
        0x00000001,
        0x0004001c,
        0x00000010,
        0x00000006,
        0x0000000f,
        0x0006001e,
        0x00000011,
        0x0000000d,
        0x00000006,
        0x00000010,
        0x00000010,
        0x00040020,
        0x00000012,
        0x00000003,
        0x00000011,
        0x0004003b,
        0x00000012,
        0x00000013,
        0x00000003,
        0x00040015,
        0x00000014,
        0x00000020,
        0x00000001,
        0x0004002b,
        0x00000014,
        0x00000015,
        0x00000000,
        0x00040018,
        0x00000016,
        0x0000000d,
        0x00000004,
        0x0003001e,
        0x00000017,
        0x00000016,
        0x00040020,
        0x00000018,
        0x00000009,
        0x00000017,
        0x0004003b,
        0x00000018,
        0x00000019,
        0x00000009,
        0x00040020,
        0x0000001a,
        0x00000009,
        0x00000016,
        0x0004002b,
        0x00000006,
        0x0000001e,
        0x3f800000,
        0x00040020,
        0x00000024,
        0x00000003,
        0x0000000d,
        0x00050036,
        0x00000002,
        0x00000004,
        0x00000000,
        0x00000003,
        0x000200f8,
        0x00000005,
        0x0004003d,
        0x00000007,
        0x0000000c,
        0x0000000b,
        0x0003003e,
        0x00000009,
        0x0000000c,
        0x00050041,
        0x0000001a,
        0x0000001b,
        0x00000019,
        0x00000015,
        0x0004003d,
        0x00000016,
        0x0000001c,
        0x0000001b,
        0x0004003d,
        0x00000007,
        0x0000001d,
        0x0000000b,
        0x00050051,
        0x00000006,
        0x0000001f,
        0x0000001d,
        0x00000000,
        0x00050051,
        0x00000006,
        0x00000020,
        0x0000001d,
        0x00000001,
        0x00050051,
        0x00000006,
        0x00000021,
        0x0000001d,
        0x00000002,
        0x00070050,
        0x0000000d,
        0x00000022,
        0x0000001f,
        0x00000020,
        0x00000021,
        0x0000001e,
        0x00050091,
        0x0000000d,
        0x00000023,
        0x0000001c,
        0x00000022,
        0x00050041,
        0x00000024,
        0x00000025,
        0x00000013,
        0x00000015,
        0x0003003e,
        0x00000025,
        0x00000023,
        0x000100fd,
        0x00010038,
    };

    // -----------------------------------------------------------------------------------------------------------------------------------

    static const unsigned int kCONVERT_FRAG_SPIRV_size = 1544;
    static const unsigned int kCONVERT_FRAG_SPIRV_data[1544 / 4] = {
        0x07230203,
        0x00010000,
        0x000d000a,
        0x0000003e,
        0x00000000,
        0x00020011,
        0x00000001,
        0x0006000b,
        0x00000001,
        0x4c534c47,
        0x6474732e,
        0x3035342e,
        0x00000000,
        0x0003000e,
        0x00000000,
        0x00000001,
        0x0007000f,
        0x00000004,
        0x00000004,
        0x6e69616d,
        0x00000000,
        0x0000002c,
        0x0000003c,
        0x00030010,
        0x00000004,
        0x00000007,
        0x00030003,
        0x00000002,
        0x000001c2,
        0x000a0004,
        0x475f4c47,
        0x4c474f4f,
        0x70635f45,
        0x74735f70,
        0x5f656c79,
        0x656e696c,
        0x7269645f,
        0x69746365,
        0x00006576,
        0x00080004,
        0x475f4c47,
        0x4c474f4f,
        0x6e695f45,
        0x64756c63,
        0x69645f65,
        0x74636572,
        0x00657669,
        0x00040005,
        0x00000004,
        0x6e69616d,
        0x00000000,
        0x00090005,
        0x0000000c,
        0x706d6173,
        0x735f656c,
        0x72656870,
        0x6c616369,
        0x70616d5f,
        0x33667628,
        0x0000003b,
        0x00030005,
        0x0000000b,
        0x00000076,
        0x00030005,
        0x0000000f,
        0x00007675,
        0x00030005,
        0x0000002a,
        0x00007675,
        0x00060005,
        0x0000002c,
        0x495f5346,
        0x6f575f4e,
        0x50646c72,
        0x0000736f,
        0x00040005,
        0x0000002f,
        0x61726170,
        0x0000006d,
        0x00040005,
        0x00000031,
        0x6f6c6f63,
        0x00000072,
        0x00050005,
        0x00000035,
        0x6e455f73,
        0x70614d76,
        0x00000000,
        0x00060005,
        0x0000003c,
        0x4f5f5346,
        0x435f5455,
        0x726f6c6f,
        0x00000000,
        0x00040047,
        0x0000002c,
        0x0000001e,
        0x00000000,
        0x00040047,
        0x00000035,
        0x00000022,
        0x00000000,
        0x00040047,
        0x00000035,
        0x00000021,
        0x00000000,
        0x00040047,
        0x0000003c,
        0x0000001e,
        0x00000000,
        0x00020013,
        0x00000002,
        0x00030021,
        0x00000003,
        0x00000002,
        0x00030016,
        0x00000006,
        0x00000020,
        0x00040017,
        0x00000007,
        0x00000006,
        0x00000003,
        0x00040020,
        0x00000008,
        0x00000007,
        0x00000007,
        0x00040017,
        0x00000009,
        0x00000006,
        0x00000002,
        0x00040021,
        0x0000000a,
        0x00000009,
        0x00000008,
        0x00040020,
        0x0000000e,
        0x00000007,
        0x00000009,
        0x00040015,
        0x00000010,
        0x00000020,
        0x00000000,
        0x0004002b,
        0x00000010,
        0x00000011,
        0x00000002,
        0x00040020,
        0x00000012,
        0x00000007,
        0x00000006,
        0x0004002b,
        0x00000010,
        0x00000015,
        0x00000000,
        0x0004002b,
        0x00000010,
        0x00000019,
        0x00000001,
        0x0004002b,
        0x00000006,
        0x0000001e,
        0x3e22eb1c,
        0x0004002b,
        0x00000006,
        0x0000001f,
        0x3ea2f838,
        0x0005002c,
        0x00000009,
        0x00000020,
        0x0000001e,
        0x0000001f,
        0x0004002b,
        0x00000006,
        0x00000023,
        0x3f000000,
        0x00040020,
        0x0000002b,
        0x00000001,
        0x00000007,
        0x0004003b,
        0x0000002b,
        0x0000002c,
        0x00000001,
        0x00090019,
        0x00000032,
        0x00000006,
        0x00000001,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000001,
        0x00000000,
        0x0003001b,
        0x00000033,
        0x00000032,
        0x00040020,
        0x00000034,
        0x00000000,
        0x00000033,
        0x0004003b,
        0x00000034,
        0x00000035,
        0x00000000,
        0x00040017,
        0x00000038,
        0x00000006,
        0x00000004,
        0x00040020,
        0x0000003b,
        0x00000003,
        0x00000007,
        0x0004003b,
        0x0000003b,
        0x0000003c,
        0x00000003,
        0x00050036,
        0x00000002,
        0x00000004,
        0x00000000,
        0x00000003,
        0x000200f8,
        0x00000005,
        0x0004003b,
        0x0000000e,
        0x0000002a,
        0x00000007,
        0x0004003b,
        0x00000008,
        0x0000002f,
        0x00000007,
        0x0004003b,
        0x00000008,
        0x00000031,
        0x00000007,
        0x0004003d,
        0x00000007,
        0x0000002d,
        0x0000002c,
        0x0006000c,
        0x00000007,
        0x0000002e,
        0x00000001,
        0x00000045,
        0x0000002d,
        0x0003003e,
        0x0000002f,
        0x0000002e,
        0x00050039,
        0x00000009,
        0x00000030,
        0x0000000c,
        0x0000002f,
        0x0003003e,
        0x0000002a,
        0x00000030,
        0x0004003d,
        0x00000033,
        0x00000036,
        0x00000035,
        0x0004003d,
        0x00000009,
        0x00000037,
        0x0000002a,
        0x00050057,
        0x00000038,
        0x00000039,
        0x00000036,
        0x00000037,
        0x0008004f,
        0x00000007,
        0x0000003a,
        0x00000039,
        0x00000039,
        0x00000000,
        0x00000001,
        0x00000002,
        0x0003003e,
        0x00000031,
        0x0000003a,
        0x0004003d,
        0x00000007,
        0x0000003d,
        0x00000031,
        0x0003003e,
        0x0000003c,
        0x0000003d,
        0x000100fd,
        0x00010038,
        0x00050036,
        0x00000009,
        0x0000000c,
        0x00000000,
        0x0000000a,
        0x00030037,
        0x00000008,
        0x0000000b,
        0x000200f8,
        0x0000000d,
        0x0004003b,
        0x0000000e,
        0x0000000f,
        0x00000007,
        0x00050041,
        0x00000012,
        0x00000013,
        0x0000000b,
        0x00000011,
        0x0004003d,
        0x00000006,
        0x00000014,
        0x00000013,
        0x00050041,
        0x00000012,
        0x00000016,
        0x0000000b,
        0x00000015,
        0x0004003d,
        0x00000006,
        0x00000017,
        0x00000016,
        0x0007000c,
        0x00000006,
        0x00000018,
        0x00000001,
        0x00000019,
        0x00000014,
        0x00000017,
        0x00050041,
        0x00000012,
        0x0000001a,
        0x0000000b,
        0x00000019,
        0x0004003d,
        0x00000006,
        0x0000001b,
        0x0000001a,
        0x0006000c,
        0x00000006,
        0x0000001c,
        0x00000001,
        0x00000010,
        0x0000001b,
        0x00050050,
        0x00000009,
        0x0000001d,
        0x00000018,
        0x0000001c,
        0x0003003e,
        0x0000000f,
        0x0000001d,
        0x0004003d,
        0x00000009,
        0x00000021,
        0x0000000f,
        0x00050085,
        0x00000009,
        0x00000022,
        0x00000021,
        0x00000020,
        0x0003003e,
        0x0000000f,
        0x00000022,
        0x0004003d,
        0x00000009,
        0x00000024,
        0x0000000f,
        0x00050050,
        0x00000009,
        0x00000025,
        0x00000023,
        0x00000023,
        0x00050081,
        0x00000009,
        0x00000026,
        0x00000024,
        0x00000025,
        0x0003003e,
        0x0000000f,
        0x00000026,
        0x0004003d,
        0x00000009,
        0x00000027,
        0x0000000f,
        0x000200fe,
        0x00000027,
        0x00010038,
    };
}

namespace Raysterizer
{
	namespace Pass
	{
		Error EquirectangularToCubemap::Setup(std::shared_ptr<CommonResources> common_resources_, vk::Format image_format)
		{
			common_resources = common_resources_;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

            glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 capture_views[] = {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
            };

			view_projection_mats.resize(6);

			for (int i = 0; i < 6; i++)
				view_projection_mats[i] = capture_projection * capture_views[i];

            float cube_vertices[] = {
                // back face
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                // front face
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                1.0f,
                1.0f,
                1.0f,
                // top-right
                1.0f,
                1.0f,
                1.0f,
                // top-right
                -1.0f,
                1.0f,
                1.0f,
                // top-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                // left face
                -1.0f,
                1.0f,
                1.0f,
                // top-right
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                1.0f,
                1.0f,
                // top-right
                // right face
                1.0f,
                1.0f,
                1.0f,
                // top-left
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                1.0f,
                // top-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                // bottom face
                -1.0f,
                -1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // top-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                -1.0f,
                -1.0f,
                // top-right
                // top face
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                1.0f,
                1.0f,
                1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                -1.0f,
                1.0f,
                1.0f // bottom-left
            };

			std::vector<vk::AttachmentDescription> attachments(1);

			// GBuffer1 attachment
			attachments[0].format = image_format;
			attachments[0].samples = vk::SampleCountFlagBits::e1;
			attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
			attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[0].initialLayout = vk::ImageLayout::eUndefined;
			attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			vk::AttachmentReference color_reference;

			color_reference.attachment = 0;
			color_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

			std::vector<vk::SubpassDescription> subpass_description(1);

			subpass_description[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
			subpass_description[0].setColorAttachments(color_reference);
			subpass_description[0].pDepthStencilAttachment = nullptr;
			subpass_description[0].inputAttachmentCount = 0;
			subpass_description[0].pInputAttachments = nullptr;
			subpass_description[0].preserveAttachmentCount = 0;
			subpass_description[0].pPreserveAttachments = nullptr;
			subpass_description[0].pResolveAttachments = nullptr;

			// Subpass dependencies for layout transitions
			std::vector<vk::SubpassDependency> dependencies(2);

			dependencies[0] = vk::SubpassDependency{}
				.setSrcSubpass(VK_SUBPASS_EXTERNAL)
				.setDstSubpass(0)
				.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

			dependencies[1] = vk::SubpassDependency{}
				.setSrcSubpass(0)
				.setDstSubpass(VK_SUBPASS_EXTERNAL)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion);


			RenderPassCreateInfo render_pass_create_info{};
			render_pass_create_info.attachment_descriptions = attachments;
			render_pass_create_info.subpass_descriptions = subpass_description;
			render_pass_create_info.subpass_dependencies = dependencies;

			AssignOrReturnError(cubemap_renderpass, c.Get(render_pass_create_info));

			AssignOrReturnError(auto cube_vbo_transfer_job, render_frame.UploadDataToGPUBuffer(PointerView(cube_vertices), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst));
			cube_vbo = cube_vbo_transfer_job.gpu_buffer;

			return NoError();
		}

		void EquirectangularToCubemap::Convert(CMShared<Image> input_image, CMShared<Image> output_image)
		{
            ScopedCPUProfileRaysterizerCurrentFunction();

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

			auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
			{
				ShaderModuleCreateInfo{Spirv{std::begin(kCONVERT_VERT_SPIRV_data), std::end(kCONVERT_VERT_SPIRV_data)}},
				ShaderModuleCreateInfo{Spirv{std::begin(kCONVERT_FRAG_SPIRV_data), std::end(kCONVERT_FRAG_SPIRV_data)}},
			};

			auto plci = PipelineLayoutCreateInfo
			{
				shader_module_create_infos,
			};

			CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
			CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
			CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

			// ---------------------------------------------------------------------------
			// Create vertex input state
			// ---------------------------------------------------------------------------

			std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions = {
				vk::VertexInputBindingDescription{}
					.setBinding(0)
					.setStride(sizeof(glm::vec3))
					.setInputRate(vk::VertexInputRate::eVertex)
			};

			std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions = {
				vk::VertexInputAttributeDescription{}
					.setLocation(0)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(0),
			};

			auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
				.setVertexBindingDescriptions(vertex_input_binding_descriptions)
				.setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

			auto pcbas = vk::PipelineColorBlendAttachmentState{}
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrc1Alpha)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
				.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setBlendEnable(VK_FALSE);

			std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ pcbas };

            std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

			auto gpci = GraphicsPipelineCreateInfo
			{
				cubemap_renderpass,
				pl,
				vertex_input_state_create_info,
				vk::PipelineTessellationStateCreateInfo{},
				vk::PipelineInputAssemblyStateCreateInfo{}
					.setTopology(vk::PrimitiveTopology::eTriangleList)
					.setPrimitiveRestartEnable(VK_FALSE),
				vk::Viewport{}
					.setX(0.0f)
					.setY(0.0f)
					.setWidth(SKY_CUBEMAP_SIZE)
					.setHeight(SKY_CUBEMAP_SIZE)
					.setMinDepth(0.0f)
					.setMaxDepth(1.0f),
				vk::Rect2D{}
					.setOffset({ 0, 0 })
					.setExtent(vk::Extent2D{ SKY_CUBEMAP_SIZE, SKY_CUBEMAP_SIZE }),
				vk::PipelineRasterizationStateCreateInfo{}
					.setDepthClampEnable(VK_FALSE)
					.setRasterizerDiscardEnable(VK_FALSE)
					.setPolygonMode(vk::PolygonMode::eFill)
					.setLineWidth(1.0f)
					.setCullMode(vk::CullModeFlagBits::eNone)
					.setFrontFace(vk::FrontFace::eCounterClockwise)
					.setDepthBiasEnable(VK_FALSE)
					.setDepthBiasConstantFactor(0.0f)
					.setDepthBiasClamp(0.0f)
					.setDepthBiasSlopeFactor(0.0f),
				vk::PipelineColorBlendStateCreateInfo{}
					.setLogicOpEnable(VK_FALSE)
					.setLogicOp(vk::LogicOp::eCopy)
					.setAttachments(pcbases),
				vk::PipelineMultisampleStateCreateInfo{}
					.setRasterizationSamples(vk::SampleCountFlagBits::e1)
					.setSampleShadingEnable(VK_FALSE)
					.setMinSampleShading(1.0f)
					.setPSampleMask(nullptr)
					.setAlphaToCoverageEnable(VK_FALSE)
					.setAlphaToOneEnable(VK_FALSE),
				vk::PipelineDepthStencilStateCreateInfo{}
					.setDepthTestEnable(VK_TRUE)
					.setDepthWriteEnable(VK_TRUE)
					.setDepthCompareOp(vk::CompareOp::eLess)
					.setDepthBoundsTestEnable(VK_FALSE)
					.setMinDepthBounds(0.0f)
					.setMaxDepthBounds(1.0f)
					.setStencilTestEnable(VK_FALSE),
				vk::PipelineDynamicStateCreateInfo{}
                    .setDynamicStates(dynamic_states),
				vk::PipelineCache{}
			};

			CMShared<GraphicsPipeline> cp = AssignOrPanic(c.Get(gpci));
			c.SetName(pl, "equirectangular_to_cubemap pipeline layout");
			c.SetName(cp, "equirectangular_to_cubemap compute pipeline");

            std::vector<CMShared<ImageView>>   face_image_views(6);
            std::vector<vk::UniqueFramebuffer> face_framebuffers(6);

            for (int i = 0; i < 6; i++)
            {
                face_image_views[i] = CommonResources::CreateImageView(output_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 0, 1, i, 1);
                c.SetName(face_image_views[i], fmt::format("equirectangular_to_cubemap face image view {}", i));

                std::vector<vk::ImageView> attachments{ *face_image_views[i] };

                auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
                    .setRenderPass(*cubemap_renderpass)
                    .setAttachments(attachments)
                    .setWidth(output_image->image_create_info.image_create_info.extent.width)
                    .setHeight(output_image->image_create_info.image_create_info.extent.height)
                    .setLayers(1);

                auto frame_buffer = AssignOrPanicVkError(device.createFramebufferUnique(frame_buffer_create_info));
                face_framebuffers[i] = std::move(frame_buffer);
            }

            PanicIfError(c.ImmediateGraphicsSubmit([&](CommandBuffer& command_buffer)
                {
                    ScopedGPUProfileRaysterizer(&command_buffer, "Equirectangular To Cubemap");

                    auto& cb = *command_buffer;

                    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, cp->pipeline);
                    cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                    for (int i = 0; i < 6; i++)
                    {
                        vk::ClearValue clear_value;

                        clear_value.color.float32[0] = 0.0f;
                        clear_value.color.float32[1] = 0.0f;
                        clear_value.color.float32[2] = 0.0f;
                        clear_value.color.float32[3] = 1.0f;

                        auto render_pass_begin_info = vk::RenderPassBeginInfo{}
                            .setRenderPass(cubemap_renderpass->render_pass)
                            .setRenderArea(vk::Rect2D{}
                                .setOffset({ 0, 0 })
                                .setExtent({ output_image->image_create_info.image_create_info.extent.width, output_image->image_create_info.image_create_info.extent.height })
                            )
                            .setFramebuffer(*face_framebuffers[i])
                            .setClearValues(clear_value);

                        cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

                        cb.setViewport(0, vk::Viewport{ 0, 0, static_cast<float>(output_image->image_create_info.image_create_info.extent.width), static_cast<float>(output_image->image_create_info.image_create_info.extent.height), 0.0f, 1.0f });
                        cb.setScissor(0, vk::Rect2D(output_image->image_create_info.image_create_info.extent.width, output_image->image_create_info.image_create_info.extent.height));

                        {
                            struct PushContants
                            {
                                glm::mat4 view_projection_mat;
                            };

                            PushContants push_constants;

                            push_constants.view_projection_mat = view_projection_mats[i];

                            PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                        }

                        cb.bindVertexBuffers(0, { **cube_vbo }, { 0 });
                        cb.draw(36, 1, 0, 0);

                        cb.endRenderPass();
                    }
                }));
		}
	}
}