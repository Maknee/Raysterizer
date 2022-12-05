#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		class BaseTexture
		{
		public:
			explicit BaseTexture(GLuint id_) :
				id(id_)
			{
			}

			GLuint GetId()
			{
				return id;
			}

			void SetLevelOfDetail(GLuint level_of_detail_)
			{
				level_of_detail = level_of_detail_;
			}

			GLuint GetLevelOfDetail()
			{
				return level_of_detail;
			}

			void SetColorFormat(GLenum color_format_)
			{
				color_format = Util::InternalFormatToBaseFormat(color_format_);
			}

			GLuint GetColorFormat()
			{
				return color_format;
			}

			void SetPixelFormat(GLenum pixel_format_)
			{
				pixel_format = pixel_format_;
			}

			GLenum GetPixelFormat()
			{
				return pixel_format;
			}

			void SetPixelDataType(GLenum pixel_data_type_)
			{
				pixel_data_type = pixel_data_type_;
			}

			GLenum GetPixelDataType()
			{
				return pixel_data_type;
			}

			std::size_t GetPixelFormatStride() const
			{
				auto pixel_stride = 0;
				switch (pixel_format)
				{
				case GL_RED:
				{
					pixel_stride = 1;
					break;
				}
				case GL_RG:
				{
					pixel_stride = 2;
					break;
				}
				case GL_RGB:
				{
					pixel_stride = 3;
					break;
				}
				case GL_RGBA:
				case GL_BGRA:
				{
					pixel_stride = 4;
					break;
				}
				case GL_DEPTH_COMPONENT:
				case GL_DEPTH_STENCIL:
				{
					pixel_stride = 4;
					break;
				}
				case GL_LUMINANCE:
				{
					pixel_stride = 1;
					break;
				}
				default:
				{
					PANIC("Not supoported {}", pixel_format);
					break;
				}
				}

				return pixel_stride;
			}

			void SetWidth(GLuint width_)
			{
				width = width_;
			}

			GLuint GetWidth()
			{
				return width;
			}

			void SetHeight(GLuint height_)
			{
				height = height_;
			}

			GLuint GetHeight()
			{
				return height;
			}

			void SetDepth(GLuint depth_) { depth = depth_; }
			GLuint GetDepth() { return depth; }

			void ResizeBuffer(std::size_t stride, std::size_t total_dimension)
			{
				const auto total_size = stride * total_dimension;
				if (data_view.GetTotalSize() != total_size)
				{
					data = std::vector<uint8_t>(total_size);
					data_view = HashedPointerView(data.data(), stride, total_dimension);
				}
			}

			Error InitializeBuffer()
			{
				switch (color_format)
				{
				case GL_RGBA:
				{
					stride = 4;
					break;
				}
				case GL_RGB:
				{
					stride = 3;
					break;
				}
				case GL_RED:
				case GL_R8:
				{
					stride = 1;
					break;
				}
				case GL_RG8:
				{
					stride = 2;
					break;
				}
				case GL_RGB10_A2:
#define COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
				case COMPRESSED_RGBA_S3TC_DXT1_EXT:
#define COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
				case COMPRESSED_RGBA_S3TC_DXT5_EXT:
				case GL_RG16F:
				{
					// TODO:
					// THESE ARE not correct stride
					// correct way is to translate data into some other format like GL_RGBA
					stride = 4;
					break;
				}
				case GL_R32F:
				case GL_DEPTH_COMPONENT32F:
				case GL_DEPTH24_STENCIL8:
				{
					stride = 4;
					break;
				}
				case GL_LUMINANCE8:
				{
					stride = 1;
					break;
				}
				default:
				{
					PANIC("Can't set data type {}", color_format);
					break;
				}
				}

				if (width == 0)
				{
					return StringError("Width not initialized");
				}
				if (height == 0)
				{
					return StringError("Height not initialized");
				}
				if (depth == 0)
				{
					return StringError("Depth not initialized");
				}

				auto total_dimension = width * height * depth;
				ResizeBuffer(stride, total_dimension);

				return NoError();
			}

			Error SetData(const void* raw_data, GLuint copy_width = 0, GLuint copy_height = 1, GLuint copy_depth = 1, GLuint x_offset = 0, GLuint y_offset = 0, GLuint z_offset = 0);
			HashedPointerView GetDataView() { return data_view; }

		protected:
			GLuint id{};

			GLuint level_of_detail{};
			
			GLenum color_format{};
			GLenum pixel_format{};
			GLenum pixel_data_type{};
			std::size_t stride;

			GLuint width{};
			GLuint height = 1;
			GLuint depth = 1;

			HashedPointerView data_view{};
			std::vector<uint8_t> data{};
		};

		class Texture1D : public BaseTexture
		{
		public:
			explicit Texture1D(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		class Texture2D : public BaseTexture
		{
		public:
			explicit Texture2D(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		class Texture2DArray : public BaseTexture
		{
		public:
			explicit Texture2DArray(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		class Texture3D : public BaseTexture
		{
		public:
			explicit Texture3D(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		class TextureCubeMap : public BaseTexture
		{
		public:
			explicit TextureCubeMap(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		class UnsupportedTexture : public BaseTexture
		{
		public:
			explicit UnsupportedTexture(GLuint id) :
				BaseTexture(id)
			{
			}
		private:
		};

		using Texture = std::variant<
			Texture1D,
			Texture2D,
			Texture2DArray,
			Texture3D,
			TextureCubeMap,
			UnsupportedTexture
		>;

		class TextureManager
		{
		public:
			explicit TextureManager();

			Error BindTextureUnitToTextureId(GLenum texture_unit, GLuint id);
			Expected<GLuint> GetTextureIdFromTextureUnit(GLenum texture_unit);
			
			Error AllocateTexture(GLuint id);
			Error DeallocateTexture(GLuint id);
			Error SetTextureType(GLuint id, GLenum texture_type);
			Expected<std::optional<Texture>&> GetOptionalTexture(GLuint id);
			Expected<Texture&> GetTexture(GLuint id);

			template<typename T>
			Expected<T&> GetTextureAs(GLuint id)
			{
				if (auto texture_or_err = GetTexture(id))
				{
					auto& texture = *texture_or_err;
					if (auto underlying_texture = std::get_if<T>(&texture))
					{
						return *underlying_texture;
					}
					else
					{
						return StringError("Texture casting not possible");
					}
				}
				else
				{
					return texture_or_err.takeError();
				}
			}

			static TextureManager& Get()
			{
				static TextureManager texture_manager;
				return texture_manager;
			}

			flat_hash_map<GLuint, std::optional<Texture>>& GetTextures() { return textures; }

			/*
			Error BindActiveTextureUnitToName(GLenum active_texture_unit, std::string_view name);
			Error UnbindActiveTextureUnitToName(GLenum active_texture_unit);
			flat_hash_map<GLenum, std::string>& GetActiveTextureUnitToName() { return active_texture_unit_to_name; }
			*/

		private:
			static const size_t TEXTURE_UNIT_SIZE = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1;
			static const auto INVALID_TEXTURE_ID = std::numeric_limits<GLuint>::max();
			std::array<GLenum, TEXTURE_UNIT_SIZE> texture_unit_to_texture_id;
			flat_hash_map<GLuint, std::optional<Texture>> textures;
			flat_hash_map<GLenum, std::string> active_texture_unit_to_name;
		};

		struct Sampler
		{
			GLuint id;
			GLuint bound_texture_unit;
			std::map<GLuint, GLuint> binded_texture_unit_to_texture_ids;
			GLenum min_filter = GL_LINEAR;
			GLenum mag_filter = GL_LINEAR;
			GLenum mip_filter = GL_LINEAR;
			GLenum wrap_s = GL_REPEAT;
			GLenum wrap_t = GL_REPEAT;
			GLenum wrap_r = GL_REPEAT;
			float min_lod = 0.0f;
			float max_lod = 0.0f;
			float lod_bias = 0.0f;

		public:
			void BindTextureUnitToTextureId(GLuint texture_unit, GLuint id) { binded_texture_unit_to_texture_ids[texture_unit] = id; }
			void UnbindTextureUnitToTextureId(GLuint texture_unit) { binded_texture_unit_to_texture_ids.erase(texture_unit); }
			const auto& GetTextureUnitToTextureId() const { return binded_texture_unit_to_texture_ids; }

			void SetMinFilter(GLenum filter) { min_filter = filter; }
			GLenum GetMinFilter() const { return min_filter; }
			void SetMagFilter(GLenum filter) { mag_filter = filter; }
			GLenum GetMagFilter() const { return mag_filter; }
			void SetMipFilter(GLenum filter) { mip_filter = filter; }
			GLenum GetMipFilter() const { return mip_filter; }
			void SetWrapS(GLenum wrap) { wrap_s = wrap; }
			GLenum GetWrapS() const { return wrap_s; }
			void SetWrapT(GLenum wrap) { wrap_t = wrap; }
			GLenum GetWrapT() const { return wrap_t; }
			void SetWrapR(GLenum wrap) { wrap_r = wrap; }
			GLenum GetWrapR() const { return wrap_r; }
			void SetMinLod(float lod) { min_lod = lod; }
			GLenum GetMinLod() const { return min_lod; }
			void SetMaxLod(float lod) { max_lod = lod; }
			GLenum GetMaxLod() const { return max_lod; }
			void SetLodBias(float lod) { lod_bias = lod; }
			GLenum GetLodBias() const { return lod_bias; }
		};

		class SamplerManager
		{
		public:
			explicit SamplerManager();

			Error AllocateSampler(GLuint id);
			Error DeallocateSampler(GLuint id);
			Expected<Sampler&> GetSampler(GLuint id);
			Error BindSamplerToTextureUnit(GLuint id, GLuint texture_unit);
			Expected<Sampler&> GetSamplerFromTextureUnit(GLuint texture_unit);

			static SamplerManager& Get()
			{
				static SamplerManager m;
				return m;
			}

		private:
			flat_hash_map<GLuint, std::shared_ptr<Sampler>> samplers;
			flat_hash_map<GLuint, std::shared_ptr<Sampler>> texture_unit_to_sampler;
		};
	}
}
