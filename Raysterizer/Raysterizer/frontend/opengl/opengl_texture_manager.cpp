#include "opengl_texture_manager.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		TextureManager::TextureManager()
		{
			std::fill(std::begin(texture_unit_to_texture_id), std::end(texture_unit_to_texture_id), INVALID_TEXTURE_ID);
		}

		Error TextureManager::BindTextureUnitToTextureId(GLenum texture_unit, GLuint id)
		{
			texture_unit_to_texture_id[texture_unit] = id;
			return NoError();
		}

		Expected<GLuint> TextureManager::GetTextureIdFromTextureUnit(GLenum texture_unit)
		{
			auto texture_id = texture_unit_to_texture_id[texture_unit];
			if (texture_id == INVALID_TEXTURE_ID)
			{
				return StringError("Invalid texture id");
			}

			return texture_id;
		}

		Error TextureManager::AllocateTexture(GLuint id)
		{
			auto [_, success] = textures.try_emplace(id, std::nullopt);
			if (!success)
			{
				return StringError("Texture already exists");
			}
			return NoError();
		}

		Error TextureManager::DeallocateTexture(GLuint id)
		{
			if (textures.erase(id) > 0)
			{
				return NoError();
			}
			return StringError("Texture with id {} does not exist", id);
		}

		Error TextureManager::SetTextureType(GLuint id, GLenum texture_type)
		{
			if (auto& texture_or_err = GetOptionalTexture(id))
			{
				auto& optional_texture = *texture_or_err;
				if (!optional_texture)
				{
					switch (texture_type)
					{
					case GL_TEXTURE_2D:
					{
						optional_texture = Texture2D(texture_type);
						break;
					}
					case GL_TEXTURE_2D_ARRAY:
					{
						optional_texture = Texture2DArray(texture_type);
						break;
					}
					case GL_TEXTURE_3D:
					{
						optional_texture = Texture3D(texture_type);
						break;
					}
					case GL_TEXTURE_CUBE_MAP:
					{
						optional_texture = TextureCubeMap(texture_type);
						break;
					}
					default:
					{
						optional_texture = UnsupportedTexture(texture_type);
						return StringError("Texture {}: type {0x:X} has not been used", id, texture_type);
					}
					}
				}
				else
				{
					//return StringError("Texture have been already initialized");
				}
			}
			else
			{
				return texture_or_err.takeError();
			}
			return NoError();
		}

		Expected<std::optional<Texture>&> TextureManager::GetOptionalTexture(GLuint id)
		{
			if (auto found = textures.find(id); found != std::end(textures))
			{
				auto& texture = found->second;
				return texture;
			}
			else
			{
				return StringError("Texture not found");
			}
		}

		Expected<Texture&> TextureManager::GetTexture(GLuint id)
		{
			if (auto found = textures.find(id); found != std::end(textures))
			{
				auto& texture = found->second;
				if (texture)
				{
					return *texture;
				}
				else
				{
					return StringError("Texture not allocated");
				}
			}
			else
			{
				return StringError("Texture not found");
			}
		}

		/*
		Error TextureManager::BindActiveTextureUnitToName(GLuint program_id, GLenum active_texture_unit, std::string_view name)
		{
			auto actual_active_texture_unit = GL_TEXTURE0 + active_texture_unit;
			active_texture_unit_to_name[actual_active_texture_unit] = std::string(name);

			return NoError();
		}

		Error TextureManager::UnbindActiveTextureUnitToName(GLuint program_id, GLenum active_texture_unit)
		{
			auto num_erased = active_texture_unit_to_name.erase(active_texture_unit);

			return NoError();
		}
		*/

		Error BaseTexture::SetData(const void* raw_data, GLuint copy_width, GLuint copy_height, GLuint copy_depth, GLuint x_offset, GLuint y_offset, GLuint z_offset)
		{
			if (copy_width == 0)
			{
				return StringError("Width not initialized");
			}
			if (copy_height == 0)
			{
				return StringError("Height not initialized");
			}
			if (copy_depth == 0)
			{
				return StringError("Depth not initialized");
			}

			/*
			if (color_format != pixel_format)
			{
				return StringError("Color format doesn't match pixel format");
			}
			*/
			
			std::size_t copy_size = 0;
			switch (pixel_data_type)
			{
			case GL_UNSIGNED_BYTE:
			{
				copy_size = sizeof(uint8_t);
				break;
			}
			case GL_HALF_FLOAT:
			{
				copy_size = sizeof(uint16_t);
				break;
			}
			case GL_FLOAT:
			{
				copy_size = sizeof(float);
				break;
			}
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			{
				copy_size = 1;
				stride = sizeof(uint16_t);
				auto total_dimension = width * height * depth;
				ResizeBuffer(stride, total_dimension);
				break;
			}
			case GL_UNSIGNED_SHORT_4_4_4_4:
			{
				copy_size = 1;
				stride = sizeof(uint16_t);
				auto total_dimension = width * height * depth;
				ResizeBuffer(stride, total_dimension);
				break;
			}
			case GL_UNSIGNED_SHORT_5_6_5:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			{
				copy_size = 1;
				stride = sizeof(uint16_t);
				auto total_dimension = width * height * depth;
				ResizeBuffer(stride, total_dimension);
				break;
			}
			// Ignore
			case GL_UNSIGNED_INT_8_8_8_8_REV:
			{
				break;
			}
			case GL_UNSIGNED_INT_24_8:
			{
				copy_size = sizeof(uint32_t);
				break;
			}
			case GL_UNSIGNED_INT_2_10_10_10_REV:
			{
				copy_size = 1;
				stride = sizeof(uint32_t);
				auto total_dimension = width * height * depth;
				ResizeBuffer(stride, total_dimension);
				break;
			}
			default:
			{
				PANIC("Can't set data type");
				break;
			}
			}

			auto src_data = (uint8_t*)raw_data;
			//auto src_view = Raysterizer::PointerView(src_data, copy_size * stride, copy_width * copy_height);
			auto src_stride = GetPixelFormatStride();

			auto dst_data = data.data();
			
			if (copy_depth != 1)
			{
				//PANIC("Copy depth assumed to be just 1! Need to implment it for depth");
			}

			if (0)
			{
				if (src_data != nullptr)
				{
					for (GLuint h = 0; h < copy_height; h++)
					{
						auto src_total_width = copy_width * copy_size * src_stride;
						auto src_data_offset = src_data + (h * src_total_width);

						auto dst_height = h + y_offset;
						auto dst_total_width = width * stride;
						auto height_total = (dst_height * dst_total_width);
						auto depth_total = (z_offset * height * dst_total_width);
						auto dst_data_offset = dst_data + x_offset + height_total + depth_total;

						for (auto i = 0; i < copy_width; i++)
						{
							memcpy(dst_data_offset + i * stride, src_data_offset + i * copy_size * src_stride, copy_size * src_stride);
						}
					}
				}
			}

			if (1)
			{
				if (src_data != nullptr)
				{
					for (GLuint j = 0; j < copy_depth; j++)
					{
						auto src_total_width_height = copy_height * copy_width * copy_size * src_stride;
						auto dst_total_width_height = height * width * stride;
						for (GLuint h = 0; h < copy_height; h++)
						{
							auto src_total_width = copy_width * copy_size * src_stride;
							auto src_data_offset = src_data + (h * src_total_width) + (j * src_total_width_height);

							auto dst_height = h + y_offset;
							auto dst_total_width = width * stride;
							auto height_total = (dst_height * dst_total_width);
							auto depth_total = (z_offset * height * dst_total_width);
							auto dst_data_offset = dst_data + x_offset + height_total + depth_total + (j * dst_total_width_height);

							for (auto i = 0; i < copy_width; i++)
							{
								memcpy(dst_data_offset + i * stride, src_data_offset + i * copy_size * src_stride, copy_size * src_stride);
							}
						}
					}
				}
			}

			// Recompute hash
			data_view = HashedPointerView(data.data(), stride, width * height * depth);
			return NoError();
		}

		SamplerManager::SamplerManager()
		{
		}

		Error SamplerManager::AllocateSampler(GLuint id)
		{
			auto sampler = std::make_shared<Sampler>();
			sampler->id = id;
			samplers[id] = sampler;
			return NoError();
		}

		Error SamplerManager::DeallocateSampler(GLuint id)
		{
			samplers.erase(id);
			return NoError();
		}

		Expected<Sampler&> SamplerManager::GetSampler(GLuint id)
		{
			if (auto found = samplers.find(id); found != std::end(samplers))
			{
				return *found->second;
			}
			return StringError("Sampler {} could not be found", id);
		}

		Error SamplerManager::BindSamplerToTextureUnit(GLuint id, GLuint texture_unit)
		{
			if (auto found = samplers.find(id); found != std::end(samplers))
			{
				auto sampler = found->second;
				texture_unit_to_sampler[texture_unit] = sampler;
				sampler->bound_texture_unit = texture_unit;
				return NoError();

			}
			else
			{
				return StringError("Sampler {} could not be found, so texture unit {} cannot be bound", id, texture_unit);
			}
		}

		Expected<Sampler&> SamplerManager::GetSamplerFromTextureUnit(GLuint texture_unit)
		{
			if (auto found = texture_unit_to_sampler.find(texture_unit); found != std::end(texture_unit_to_sampler))
			{
				return *found->second;
			}
			return StringError("Sampler could not be found for texture unit {}", texture_unit);
		}
	}
}