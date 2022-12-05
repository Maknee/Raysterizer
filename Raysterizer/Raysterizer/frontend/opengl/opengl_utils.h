#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		static const GLuint UNDEFINED_ID = std::numeric_limits<GLuint>::max();
		static const GLuint DEFAULT_FRAME_BUFFER_OBJECT_ID = GLuint{ 0 };

		namespace Util
		{
			std::size_t GLenumToSize(GLenum type);
			GLenum InternalFormatToBaseFormat(GLenum internalFormat);
			GLenum BaseFormatAndTypeToInterlopeConsistentFormat(GLenum format, GLenum type);

			enum GLColorMaskBit {
				GLC_RED = 0,
				GLC_GREEN = 1,
				GLC_BLUE = 2,
				GLC_ALPHA = 3
			};

			GLboolean               GlColorMaskHasBit(GLubyte colorMask, GLColorMaskBit bit);
			GLubyte                 GlColorMaskPack(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
			const char* GlAttribTypeToString(GLenum type);
			GLenum                  GlFormatToGlInternalFormat(GLenum format, GLenum type);
			GLenum                  GlInternalFormatToGlType(GLenum internalFormat);
			GLenum                  GlInternalFormatToGlFormat(GLenum internalFormat);
			int                     GlInternalFormatTypeToNumElements(GLenum format, GLenum type);
			int32_t                 GlAttribTypeToElementSize(GLenum type);
			int                     GlTypeToElementSize(GLenum type);
			void                    GlFormatToStorageBits(GLenum format, GLint* r_, GLint* g_, GLint* b_, GLint* a_, GLint* d_, GLint* s_);
			void                    GlFormatToStorageBits(GLenum format, GLfloat* r_, GLfloat* g_, GLfloat* b_, GLfloat* a_, GLfloat* d_, GLfloat* s_);
			void                    GlFormatToStorageBits(GLenum format, GLboolean* r_, GLboolean* g_, GLboolean* b_, GLboolean* a_, GLboolean* d_, GLboolean* s_);
			bool                    GlFormatIsDepthRenderable(GLenum format);
			bool                    GlFormatIsStencilRenderable(GLenum format);
			bool                    GlFormatIsColorRenderable(GLenum format);
			uint32_t                OccupiedLocationsPerGlType(GLenum type);
			bool                    IsGlSampler(GLenum type);

			vk::Bool32                GlBooleanToVkBool(GLboolean value);
			vk::ColorComponentFlags   GLColorMaskToVkColorComponentFlags(GLubyte colorMask);
			vk::BlendFactor           GlBlendFactorToVkBlendFactor(GLenum mode);
			vk::LogicOp           	  GlLogicOpToVkLogicOp(GLenum mode);
			vk::BlendOp               GlBlendEquationToVkBlendOp(GLenum mode);
			vk::CompareOp             GlCompareFuncToVkCompareOp(GLenum mode);
			vk::CullModeFlagBits      GlCullModeToVkCullMode(GLenum mode);
			vk::FrontFace             GlFrontFaceToVkFrontFace(GLenum mode);
			vk::PolygonMode           GLPrimitiveModeToVkPolygonMode(GLenum mode);
			vk::PrimitiveTopology     GlPrimitiveTopologyToVkPrimitiveTopology(GLenum mode);
			vk::SampleCountFlagBits   GlSampleCoverageBitsToVkSampleCountFlagBits(GLint bits);
			vk::StencilOp             GlStencilFuncToVkStencilOp(GLenum mode);
			vk::SamplerAddressMode    GlTexAddressToVkTexAddress(GLenum mode);
			vk::Filter                GlTexFilterToVkTexFilter(GLenum mode);
			vk::SamplerMipmapMode     GlTexMipMapModeToVkMipMapMode(GLenum mode);
			vk::Format                GlTexInternalFormatToVkFormat(GLenum internalformat);
			vk::Format                GlInternalFormatToVkFormat(GLenum internalformat);
			vk::Format                GlInternalFormatToVkFormat(GLenum internalformatDepth, GLenum internalformatStencil);
			vk::Format                GlAttribPointerToVkFormat(GLint nElements, GLenum type, GLboolean normalized);
			vk::IndexType             GlToVkIndexType(GLenum type);
			vk::Format                GlColorFormatToVkColorFormat(GLenum format, GLenum type);
		}
	}
}
