#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Analysis
	{
		using namespace glslang;
		
		enum class ShaderType
		{
			INVALID,
			VERTEX,
			FRAGMENT
		};

		class BaseGLSLInfo
		{
		public:
			explicit BaseGLSLInfo(std::string_view name_, const TObjectReflection& object_reflection_, std::size_t index_);

			const auto& GetObjectReflection() const { return object_reflection; }
			const auto* GetGLSLType() const { return glsl_type; }
			void SetGLSLType(const glslang::TType* glsl_type_) { glsl_type = glsl_type_; }
			void SetOwnGLSLType(std::unique_ptr<glslang::TType> glsl_type_) { own_glsl_type = std::move(glsl_type_); glsl_type = own_glsl_type.get(); }
			std::unique_ptr<glslang::TType> GetCopyGLSLType() { auto copy = std::unique_ptr<glslang::TType>(); memcpy(copy.get(), glsl_type, sizeof(*glsl_type)); return copy; }
			const std::string& GetName() const { return name; }
			std::optional<uint32_t> GetBinding() const { return binding; }
			void SetLocation(uint32_t location_) { location = location_; }
			std::optional<uint32_t> GetLocation() const { return location; }

			auto GetGLType() const { return gl_type; }
			auto GetElementSize() const { return element_size; }
			auto GetNumElements() const { return num_elements; }
			auto GetOffset() const { return offset; }
			auto IsPartOfUniformBlock() const { return !(offset == std::numeric_limits<decltype(offset)>::max()); }
			auto GetIndex() const { return index; }

			void SetIndex(std::size_t index_) { index = index_; }
			void SetOverrideVectorSize(std::size_t size) { override_vector_size = size; }
			auto GetOverrideVectorSize() const { return override_vector_size; }
			void SetOverrideGLSLType(glslang::TBasicType glsl_type_) { override_glsl_type = glsl_type_; }
			auto GetOverrideGLSLType() const { return override_glsl_type; }

			std::string GetInferredDefinition(uint32_t set, uint32_t binding, bool use_override_size = true, bool use_override_glsl_type = true) const;
			std::string GetInferredDefinitionWithQualifiers(bool use_override_size, bool use_override_glsl_type, std::optional<uint32_t> override_location = {}) const;
			std::string GetInferredDefinitionWithoutQualifiers(bool use_override_size = true, bool use_override_glsl_type = true) const;
			std::string UpdateCastMember(glslang::TBasicType glsl_basic_type, bool use_override_size) const;

			void SetOverrideDescription(std::string override_description_);
			const std::string& BaseGLSLInfo::GetOverrideDescription() const;

			void SetReflectionAttribLocation(uint32_t location_) { reflection_attrib_location = location_; }
			std::optional<uint32_t> GetReflectionAttribLocation() const { return reflection_attrib_location; }

		protected:
			//const TObjectReflection& object_reflection;
			TObjectReflection object_reflection;
			const glslang::TType* glsl_type{};
			std::string name{};
			std::optional<uint32_t> set{};
			std::optional<uint32_t> binding{};
			std::optional<uint32_t> location{};

			std::optional<uint32_t> reflection_attrib_location;

			GLenum gl_type{};
			std::size_t element_size{};
			std::size_t num_elements{};
			std::size_t offset{};

			std::size_t index{};

			std::optional<std::size_t> override_vector_size{};

			std::optional<glslang::TBasicType> override_glsl_type{};
			std::string override_description{};

			std::unique_ptr<glslang::TType> own_glsl_type{};
		};

		class Uniform : public BaseGLSLInfo
		{
		public:
			explicit Uniform(const TObjectReflection& object_reflection_, std::size_t index_);

			void SetParentUniformBlockName(std::string_view name) { parent_uniform_block_name = std::string(name); }
			const std::string& GetParentUniformBlockName() const { return parent_uniform_block_name; }

		private:
			std::string parent_uniform_block_name;
		};

		class UniformBlock : public BaseGLSLInfo
		{
		public:
			explicit UniformBlock(const TObjectReflection& object_reflection_, std::size_t index_);
		};

		class PipelineInput : public BaseGLSLInfo
		{
		public:
			explicit PipelineInput(const TObjectReflection& object_reflection_, std::size_t index_, bool is_part_of_struct_ = false);
			bool IsPartOfStruct() { return is_part_of_struct; }
		private:
			bool is_part_of_struct;
		private:
		};

		class PipelineOutput : public BaseGLSLInfo
		{
		public:
			explicit PipelineOutput(const TObjectReflection& object_reflection_, std::size_t index_, bool is_part_of_struct_);
			bool IsPartOfStruct() { return is_part_of_struct; }
		private:
			bool is_part_of_struct;
		};

		enum yytokentype
		{
			YYEMPTY = -2,
			YYEOF = 0,                     /* "end of file"  */
			YYerror = 256,                 /* error  */
			YYUNDEF = 257,                 /* "invalid token"  */
			CONST_ = 258,                   /* CONST  */
			BOOL_ = 259,                    /* BOOL  */
			INT_ = 260,                     /* INT  */
			UINT_ = 261,                    /* UINT  */
			FLOAT_ = 262,                   /* FLOAT  */
			BVEC2 = 263,                   /* BVEC2  */
			BVEC3 = 264,                   /* BVEC3  */
			BVEC4 = 265,                   /* BVEC4  */
			IVEC2 = 266,                   /* IVEC2  */
			IVEC3 = 267,                   /* IVEC3  */
			IVEC4 = 268,                   /* IVEC4  */
			UVEC2 = 269,                   /* UVEC2  */
			UVEC3 = 270,                   /* UVEC3  */
			UVEC4 = 271,                   /* UVEC4  */
			VEC2 = 272,                    /* VEC2  */
			VEC3 = 273,                    /* VEC3  */
			VEC4 = 274,                    /* VEC4  */
			MAT2_ = 275,                    /* MAT2  */
			MAT3_ = 276,                    /* MAT3  */
			MAT4_ = 277,                    /* MAT4  */
			MAT2X2 = 278,                  /* MAT2X2  */
			MAT2X3 = 279,                  /* MAT2X3  */
			MAT2X4 = 280,                  /* MAT2X4  */
			MAT3X2 = 281,                  /* MAT3X2  */
			MAT3X3 = 282,                  /* MAT3X3  */
			MAT3X4 = 283,                  /* MAT3X4  */
			MAT4X2 = 284,                  /* MAT4X2  */
			MAT4X3 = 285,                  /* MAT4X3  */
			MAT4X4 = 286,                  /* MAT4X4  */
			SAMPLER2D = 287,               /* SAMPLER2D  */
			SAMPLER3D = 288,               /* SAMPLER3D  */
			SAMPLERCUBE = 289,             /* SAMPLERCUBE  */
			SAMPLER2DSHADOW = 290,         /* SAMPLER2DSHADOW  */
			SAMPLERCUBESHADOW = 291,       /* SAMPLERCUBESHADOW  */
			SAMPLER2DARRAY = 292,          /* SAMPLER2DARRAY  */
			SAMPLER2DARRAYSHADOW = 293,    /* SAMPLER2DARRAYSHADOW  */
			ISAMPLER2D = 294,              /* ISAMPLER2D  */
			ISAMPLER3D = 295,              /* ISAMPLER3D  */
			ISAMPLERCUBE = 296,            /* ISAMPLERCUBE  */
			ISAMPLER2DARRAY = 297,         /* ISAMPLER2DARRAY  */
			USAMPLER2D = 298,              /* USAMPLER2D  */
			USAMPLER3D = 299,              /* USAMPLER3D  */
			USAMPLERCUBE = 300,            /* USAMPLERCUBE  */
			USAMPLER2DARRAY = 301,         /* USAMPLER2DARRAY  */
			SAMPLER = 302,                 /* SAMPLER  */
			SAMPLERSHADOW = 303,           /* SAMPLERSHADOW  */
			TEXTURE2D = 304,               /* TEXTURE2D  */
			TEXTURE3D = 305,               /* TEXTURE3D  */
			TEXTURECUBE = 306,             /* TEXTURECUBE  */
			TEXTURE2DARRAY = 307,          /* TEXTURE2DARRAY  */
			ITEXTURE2D = 308,              /* ITEXTURE2D  */
			ITEXTURE3D = 309,              /* ITEXTURE3D  */
			ITEXTURECUBE = 310,            /* ITEXTURECUBE  */
			ITEXTURE2DARRAY = 311,         /* ITEXTURE2DARRAY  */
			UTEXTURE2D = 312,              /* UTEXTURE2D  */
			UTEXTURE3D = 313,              /* UTEXTURE3D  */
			UTEXTURECUBE = 314,            /* UTEXTURECUBE  */
			UTEXTURE2DARRAY = 315,         /* UTEXTURE2DARRAY  */
			ATTRIBUTE = 316,               /* ATTRIBUTE  */
			VARYING = 317,                 /* VARYING  */
			FLOAT16_T = 318,               /* FLOAT16_T  */
			FLOAT32_T = 319,               /* FLOAT32_T  */
			DOUBLE = 320,                  /* DOUBLE  */
			FLOAT64_T = 321,               /* FLOAT64_T  */
			INT64_T = 322,                 /* INT64_T  */
			UINT64_T = 323,                /* UINT64_T  */
			INT32_T = 324,                 /* INT32_T  */
			UINT32_T = 325,                /* UINT32_T  */
			INT16_T = 326,                 /* INT16_T  */
			UINT16_T = 327,                /* UINT16_T  */
			INT8_T = 328,                  /* INT8_T  */
			UINT8_T = 329,                 /* UINT8_T  */
			I64VEC2 = 330,                 /* I64VEC2  */
			I64VEC3 = 331,                 /* I64VEC3  */
			I64VEC4 = 332,                 /* I64VEC4  */
			U64VEC2 = 333,                 /* U64VEC2  */
			U64VEC3 = 334,                 /* U64VEC3  */
			U64VEC4 = 335,                 /* U64VEC4  */
			I32VEC2 = 336,                 /* I32VEC2  */
			I32VEC3 = 337,                 /* I32VEC3  */
			I32VEC4 = 338,                 /* I32VEC4  */
			U32VEC2 = 339,                 /* U32VEC2  */
			U32VEC3 = 340,                 /* U32VEC3  */
			U32VEC4 = 341,                 /* U32VEC4  */
			I16VEC2 = 342,                 /* I16VEC2  */
			I16VEC3 = 343,                 /* I16VEC3  */
			I16VEC4 = 344,                 /* I16VEC4  */
			U16VEC2 = 345,                 /* U16VEC2  */
			U16VEC3 = 346,                 /* U16VEC3  */
			U16VEC4 = 347,                 /* U16VEC4  */
			I8VEC2 = 348,                  /* I8VEC2  */
			I8VEC3 = 349,                  /* I8VEC3  */
			I8VEC4 = 350,                  /* I8VEC4  */
			U8VEC2 = 351,                  /* U8VEC2  */
			U8VEC3 = 352,                  /* U8VEC3  */
			U8VEC4 = 353,                  /* U8VEC4  */
			DVEC2 = 354,                   /* DVEC2  */
			DVEC3 = 355,                   /* DVEC3  */
			DVEC4 = 356,                   /* DVEC4  */
			DMAT2 = 357,                   /* DMAT2  */
			DMAT3 = 358,                   /* DMAT3  */
			DMAT4 = 359,                   /* DMAT4  */
			F16VEC2 = 360,                 /* F16VEC2  */
			F16VEC3 = 361,                 /* F16VEC3  */
			F16VEC4 = 362,                 /* F16VEC4  */
			F16MAT2 = 363,                 /* F16MAT2  */
			F16MAT3 = 364,                 /* F16MAT3  */
			F16MAT4 = 365,                 /* F16MAT4  */
			F32VEC2 = 366,                 /* F32VEC2  */
			F32VEC3 = 367,                 /* F32VEC3  */
			F32VEC4 = 368,                 /* F32VEC4  */
			F32MAT2 = 369,                 /* F32MAT2  */
			F32MAT3 = 370,                 /* F32MAT3  */
			F32MAT4 = 371,                 /* F32MAT4  */
			F64VEC2 = 372,                 /* F64VEC2  */
			F64VEC3 = 373,                 /* F64VEC3  */
			F64VEC4 = 374,                 /* F64VEC4  */
			F64MAT2 = 375,                 /* F64MAT2  */
			F64MAT3 = 376,                 /* F64MAT3  */
			F64MAT4 = 377,                 /* F64MAT4  */
			DMAT2X2 = 378,                 /* DMAT2X2  */
			DMAT2X3 = 379,                 /* DMAT2X3  */
			DMAT2X4 = 380,                 /* DMAT2X4  */
			DMAT3X2 = 381,                 /* DMAT3X2  */
			DMAT3X3 = 382,                 /* DMAT3X3  */
			DMAT3X4 = 383,                 /* DMAT3X4  */
			DMAT4X2 = 384,                 /* DMAT4X2  */
			DMAT4X3 = 385,                 /* DMAT4X3  */
			DMAT4X4 = 386,                 /* DMAT4X4  */
			F16MAT2X2 = 387,               /* F16MAT2X2  */
			F16MAT2X3 = 388,               /* F16MAT2X3  */
			F16MAT2X4 = 389,               /* F16MAT2X4  */
			F16MAT3X2 = 390,               /* F16MAT3X2  */
			F16MAT3X3 = 391,               /* F16MAT3X3  */
			F16MAT3X4 = 392,               /* F16MAT3X4  */
			F16MAT4X2 = 393,               /* F16MAT4X2  */
			F16MAT4X3 = 394,               /* F16MAT4X3  */
			F16MAT4X4 = 395,               /* F16MAT4X4  */
			F32MAT2X2 = 396,               /* F32MAT2X2  */
			F32MAT2X3 = 397,               /* F32MAT2X3  */
			F32MAT2X4 = 398,               /* F32MAT2X4  */
			F32MAT3X2 = 399,               /* F32MAT3X2  */
			F32MAT3X3 = 400,               /* F32MAT3X3  */
			F32MAT3X4 = 401,               /* F32MAT3X4  */
			F32MAT4X2 = 402,               /* F32MAT4X2  */
			F32MAT4X3 = 403,               /* F32MAT4X3  */
			F32MAT4X4 = 404,               /* F32MAT4X4  */
			F64MAT2X2 = 405,               /* F64MAT2X2  */
			F64MAT2X3 = 406,               /* F64MAT2X3  */
			F64MAT2X4 = 407,               /* F64MAT2X4  */
			F64MAT3X2 = 408,               /* F64MAT3X2  */
			F64MAT3X3 = 409,               /* F64MAT3X3  */
			F64MAT3X4 = 410,               /* F64MAT3X4  */
			F64MAT4X2 = 411,               /* F64MAT4X2  */
			F64MAT4X3 = 412,               /* F64MAT4X3  */
			F64MAT4X4 = 413,               /* F64MAT4X4  */
			ATOMIC_UINT = 414,             /* ATOMIC_UINT  */
			ACCSTRUCTNV = 415,             /* ACCSTRUCTNV  */
			ACCSTRUCTEXT = 416,            /* ACCSTRUCTEXT  */
			RAYQUERYEXT = 417,             /* RAYQUERYEXT  */
			FCOOPMATNV = 418,              /* FCOOPMATNV  */
			ICOOPMATNV = 419,              /* ICOOPMATNV  */
			UCOOPMATNV = 420,              /* UCOOPMATNV  */
			SAMPLERCUBEARRAY = 421,        /* SAMPLERCUBEARRAY  */
			SAMPLERCUBEARRAYSHADOW = 422,  /* SAMPLERCUBEARRAYSHADOW  */
			ISAMPLERCUBEARRAY = 423,       /* ISAMPLERCUBEARRAY  */
			USAMPLERCUBEARRAY = 424,       /* USAMPLERCUBEARRAY  */
			SAMPLER1D = 425,               /* SAMPLER1D  */
			SAMPLER1DARRAY = 426,          /* SAMPLER1DARRAY  */
			SAMPLER1DARRAYSHADOW = 427,    /* SAMPLER1DARRAYSHADOW  */
			ISAMPLER1D = 428,              /* ISAMPLER1D  */
			SAMPLER1DSHADOW = 429,         /* SAMPLER1DSHADOW  */
			SAMPLER2DRECT = 430,           /* SAMPLER2DRECT  */
			SAMPLER2DRECTSHADOW = 431,     /* SAMPLER2DRECTSHADOW  */
			ISAMPLER2DRECT = 432,          /* ISAMPLER2DRECT  */
			USAMPLER2DRECT = 433,          /* USAMPLER2DRECT  */
			SAMPLERBUFFER = 434,           /* SAMPLERBUFFER  */
			ISAMPLERBUFFER = 435,          /* ISAMPLERBUFFER  */
			USAMPLERBUFFER = 436,          /* USAMPLERBUFFER  */
			SAMPLER2DMS = 437,             /* SAMPLER2DMS  */
			ISAMPLER2DMS = 438,            /* ISAMPLER2DMS  */
			USAMPLER2DMS = 439,            /* USAMPLER2DMS  */
			SAMPLER2DMSARRAY = 440,        /* SAMPLER2DMSARRAY  */
			ISAMPLER2DMSARRAY = 441,       /* ISAMPLER2DMSARRAY  */
			USAMPLER2DMSARRAY = 442,       /* USAMPLER2DMSARRAY  */
			SAMPLEREXTERNALOES = 443,      /* SAMPLEREXTERNALOES  */
			SAMPLEREXTERNAL2DY2YEXT = 444, /* SAMPLEREXTERNAL2DY2YEXT  */
			ISAMPLER1DARRAY = 445,         /* ISAMPLER1DARRAY  */
			USAMPLER1D = 446,              /* USAMPLER1D  */
			USAMPLER1DARRAY = 447,         /* USAMPLER1DARRAY  */
			F16SAMPLER1D = 448,            /* F16SAMPLER1D  */
			F16SAMPLER2D = 449,            /* F16SAMPLER2D  */
			F16SAMPLER3D = 450,            /* F16SAMPLER3D  */
			F16SAMPLER2DRECT = 451,        /* F16SAMPLER2DRECT  */
			F16SAMPLERCUBE = 452,          /* F16SAMPLERCUBE  */
			F16SAMPLER1DARRAY = 453,       /* F16SAMPLER1DARRAY  */
			F16SAMPLER2DARRAY = 454,       /* F16SAMPLER2DARRAY  */
			F16SAMPLERCUBEARRAY = 455,     /* F16SAMPLERCUBEARRAY  */
			F16SAMPLERBUFFER = 456,        /* F16SAMPLERBUFFER  */
			F16SAMPLER2DMS = 457,          /* F16SAMPLER2DMS  */
			F16SAMPLER2DMSARRAY = 458,     /* F16SAMPLER2DMSARRAY  */
			F16SAMPLER1DSHADOW = 459,      /* F16SAMPLER1DSHADOW  */
			F16SAMPLER2DSHADOW = 460,      /* F16SAMPLER2DSHADOW  */
			F16SAMPLER1DARRAYSHADOW = 461, /* F16SAMPLER1DARRAYSHADOW  */
			F16SAMPLER2DARRAYSHADOW = 462, /* F16SAMPLER2DARRAYSHADOW  */
			F16SAMPLER2DRECTSHADOW = 463,  /* F16SAMPLER2DRECTSHADOW  */
			F16SAMPLERCUBESHADOW = 464,    /* F16SAMPLERCUBESHADOW  */
			F16SAMPLERCUBEARRAYSHADOW = 465, /* F16SAMPLERCUBEARRAYSHADOW  */
			IMAGE1D = 466,                 /* IMAGE1D  */
			IIMAGE1D = 467,                /* IIMAGE1D  */
			UIMAGE1D = 468,                /* UIMAGE1D  */
			IMAGE2D = 469,                 /* IMAGE2D  */
			IIMAGE2D = 470,                /* IIMAGE2D  */
			UIMAGE2D = 471,                /* UIMAGE2D  */
			IMAGE3D = 472,                 /* IMAGE3D  */
			IIMAGE3D = 473,                /* IIMAGE3D  */
			UIMAGE3D = 474,                /* UIMAGE3D  */
			IMAGE2DRECT = 475,             /* IMAGE2DRECT  */
			IIMAGE2DRECT = 476,            /* IIMAGE2DRECT  */
			UIMAGE2DRECT = 477,            /* UIMAGE2DRECT  */
			IMAGECUBE = 478,               /* IMAGECUBE  */
			IIMAGECUBE = 479,              /* IIMAGECUBE  */
			UIMAGECUBE = 480,              /* UIMAGECUBE  */
			IMAGEBUFFER = 481,             /* IMAGEBUFFER  */
			IIMAGEBUFFER = 482,            /* IIMAGEBUFFER  */
			UIMAGEBUFFER = 483,            /* UIMAGEBUFFER  */
			IMAGE1DARRAY = 484,            /* IMAGE1DARRAY  */
			IIMAGE1DARRAY = 485,           /* IIMAGE1DARRAY  */
			UIMAGE1DARRAY = 486,           /* UIMAGE1DARRAY  */
			IMAGE2DARRAY = 487,            /* IMAGE2DARRAY  */
			IIMAGE2DARRAY = 488,           /* IIMAGE2DARRAY  */
			UIMAGE2DARRAY = 489,           /* UIMAGE2DARRAY  */
			IMAGECUBEARRAY = 490,          /* IMAGECUBEARRAY  */
			IIMAGECUBEARRAY = 491,         /* IIMAGECUBEARRAY  */
			UIMAGECUBEARRAY = 492,         /* UIMAGECUBEARRAY  */
			IMAGE2DMS = 493,               /* IMAGE2DMS  */
			IIMAGE2DMS = 494,              /* IIMAGE2DMS  */
			UIMAGE2DMS = 495,              /* UIMAGE2DMS  */
			IMAGE2DMSARRAY = 496,          /* IMAGE2DMSARRAY  */
			IIMAGE2DMSARRAY = 497,         /* IIMAGE2DMSARRAY  */
			UIMAGE2DMSARRAY = 498,         /* UIMAGE2DMSARRAY  */
			F16IMAGE1D = 499,              /* F16IMAGE1D  */
			F16IMAGE2D = 500,              /* F16IMAGE2D  */
			F16IMAGE3D = 501,              /* F16IMAGE3D  */
			F16IMAGE2DRECT = 502,          /* F16IMAGE2DRECT  */
			F16IMAGECUBE = 503,            /* F16IMAGECUBE  */
			F16IMAGE1DARRAY = 504,         /* F16IMAGE1DARRAY  */
			F16IMAGE2DARRAY = 505,         /* F16IMAGE2DARRAY  */
			F16IMAGECUBEARRAY = 506,       /* F16IMAGECUBEARRAY  */
			F16IMAGEBUFFER = 507,          /* F16IMAGEBUFFER  */
			F16IMAGE2DMS = 508,            /* F16IMAGE2DMS  */
			F16IMAGE2DMSARRAY = 509,       /* F16IMAGE2DMSARRAY  */
			I64IMAGE1D = 510,              /* I64IMAGE1D  */
			U64IMAGE1D = 511,              /* U64IMAGE1D  */
			I64IMAGE2D = 512,              /* I64IMAGE2D  */
			U64IMAGE2D = 513,              /* U64IMAGE2D  */
			I64IMAGE3D = 514,              /* I64IMAGE3D  */
			U64IMAGE3D = 515,              /* U64IMAGE3D  */
			I64IMAGE2DRECT = 516,          /* I64IMAGE2DRECT  */
			U64IMAGE2DRECT = 517,          /* U64IMAGE2DRECT  */
			I64IMAGECUBE = 518,            /* I64IMAGECUBE  */
			U64IMAGECUBE = 519,            /* U64IMAGECUBE  */
			I64IMAGEBUFFER = 520,          /* I64IMAGEBUFFER  */
			U64IMAGEBUFFER = 521,          /* U64IMAGEBUFFER  */
			I64IMAGE1DARRAY = 522,         /* I64IMAGE1DARRAY  */
			U64IMAGE1DARRAY = 523,         /* U64IMAGE1DARRAY  */
			I64IMAGE2DARRAY = 524,         /* I64IMAGE2DARRAY  */
			U64IMAGE2DARRAY = 525,         /* U64IMAGE2DARRAY  */
			I64IMAGECUBEARRAY = 526,       /* I64IMAGECUBEARRAY  */
			U64IMAGECUBEARRAY = 527,       /* U64IMAGECUBEARRAY  */
			I64IMAGE2DMS = 528,            /* I64IMAGE2DMS  */
			U64IMAGE2DMS = 529,            /* U64IMAGE2DMS  */
			I64IMAGE2DMSARRAY = 530,       /* I64IMAGE2DMSARRAY  */
			U64IMAGE2DMSARRAY = 531,       /* U64IMAGE2DMSARRAY  */
			TEXTURECUBEARRAY = 532,        /* TEXTURECUBEARRAY  */
			ITEXTURECUBEARRAY = 533,       /* ITEXTURECUBEARRAY  */
			UTEXTURECUBEARRAY = 534,       /* UTEXTURECUBEARRAY  */
			TEXTURE1D = 535,               /* TEXTURE1D  */
			ITEXTURE1D = 536,              /* ITEXTURE1D  */
			UTEXTURE1D = 537,              /* UTEXTURE1D  */
			TEXTURE1DARRAY = 538,          /* TEXTURE1DARRAY  */
			ITEXTURE1DARRAY = 539,         /* ITEXTURE1DARRAY  */
			UTEXTURE1DARRAY = 540,         /* UTEXTURE1DARRAY  */
			TEXTURE2DRECT = 541,           /* TEXTURE2DRECT  */
			ITEXTURE2DRECT = 542,          /* ITEXTURE2DRECT  */
			UTEXTURE2DRECT = 543,          /* UTEXTURE2DRECT  */
			TEXTUREBUFFER = 544,           /* TEXTUREBUFFER  */
			ITEXTUREBUFFER = 545,          /* ITEXTUREBUFFER  */
			UTEXTUREBUFFER = 546,          /* UTEXTUREBUFFER  */
			TEXTURE2DMS = 547,             /* TEXTURE2DMS  */
			ITEXTURE2DMS = 548,            /* ITEXTURE2DMS  */
			UTEXTURE2DMS = 549,            /* UTEXTURE2DMS  */
			TEXTURE2DMSARRAY = 550,        /* TEXTURE2DMSARRAY  */
			ITEXTURE2DMSARRAY = 551,       /* ITEXTURE2DMSARRAY  */
			UTEXTURE2DMSARRAY = 552,       /* UTEXTURE2DMSARRAY  */
			F16TEXTURE1D = 553,            /* F16TEXTURE1D  */
			F16TEXTURE2D = 554,            /* F16TEXTURE2D  */
			F16TEXTURE3D = 555,            /* F16TEXTURE3D  */
			F16TEXTURE2DRECT = 556,        /* F16TEXTURE2DRECT  */
			F16TEXTURECUBE = 557,          /* F16TEXTURECUBE  */
			F16TEXTURE1DARRAY = 558,       /* F16TEXTURE1DARRAY  */
			F16TEXTURE2DARRAY = 559,       /* F16TEXTURE2DARRAY  */
			F16TEXTURECUBEARRAY = 560,     /* F16TEXTURECUBEARRAY  */
			F16TEXTUREBUFFER = 561,        /* F16TEXTUREBUFFER  */
			F16TEXTURE2DMS = 562,          /* F16TEXTURE2DMS  */
			F16TEXTURE2DMSARRAY = 563,     /* F16TEXTURE2DMSARRAY  */
			SUBPASSINPUT = 564,            /* SUBPASSINPUT  */
			SUBPASSINPUTMS = 565,          /* SUBPASSINPUTMS  */
			ISUBPASSINPUT = 566,           /* ISUBPASSINPUT  */
			ISUBPASSINPUTMS = 567,         /* ISUBPASSINPUTMS  */
			USUBPASSINPUT = 568,           /* USUBPASSINPUT  */
			USUBPASSINPUTMS = 569,         /* USUBPASSINPUTMS  */
			F16SUBPASSINPUT = 570,         /* F16SUBPASSINPUT  */
			F16SUBPASSINPUTMS = 571,       /* F16SUBPASSINPUTMS  */
			SPIRV_INSTRUCTION = 572,       /* SPIRV_INSTRUCTION  */
			SPIRV_EXECUTION_MODE = 573,    /* SPIRV_EXECUTION_MODE  */
			SPIRV_EXECUTION_MODE_ID = 574, /* SPIRV_EXECUTION_MODE_ID  */
			SPIRV_DECORATE = 575,          /* SPIRV_DECORATE  */
			SPIRV_DECORATE_ID = 576,       /* SPIRV_DECORATE_ID  */
			SPIRV_DECORATE_STRING = 577,   /* SPIRV_DECORATE_STRING  */
			SPIRV_TYPE = 578,              /* SPIRV_TYPE  */
			SPIRV_STORAGE_CLASS = 579,     /* SPIRV_STORAGE_CLASS  */
			SPIRV_BY_REFERENCE = 580,      /* SPIRV_BY_REFERENCE  */
			SPIRV_LITERAL = 581,           /* SPIRV_LITERAL  */
			LEFT_OP = 582,                 /* LEFT_OP  */
			RIGHT_OP = 583,                /* RIGHT_OP  */
			INC_OP = 584,                  /* INC_OP  */
			DEC_OP = 585,                  /* DEC_OP  */
			LE_OP = 586,                   /* LE_OP  */
			GE_OP = 587,                   /* GE_OP  */
			EQ_OP = 588,                   /* EQ_OP  */
			NE_OP = 589,                   /* NE_OP  */
			AND_OP = 590,                  /* AND_OP  */
			OR_OP = 591,                   /* OR_OP  */
			XOR_OP = 592,                  /* XOR_OP  */
			MUL_ASSIGN = 593,              /* MUL_ASSIGN  */
			DIV_ASSIGN = 594,              /* DIV_ASSIGN  */
			ADD_ASSIGN = 595,              /* ADD_ASSIGN  */
			MOD_ASSIGN = 596,              /* MOD_ASSIGN  */
			LEFT_ASSIGN = 597,             /* LEFT_ASSIGN  */
			RIGHT_ASSIGN = 598,            /* RIGHT_ASSIGN  */
			AND_ASSIGN = 599,              /* AND_ASSIGN  */
			XOR_ASSIGN = 600,              /* XOR_ASSIGN  */
			OR_ASSIGN = 601,               /* OR_ASSIGN  */
			SUB_ASSIGN = 602,              /* SUB_ASSIGN  */
			STRING_LITERAL = 603,          /* STRING_LITERAL  */
			LEFT_PAREN = 604,              /* LEFT_PAREN  */
			RIGHT_PAREN = 605,             /* RIGHT_PAREN  */
			LEFT_BRACKET = 606,            /* LEFT_BRACKET  */
			RIGHT_BRACKET = 607,           /* RIGHT_BRACKET  */
			LEFT_BRACE = 608,              /* LEFT_BRACE  */
			RIGHT_BRACE = 609,             /* RIGHT_BRACE  */
			DOT = 610,                     /* DOT  */
			COMMA = 611,                   /* COMMA  */
			COLON = 612,                   /* COLON  */
			EQUAL = 613,                   /* EQUAL  */
			SEMICOLON = 614,               /* SEMICOLON  */
			BANG = 615,                    /* BANG  */
			DASH = 616,                    /* DASH  */
			TILDE = 617,                   /* TILDE  */
			PLUS = 618,                    /* PLUS  */
			STAR = 619,                    /* STAR  */
			SLASH = 620,                   /* SLASH  */
			PERCENT = 621,                 /* PERCENT  */
			LEFT_ANGLE = 622,              /* LEFT_ANGLE  */
			RIGHT_ANGLE = 623,             /* RIGHT_ANGLE  */
			VERTICAL_BAR = 624,            /* VERTICAL_BAR  */
			CARET = 625,                   /* CARET  */
			AMPERSAND = 626,               /* AMPERSAND  */
			QUESTION = 627,                /* QUESTION  */
			INVARIANT = 628,               /* INVARIANT  */
			HIGH_PRECISION = 629,          /* HIGH_PRECISION  */
			MEDIUM_PRECISION = 630,        /* MEDIUM_PRECISION  */
			LOW_PRECISION = 631,           /* LOW_PRECISION  */
			PRECISION = 632,               /* PRECISION  */
			PACKED = 633,                  /* PACKED  */
			RESOURCE = 634,                /* RESOURCE  */
			SUPERP = 635,                  /* SUPERP  */
			FLOATCONSTANT = 636,           /* FLOATCONSTANT  */
			INTCONSTANT = 637,             /* INTCONSTANT  */
			UINTCONSTANT = 638,            /* UINTCONSTANT  */
			BOOLCONSTANT = 639,            /* BOOLCONSTANT  */
			IDENTIFIER = 640,              /* IDENTIFIER  */
			TYPE_NAME = 641,               /* TYPE_NAME  */
			CENTROID = 642,                /* CENTROID  */
			IN_ = 643,                      /* IN  */
			OUT_ = 644,                     /* OUT  */
			INOUT = 645,                   /* INOUT  */
			STRUCT = 646,                  /* STRUCT  */
			VOID_ = 647,                    /* VOID  */
			WHILE = 648,                   /* WHILE  */
			BREAK = 649,                   /* BREAK  */
			CONTINUE = 650,                /* CONTINUE  */
			DO = 651,                      /* DO  */
			ELSE = 652,                    /* ELSE  */
			FOR = 653,                     /* FOR  */
			IF = 654,                      /* IF  */
			DISCARD = 655,                 /* DISCARD  */
			RETURN = 656,                  /* RETURN  */
			SWITCH = 657,                  /* SWITCH  */
			CASE = 658,                    /* CASE  */
			DEFAULT = 659,                 /* DEFAULT  */
			TERMINATE_INVOCATION = 660,    /* TERMINATE_INVOCATION  */
			TERMINATE_RAY = 661,           /* TERMINATE_RAY  */
			IGNORE_INTERSECTION = 662,     /* IGNORE_INTERSECTION  */
			UNIFORM = 663,                 /* UNIFORM  */
			SHARED = 664,                  /* SHARED  */
			BUFFER = 665,                  /* BUFFER  */
			FLAT = 666,                    /* FLAT  */
			SMOOTH = 667,                  /* SMOOTH  */
			LAYOUT = 668,                  /* LAYOUT  */
			DOUBLECONSTANT = 669,          /* DOUBLECONSTANT  */
			INT16CONSTANT = 670,           /* INT16CONSTANT  */
			UINT16CONSTANT = 671,          /* UINT16CONSTANT  */
			FLOAT16CONSTANT = 672,         /* FLOAT16CONSTANT  */
			INT32CONSTANT = 673,           /* INT32CONSTANT  */
			UINT32CONSTANT = 674,          /* UINT32CONSTANT  */
			INT64CONSTANT = 675,           /* INT64CONSTANT  */
			UINT64CONSTANT = 676,          /* UINT64CONSTANT  */
			SUBROUTINE = 677,              /* SUBROUTINE  */
			DEMOTE = 678,                  /* DEMOTE  */
			PAYLOADNV = 679,               /* PAYLOADNV  */
			PAYLOADINNV = 680,             /* PAYLOADINNV  */
			HITATTRNV = 681,               /* HITATTRNV  */
			CALLDATANV = 682,              /* CALLDATANV  */
			CALLDATAINNV = 683,            /* CALLDATAINNV  */
			PAYLOADEXT = 684,              /* PAYLOADEXT  */
			PAYLOADINEXT = 685,            /* PAYLOADINEXT  */
			HITATTREXT = 686,              /* HITATTREXT  */
			CALLDATAEXT = 687,             /* CALLDATAEXT  */
			CALLDATAINEXT = 688,           /* CALLDATAINEXT  */
			PATCH = 689,                   /* PATCH  */
			SAMPLE = 690,                  /* SAMPLE  */
			NONUNIFORM = 691,              /* NONUNIFORM  */
			COHERENT = 692,                /* COHERENT  */
			VOLATILE = 693,                /* VOLATILE  */
			RESTRICT = 694,                /* RESTRICT  */
			READONLY = 695,                /* READONLY  */
			WRITEONLY = 696,               /* WRITEONLY  */
			DEVICECOHERENT = 697,          /* DEVICECOHERENT  */
			QUEUEFAMILYCOHERENT = 698,     /* QUEUEFAMILYCOHERENT  */
			WORKGROUPCOHERENT = 699,       /* WORKGROUPCOHERENT  */
			SUBGROUPCOHERENT = 700,        /* SUBGROUPCOHERENT  */
			NONPRIVATE = 701,              /* NONPRIVATE  */
			SHADERCALLCOHERENT = 702,      /* SHADERCALLCOHERENT  */
			NOPERSPECTIVE = 703,           /* NOPERSPECTIVE  */
			EXPLICITINTERPAMD = 704,       /* EXPLICITINTERPAMD  */
			PERVERTEXNV = 705,             /* PERVERTEXNV  */
			PERPRIMITIVENV = 706,          /* PERPRIMITIVENV  */
			PERVIEWNV = 707,               /* PERVIEWNV  */
			PERTASKNV = 708,               /* PERTASKNV  */
			PRECISE = 709                  /* PRECISE  */
		};

		struct Token
		{
			yytokentype type;
			std::string s;
			std::size_t line;
			union
			{
				bool b;
				int i;
				unsigned int u;
				double d;
			};

			explicit Token(yytokentype type_) :
				type(type_) {}
			explicit Token(yytokentype type_, bool b_) :
				type(type_), b(b_) {}
			explicit Token(yytokentype type_, int i_) :
				type(type_), i(i_) {}
			explicit Token(yytokentype type_, unsigned int u_) :
				type(type_), u(u_) {}
			explicit Token(yytokentype type_, double d_) :
				type(type_), d(d_) {}
			explicit Token(yytokentype type_, std::string_view s_) :
				type(type_), s(std::string(s_)) {}
			explicit Token(yytokentype type_, std::string s_) :
				type(type_), s(std::move(s_)) {}
			explicit Token(yytokentype type_, std::string s_, std::size_t line_) :
				type(type_), s(std::move(s_)), line(line_) {}
			~Token() {};
		};

		struct GLSLTokenizerIterator
		{
			int index{};
			std::size_t line{};

			GLSLTokenizerIterator Clone() { return *this; }

			GLSLTokenizerIterator& operator++()
			{
				index++;
				return *this;
			}

			GLSLTokenizerIterator operator++(int)
			{
				GLSLTokenizerIterator old = *this;
				operator++();
				return old;
			}

			GLSLTokenizerIterator operator+=(int v)
			{
				GLSLTokenizerIterator old = *this;
				for (auto i = 0; i < v; i++)
				{
					operator++();
				}
				return old;
			}

			operator bool()
			{
				return index == 0 && line == 0;
			}
		};

		class GLSLTokenizer
		{
		public:
			explicit GLSLTokenizer() = default;
			~GLSLTokenizer() = default;
			GLSLTokenizer(const GLSLTokenizer& other) = delete;
			GLSLTokenizer& operator=(const GLSLTokenizer& other) = delete;
			GLSLTokenizer(GLSLTokenizer&& other) = default;
			GLSLTokenizer& operator=(GLSLTokenizer&& other) = default;

			Error Init(std::string_view source, EShLanguage shader_type_ = EShLanguage::EShLangVertex);
			Error Init(std::vector<Token> tokens_, EShLanguage shader_type_ = EShLanguage::EShLangVertex);

			template<typename F>
			void IterateMut(F f)
			{
				GLSLTokenizerIterator iterator{};
				while (iterator.index < tokens.size())
				{
					auto& t = tokens[iterator.index];
					iterator.line = t.line;
					bool result = f(iterator, t);
					if (!result)
					{
						break;
					}
					iterator++;
				}
			}

			template<typename F>
			void IterateStartMut(GLSLTokenizerIterator& iterator, F f)
			{
				while (iterator.index < tokens.size())
				{
					auto& t = tokens[iterator.index];
					iterator.line = t.line;
					bool result = f(iterator, t);
					if (!result)
					{
						break;
					}
					iterator++;
				}
			}

			template<typename F>
			void IterateStartPrevMut(GLSLTokenizerIterator& iterator, F f)
			{
				while (iterator.index >= 0)
				{
					auto& t = tokens[iterator.index];
					iterator.line = t.line;
					bool result = f(iterator, t);
					if (!result)
					{
						break;
					}
					iterator.index--;
				}
				iterator.index = std::max(iterator.index, 0);
			}

			void EraseToken(int index);
			bool HasTokenAtIndex(GLSLTokenizerIterator& i);
			Token& GetTokenAtIndex(GLSLTokenizerIterator& i);
			Error InsertTokenAtIndex(std::size_t i, yytokentype type);
			Error InsertTokenAtIndex(std::size_t i, std::string_view s);
			Error InsertCustomTypeAtIndex(std::size_t i, yytokentype type, std::string_view s);
			Error InsertLiteralAtIndex(std::size_t i, std::string_view s);
			Error InsertTokensAtIndexVec(GLSLTokenizerIterator& iter, std::vector<Token> tokens);
			Error InsertTokensAtIndexVec(GLSLTokenizerIterator& iter, std::vector<std::string> tokens);
			Error RemoveTokenAtIndex(GLSLTokenizerIterator& iter);
			Error RemoveLine(GLSLTokenizerIterator& iter);
			Expected<std::string> GetLine(const GLSLTokenizerIterator& iter) const;
			std::optional<GLSLTokenizerIterator> CheckContainsTokenInLine(GLSLTokenizerIterator& iter, yytokentype type);
			std::optional<GLSLTokenizerIterator> CheckContainsTokenInLine(GLSLTokenizerIterator& iter, std::string_view s);
			
			static std::string CovertTokensToString(const std::vector<Token>& tokens);
			std::string ConvertToString();
			std::string ConvertToStringWithoutHeader();

			auto& GetTokens() { return tokens; }
			auto GetShaderType() { return shader_type; }
		private:
			std::vector<Token> tokens;
			EShLanguage shader_type;
		};

		class GLSLAnalyzer
		{
		public:
			explicit GLSLAnalyzer() = default;
			~GLSLAnalyzer() = default;
			GLSLAnalyzer(const GLSLAnalyzer& other) = delete;
			GLSLAnalyzer& operator=(const GLSLAnalyzer& other) = delete;
			GLSLAnalyzer(GLSLAnalyzer&& other) = default;
			GLSLAnalyzer& operator=(GLSLAnalyzer&& other) = default;

			Error Init(std::string_view source_, EShLanguage shader_type = EShLanguage::EShLangVertex);
			
			const std::string& GetSource() { return source; }
			const std::vector<uint32_t>& GetSpirv() { return spirv; }

			flat_hash_map<std::string, Uniform>& GetUniforms() { return uniforms; }
			const flat_hash_map<std::string, Uniform>& GetUniforms() const { return uniforms; }
			const flat_hash_map<std::string, UniformBlock>& GetUniformBlocks() const { return uniform_blocks; }
			flat_hash_map<std::string, PipelineInput>& GetPipelineInputs() { return pipeline_inputs; }
			const flat_hash_map<std::string, PipelineInput>& GetPipelineInputs() const { return pipeline_inputs; }
			const std::map<uint32_t, PipelineInput*>& GetPipelineIndexToPipelineInput() const { return index_to_pipeline_input; }
			Error UpdatePipelineInputIndex(std::string_view name, std::size_t index)
			{
				if (auto found = pipeline_inputs.find(name); found != std::end(pipeline_inputs))
				{
					auto& [name, pipeline_input] = *found;
					pipeline_input.SetIndex(index);

					const auto& glsl_type = pipeline_input.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					if (qualifier.hasLayout() && qualifier.hasLocation())
					{
						if (index != qualifier.layoutLocation)
						{
							// override
							index = qualifier.layoutLocation;
						}
					}

					index_to_pipeline_input[index] = &pipeline_input;
				}
				else
				{
					return StringError("{} not found!\n", name);
				}
				return NoError();
			}
			flat_hash_map<std::string, PipelineOutput>& GetPipelineOutputs() { return pipeline_outputs; }

			const auto GetShaderType() const { return shader_type; }
			glslang::TShader& GetShader() { return *shader; }
			const glslang::TProgram& GetProgram() { return *program; }
			auto& GetTokenizer() { return tokenizer; }
		private:
			std::string source;
			bool initialized = false;

			EShLanguage shader_type{};
			std::unique_ptr<glslang::TShader> shader{};
			std::unique_ptr<glslang::TProgram> program{};

			std::vector<uint32_t> spirv;

			flat_hash_map<std::string, Uniform> uniforms;
#ifndef NDEBUG
			std::map<std::string, const Uniform*> uniforms_debug;
#endif
			flat_hash_map<std::string, UniformBlock> uniform_blocks;
			flat_hash_map<std::string, PipelineInput> pipeline_inputs;
			flat_hash_map<std::string, PipelineOutput> pipeline_outputs;

			std::map<uint32_t, PipelineInput*> index_to_pipeline_input;

			GLSLTokenizer tokenizer;
		};

		const EProfile glsl_profile = ENoProfile;
		const int glsl_version = 450;
	}
}
