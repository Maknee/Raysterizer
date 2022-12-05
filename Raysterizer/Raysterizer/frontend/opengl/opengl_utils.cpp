#include "opengl_utils.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		namespace Util
		{
			std::size_t GLenumToSize(GLenum type)
			{
				switch (type)
				{
				case GL_BOOL:
					return sizeof(GLboolean);
				case GL_BOOL_VEC2:
					return sizeof(GLboolean) * 2;
				case GL_BOOL_VEC3:
					return sizeof(GLboolean) * 3;
				case GL_BOOL_VEC4:
					return sizeof(GLboolean) * 3;
				case GL_FLOAT:
					return sizeof(GLfloat);
				case GL_FLOAT_MAT2:
					return sizeof(GLfloat) * 2 * 2;
				case GL_FLOAT_MAT3:
					return sizeof(GLfloat) * 3 * 3;
				case GL_FLOAT_MAT4:
					return sizeof(GLfloat) * 4 * 4;
				case GL_FLOAT_MAT2x3:
					return sizeof(GLfloat) * 2 * 3;
				case GL_FLOAT_MAT2x4:
					return sizeof(GLfloat) * 2 * 4;
				case GL_FLOAT_MAT3x2:
					return sizeof(GLfloat) * 3 * 2;
				case GL_FLOAT_MAT3x4:
					return sizeof(GLfloat) * 3 * 4;
				case GL_FLOAT_MAT4x2:
					return sizeof(GLfloat) * 4 * 2;
				case GL_FLOAT_MAT4x3:
					return sizeof(GLfloat) * 4 * 3;
				case GL_FLOAT_VEC2:
					return sizeof(GLfloat) * 2;
				case GL_FLOAT_VEC3:
					return sizeof(GLfloat) * 3;
				case GL_FLOAT_VEC4:
					return sizeof(GLfloat) * 4;
				case GL_INT:
				case GL_INT_SAMPLER_1D:
				case GL_INT_SAMPLER_1D_ARRAY:
				case GL_INT_SAMPLER_2D:
				case GL_INT_SAMPLER_2D_ARRAY:
				case GL_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_INT_SAMPLER_2D_RECT:
				case GL_INT_SAMPLER_3D:
				case GL_INT_SAMPLER_BUFFER:
				case GL_INT_SAMPLER_CUBE:
					return sizeof(GLint);
				case GL_INT_VEC2:
					return sizeof(GLint) * 2;
				case GL_INT_VEC3:
					return sizeof(GLint) * 3;
				case GL_INT_VEC4:
					return sizeof(GLint) * 4;
				case GL_SAMPLER_1D:
				case GL_SAMPLER_1D_ARRAY:
				case GL_SAMPLER_1D_ARRAY_SHADOW:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D:
				case GL_SAMPLER_2D_ARRAY:
				case GL_SAMPLER_2D_ARRAY_SHADOW:
				case GL_SAMPLER_2D_MULTISAMPLE:
				case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_SAMPLER_2D_RECT:
				case GL_SAMPLER_2D_RECT_SHADOW:
				case GL_SAMPLER_2D_SHADOW:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_BUFFER:
				case GL_SAMPLER_CUBE:
				case GL_SAMPLER_CUBE_SHADOW:
					return sizeof(GLint);
				case GL_UNSIGNED_INT:
					return sizeof(GLuint);
				case GL_UNSIGNED_INT_SAMPLER_1D:
				case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D:
				case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
				case GL_UNSIGNED_INT_SAMPLER_3D:
				case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				case GL_UNSIGNED_INT_SAMPLER_CUBE:
					return sizeof(GLuint);
				case GL_UNSIGNED_INT_VEC2:
					return sizeof(GLuint) * 2;
				case GL_UNSIGNED_INT_VEC3:
					return sizeof(GLuint) * 3;
				case GL_UNSIGNED_INT_VEC4:
					return sizeof(GLuint) * 4;
				case GL_UNSIGNED_BYTE:
					return sizeof(GLubyte);
				case GL_BYTE:
					return sizeof(GLbyte);

					/*
				case GL_BOOL:
				case GL_BOOL_VEC2:
				case GL_BOOL_VEC3:
				case GL_BOOL_VEC4:
				case GL_FLOAT:
				case GL_FLOAT_MAT2:
				case GL_FLOAT_MAT3:
				case GL_FLOAT_MAT4:
				case GL_FLOAT_MAT2x3:
				case GL_FLOAT_MAT2x4:
				case GL_FLOAT_MAT3x2:
				case GL_FLOAT_MAT3x4:
				case GL_FLOAT_MAT4x2:
				case GL_FLOAT_MAT4x3:
				case GL_FLOAT_VEC2:
				case GL_FLOAT_VEC3:
				case GL_FLOAT_VEC4:
				case GL_INT:
				case GL_INT_SAMPLER_1D:
				case GL_INT_SAMPLER_1D_ARRAY:
				case GL_INT_SAMPLER_2D:
				case GL_INT_SAMPLER_2D_ARRAY:
				case GL_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_INT_SAMPLER_2D_RECT:
				case GL_INT_SAMPLER_3D:
				case GL_INT_SAMPLER_BUFFER:
				case GL_INT_SAMPLER_CUBE:
				case GL_INT_VEC2:
				case GL_INT_VEC3:
				case GL_INT_VEC4:
				case GL_SAMPLER_1D:
				case GL_SAMPLER_1D_ARRAY:
				case GL_SAMPLER_1D_ARRAY_SHADOW:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D:
				case GL_SAMPLER_2D_ARRAY:
				case GL_SAMPLER_2D_ARRAY_SHADOW:
				case GL_SAMPLER_2D_MULTISAMPLE:
				case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_SAMPLER_2D_RECT:
				case GL_SAMPLER_2D_RECT_SHADOW:
				case GL_SAMPLER_2D_SHADOW:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_BUFFER:
				case GL_SAMPLER_CUBE:
				case GL_SAMPLER_CUBE_SHADOW:
				case GL_UNSIGNED_INT:
				case GL_UNSIGNED_INT_SAMPLER_1D:
				case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D:
				case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
				case GL_UNSIGNED_INT_SAMPLER_3D:
				case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				case GL_UNSIGNED_INT_SAMPLER_CUBE:
				case GL_UNSIGNED_INT_VEC2:
				case GL_UNSIGNED_INT_VEC3:
				case GL_UNSIGNED_INT_VEC4:
					*/
				default:
				{
					PANIC("Cannot convert type");
					break;
				}
				}
			}

			GLenum InternalFormatToBaseFormat(GLenum internalFormat)
			{
				switch (internalFormat)
				{
				case GL_ALPHA:                            return GL_ALPHA;
				case GL_LUMINANCE:                        return GL_LUMINANCE;
				case GL_LUMINANCE_ALPHA:                  return GL_LUMINANCE_ALPHA;
				case GL_RGB:
				case GL_RGB565:
				case GL_RGB8:                         return GL_RGB;
				case GL_RGBA:
				case GL_RGBA8:
				case GL_RGBA4:
				case GL_RGB5_A1:                          return GL_RGBA;
				case GL_DEPTH_COMPONENT16:                return GL_DEPTH_COMPONENT16;
				case GL_DEPTH_COMPONENT24:            return GL_DEPTH_COMPONENT24;
				case GL_DEPTH_COMPONENT32:            return GL_DEPTH_COMPONENT32;
				case GL_DEPTH24_STENCIL8:             return GL_DEPTH24_STENCIL8;
				case GL_STENCIL_INDEX8:                   return GL_STENCIL_INDEX8;
				case GL_STENCIL_INDEX4:               return GL_STENCIL_INDEX4;

				case GL_R8:
				{
					return GL_R8;
				}
				default:
				{
					//PANIC("Conversion not supported");
					return internalFormat;
					//break;
				}
				}
				return GL_INVALID_ENUM;
			}

			GLenum BaseFormatAndTypeToInterlopeConsistentFormat(GLenum format, GLenum type)
			{
				switch (type)
				{
				case GL_UNSIGNED_BYTE:
					switch (format)
					{
					case GL_RGB:
						return GL_RGB8;
					case GL_RGBA:
					case GL_RGBA8:
						return GL_RGBA8;
					case GL_R8:
						return GL_R8;
					default:
						PANIC("Conversion not supported");
					}
				default:
					PANIC("Conversion not supported");
				}
				return GL_INVALID_ENUM;
			}

			std::size_t InternalFormatAndTypeToNumElements(GLenum internalFormat, GLenum type)
			{
				switch (type) {
				case GL_UNSIGNED_BYTE:
					switch (internalFormat) {
					case GL_STENCIL_INDEX8:
					case GL_ALPHA:
					case GL_LUMINANCE:                          return 1;
					case GL_LUMINANCE_ALPHA:                    return 2;
					case GL_RGB:                                return 3;
					case GL_BGRA_EXT:
					case GL_RGBA:                               return 4;
					default: { PANIC("Not found");  return 1; }
					}
				case GL_UNSIGNED_SHORT_4_4_4_4:
					switch (internalFormat) {
					case GL_RGBA4:
					case GL_BGRA_EXT:
					case GL_RGBA:                              return 1;
					default: { PANIC("Not found");  return 1; }
					}
				case GL_UNSIGNED_SHORT_5_5_5_1:
					switch (internalFormat) {
					case GL_RGB5_A1:
					case GL_BGRA_EXT:
					case GL_RGBA:                              return 1;
					default: {PANIC("Not found"); return 1; }
					}
				case GL_UNSIGNED_SHORT_5_6_5:
					switch (internalFormat) {
					case GL_RGB565:
					case GL_RGB:                               return 1;
					default: { PANIC("Not found");  return 1; }
					}
				default: { PANIC("Not found");             return 1; }
				}
			}

			void PanicIfGLError()
			{
				auto err_to_string = [](GLenum err) -> std::string
				{
					switch (err)
					{
						// opengl 2 errors (8)
					case GL_NO_ERROR:
						return "GL_NO_ERROR";

					case GL_INVALID_ENUM:
						return "GL_INVALID_ENUM";

					case GL_INVALID_VALUE:
						return "GL_INVALID_VALUE";

					case GL_INVALID_OPERATION:
						return "GL_INVALID_OPERATION";

					case GL_STACK_OVERFLOW:
						return "GL_STACK_OVERFLOW";

					case GL_STACK_UNDERFLOW:
						return "GL_STACK_UNDERFLOW";

					case GL_OUT_OF_MEMORY:
						return "GL_OUT_OF_MEMORY";

						// opengl 3 errors (1)
					case GL_INVALID_FRAMEBUFFER_OPERATION:
						return "GL_INVALID_FRAMEBUFFER_OPERATION";

						// gles 2, 3 and gl 4 error are handled by the switch above
					default:
						return fmt::format("{}\n", err);
					}
				};
				GLenum err{};
				auto err_count = 0;
				while (err = glGetError() != GL_NO_ERROR)
				{
					DEBUG("{}\n", err_to_string(err));
					err_count++;
				}
				if (err_count > 0)
				{
					PANIC("OpenGLError");
				}
			}

#define CASE_STR(c)                                     case GL_ ##c: return "GL_" STRINGIFY(c);

#   define NOT_REACHED()                                printf("You shouldn't be here. (function %s at line %d of file %s)\n", __func__, __LINE__, __FILE__)
#   define NOT_FOUND_ENUM(inv_enum)                     printf("Invalid enum: %#04x. (function %s at line %d of file %s)\n", inv_enum, __func__, __LINE__, __FILE__)
#   define NOT_IMPLEMENTED()                            printf("Function %s (line %d of file %s) not implemented yet.\n", __func__, __LINE__, __FILE__)

			GLboolean GlColorMaskHasBit(GLubyte colorMask, GLColorMaskBit bit)
			{
				return ((colorMask >> bit) & 0x1) == 1 ? GL_TRUE : GL_FALSE;
			}

			GLubyte GlColorMaskPack(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
			{
				return static_cast<GLubyte>((red << GLColorMaskBit::GLC_RED) |
					(green << GLColorMaskBit::GLC_GREEN) |
					(blue << GLColorMaskBit::GLC_BLUE) |
					(alpha << GLColorMaskBit::GLC_ALPHA));
			}

			const char* GlAttribTypeToString(GLenum type)
			{
				switch (type) {
					CASE_STR(BOOL);
					CASE_STR(BOOL_VEC2);
					CASE_STR(BOOL_VEC3);
					CASE_STR(BOOL_VEC4);

					CASE_STR(INT);
					CASE_STR(INT_VEC2);
					CASE_STR(INT_VEC3);
					CASE_STR(INT_VEC4);

					CASE_STR(FLOAT);
					CASE_STR(FLOAT_VEC2);
					CASE_STR(FLOAT_VEC3);
					CASE_STR(FLOAT_VEC4);

					CASE_STR(FLOAT_MAT2);
					CASE_STR(FLOAT_MAT3);
					CASE_STR(FLOAT_MAT4);
				default: { NOT_FOUND_ENUM(type); return STRINGIFY(GL_INVALID_ENUM); }
				}
			}

			GLenum
				GlFormatToGlInternalFormat(GLenum format, GLenum type)
			{
				switch (format) {
				case GL_DEPTH_COMPONENT16:              return GL_DEPTH_COMPONENT16;
				case GL_DEPTH_COMPONENT24:          return GL_DEPTH_COMPONENT24;
				case GL_DEPTH_COMPONENT32:          return GL_DEPTH_COMPONENT32;
				case GL_STENCIL_INDEX8:                 return GL_STENCIL_INDEX8;
				case GL_STENCIL_INDEX4:             return GL_STENCIL_INDEX4;
				case GL_BGRA_EXT:
					switch (type) {
					case GL_UNSIGNED_BYTE:              return GL_BGRA_EXT;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}
				case GL_RGBA:
					switch (type) {
					case GL_UNSIGNED_SHORT_4_4_4_4:     return GL_RGBA4;
					case GL_UNSIGNED_SHORT_5_5_5_1:     return GL_RGB5_A1;
					case GL_UNSIGNED_BYTE:              return GL_RGBA8;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				case GL_RGB:
					switch (type) {
					case GL_UNSIGNED_SHORT_5_6_5:       return GL_RGB565;
					case GL_UNSIGNED_BYTE:              return GL_RGB8;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				case GL_LUMINANCE_ALPHA:
					switch (type) {
					case GL_UNSIGNED_BYTE:              return GL_LUMINANCE_ALPHA;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				case GL_LUMINANCE:
					switch (type) {
					case GL_UNSIGNED_BYTE:              return GL_LUMINANCE;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				case GL_ALPHA:
					switch (type) {
					case GL_UNSIGNED_BYTE:              return GL_ALPHA;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				case GL_DEPTH24_STENCIL8:
					switch (type) {
					case GL_UNSIGNED_INT_24_8:      return GL_DEPTH24_STENCIL8;
					default: NOT_FOUND_ENUM(type);      return GL_INVALID_VALUE;
					}

				default: NOT_FOUND_ENUM(format);        return GL_INVALID_VALUE;
				}
			}

			GLenum
				GlInternalFormatToGlType(GLenum internalformat)
			{
				switch (internalformat) {
				case GL_RGBA4:                     return GL_UNSIGNED_SHORT_4_4_4_4;
				case GL_RGB5_A1:                     return GL_UNSIGNED_SHORT_5_5_5_1;
				case GL_RGB565:                     return GL_UNSIGNED_SHORT_5_6_5;
				case GL_ALPHA:
				case GL_LUMINANCE:
				case GL_LUMINANCE_ALPHA:
				case GL_DEPTH_COMPONENT16:
				case GL_DEPTH_COMPONENT24:
				case GL_DEPTH_COMPONENT32:
				case GL_STENCIL_INDEX8:
				case GL_STENCIL_INDEX4:
				case GL_RGB:
				case GL_RGBA:
				case GL_RGB8:
				case GL_RGBA8:                     return GL_UNSIGNED_BYTE;
				case GL_DEPTH24_STENCIL8:           return GL_UNSIGNED_INT_24_8;
				default: NOT_FOUND_ENUM(internalformat); return GL_INVALID_VALUE;
				}
			}

			GLenum
				GlInternalFormatToGlFormat(GLenum internalFormat)
			{
				switch (internalFormat) {
				case GL_ALPHA:                            return GL_ALPHA;
				case GL_LUMINANCE:                        return GL_LUMINANCE;
				case GL_LUMINANCE_ALPHA:                  return GL_LUMINANCE_ALPHA;
				case GL_RGB:
				case GL_RGB565:
				case GL_RGB8:                         return GL_RGB;
				case GL_RGBA:
				case GL_RGBA8:
				case GL_RGBA4:
				case GL_RGB5_A1:                          return GL_RGBA;
				case GL_DEPTH_COMPONENT16:                return GL_DEPTH_COMPONENT16;
				case GL_DEPTH_COMPONENT24:            return GL_DEPTH_COMPONENT24;
				case GL_DEPTH_COMPONENT32:            return GL_DEPTH_COMPONENT32;
				case GL_DEPTH24_STENCIL8:             return GL_DEPTH24_STENCIL8;
				case GL_STENCIL_INDEX8:                   return GL_STENCIL_INDEX8;
				case GL_STENCIL_INDEX4:               return GL_STENCIL_INDEX4;
				default: NOT_FOUND_ENUM(internalFormat);  return GL_INVALID_VALUE;
				}
			}

			int
				GlInternalFormatTypeToNumElements(GLenum internalFormat, GLenum type)
			{
				switch (type) {
				case GL_UNSIGNED_BYTE:
					switch (internalFormat) {
					case GL_STENCIL_INDEX4:
					case GL_STENCIL_INDEX8:
					case GL_ALPHA8:
					case GL_ALPHA:
					case GL_LUMINANCE:                          return 1;
					case GL_LUMINANCE_ALPHA:                    return 2;
					case GL_RGB8:
					case GL_RGB:                                return 3;
					case GL_RGBA8:
					case GL_BGRA_EXT:
					case GL_RGBA:                               return 4;
					default: { NOT_FOUND_ENUM(internalFormat);  return 1; }
					}
				case GL_UNSIGNED_SHORT_4_4_4_4:
					switch (internalFormat) {
					case GL_RGBA4:
					case GL_RGBA8:
					case GL_BGRA_EXT:
					case GL_RGBA:                              return 1;
					default: { NOT_FOUND_ENUM(internalFormat); return 1; }
					}
				case GL_UNSIGNED_SHORT_5_5_5_1:
					switch (internalFormat) {
					case GL_RGB5_A1:
					case GL_RGBA8:
					case GL_BGRA_EXT:
					case GL_RGBA:                              return 1;
					default: { NOT_FOUND_ENUM(internalFormat); return 1; }
					}
				case GL_UNSIGNED_SHORT_5_6_5:
					switch (internalFormat) {
					case GL_RGB565:
					case GL_RGB8:
					case GL_RGB:                               return 1;
					default: { NOT_FOUND_ENUM(internalFormat); return 1; }
					}
				case GL_UNSIGNED_INT_24_8:
					switch (internalFormat) {
					case GL_DEPTH24_STENCIL8:              return 1;
					default: { NOT_FOUND_ENUM(internalFormat); return 1; }
					}
				default: { NOT_FOUND_ENUM(type);               return 1; }
				}
			}

			int32_t
				GlAttribTypeToElementSize(GLenum type)
			{
				switch (type) {
				case GL_BYTE:                           return sizeof(GLbyte);
				case GL_UNSIGNED_BYTE:                  return sizeof(GLubyte);
				case GL_SHORT:                          return sizeof(GLshort);
				case GL_UNSIGNED_SHORT:                 return sizeof(GLushort);
				case GL_INT:                            return sizeof(GLint);
				case GL_FIXED:                          return sizeof(GLfixed);
				case GL_FLOAT:                          return sizeof(GLfloat);
				default: { PANIC("Not element size");   return sizeof(GLubyte); }
				}
			}

			int
				GlTypeToElementSize(GLenum type)
			{
				switch (type) {
				case GL_UNSIGNED_BYTE:                  return sizeof(GLubyte);
				case GL_UNSIGNED_SHORT_4_4_4_4:
				case GL_UNSIGNED_SHORT_5_5_5_1:
				case GL_UNSIGNED_SHORT_5_6_5:           return sizeof(GLushort);
				case GL_UNSIGNED_INT_24_8:          return sizeof(GLuint);
				default: { NOT_FOUND_ENUM(type);        return sizeof(GLubyte); }
				}
			}

			void
				GlFormatToStorageBits(GLenum format, GLint* r_, GLint* g_, GLint* b_, GLint* a_, GLint* d_, GLint* s_)
			{
				GLint r, g, b, a, d, s;

				switch (format) {
				case GL_RGB565:
					r = 5;
					g = 6;
					b = 5;
					a = 0;
					d = 0;
					s = 0;
					break;

				case GL_RGBA4:
					r = 4;
					g = 4;
					b = 4;
					a = 4;
					d = 0;
					s = 0;
					break;

				case GL_RGB5_A1:
					r = 5;
					g = 5;
					b = 5;
					a = 1;
					d = 0;
					s = 0;
					break;

				case GL_RGB:
				case GL_RGB8:
					r = 8;
					g = 8;
					b = 8;
					a = 0;
					d = 0;
					s = 0;
					break;

				case GL_RGBA:
				case GL_RGBA8:
				case GL_BGRA_EXT:
					r = 8;
					g = 8;
					b = 8;
					a = 8;
					d = 0;
					s = 0;
					break;

				case GL_DEPTH_COMPONENT16:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 16;
					s = 0;
					break;

				case GL_DEPTH_COMPONENT24:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 24;
					s = 0;
					break;

				case GL_DEPTH24_STENCIL8:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 24;
					s = 8;
					break;

				case GL_DEPTH_COMPONENT32:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 32;
					s = 0;
					break;


				case GL_STENCIL_INDEX1:
				case GL_STENCIL_INDEX4:
				case GL_STENCIL_INDEX8:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 0;
					s = 8;
					break;


				default:
					r = 0;
					g = 0;
					b = 0;
					a = 0;
					d = 0;
					s = 0;
					break;
				}

				if (r_)  *r_ = r;
				if (g_)  *g_ = g;
				if (b_)  *b_ = b;
				if (a_)  *a_ = a;
				if (d_)  *d_ = d;
				if (s_)  *s_ = s;
			}

			void
				GlFormatToStorageBits(GLenum format, GLfloat* r_, GLfloat* g_, GLfloat* b_, GLfloat* a_, GLfloat* d_, GLfloat* s_)
			{
				GLfloat r, g, b, a, d, s;

				switch (format) {
				case GL_RGB565:
					r = 5.0f;
					g = 6.0f;
					b = 5.0f;
					a = 0.0f;
					d = 0.0f;
					s = 0.0f;
					break;

				case GL_RGBA4:
					r = 4.0f;
					g = 4.0f;
					b = 4.0f;
					a = 4.0f;
					d = 0.0f;
					s = 0.0f;
					break;

				case GL_RGB5_A1:
					r = 5.0f;
					g = 5.0f;
					b = 5.0f;
					a = 1.0f;
					d = 0.0f;
					s = 0.0f;
					break;

				case GL_RGB:
				case GL_RGB8:
					r = 8.0f;
					g = 8.0f;
					b = 8.0f;
					a = 0.0f;
					d = 0.0f;
					s = 0.0f;
					break;

				case GL_RGBA:
				case GL_RGBA8:
				case GL_BGRA_EXT:
					r = 8.0f;
					g = 8.0f;
					b = 8.0f;
					a = 8.0f;
					d = 0.0f;
					s = 0.0f;
					break;

				case GL_DEPTH_COMPONENT16:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 16.0f;
					s = 0.0f;
					break;

				case GL_DEPTH_COMPONENT24:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 24.0f;
					s = 0.0f;
					break;

				case GL_DEPTH24_STENCIL8:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 24.0f;
					s = 8.0f;
					break;

				case GL_DEPTH_COMPONENT32:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 32.0f;
					s = 0.0f;
					break;

				case GL_STENCIL_INDEX1:
				case GL_STENCIL_INDEX4:
				case GL_STENCIL_INDEX8:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 0.0f;
					s = 8.0f;
					break;

				default:
					r = 0.0f;
					g = 0.0f;
					b = 0.0f;
					a = 0.0f;
					d = 0.0f;
					s = 0.0f;
					break;
				}

				if (r_)  *r_ = r;
				if (g_)  *g_ = g;
				if (b_)  *b_ = b;
				if (a_)  *a_ = a;
				if (d_)  *d_ = d;
				if (s_)  *s_ = s;
			}

			void
				GlFormatToStorageBits(GLenum format, GLboolean* r_, GLboolean* g_, GLboolean* b_, GLboolean* a_, GLboolean* d_, GLboolean* s_)
			{
				GLboolean r, g, b, a, d, s;

				switch (format) {
				case GL_RGB565:
					r = GL_TRUE;
					g = GL_TRUE;
					b = GL_TRUE;
					a = GL_FALSE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;

				case GL_RGBA4:
					r = GL_TRUE;
					g = GL_TRUE;
					b = GL_TRUE;
					a = GL_TRUE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;

				case GL_RGB5_A1:
					r = GL_TRUE;
					g = GL_TRUE;
					b = GL_TRUE;
					a = GL_TRUE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;

				case GL_RGB:
				case GL_RGB8:
					r = GL_TRUE;
					g = GL_TRUE;
					b = GL_TRUE;
					a = GL_FALSE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;

				case GL_RGBA:
				case GL_RGBA8:
				case GL_BGRA_EXT:
					r = GL_TRUE;
					g = GL_TRUE;
					b = GL_TRUE;
					a = GL_TRUE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;

				case GL_DEPTH_COMPONENT16:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_TRUE;
					s = GL_FALSE;
					break;

				case GL_DEPTH_COMPONENT24:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_TRUE;
					s = GL_FALSE;
					break;

				case GL_DEPTH24_STENCIL8:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_TRUE;
					s = GL_TRUE;
					break;

				case GL_DEPTH_COMPONENT32:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_TRUE;
					s = GL_FALSE;
					break;


				case GL_STENCIL_INDEX1:
				case GL_STENCIL_INDEX4:
				case GL_STENCIL_INDEX8:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_FALSE;
					s = GL_TRUE;
					break;


				default:
					r = GL_FALSE;
					g = GL_FALSE;
					b = GL_FALSE;
					a = GL_FALSE;
					d = GL_FALSE;
					s = GL_FALSE;
					break;
				}

				if (r_)  *r_ = r;
				if (g_)  *g_ = g;
				if (b_)  *b_ = b;
				if (a_)  *a_ = a;
				if (d_)  *d_ = d;
				if (s_)  *s_ = s;
			}


			bool
				GlFormatIsDepthRenderable(GLenum format)
			{
				switch (format) {
				case GL_DEPTH_COMPONENT16:
				case GL_DEPTH_COMPONENT24:
				case GL_DEPTH_COMPONENT32:      return true;
				default:                            return false;
				}
			}

			bool
				GlFormatIsStencilRenderable(GLenum format)
			{
				switch (format) {
				case GL_STENCIL_INDEX4:
				case GL_STENCIL_INDEX8:             return true;
				default:                            return false;
				}
			}

			bool
				GlFormatIsColorRenderable(GLenum format)
			{
				switch (format) {
				case GL_RGB8:
				case GL_RGBA8:
				case GL_RGB565:
				case GL_RGBA4:
				case GL_RGB5_A1:                    return true;
				default:                            return false;
				}
			}

			uint32_t
				OccupiedLocationsPerGlType(GLenum type)
			{
				switch (type) {
				case GL_BOOL:
				case GL_INT:
				case GL_FLOAT:
				case GL_BOOL_VEC2:
				case GL_INT_VEC2:
				case GL_FLOAT_VEC2:
				case GL_BOOL_VEC3:
				case GL_INT_VEC3:
				case GL_FLOAT_VEC3:
				case GL_BOOL_VEC4:
				case GL_INT_VEC4:
				case GL_FLOAT_VEC4:         return 1;
				case GL_FLOAT_MAT2:         return 2;
				case GL_FLOAT_MAT3:         return 3;
				case GL_FLOAT_MAT4:         return 4;
				default: NOT_REACHED();     return 0;
				}
			}

			bool
				IsGlSampler(GLenum type)
			{
				return (type == GL_SAMPLER_2D) || (type == GL_SAMPLER_CUBE);
			}

			vk::Bool32 GlBooleanToVkBool(GLboolean value)
			{
				return value ? VK_TRUE : VK_FALSE;
			}

			vk::ColorComponentFlags GLColorMaskToVkColorComponentFlags(GLubyte colorMask)
			{
				//assert(colorMask > 0);
				VkColorComponentFlags bits = 0;
				if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_RED)) {
					bits |= VK_COLOR_COMPONENT_R_BIT;
				}
				if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_GREEN)) {
					bits |= VK_COLOR_COMPONENT_G_BIT;
				}
				if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_BLUE)) {
					bits |= VK_COLOR_COMPONENT_B_BIT;
				}
				if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_ALPHA)) {
					bits |= VK_COLOR_COMPONENT_A_BIT;
				}
				return static_cast<vk::ColorComponentFlags>(bits);
			}

			vk::PolygonMode
				GLPrimitiveModeToVkPolygonMode(GLenum mode)
			{
				switch (mode) {
				case GL_POINTS:                return vk::PolygonMode::ePoint;
				case GL_LINE_STRIP:
				case GL_LINE_LOOP:
				case GL_LINES:                 return vk::PolygonMode::eLine;
				case GL_TRIANGLE_STRIP:
				case GL_TRIANGLE_FAN:
				case GL_TRIANGLES:             return vk::PolygonMode::eFill;
				default: NOT_FOUND_ENUM(mode); return vk::PolygonMode::eFill;
				}
			}

			vk::SampleCountFlagBits
				GlSampleCoverageBitsToVkSampleCountFlagBits(GLint bits)
			{
				switch (bits) {
				case 1:                return vk::SampleCountFlagBits::e1;
				case 2:                return vk::SampleCountFlagBits::e2;
				case 4:                return vk::SampleCountFlagBits::e4;
				case 8:                return vk::SampleCountFlagBits::e8;
				case 16:                return vk::SampleCountFlagBits::e16;
				case 32:                return vk::SampleCountFlagBits::e32;
				case 64:                return vk::SampleCountFlagBits::e64;
				default: NOT_REACHED(); return vk::SampleCountFlagBits::e1;
				}
			}

			vk::StencilOp
				GlStencilFuncToVkStencilOp(GLenum mode)
			{
				switch (mode) {
				case GL_KEEP:                     return vk::StencilOp::eKeep;
				case GL_ZERO:                     return vk::StencilOp::eZero;
				case GL_REPLACE:                  return vk::StencilOp::eReplace;
				case GL_INCR:                     return vk::StencilOp::eIncrementAndClamp;
				case GL_INCR_WRAP:                return vk::StencilOp::eIncrementAndWrap;
				case GL_DECR:                     return vk::StencilOp::eDecrementAndClamp;
				case GL_DECR_WRAP:                return vk::StencilOp::eDecrementAndWrap;
				case GL_INVERT:                   return vk::StencilOp::eInvert;
				default: NOT_FOUND_ENUM(mode);    return vk::StencilOp::eKeep;
				}
			}

			vk::LogicOp
				GlLogicOpToVkLogicOp(GLenum mode)
			{
				switch (mode) {
				case GL_CLEAR:                          return vk::LogicOp::eClear;
				case GL_AND:                            return vk::LogicOp::eAnd;
				case GL_AND_REVERSE:                    return vk::LogicOp::eAndReverse;
				case GL_COPY:                           return vk::LogicOp::eCopy;
				case GL_AND_INVERTED:                   return vk::LogicOp::eAndInverted;
				case GL_NOOP:                           return vk::LogicOp::eNoOp;
				case GL_XOR:                            return vk::LogicOp::eXor;
				case GL_OR:                             return vk::LogicOp::eOr;
				case GL_NOR:                            return vk::LogicOp::eNor;
				case GL_EQUIV:                          return vk::LogicOp::eEquivalent;
				case GL_INVERT:                         return vk::LogicOp::eInvert;
				case GL_OR_REVERSE:                     return vk::LogicOp::eOrReverse;
				case GL_COPY_INVERTED:                  return vk::LogicOp::eCopyInverted;
				case GL_OR_INVERTED:                    return vk::LogicOp::eOrInverted;
				case GL_NAND:                           return vk::LogicOp::eNand;
				case GL_SET:                            return vk::LogicOp::eSet;
				default: NOT_FOUND_ENUM(mode);          return vk::LogicOp::eClear;
				}
			}

			vk::CompareOp
				GlCompareFuncToVkCompareOp(GLenum mode)
			{
				switch (mode) {
				case GL_NEVER:                          return vk::CompareOp::eNever;
				case GL_EQUAL:                          return vk::CompareOp::eEqual;
				case GL_LEQUAL:                         return vk::CompareOp::eLessOrEqual;
				case GL_GREATER:                        return vk::CompareOp::eGreater;
				case GL_NOTEQUAL:                       return vk::CompareOp::eNotEqual;
				case GL_GEQUAL:                         return vk::CompareOp::eGreaterOrEqual;
				case GL_ALWAYS:                         return vk::CompareOp::eAlways;
				case GL_LESS:                           return vk::CompareOp::eLess;
				default: NOT_FOUND_ENUM(mode);          return vk::CompareOp::eLessOrEqual;
				}
			}

			vk::CullModeFlagBits
				GlCullModeToVkCullMode(GLenum mode)
			{
				switch (mode) {
				case GL_BACK:                           return vk::CullModeFlagBits::eBack;
				case GL_FRONT:                          return vk::CullModeFlagBits::eFront;
				case GL_FRONT_AND_BACK:                 return vk::CullModeFlagBits::eFrontAndBack;
				default: NOT_FOUND_ENUM(mode);          return vk::CullModeFlagBits::eNone;
				}
			}

			vk::FrontFace
				GlFrontFaceToVkFrontFace(GLenum mode)
			{
				switch (mode) {
				case GL_CW:                             return vk::FrontFace::eClockwise;
				case GL_CCW:                            return vk::FrontFace::eCounterClockwise;
				default: NOT_FOUND_ENUM(mode);          return vk::FrontFace::eClockwise;
				}
			}

			vk::SamplerAddressMode
				GlTexAddressToVkTexAddress(GLenum mode)
			{
				switch (mode) {
				case GL_CLAMP_TO_EDGE:                  return vk::SamplerAddressMode::eClampToEdge;
				case GL_REPEAT:                         return vk::SamplerAddressMode::eRepeat;
				case GL_MIRRORED_REPEAT:                return vk::SamplerAddressMode::eMirroredRepeat;
				default: NOT_REACHED();                 return vk::SamplerAddressMode::eClampToBorder;
				}
			}

			vk::Filter
				GlTexFilterToVkTexFilter(GLenum mode)
			{
				switch (mode) {
				case GL_NEAREST:
				case GL_NEAREST_MIPMAP_NEAREST:
				case GL_NEAREST_MIPMAP_LINEAR:          return vk::Filter::eNearest;

				case GL_LINEAR:
				case GL_LINEAR_MIPMAP_NEAREST:
				case GL_LINEAR_MIPMAP_LINEAR:           return vk::Filter::eLinear;
				default: NOT_FOUND_ENUM(mode);          return vk::Filter::eLinear;
				}
			}

			vk::SamplerMipmapMode
				GlTexMipMapModeToVkMipMapMode(GLenum mode)
			{
				switch (mode) {
				case GL_LINEAR:
				case GL_NEAREST:
				case GL_NEAREST_MIPMAP_NEAREST:
				case GL_LINEAR_MIPMAP_NEAREST:          return vk::SamplerMipmapMode::eNearest;

				case GL_NEAREST_MIPMAP_LINEAR:
				case GL_LINEAR_MIPMAP_LINEAR:           return vk::SamplerMipmapMode::eLinear;
				default: NOT_FOUND_ENUM(mode);          return vk::SamplerMipmapMode::eNearest;
				}
			}

			vk::PrimitiveTopology
				GlPrimitiveTopologyToVkPrimitiveTopology(GLenum mode)
			{
				switch (mode) {
				case GL_POINTS:                         return vk::PrimitiveTopology::ePointList;
				case GL_LINES:                          return vk::PrimitiveTopology::eLineList;
				case GL_LINE_LOOP:
				case GL_LINE_STRIP:                     return vk::PrimitiveTopology::eLineStrip;
				case GL_TRIANGLES:                      return vk::PrimitiveTopology::eTriangleList;
				case GL_TRIANGLE_STRIP:                 return vk::PrimitiveTopology::eTriangleStrip;
				case GL_TRIANGLE_FAN:                   return vk::PrimitiveTopology::eTriangleFan;
				default: NOT_FOUND_ENUM(mode);          return vk::PrimitiveTopology::eTriangleList;
				}
			}

			vk::BlendFactor
				GlBlendFactorToVkBlendFactor(GLenum mode)
			{
				switch (mode) {
				case GL_ONE:                            return vk::BlendFactor::eOne;
				case GL_ZERO:                           return vk::BlendFactor::eZero;
				case GL_SRC_COLOR:                      return vk::BlendFactor::eSrcColor;
				case GL_ONE_MINUS_SRC_COLOR:            return vk::BlendFactor::eOneMinusSrcColor;
				case GL_DST_COLOR:                      return vk::BlendFactor::eDstColor;
				case GL_ONE_MINUS_DST_COLOR:            return vk::BlendFactor::eOneMinusDstColor;
				case GL_SRC_ALPHA:                      return vk::BlendFactor::eSrcAlpha;
				case GL_ONE_MINUS_SRC_ALPHA:            return vk::BlendFactor::eOneMinusSrcAlpha;
				case GL_DST_ALPHA:                      return vk::BlendFactor::eDstAlpha;
				case GL_ONE_MINUS_DST_ALPHA:            return vk::BlendFactor::eOneMinusDstAlpha;
				case GL_CONSTANT_COLOR:                 return vk::BlendFactor::eConstantColor;
				case GL_ONE_MINUS_CONSTANT_COLOR:       return vk::BlendFactor::eOneMinusConstantColor;
				case GL_CONSTANT_ALPHA:                 return vk::BlendFactor::eConstantAlpha;
				case GL_ONE_MINUS_CONSTANT_ALPHA:       return vk::BlendFactor::eOneMinusConstantAlpha;
				case GL_SRC_ALPHA_SATURATE:             return vk::BlendFactor::eSrcAlphaSaturate;
				/*
				case GL_SRC1_ALPHA:                     return vk::BlendFactor::eSrcAlpha;
				case GL_ONE_MINUS_SRC1_ALPHA:           return vk::BlendFactor::eOneMinusSrcAlpha;
				case GL_SRC1_COLOR:                     return vk::BlendFactor::eSrcColor;
				case GL_ONE_MINUS_SRC1_COLOR:           return vk::BlendFactor::eOneMinusSrcColor;
				*/
				case GL_SRC1_ALPHA:                     return vk::BlendFactor::eSrc1Alpha;
				case GL_ONE_MINUS_SRC1_ALPHA:           return vk::BlendFactor::eOneMinusSrc1Alpha;
				case GL_SRC1_COLOR:                     return vk::BlendFactor::eSrc1Color;
				case GL_ONE_MINUS_SRC1_COLOR:           return vk::BlendFactor::eOneMinusSrc1Color;
				default: NOT_FOUND_ENUM(mode);          return vk::BlendFactor::eZero;
				}
			}

			vk::BlendOp
				GlBlendEquationToVkBlendOp(GLenum mode)
			{
				switch (mode) {
				case GL_FUNC_ADD:                       return vk::BlendOp::eAdd;
				case GL_FUNC_SUBTRACT:                  return vk::BlendOp::eSubtract;
				case GL_FUNC_REVERSE_SUBTRACT:          return vk::BlendOp::eReverseSubtract;
				default: NOT_FOUND_ENUM(mode);          return vk::BlendOp::eAdd;
				}
			}

			vk::Format
				GlInternalFormatToVkFormat(GLenum internalformatDepth, GLenum internalformatStencil)
			{
				switch (internalformatDepth) {
				case GL_DEPTH24_STENCIL8:
				case GL_UNSIGNED_INT_24_8:          return vk::Format::eD24UnormS8Uint;
				case GL_DEPTH_COMPONENT16:              return internalformatStencil == GL_STENCIL_INDEX8 ? vk::Format::eD16UnormS8Uint : vk::Format::eD16Unorm;
				case GL_DEPTH_COMPONENT24:          return internalformatStencil == GL_STENCIL_INDEX8 ? vk::Format::eD24UnormS8Uint : vk::Format::eX8D24UnormPack32;
				case GL_DEPTH_COMPONENT32:          return internalformatStencil == GL_STENCIL_INDEX8 ? vk::Format::eD32SfloatS8Uint : vk::Format::eD32Sfloat;
				default:
					switch (internalformatStencil) {
					case GL_STENCIL_INDEX1:
					case GL_STENCIL_INDEX4:
					case GL_STENCIL_INDEX8:             return vk::Format::eS8Uint;
					default: {                          NOT_FOUND_ENUM(internalformatDepth); NOT_FOUND_ENUM(internalformatStencil); }
					}
				}

				return vk::Format::eUndefined;
			}

			vk::Format
				GlInternalFormatToVkFormat(GLenum internalformat)
			{
				switch (internalformat) {
				case GL_RGB565:                           return vk::Format::eR5G6B5UnormPack16;
				case GL_RGBA4:                            return vk::Format::eR4G4B4A4UnormPack16;
				case GL_RGB5_A1:                          return vk::Format::eR5G5B5A1UnormPack16;

				case GL_RGB8:                         return vk::Format::eR8G8B8Unorm;

				case GL_ALPHA:
				case GL_LUMINANCE:
				case GL_LUMINANCE_ALPHA:
				case GL_RGBA:
				case GL_RGBA8:                        return vk::Format::eR8G8B8A8Unorm;

				case GL_BGRA_EXT:                         return vk::Format::eB8G8R8A8Unorm;

				case GL_DEPTH_COMPONENT16:                return vk::Format::eD16Unorm;
				case GL_DEPTH24_STENCIL8:
				case GL_UNSIGNED_INT_24_8:            return vk::Format::eD24UnormS8Uint;
				case GL_DEPTH_COMPONENT24:            return vk::Format::eX8D24UnormPack32;
				case GL_DEPTH_COMPONENT32:            return vk::Format::eD32Sfloat;

				case GL_STENCIL_INDEX1:
				case GL_STENCIL_INDEX4:
				case GL_STENCIL_INDEX8:                   return vk::Format::eS8Uint;

				default: { NOT_FOUND_ENUM(internalformat); return vk::Format::eUndefined; }
				}
			}

			vk::Format GlColorFormatToVkColorFormat(GLenum format, GLenum type)
			{
				switch (type) {
				case GL_UNSIGNED_BYTE: {
					switch (format) {
					case GL_RGB:                        return vk::Format::eR8G8B8Unorm;
					case GL_LUMINANCE:
					case GL_ALPHA:
					case GL_LUMINANCE_ALPHA:
					case GL_RGBA:                       return vk::Format::eR8G8B8A8Unorm;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}
				}
				case GL_UNSIGNED_SHORT_5_6_5: {
					assert(format == GL_RGB);
					return          vk::Format::eR5G6B5UnormPack16;
				}
				case GL_UNSIGNED_SHORT_4_4_4_4: {
					assert(format == GL_RGBA);
					return          vk::Format::eR4G4B4A4UnormPack16;
				}
				case GL_UNSIGNED_SHORT_5_5_5_1: {
					assert(format == GL_RGBA);
					return          vk::Format::eR5G5B5A1UnormPack16;
				}
				default: {
					return vk::Format::eR8G8B8A8Unorm;
				}
				}
			}

			vk::Format
				GlAttribPointerToVkFormat(GLint nElements, GLenum type, GLboolean normalized)
			{
				switch (type) {
				case GL_FLOAT:
				case GL_FIXED:
					switch (nElements) {
					case 1:                             return vk::Format::eR32Sfloat;
					case 2:                             return vk::Format::eR32G32Sfloat;
					case 3:                             return vk::Format::eR32G32B32Sfloat;
					case 4:                             return vk::Format::eR32G32B32A32Sfloat;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_INT:
					switch (nElements) {
					case 1:                             return vk::Format::eR32Sint;
					case 2:                             return vk::Format::eR32G32Sint;
					case 3:                             return vk::Format::eR32G32B32Sint;
					case 4:                             return vk::Format::eR32G32B32A32Sint;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_UNSIGNED_INT:
					switch (nElements) {
					case 1:                             return vk::Format::eR32Uint;
					case 2:                             return vk::Format::eR32G32Uint;
					case 3:                             return vk::Format::eR32G32B32Uint;
					case 4:                             return vk::Format::eR32G32B32A32Uint;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_BYTE:
					switch (nElements) {
					case 1:                             return normalized ? vk::Format::eR8Snorm : vk::Format::eR8Sscaled;
					case 2:                             return normalized ? vk::Format::eR8G8Snorm : vk::Format::eR8G8Sscaled;
					case 3:                             return normalized ? vk::Format::eR8G8B8Snorm : vk::Format::eR8G8B8Sscaled;
					case 4:                             return normalized ? vk::Format::eR8G8B8A8Snorm : vk::Format::eR8G8B8A8Sscaled;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_UNSIGNED_BYTE:
					switch (nElements) {
					case 1:                             return normalized ? vk::Format::eR8Unorm : vk::Format::eR8Uscaled;
					case 2:                             return normalized ? vk::Format::eR8G8Unorm : vk::Format::eR8G8Uscaled;
					case 3:                             return normalized ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8Uscaled;
					case 4:                             return normalized ? vk::Format::eR8G8B8A8Unorm : vk::Format::eR8G8B8A8Uscaled;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_SHORT:
					switch (nElements) {
					case 1:                             return normalized ? vk::Format::eR16Snorm : vk::Format::eR16Sscaled;
					case 2:                             return normalized ? vk::Format::eR16G16Snorm : vk::Format::eR16G16Sscaled;
					case 3:                             return normalized ? vk::Format::eR16G16B16Snorm : vk::Format::eR16G16B16Sscaled;
					case 4:                             return normalized ? vk::Format::eR16G16B16A16Snorm : vk::Format::eR16G16B16A16Sscaled;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				case GL_UNSIGNED_SHORT:
					switch (nElements) {
					case 1:                             return normalized ? vk::Format::eR16Unorm : vk::Format::eR16Uscaled;
					case 2:                             return normalized ? vk::Format::eR16G16Unorm : vk::Format::eR16G16Uscaled;
					case 3:                             return normalized ? vk::Format::eR16G16B16Unorm : vk::Format::eR16G16B16Uscaled;
					case 4:                             return normalized ? vk::Format::eR16G16B16A16Unorm : vk::Format::eR16G16B16A16Uscaled;
					default: { NOT_REACHED();           return vk::Format::eUndefined; }
					}

				default: { NOT_REACHED();               return vk::Format::eUndefined; }
				}
			}

			vk::IndexType
				GlToVkIndexType(GLenum type)
			{
				switch (type) {
				case GL_UNSIGNED_BYTE:
				case GL_UNSIGNED_SHORT:                 return vk::IndexType::eUint16;
				case GL_UNSIGNED_INT:                   return vk::IndexType::eUint32;
				case GL_INVALID_ENUM:                   return vk::IndexType::eNoneKHR;
				default: NOT_FOUND_ENUM(type);          return vk::IndexType::eNoneKHR;
				}
			}


		}
	}
}
