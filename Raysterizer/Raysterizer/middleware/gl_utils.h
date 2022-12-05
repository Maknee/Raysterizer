#pragma once

#include "pch.h"

/*
#define GL_ALPHA8_EXT                     0x803C
#define GL_LUMINANCE8_EXT                 0x8040
#define GL_LUMINANCE8_ALPHA8_EXT          0x8045
#define GL_RGBA32F_EXT                    0x8814
#define GL_RGB32F_EXT                     0x8815
#define GL_ALPHA32F_EXT                   0x8816
#define GL_LUMINANCE32F_EXT               0x8818
#define GL_LUMINANCE_ALPHA32F_EXT         0x8819
#define GL_ALPHA16F_EXT                   0x881C
#define GL_LUMINANCE16F_EXT               0x881E
#define GL_LUMINANCE_ALPHA16F_EXT         0x881F
*/
#define GL_RG16F 0x822F
#define GL_R32F_EXT                       0x822E
//#define GL_RG32F_EXT                      0x8230

#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366

#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034

#define GL_R8 0x8229
#define GL_RG8  0x822B

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#define GL_HALF_FLOAT_ARB 0x140B

#define GL_DEPTH_COMPONENT32F 0x8CAC

namespace Raysterizer
{
	namespace MiddleWare
	{
		inline VkFormat GlColorFormatToVkColorFormat(GLenum format, GLenum type)
		{
            if (type == 0)
            {
                // assume from glTexStorage2D, where data is not buffered into texture yet
                type = GL_UNSIGNED_BYTE;
            }
			switch (type)
			{
			case GL_UNSIGNED_BYTE:
            {
                switch (format)
                {
                case GL_RGB:
                case GL_RGB8:
                    return VK_FORMAT_R8G8B8_UNORM;
                case GL_LUMINANCE:
                case GL_ALPHA:
                case GL_LUMINANCE_ALPHA:
                case GL_RGBA:
                case GL_RGBA8:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                case GL_R32F_EXT:
                    return VK_FORMAT_R32_SFLOAT;
                case GL_RGB10_A2:
                    return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                case GL_R8:
                    return VK_FORMAT_R8_UNORM;
                case GL_RG8:
                    return VK_FORMAT_R8G8_UNORM;
                case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                case GL_DEPTH_COMPONENT32F:
                    return VK_FORMAT_R32_SFLOAT;
                }
			}
            case GL_FLOAT:
            {
                switch (format)
                {
                case GL_RGB:
                case GL_RGB8:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case GL_LUMINANCE:
                case GL_ALPHA:
                case GL_LUMINANCE_ALPHA:
                case GL_RGBA:
                case GL_RGBA8:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case GL_R32F_EXT:
                    return VK_FORMAT_R32_SFLOAT;
                case GL_RGB10_A2:
                    return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
                case GL_R8:
                    return VK_FORMAT_R8_SNORM;
                }
            }
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            {
                switch (format)
                {
                case GL_RGBA:
                {
                    return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
                }
                }
            }
            case GL_UNSIGNED_SHORT_5_6_5:
            {
                switch (format)
                {
                case GL_RGB:
                {
                    return VK_FORMAT_R5G6B5_UNORM_PACK16;
                }
                }
            }
            case GL_UNSIGNED_SHORT_5_5_5_1:
            {
                switch (format)
                {
                case GL_RGBA:
                {
                    return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
                }
                }
            }
            case GL_HALF_FLOAT_ARB:
            {
                switch (format)
                {
                case GL_RG16F:
                {
                    return VK_FORMAT_R16G16_SFLOAT;
                }
                }
            }
			/*
			case GL_UNSIGNED_SHORT_5_6_5: {
				assert(format == GL_RGB);
				return          VK_FORMAT_R5G6B5_UNORM_PACK16;
			}
			case GL_UNSIGNED_SHORT_4_4_4_4: {
				assert(format == GL_RGBA);
				return          VK_FORMAT_R4G4B4A4_UNORM_PACK16;
			}
			case GL_UNSIGNED_SHORT_5_5_5_1: {
				assert(format == GL_RGBA);
				return          VK_FORMAT_R5G5B5A1_UNORM_PACK16;
			}
			*/
			default: {
				PANIC("Not supported! {} {}", format, type);
				return VK_FORMAT_R8G8B8A8_UNORM;
			}
			}
		}

        auto SPIRVCrossBaseTypeToToGLType = [](const spirv_cross::SPIRType& spirv_type)
        {
            const spirv_cross::SPIRType::BaseType& basetype = spirv_type.basetype;
            switch (basetype)
            {
            case spirv_cross::SPIRType::Unknown:
            case spirv_cross::SPIRType::Void:
            case spirv_cross::SPIRType::Struct:
            case spirv_cross::SPIRType::Image:
            case spirv_cross::SPIRType::SampledImage:
            case spirv_cross::SPIRType::Sampler:
            case spirv_cross::SPIRType::AccelerationStructure:
            case spirv_cross::SPIRType::RayQuery:
            {
                return 0;
                break;
            }
            case spirv_cross::SPIRType::Boolean:
            {
                return GL_BOOL;
            }
            case spirv_cross::SPIRType::SByte:
            {
                return GL_BYTE;
            }
            case spirv_cross::SPIRType::UByte:
            {
                return GL_UNSIGNED_BYTE;
            }
            case spirv_cross::SPIRType::Short:
            {
                return GL_SHORT;
            }
            case spirv_cross::SPIRType::UShort:
            {
                return GL_UNSIGNED_SHORT;
            }
            case spirv_cross::SPIRType::Int:
            {
                return GL_INT;
            }
            case spirv_cross::SPIRType::UInt:
            {
                return GL_UNSIGNED_INT;
            }
            case spirv_cross::SPIRType::Int64:
            case spirv_cross::SPIRType::UInt64:
            case spirv_cross::SPIRType::AtomicCounter:
            {
                break;
            }
            case spirv_cross::SPIRType::Half:
            {
                PANIC("No expected");
                //return GL_HALF_FLOAT;
            }
            case spirv_cross::SPIRType::Float:
            {
                return GL_FLOAT;
            }
            case spirv_cross::SPIRType::Double:
            {
                return GL_DOUBLE;
            }
            default:
            {
                break;
            }
            }
            PANIC("Cannot convert unknown type");
            return GL_BOOL;
        };
	}
}