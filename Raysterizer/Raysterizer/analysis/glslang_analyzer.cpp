#include "analysis/glslang_analyzer.h"

union YYSTYPE
{
    struct {
        glslang::TSourceLoc loc;
        union {
            glslang::TString* string;
            int i;
            unsigned int u;
            long long i64;
            unsigned long long u64;
            bool b;
            double d;
        };
        glslang::TSymbol* symbol;
    } lex;
    struct {
        glslang::TSourceLoc loc;
        glslang::TOperator op;
        union {
            TIntermNode* intermNode;
            glslang::TIntermNodePair nodePair;
            glslang::TIntermTyped* intermTypedNode;
            glslang::TAttributes* attributes;
        };
        union {
            glslang::TPublicType type;
            glslang::TFunction* function;
            glslang::TParameter param;
            glslang::TTypeLoc typeLine;
            glslang::TTypeList* typeList;
            glslang::TArraySizes* arraySizes;
            glslang::TIdentifierList* identifierList;
        };
        glslang::TArraySizes* typeParameters;
    } interm;
};

class TParserToken {
public:
    explicit TParserToken(YYSTYPE& b) : sType(b) { }

    YYSTYPE& sType;
protected:
    TParserToken(TParserToken&);
    TParserToken& operator=(TParserToken&);
};

namespace Raysterizer
{
    namespace Analysis
    {
        namespace
        {
            const TBuiltInResource DefaultTBuiltInResource = {
                /* .MaxLights = */ 32,
                /* .MaxClipPlanes = */ 6,
                /* .MaxTextureUnits = */ 32,
                /* .MaxTextureCoords = */ 32,
                /* .MaxVertexAttribs = */ 64,
                /* .MaxVertexUniformComponents = */ 4096,
                /* .MaxVaryingFloats = */ 64,
                /* .MaxVertexTextureImageUnits = */ 32,
                /* .MaxCombinedTextureImageUnits = */ 80,
                /* .MaxTextureImageUnits = */ 32,
                /* .MaxFragmentUniformComponents = */ 4096,
                /* .MaxDrawBuffers = */ 32,
                /* .MaxVertexUniformVectors = */ 128,
                /* .MaxVaryingVectors = */ 8,
                /* .MaxFragmentUniformVectors = */ 16,
                /* .MaxVertexOutputVectors = */ 16,
                /* .MaxFragmentInputVectors = */ 15,
                /* .MinprogramTexelOffset = */ -8,
                /* .MaxprogramTexelOffset = */ 7,
                /* .MaxClipDistances = */ 8,
                /* .MaxComputeWorkGroupCountX = */ 65535,
                /* .MaxComputeWorkGroupCountY = */ 65535,
                /* .MaxComputeWorkGroupCountZ = */ 65535,
                /* .MaxComputeWorkGroupSizeX = */ 1024,
                /* .MaxComputeWorkGroupSizeY = */ 1024,
                /* .MaxComputeWorkGroupSizeZ = */ 64,
                /* .MaxComputeUniformComponents = */ 1024,
                /* .MaxComputeTextureImageUnits = */ 16,
                /* .MaxComputeImageUniforms = */ 8,
                /* .MaxComputeAtomicCounters = */ 8,
                /* .MaxComputeAtomicCounterBuffers = */ 1,
                /* .MaxVaryingComponents = */ 60,
                /* .MaxVertexOutputComponents = */ 64,
                /* .MaxGeometryInputComponents = */ 64,
                /* .MaxGeometryOutputComponents = */ 128,
                /* .MaxFragmentInputComponents = */ 128,
                /* .MaxImageUnits = */ 8,
                /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
                /* .MaxCombinedShaderOutputResources = */ 8,
                /* .MaxImageSamples = */ 0,
                /* .MaxVertexImageUniforms = */ 0,
                /* .MaxTessControlImageUniforms = */ 0,
                /* .MaxTessEvaluationImageUniforms = */ 0,
                /* .MaxGeometryImageUniforms = */ 0,
                /* .MaxFragmentImageUniforms = */ 8,
                /* .MaxCombinedImageUniforms = */ 8,
                /* .MaxGeometryTextureImageUnits = */ 16,
                /* .MaxGeometryOutputVertices = */ 256,
                /* .MaxGeometryTotalOutputComponents = */ 1024,
                /* .MaxGeometryUniformComponents = */ 1024,
                /* .MaxGeometryVaryingComponents = */ 64,
                /* .MaxTessControlInputComponents = */ 128,
                /* .MaxTessControlOutputComponents = */ 128,
                /* .MaxTessControlTextureImageUnits = */ 16,
                /* .MaxTessControlUniformComponents = */ 1024,
                /* .MaxTessControlTotalOutputComponents = */ 4096,
                /* .MaxTessEvaluationInputComponents = */ 128,
                /* .MaxTessEvaluationOutputComponents = */ 128,
                /* .MaxTessEvaluationTextureImageUnits = */ 16,
                /* .MaxTessEvaluationUniformComponents = */ 1024,
                /* .MaxTessPatchComponents = */ 120,
                /* .MaxPatchVertices = */ 32,
                /* .MaxTessGenLevel = */ 64,
                /* .MaxViewports = */ 16,
                /* .MaxVertexAtomicCounters = */ 0,
                /* .MaxTessControlAtomicCounters = */ 0,
                /* .MaxTessEvaluationAtomicCounters = */ 0,
                /* .MaxGeometryAtomicCounters = */ 0,
                /* .MaxFragmentAtomicCounters = */ 8,
                /* .MaxCombinedAtomicCounters = */ 8,
                /* .MaxAtomicCounterBindings = */ 1,
                /* .MaxVertexAtomicCounterBuffers = */ 0,
                /* .MaxTessControlAtomicCounterBuffers = */ 0,
                /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
                /* .MaxGeometryAtomicCounterBuffers = */ 0,
                /* .MaxFragmentAtomicCounterBuffers = */ 1,
                /* .MaxCombinedAtomicCounterBuffers = */ 1,
                /* .MaxAtomicCounterBufferSize = */ 16384,
                /* .MaxTransformFeedbackBuffers = */ 4,
                /* .MaxTransformFeedbackInterleavedComponents = */ 64,
                /* .MaxCullDistances = */ 8,
                /* .MaxCombinedClipAndCullDistances = */ 8,
                /* .MaxSamples = */ 4,
                /* .maxMeshOutputVerticesNV = */ 256,
                /* .maxMeshOutputPrimitivesNV = */ 512,
                /* .maxMeshWorkGroupSizeX_NV = */ 32,
                /* .maxMeshWorkGroupSizeY_NV = */ 1,
                /* .maxMeshWorkGroupSizeZ_NV = */ 1,
                /* .maxTaskWorkGroupSizeX_NV = */ 32,
                /* .maxTaskWorkGroupSizeY_NV = */ 1,
                /* .maxTaskWorkGroupSizeZ_NV = */ 1,
                /* .maxMeshViewCountNV = */ 4,
                /* .maxDualSourceDrawBuffersEXT = */ 1,

                /* .limits = */ {
                    /* .nonInductiveForLoops = */ 1,
                    /* .whileLoops = */ 1,
                    /* .doWhileLoops = */ 1,
                    /* .generalUniformIndexing = */ 1,
                    /* .generalAttributeMatrixVectorIndexing = */ 1,
                    /* .generalVaryingIndexing = */ 1,
                    /* .generalSamplerIndexing = */ 1,
                    /* .generalVariableIndexing = */ 1,
                    /* .generalConstantMatrixVectorIndexing = */ 1,
                }
            };
        }
        
        static const flat_hash_map<yytokentype, std::string> glsl_token_type_to_string =
        {
   {yytokentype::YYEMPTY, "yyempty"},
   {yytokentype::YYEOF, "yyeof"},
   {yytokentype::YYerror, "yyerror"},
   {yytokentype::YYUNDEF, "yyundef"},
   {yytokentype::CONST_, "const"},
   {yytokentype::BOOL_, "bool"},
   {yytokentype::INT_, "int"},
   {yytokentype::UINT_, "uint"},
   {yytokentype::FLOAT_, "float"},
   {yytokentype::BVEC2, "bvec2"},
   {yytokentype::BVEC3, "bvec3"},
   {yytokentype::BVEC4, "bvec4"},
   {yytokentype::IVEC2, "ivec2"},
   {yytokentype::IVEC3, "ivec3"},
   {yytokentype::IVEC4, "ivec4"},
   {yytokentype::UVEC2, "uvec2"},
   {yytokentype::UVEC3, "uvec3"},
   {yytokentype::UVEC4, "uvec4"},
   {yytokentype::VEC2, "vec2"},
   {yytokentype::VEC3, "vec3"},
   {yytokentype::VEC4, "vec4"},
   {yytokentype::MAT2_, "mat2"},
   {yytokentype::MAT3_, "mat3"},
   {yytokentype::MAT4_, "mat4"},
   {yytokentype::MAT2X2, "mat2x2"},
   {yytokentype::MAT2X3, "mat2x3"},
   {yytokentype::MAT2X4, "mat2x4"},
   {yytokentype::MAT3X2, "mat3x2"},
   {yytokentype::MAT3X3, "mat3x3"},
   {yytokentype::MAT3X4, "mat3x4"},
   {yytokentype::MAT4X2, "mat4x2"},
   {yytokentype::MAT4X3, "mat4x3"},
   {yytokentype::MAT4X4, "mat4x4"},
   {yytokentype::SAMPLER2D, "sampler2D"},
   {yytokentype::SAMPLER3D, "sampler3D"},
   {yytokentype::SAMPLERCUBE, "samplerCube"},
   {yytokentype::SAMPLER2DSHADOW, "sampler2Dshadow"},
   {yytokentype::SAMPLERCUBESHADOW, "samplerCubeshadow"},
   {yytokentype::SAMPLER2DARRAY, "sampler2DArray"},
   {yytokentype::SAMPLER2DARRAYSHADOW, "sampler2DArrayshadow"},
   {yytokentype::ISAMPLER2D, "isampler2D"},
   {yytokentype::ISAMPLER3D, "isampler3D"},
   {yytokentype::ISAMPLERCUBE, "isamplerCube"},
   {yytokentype::ISAMPLER2DARRAY, "isampler2DArray"},
   {yytokentype::USAMPLER2D, "usampler2D"},
   {yytokentype::USAMPLER3D, "usampler3D"},
   {yytokentype::USAMPLERCUBE, "usamplerCube"},
   {yytokentype::USAMPLER2DARRAY, "usampler2DArray"},
   {yytokentype::SAMPLER, "sampler"},
   {yytokentype::SAMPLERSHADOW, "samplershadow"},
   {yytokentype::TEXTURE2D, "texture2D"},
   {yytokentype::TEXTURE3D, "texture3D"},
   {yytokentype::TEXTURECUBE, "texturecube"},
   {yytokentype::TEXTURE2DARRAY, "texture2Darray"},
   {yytokentype::ITEXTURE2D, "itexture2D"},
   {yytokentype::ITEXTURE3D, "itexture3D"},
   {yytokentype::ITEXTURECUBE, "itexturecube"},
   {yytokentype::ITEXTURE2DARRAY, "itexture2Darray"},
   {yytokentype::UTEXTURE2D, "utexture2D"},
   {yytokentype::UTEXTURE3D, "utexture3D"},
   {yytokentype::UTEXTURECUBE, "utexturecube"},
   {yytokentype::UTEXTURE2DARRAY, "utexture2Darray"},
   {yytokentype::ATTRIBUTE, "attribute"},
   {yytokentype::VARYING, "varying"},
   {yytokentype::FLOAT16_T, "float16_t"},
   {yytokentype::FLOAT32_T, "float32_t"},
   {yytokentype::DOUBLE, "double"},
   {yytokentype::FLOAT64_T, "float64_t"},
   {yytokentype::INT64_T, "int64_t"},
   {yytokentype::UINT64_T, "uint64_t"},
   {yytokentype::INT32_T, "int32_t"},
   {yytokentype::UINT32_T, "uint32_t"},
   {yytokentype::INT16_T, "int16_t"},
   {yytokentype::UINT16_T, "uint16_t"},
   {yytokentype::INT8_T, "int8_t"},
   {yytokentype::UINT8_T, "uint8_t"},
   {yytokentype::I64VEC2, "i64vec2"},
   {yytokentype::I64VEC3, "i64vec3"},
   {yytokentype::I64VEC4, "i64vec4"},
   {yytokentype::U64VEC2, "u64vec2"},
   {yytokentype::U64VEC3, "u64vec3"},
   {yytokentype::U64VEC4, "u64vec4"},
   {yytokentype::I32VEC2, "i32vec2"},
   {yytokentype::I32VEC3, "i32vec3"},
   {yytokentype::I32VEC4, "i32vec4"},
   {yytokentype::U32VEC2, "u32vec2"},
   {yytokentype::U32VEC3, "u32vec3"},
   {yytokentype::U32VEC4, "u32vec4"},
   {yytokentype::I16VEC2, "i16vec2"},
   {yytokentype::I16VEC3, "i16vec3"},
   {yytokentype::I16VEC4, "i16vec4"},
   {yytokentype::U16VEC2, "u16vec2"},
   {yytokentype::U16VEC3, "u16vec3"},
   {yytokentype::U16VEC4, "u16vec4"},
   {yytokentype::I8VEC2, "i8vec2"},
   {yytokentype::I8VEC3, "i8vec3"},
   {yytokentype::I8VEC4, "i8vec4"},
   {yytokentype::U8VEC2, "u8vec2"},
   {yytokentype::U8VEC3, "u8vec3"},
   {yytokentype::U8VEC4, "u8vec4"},
   {yytokentype::DVEC2, "dvec2"},
   {yytokentype::DVEC3, "dvec3"},
   {yytokentype::DVEC4, "dvec4"},
   {yytokentype::DMAT2, "dmat2"},
   {yytokentype::DMAT3, "dmat3"},
   {yytokentype::DMAT4, "dmat4"},
   {yytokentype::F16VEC2, "f16vec2"},
   {yytokentype::F16VEC3, "f16vec3"},
   {yytokentype::F16VEC4, "f16vec4"},
   {yytokentype::F16MAT2, "f16mat2"},
   {yytokentype::F16MAT3, "f16mat3"},
   {yytokentype::F16MAT4, "f16mat4"},
   {yytokentype::F32VEC2, "f32vec2"},
   {yytokentype::F32VEC3, "f32vec3"},
   {yytokentype::F32VEC4, "f32vec4"},
   {yytokentype::F32MAT2, "f32mat2"},
   {yytokentype::F32MAT3, "f32mat3"},
   {yytokentype::F32MAT4, "f32mat4"},
   {yytokentype::F64VEC2, "f64vec2"},
   {yytokentype::F64VEC3, "f64vec3"},
   {yytokentype::F64VEC4, "f64vec4"},
   {yytokentype::F64MAT2, "f64mat2"},
   {yytokentype::F64MAT3, "f64mat3"},
   {yytokentype::F64MAT4, "f64mat4"},
   {yytokentype::DMAT2X2, "dmat2x2"},
   {yytokentype::DMAT2X3, "dmat2x3"},
   {yytokentype::DMAT2X4, "dmat2x4"},
   {yytokentype::DMAT3X2, "dmat3x2"},
   {yytokentype::DMAT3X3, "dmat3x3"},
   {yytokentype::DMAT3X4, "dmat3x4"},
   {yytokentype::DMAT4X2, "dmat4x2"},
   {yytokentype::DMAT4X3, "dmat4x3"},
   {yytokentype::DMAT4X4, "dmat4x4"},
   {yytokentype::F16MAT2X2, "f16mat2x2"},
   {yytokentype::F16MAT2X3, "f16mat2x3"},
   {yytokentype::F16MAT2X4, "f16mat2x4"},
   {yytokentype::F16MAT3X2, "f16mat3x2"},
   {yytokentype::F16MAT3X3, "f16mat3x3"},
   {yytokentype::F16MAT3X4, "f16mat3x4"},
   {yytokentype::F16MAT4X2, "f16mat4x2"},
   {yytokentype::F16MAT4X3, "f16mat4x3"},
   {yytokentype::F16MAT4X4, "f16mat4x4"},
   {yytokentype::F32MAT2X2, "f32mat2x2"},
   {yytokentype::F32MAT2X3, "f32mat2x3"},
   {yytokentype::F32MAT2X4, "f32mat2x4"},
   {yytokentype::F32MAT3X2, "f32mat3x2"},
   {yytokentype::F32MAT3X3, "f32mat3x3"},
   {yytokentype::F32MAT3X4, "f32mat3x4"},
   {yytokentype::F32MAT4X2, "f32mat4x2"},
   {yytokentype::F32MAT4X3, "f32mat4x3"},
   {yytokentype::F32MAT4X4, "f32mat4x4"},
   {yytokentype::F64MAT2X2, "f64mat2x2"},
   {yytokentype::F64MAT2X3, "f64mat2x3"},
   {yytokentype::F64MAT2X4, "f64mat2x4"},
   {yytokentype::F64MAT3X2, "f64mat3x2"},
   {yytokentype::F64MAT3X3, "f64mat3x3"},
   {yytokentype::F64MAT3X4, "f64mat3x4"},
   {yytokentype::F64MAT4X2, "f64mat4x2"},
   {yytokentype::F64MAT4X3, "f64mat4x3"},
   {yytokentype::F64MAT4X4, "f64mat4x4"},
   {yytokentype::ATOMIC_UINT, "atomic_uint"},
   {yytokentype::ACCSTRUCTNV, "accstructnv"},
   {yytokentype::ACCSTRUCTEXT, "accstructext"},
   {yytokentype::RAYQUERYEXT, "rayqueryext"},
   {yytokentype::FCOOPMATNV, "fcoopmatnv"},
   {yytokentype::ICOOPMATNV, "icoopmatnv"},
   {yytokentype::UCOOPMATNV, "ucoopmatnv"},
   {yytokentype::SAMPLERCUBEARRAY, "samplercubearray"},
   {yytokentype::SAMPLERCUBEARRAYSHADOW, "samplercubearrayshadow"},
   {yytokentype::ISAMPLERCUBEARRAY, "isamplercubearray"},
   {yytokentype::USAMPLERCUBEARRAY, "usamplercubearray"},
   {yytokentype::SAMPLER1D, "sampler1D"},
   {yytokentype::SAMPLER1DARRAY, "sampler1Darray"},
   {yytokentype::SAMPLER1DARRAYSHADOW, "sampler1Darrayshadow"},
   {yytokentype::ISAMPLER1D, "isampler1D"},
   {yytokentype::SAMPLER1DSHADOW, "sampler1Dshadow"},
   {yytokentype::SAMPLER2DRECT, "sampler2Drect"},
   {yytokentype::SAMPLER2DRECTSHADOW, "sampler2Drectshadow"},
   {yytokentype::ISAMPLER2DRECT, "isampler2Drect"},
   {yytokentype::USAMPLER2DRECT, "usampler2Drect"},
   {yytokentype::SAMPLERBUFFER, "samplerbuffer"},
   {yytokentype::ISAMPLERBUFFER, "isamplerbuffer"},
   {yytokentype::USAMPLERBUFFER, "usamplerbuffer"},
   {yytokentype::SAMPLER2DMS, "sampler2Dms"},
   {yytokentype::ISAMPLER2DMS, "isampler2Dms"},
   {yytokentype::USAMPLER2DMS, "usampler2Dms"},
   {yytokentype::SAMPLER2DMSARRAY, "sampler2Dmsarray"},
   {yytokentype::ISAMPLER2DMSARRAY, "isampler2Dmsarray"},
   {yytokentype::USAMPLER2DMSARRAY, "usampler2Dmsarray"},
   {yytokentype::SAMPLEREXTERNALOES, "samplerexternaloes"},
   {yytokentype::SAMPLEREXTERNAL2DY2YEXT, "samplerexternal2Dy2yext"},
   {yytokentype::ISAMPLER1DARRAY, "isampler1Darray"},
   {yytokentype::USAMPLER1D, "usampler1D"},
   {yytokentype::USAMPLER1DARRAY, "usampler1Darray"},
   {yytokentype::F16SAMPLER1D, "f16sampler1D"},
   {yytokentype::F16SAMPLER2D, "f16sampler2D"},
   {yytokentype::F16SAMPLER3D, "f16sampler3D"},
   {yytokentype::F16SAMPLER2DRECT, "f16sampler2Drect"},
   {yytokentype::F16SAMPLERCUBE, "f16samplercube"},
   {yytokentype::F16SAMPLER1DARRAY, "f16sampler1Darray"},
   {yytokentype::F16SAMPLER2DARRAY, "f16sampler2Darray"},
   {yytokentype::F16SAMPLERCUBEARRAY, "f16samplercubearray"},
   {yytokentype::F16SAMPLERBUFFER, "f16samplerbuffer"},
   {yytokentype::F16SAMPLER2DMS, "f16sampler2Dms"},
   {yytokentype::F16SAMPLER2DMSARRAY, "f16sampler2Dmsarray"},
   {yytokentype::F16SAMPLER1DSHADOW, "f16sampler1Dshadow"},
   {yytokentype::F16SAMPLER2DSHADOW, "f16sampler2Dshadow"},
   {yytokentype::F16SAMPLER1DARRAYSHADOW, "f16sampler1Darrayshadow"},
   {yytokentype::F16SAMPLER2DARRAYSHADOW, "f16sampler2Darrayshadow"},
   {yytokentype::F16SAMPLER2DRECTSHADOW, "f16sampler2Drectshadow"},
   {yytokentype::F16SAMPLERCUBESHADOW, "f16samplercubeshadow"},
   {yytokentype::F16SAMPLERCUBEARRAYSHADOW, "f16samplercubearrayshadow"},
   {yytokentype::IMAGE1D, "image1D"},
   {yytokentype::IIMAGE1D, "iimage1D"},
   {yytokentype::UIMAGE1D, "uimage1D"},
   {yytokentype::IMAGE2D, "image2D"},
   {yytokentype::IIMAGE2D, "iimage2D"},
   {yytokentype::UIMAGE2D, "uimage2D"},
   {yytokentype::IMAGE3D, "image3D"},
   {yytokentype::IIMAGE3D, "iimage3D"},
   {yytokentype::UIMAGE3D, "uimage3D"},
   {yytokentype::IMAGE2DRECT, "image2Drect"},
   {yytokentype::IIMAGE2DRECT, "iimage2Drect"},
   {yytokentype::UIMAGE2DRECT, "uimage2Drect"},
   {yytokentype::IMAGECUBE, "imagecube"},
   {yytokentype::IIMAGECUBE, "iimagecube"},
   {yytokentype::UIMAGECUBE, "uimagecube"},
   {yytokentype::IMAGEBUFFER, "imagebuffer"},
   {yytokentype::IIMAGEBUFFER, "iimagebuffer"},
   {yytokentype::UIMAGEBUFFER, "uimagebuffer"},
   {yytokentype::IMAGE1DARRAY, "image1Darray"},
   {yytokentype::IIMAGE1DARRAY, "iimage1Darray"},
   {yytokentype::UIMAGE1DARRAY, "uimage1Darray"},
   {yytokentype::IMAGE2DARRAY, "image2Darray"},
   {yytokentype::IIMAGE2DARRAY, "iimage2Darray"},
   {yytokentype::UIMAGE2DARRAY, "uimage2Darray"},
   {yytokentype::IMAGECUBEARRAY, "imagecubearray"},
   {yytokentype::IIMAGECUBEARRAY, "iimagecubearray"},
   {yytokentype::UIMAGECUBEARRAY, "uimagecubearray"},
   {yytokentype::IMAGE2DMS, "image2Dms"},
   {yytokentype::IIMAGE2DMS, "iimage2Dms"},
   {yytokentype::UIMAGE2DMS, "uimage2Dms"},
   {yytokentype::IMAGE2DMSARRAY, "image2Dmsarray"},
   {yytokentype::IIMAGE2DMSARRAY, "iimage2Dmsarray"},
   {yytokentype::UIMAGE2DMSARRAY, "uimage2Dmsarray"},
   {yytokentype::F16IMAGE1D, "f16image1D"},
   {yytokentype::F16IMAGE2D, "f16image2D"},
   {yytokentype::F16IMAGE3D, "f16image3D"},
   {yytokentype::F16IMAGE2DRECT, "f16image2Drect"},
   {yytokentype::F16IMAGECUBE, "f16imagecube"},
   {yytokentype::F16IMAGE1DARRAY, "f16image1Darray"},
   {yytokentype::F16IMAGE2DARRAY, "f16image2Darray"},
   {yytokentype::F16IMAGECUBEARRAY, "f16imagecubearray"},
   {yytokentype::F16IMAGEBUFFER, "f16imagebuffer"},
   {yytokentype::F16IMAGE2DMS, "f16image2Dms"},
   {yytokentype::F16IMAGE2DMSARRAY, "f16image2Dmsarray"},
   {yytokentype::I64IMAGE1D, "i64image1D"},
   {yytokentype::U64IMAGE1D, "u64image1D"},
   {yytokentype::I64IMAGE2D, "i64image2D"},
   {yytokentype::U64IMAGE2D, "u64image2D"},
   {yytokentype::I64IMAGE3D, "i64image3D"},
   {yytokentype::U64IMAGE3D, "u64image3D"},
   {yytokentype::I64IMAGE2DRECT, "i64image2Drect"},
   {yytokentype::U64IMAGE2DRECT, "u64image2Drect"},
   {yytokentype::I64IMAGECUBE, "i64imagecube"},
   {yytokentype::U64IMAGECUBE, "u64imagecube"},
   {yytokentype::I64IMAGEBUFFER, "i64imagebuffer"},
   {yytokentype::U64IMAGEBUFFER, "u64imagebuffer"},
   {yytokentype::I64IMAGE1DARRAY, "i64image1Darray"},
   {yytokentype::U64IMAGE1DARRAY, "u64image1Darray"},
   {yytokentype::I64IMAGE2DARRAY, "i64image2Darray"},
   {yytokentype::U64IMAGE2DARRAY, "u64image2Darray"},
   {yytokentype::I64IMAGECUBEARRAY, "i64imagecubearray"},
   {yytokentype::U64IMAGECUBEARRAY, "u64imagecubearray"},
   {yytokentype::I64IMAGE2DMS, "i64image2Dms"},
   {yytokentype::U64IMAGE2DMS, "u64image2Dms"},
   {yytokentype::I64IMAGE2DMSARRAY, "i64image2Dmsarray"},
   {yytokentype::U64IMAGE2DMSARRAY, "u64image2Dmsarray"},
   {yytokentype::TEXTURECUBEARRAY, "texturecubearray"},
   {yytokentype::ITEXTURECUBEARRAY, "itexturecubearray"},
   {yytokentype::UTEXTURECUBEARRAY, "utexturecubearray"},
   {yytokentype::TEXTURE1D, "texture1D"},
   {yytokentype::ITEXTURE1D, "itexture1D"},
   {yytokentype::UTEXTURE1D, "utexture1D"},
   {yytokentype::TEXTURE1DARRAY, "texture1Darray"},
   {yytokentype::ITEXTURE1DARRAY, "itexture1Darray"},
   {yytokentype::UTEXTURE1DARRAY, "utexture1Darray"},
   {yytokentype::TEXTURE2DRECT, "texture2Drect"},
   {yytokentype::ITEXTURE2DRECT, "itexture2Drect"},
   {yytokentype::UTEXTURE2DRECT, "utexture2Drect"},
   {yytokentype::TEXTUREBUFFER, "texturebuffer"},
   {yytokentype::ITEXTUREBUFFER, "itexturebuffer"},
   {yytokentype::UTEXTUREBUFFER, "utexturebuffer"},
   {yytokentype::TEXTURE2DMS, "texture2Dms"},
   {yytokentype::ITEXTURE2DMS, "itexture2Dms"},
   {yytokentype::UTEXTURE2DMS, "utexture2Dms"},
   {yytokentype::TEXTURE2DMSARRAY, "texture2Dmsarray"},
   {yytokentype::ITEXTURE2DMSARRAY, "itexture2Dmsarray"},
   {yytokentype::UTEXTURE2DMSARRAY, "utexture2Dmsarray"},
   {yytokentype::F16TEXTURE1D, "f16texture1D"},
   {yytokentype::F16TEXTURE2D, "f16texture2D"},
   {yytokentype::F16TEXTURE3D, "f16texture3D"},
   {yytokentype::F16TEXTURE2DRECT, "f16texture2Drect"},
   {yytokentype::F16TEXTURECUBE, "f16texturecube"},
   {yytokentype::F16TEXTURE1DARRAY, "f16texture1Darray"},
   {yytokentype::F16TEXTURE2DARRAY, "f16texture2Darray"},
   {yytokentype::F16TEXTURECUBEARRAY, "f16texturecubearray"},
   {yytokentype::F16TEXTUREBUFFER, "f16texturebuffer"},
   {yytokentype::F16TEXTURE2DMS, "f16texture2Dms"},
   {yytokentype::F16TEXTURE2DMSARRAY, "f16texture2Dmsarray"},
   {yytokentype::SUBPASSINPUT, "subpassinput"},
   {yytokentype::SUBPASSINPUTMS, "subpassinputms"},
   {yytokentype::ISUBPASSINPUT, "isubpassinput"},
   {yytokentype::ISUBPASSINPUTMS, "isubpassinputms"},
   {yytokentype::USUBPASSINPUT, "usubpassinput"},
   {yytokentype::USUBPASSINPUTMS, "usubpassinputms"},
   {yytokentype::F16SUBPASSINPUT, "f16subpassinput"},
   {yytokentype::F16SUBPASSINPUTMS, "f16subpassinputms"},
   {yytokentype::SPIRV_INSTRUCTION, "spirv_instruction"},
   {yytokentype::SPIRV_EXECUTION_MODE, "spirv_execution_mode"},
   {yytokentype::SPIRV_EXECUTION_MODE_ID, "spirv_execution_mode_id"},
   {yytokentype::SPIRV_DECORATE, "spirv_decorate"},
   {yytokentype::SPIRV_DECORATE_ID, "spirv_decorate_id"},
   {yytokentype::SPIRV_DECORATE_STRING, "spirv_decorate_string"},
   {yytokentype::SPIRV_TYPE, "spirv_class"},
   {yytokentype::SPIRV_STORAGE_CLASS, "spirv_storage_class"},
   {yytokentype::SPIRV_BY_REFERENCE, "spirv_by_reference"},
   {yytokentype::SPIRV_LITERAL, "spirv_literal"},
   {yytokentype::LEFT_OP, "<<"},
   {yytokentype::RIGHT_OP, ">>"},
   {yytokentype::INC_OP, "++"},
   {yytokentype::DEC_OP, "--"},
   {yytokentype::LE_OP, "<="},
   {yytokentype::GE_OP, ">="},
   {yytokentype::EQ_OP, "=="},
   {yytokentype::NE_OP, "!="},
   {yytokentype::AND_OP, "&&"},
   {yytokentype::OR_OP, "||"},
   {yytokentype::XOR_OP, "^"},
   {yytokentype::MUL_ASSIGN, "*="},
   {yytokentype::DIV_ASSIGN, "/="},
   {yytokentype::ADD_ASSIGN, "+="},
   {yytokentype::MOD_ASSIGN, "%="},
   {yytokentype::LEFT_ASSIGN, "<<="},
   {yytokentype::RIGHT_ASSIGN, ">>="},
   {yytokentype::AND_ASSIGN, "&="},
   {yytokentype::XOR_ASSIGN, "^="},
   {yytokentype::OR_ASSIGN, "|="},
   {yytokentype::SUB_ASSIGN, "-="},
   //{yytokentype::STRING_LITERAL, "string_literal"},
   {yytokentype::LEFT_PAREN, "("},
   {yytokentype::RIGHT_PAREN, ")"},
   {yytokentype::LEFT_BRACKET, "["},
   {yytokentype::RIGHT_BRACKET, "]"},
   {yytokentype::LEFT_BRACE, "{"},
   {yytokentype::RIGHT_BRACE, "}"},
   {yytokentype::DOT, "."},
   {yytokentype::COMMA, ","},
   {yytokentype::COLON, ":"},
   {yytokentype::EQUAL, "="},
   {yytokentype::SEMICOLON, ";"},
   {yytokentype::BANG, "!"},
   {yytokentype::DASH, "-"},
   {yytokentype::TILDE, "~"},
   {yytokentype::PLUS, "+"},
   {yytokentype::STAR, "*"},
   {yytokentype::SLASH, "/"},
   {yytokentype::PERCENT, "%"},
   {yytokentype::LEFT_ANGLE, "<"},
   {yytokentype::RIGHT_ANGLE, ">"},
   {yytokentype::VERTICAL_BAR, "|"},
   {yytokentype::CARET, "^"},
   {yytokentype::AMPERSAND, "&"},
   {yytokentype::QUESTION, "?"},
   {yytokentype::INVARIANT, "invariant"},
   {yytokentype::HIGH_PRECISION, "highp"},
   {yytokentype::MEDIUM_PRECISION, "mediump"},
   {yytokentype::LOW_PRECISION, "lowp"},
   {yytokentype::PRECISION, "precision"},
   {yytokentype::PACKED, "packed"},
   {yytokentype::RESOURCE, "resource"},
   {yytokentype::SUPERP, "superp"},
   //{yytokentype::FLOATCONSTANT, "floatconstant"},
   //{yytokentype::INTCONSTANT, "intconstant"},
   //{yytokentype::UINTCONSTANT, "uintconstant"},
   //{yytokentype::BOOLCONSTANT, "boolconstant"},
   //{yytokentype::IDENTIFIER, "identifier"},
   {yytokentype::TYPE_NAME, "type_name"},
   {yytokentype::CENTROID, "centroid"},
   {yytokentype::IN_, "in"},
   {yytokentype::OUT_, "out"},
   {yytokentype::INOUT, "inout"},
   {yytokentype::STRUCT, "struct"},
   {yytokentype::VOID_, "void"},
   {yytokentype::WHILE, "while"},
   {yytokentype::BREAK, "break"},
   {yytokentype::CONTINUE, "continue"},
   {yytokentype::DO, "do"},
   {yytokentype::ELSE, "else"},
   {yytokentype::FOR, "for"},
   {yytokentype::IF, "if"},
   {yytokentype::DISCARD, "discard"},
   {yytokentype::RETURN, "return"},
   {yytokentype::SWITCH, "switch"},
   {yytokentype::CASE, "case"},
   {yytokentype::DEFAULT, "default"},
   {yytokentype::TERMINATE_INVOCATION, "terminate_invocation"},
   {yytokentype::TERMINATE_RAY, "terminate_ray"},
   {yytokentype::IGNORE_INTERSECTION, "ignore_intersection"},
   {yytokentype::UNIFORM, "uniform"},
   {yytokentype::SHARED, "shared"},
   {yytokentype::BUFFER, "buffer"},
   {yytokentype::FLAT, "flat"},
   {yytokentype::SMOOTH, "smooth"},
   {yytokentype::LAYOUT, "layout"},
   {yytokentype::DOUBLECONSTANT, "doubleconstant"},
   {yytokentype::INT16CONSTANT, "int16constant"},
   {yytokentype::UINT16CONSTANT, "uint16constant"},
   {yytokentype::FLOAT16CONSTANT, "float16constant"},
   {yytokentype::INT32CONSTANT, "int32constant"},
   {yytokentype::UINT32CONSTANT, "uint32constant"},
   {yytokentype::INT64CONSTANT, "int64constant"},
   {yytokentype::UINT64CONSTANT, "uint64constant"},
   {yytokentype::SUBROUTINE, "subroutine"},
   {yytokentype::DEMOTE, "demote"},
   {yytokentype::PAYLOADNV, "payloadnv"},
   {yytokentype::PAYLOADINNV, "payloadinnv"},
   {yytokentype::HITATTRNV, "hitattrnv"},
   {yytokentype::CALLDATANV, "calldatanv"},
   {yytokentype::CALLDATAINNV, "calldatainnv"},
   {yytokentype::PAYLOADEXT, "payloadext"},
   {yytokentype::PAYLOADINEXT, "payloadinext"},
   {yytokentype::HITATTREXT, "hitattrext"},
   {yytokentype::CALLDATAEXT, "calldataext"},
   {yytokentype::CALLDATAINEXT, "calldatainext"},
   {yytokentype::PATCH, "patch"},
   {yytokentype::SAMPLE, "sample"},
   {yytokentype::NONUNIFORM, "nonuniform"},
   {yytokentype::COHERENT, "coherent"},
   {yytokentype::VOLATILE, "volatile"},
   {yytokentype::RESTRICT, "restrict"},
   {yytokentype::READONLY, "readonly"},
   {yytokentype::WRITEONLY, "writeonly"},
   {yytokentype::DEVICECOHERENT, "devicecoherent"},
   {yytokentype::QUEUEFAMILYCOHERENT, "queuefamilycoherent"},
   {yytokentype::WORKGROUPCOHERENT, "workgroupcoherent"},
   {yytokentype::SUBGROUPCOHERENT, "subgroupcoherent"},
   {yytokentype::NONPRIVATE, "nonprivate"},
   {yytokentype::SHADERCALLCOHERENT, "shadercallcoherent"},
   {yytokentype::NOPERSPECTIVE, "noperspective"},
   {yytokentype::EXPLICITINTERPAMD, "explicitinterpamd"},
   {yytokentype::PERVERTEXNV, "pervertexnv"},
   {yytokentype::PERPRIMITIVENV, "perprimitivenv"},
   {yytokentype::PERVIEWNV, "perviewnv"},
   {yytokentype::PERTASKNV, "pertasknv"},
   {yytokentype::PRECISE, "precise"},
        };

        static const flat_hash_map<std::string, yytokentype> glsl_token_string_to_type =
        {
{"yyempty", yytokentype::YYEMPTY},
{"yyeof", yytokentype::YYEOF},
{"yyerror", yytokentype::YYerror},
{"yyundef", yytokentype::YYUNDEF},
{"const", yytokentype::CONST_},
{"bool", yytokentype::BOOL_},
{"int", yytokentype::INT_},
{"uint", yytokentype::UINT_},
{"float", yytokentype::FLOAT_},
{"bvec2", yytokentype::BVEC2},
{"bvec3", yytokentype::BVEC3},
{"bvec4", yytokentype::BVEC4},
{"ivec2", yytokentype::IVEC2},
{"ivec3", yytokentype::IVEC3},
{"ivec4", yytokentype::IVEC4},
{"uvec2", yytokentype::UVEC2},
{"uvec3", yytokentype::UVEC3},
{"uvec4", yytokentype::UVEC4},
{"vec2", yytokentype::VEC2},
{"vec3", yytokentype::VEC3},
{"vec4", yytokentype::VEC4},
{"mat2", yytokentype::MAT2_},
{"mat3", yytokentype::MAT3_},
{"mat4", yytokentype::MAT4_},
{"mat2x2", yytokentype::MAT2X2},
{"mat2x3", yytokentype::MAT2X3},
{"mat2x4", yytokentype::MAT2X4},
{"mat3x2", yytokentype::MAT3X2},
{"mat3x3", yytokentype::MAT3X3},
{"mat3x4", yytokentype::MAT3X4},
{"mat4x2", yytokentype::MAT4X2},
{"mat4x3", yytokentype::MAT4X3},
{"mat4x4", yytokentype::MAT4X4},
{"sampler2D", yytokentype::SAMPLER2D},
{"sampler3D", yytokentype::SAMPLER3D},
{"samplercube", yytokentype::SAMPLERCUBE},
{"sampler2Dshadow", yytokentype::SAMPLER2DSHADOW},
{"samplercubeshadow", yytokentype::SAMPLERCUBESHADOW},
{"sampler2Darray", yytokentype::SAMPLER2DARRAY},
{"sampler2Darrayshadow", yytokentype::SAMPLER2DARRAYSHADOW},
{"isampler2D", yytokentype::ISAMPLER2D},
{"isampler3D", yytokentype::ISAMPLER3D},
{"isamplercube", yytokentype::ISAMPLERCUBE},
{"isampler2Darray", yytokentype::ISAMPLER2DARRAY},
{"usampler2D", yytokentype::USAMPLER2D},
{"usampler3D", yytokentype::USAMPLER3D},
{"usamplercube", yytokentype::USAMPLERCUBE},
{"usampler2Darray", yytokentype::USAMPLER2DARRAY},
{"sampler", yytokentype::SAMPLER},
{"samplershadow", yytokentype::SAMPLERSHADOW},
{"texture2D", yytokentype::TEXTURE2D},
{"texture3D", yytokentype::TEXTURE3D},
{"texturecube", yytokentype::TEXTURECUBE},
{"texture2Darray", yytokentype::TEXTURE2DARRAY},
{"itexture2D", yytokentype::ITEXTURE2D},
{"itexture3D", yytokentype::ITEXTURE3D},
{"itexturecube", yytokentype::ITEXTURECUBE},
{"itexture2Darray", yytokentype::ITEXTURE2DARRAY},
{"utexture2D", yytokentype::UTEXTURE2D},
{"utexture3D", yytokentype::UTEXTURE3D},
{"utexturecube", yytokentype::UTEXTURECUBE},
{"utexture2Darray", yytokentype::UTEXTURE2DARRAY},
{"attribute", yytokentype::ATTRIBUTE},
{"varying", yytokentype::VARYING},
{"float16_t", yytokentype::FLOAT16_T},
{"float32_t", yytokentype::FLOAT32_T},
{"double", yytokentype::DOUBLE},
{"float64_t", yytokentype::FLOAT64_T},
{"int64_t", yytokentype::INT64_T},
{"uint64_t", yytokentype::UINT64_T},
{"int32_t", yytokentype::INT32_T},
{"uint32_t", yytokentype::UINT32_T},
{"int16_t", yytokentype::INT16_T},
{"uint16_t", yytokentype::UINT16_T},
{"int8_t", yytokentype::INT8_T},
{"uint8_t", yytokentype::UINT8_T},
{"i64vec2", yytokentype::I64VEC2},
{"i64vec3", yytokentype::I64VEC3},
{"i64vec4", yytokentype::I64VEC4},
{"u64vec2", yytokentype::U64VEC2},
{"u64vec3", yytokentype::U64VEC3},
{"u64vec4", yytokentype::U64VEC4},
{"i32vec2", yytokentype::I32VEC2},
{"i32vec3", yytokentype::I32VEC3},
{"i32vec4", yytokentype::I32VEC4},
{"u32vec2", yytokentype::U32VEC2},
{"u32vec3", yytokentype::U32VEC3},
{"u32vec4", yytokentype::U32VEC4},
{"i16vec2", yytokentype::I16VEC2},
{"i16vec3", yytokentype::I16VEC3},
{"i16vec4", yytokentype::I16VEC4},
{"u16vec2", yytokentype::U16VEC2},
{"u16vec3", yytokentype::U16VEC3},
{"u16vec4", yytokentype::U16VEC4},
{"i8vec2", yytokentype::I8VEC2},
{"i8vec3", yytokentype::I8VEC3},
{"i8vec4", yytokentype::I8VEC4},
{"u8vec2", yytokentype::U8VEC2},
{"u8vec3", yytokentype::U8VEC3},
{"u8vec4", yytokentype::U8VEC4},
{"dvec2", yytokentype::DVEC2},
{"dvec3", yytokentype::DVEC3},
{"dvec4", yytokentype::DVEC4},
{"dmat2", yytokentype::DMAT2},
{"dmat3", yytokentype::DMAT3},
{"dmat4", yytokentype::DMAT4},
{"f16vec2", yytokentype::F16VEC2},
{"f16vec3", yytokentype::F16VEC3},
{"f16vec4", yytokentype::F16VEC4},
{"f16mat2", yytokentype::F16MAT2},
{"f16mat3", yytokentype::F16MAT3},
{"f16mat4", yytokentype::F16MAT4},
{"f32vec2", yytokentype::F32VEC2},
{"f32vec3", yytokentype::F32VEC3},
{"f32vec4", yytokentype::F32VEC4},
{"f32mat2", yytokentype::F32MAT2},
{"f32mat3", yytokentype::F32MAT3},
{"f32mat4", yytokentype::F32MAT4},
{"f64vec2", yytokentype::F64VEC2},
{"f64vec3", yytokentype::F64VEC3},
{"f64vec4", yytokentype::F64VEC4},
{"f64mat2", yytokentype::F64MAT2},
{"f64mat3", yytokentype::F64MAT3},
{"f64mat4", yytokentype::F64MAT4},
{"dmat2x2", yytokentype::DMAT2X2},
{"dmat2x3", yytokentype::DMAT2X3},
{"dmat2x4", yytokentype::DMAT2X4},
{"dmat3x2", yytokentype::DMAT3X2},
{"dmat3x3", yytokentype::DMAT3X3},
{"dmat3x4", yytokentype::DMAT3X4},
{"dmat4x2", yytokentype::DMAT4X2},
{"dmat4x3", yytokentype::DMAT4X3},
{"dmat4x4", yytokentype::DMAT4X4},
{"f16mat2x2", yytokentype::F16MAT2X2},
{"f16mat2x3", yytokentype::F16MAT2X3},
{"f16mat2x4", yytokentype::F16MAT2X4},
{"f16mat3x2", yytokentype::F16MAT3X2},
{"f16mat3x3", yytokentype::F16MAT3X3},
{"f16mat3x4", yytokentype::F16MAT3X4},
{"f16mat4x2", yytokentype::F16MAT4X2},
{"f16mat4x3", yytokentype::F16MAT4X3},
{"f16mat4x4", yytokentype::F16MAT4X4},
{"f32mat2x2", yytokentype::F32MAT2X2},
{"f32mat2x3", yytokentype::F32MAT2X3},
{"f32mat2x4", yytokentype::F32MAT2X4},
{"f32mat3x2", yytokentype::F32MAT3X2},
{"f32mat3x3", yytokentype::F32MAT3X3},
{"f32mat3x4", yytokentype::F32MAT3X4},
{"f32mat4x2", yytokentype::F32MAT4X2},
{"f32mat4x3", yytokentype::F32MAT4X3},
{"f32mat4x4", yytokentype::F32MAT4X4},
{"f64mat2x2", yytokentype::F64MAT2X2},
{"f64mat2x3", yytokentype::F64MAT2X3},
{"f64mat2x4", yytokentype::F64MAT2X4},
{"f64mat3x2", yytokentype::F64MAT3X2},
{"f64mat3x3", yytokentype::F64MAT3X3},
{"f64mat3x4", yytokentype::F64MAT3X4},
{"f64mat4x2", yytokentype::F64MAT4X2},
{"f64mat4x3", yytokentype::F64MAT4X3},
{"f64mat4x4", yytokentype::F64MAT4X4},
{"atomic_uint", yytokentype::ATOMIC_UINT},
{"accstructnv", yytokentype::ACCSTRUCTNV},
{"accstructext", yytokentype::ACCSTRUCTEXT},
{"rayqueryext", yytokentype::RAYQUERYEXT},
{"fcoopmatnv", yytokentype::FCOOPMATNV},
{"icoopmatnv", yytokentype::ICOOPMATNV},
{"ucoopmatnv", yytokentype::UCOOPMATNV},
{"samplercubearray", yytokentype::SAMPLERCUBEARRAY},
{"samplercubearrayshadow", yytokentype::SAMPLERCUBEARRAYSHADOW},
{"isamplercubearray", yytokentype::ISAMPLERCUBEARRAY},
{"usamplercubearray", yytokentype::USAMPLERCUBEARRAY},
{"sampler1D", yytokentype::SAMPLER1D},
{"sampler1Darray", yytokentype::SAMPLER1DARRAY},
{"sampler1Darrayshadow", yytokentype::SAMPLER1DARRAYSHADOW},
{"isampler1D", yytokentype::ISAMPLER1D},
{"sampler1Dshadow", yytokentype::SAMPLER1DSHADOW},
{"sampler2Drect", yytokentype::SAMPLER2DRECT},
{"sampler2Drectshadow", yytokentype::SAMPLER2DRECTSHADOW},
{"isampler2Drect", yytokentype::ISAMPLER2DRECT},
{"usampler2Drect", yytokentype::USAMPLER2DRECT},
{"samplerbuffer", yytokentype::SAMPLERBUFFER},
{"isamplerbuffer", yytokentype::ISAMPLERBUFFER},
{"usamplerbuffer", yytokentype::USAMPLERBUFFER},
{"sampler2Dms", yytokentype::SAMPLER2DMS},
{"isampler2Dms", yytokentype::ISAMPLER2DMS},
{"usampler2Dms", yytokentype::USAMPLER2DMS},
{"sampler2Dmsarray", yytokentype::SAMPLER2DMSARRAY},
{"isampler2Dmsarray", yytokentype::ISAMPLER2DMSARRAY},
{"usampler2Dmsarray", yytokentype::USAMPLER2DMSARRAY},
{"samplerexternaloes", yytokentype::SAMPLEREXTERNALOES},
{"samplerexternal2Dy2yext", yytokentype::SAMPLEREXTERNAL2DY2YEXT},
{"isampler1Darray", yytokentype::ISAMPLER1DARRAY},
{"usampler1D", yytokentype::USAMPLER1D},
{"usampler1Darray", yytokentype::USAMPLER1DARRAY},
{"f16sampler1D", yytokentype::F16SAMPLER1D},
{"f16sampler2D", yytokentype::F16SAMPLER2D},
{"f16sampler3D", yytokentype::F16SAMPLER3D},
{"f16sampler2Drect", yytokentype::F16SAMPLER2DRECT},
{"f16samplercube", yytokentype::F16SAMPLERCUBE},
{"f16sampler1Darray", yytokentype::F16SAMPLER1DARRAY},
{"f16sampler2Darray", yytokentype::F16SAMPLER2DARRAY},
{"f16samplercubearray", yytokentype::F16SAMPLERCUBEARRAY},
{"f16samplerbuffer", yytokentype::F16SAMPLERBUFFER},
{"f16sampler2Dms", yytokentype::F16SAMPLER2DMS},
{"f16sampler2Dmsarray", yytokentype::F16SAMPLER2DMSARRAY},
{"f16sampler1Dshadow", yytokentype::F16SAMPLER1DSHADOW},
{"f16sampler2Dshadow", yytokentype::F16SAMPLER2DSHADOW},
{"f16sampler1Darrayshadow", yytokentype::F16SAMPLER1DARRAYSHADOW},
{"f16sampler2Darrayshadow", yytokentype::F16SAMPLER2DARRAYSHADOW},
{"f16sampler2Drectshadow", yytokentype::F16SAMPLER2DRECTSHADOW},
{"f16samplercubeshadow", yytokentype::F16SAMPLERCUBESHADOW},
{"f16samplercubearrayshadow", yytokentype::F16SAMPLERCUBEARRAYSHADOW},
{"image1D", yytokentype::IMAGE1D},
{"iimage1D", yytokentype::IIMAGE1D},
{"uimage1D", yytokentype::UIMAGE1D},
{"image2D", yytokentype::IMAGE2D},
{"iimage2D", yytokentype::IIMAGE2D},
{"uimage2D", yytokentype::UIMAGE2D},
{"image3D", yytokentype::IMAGE3D},
{"iimage3D", yytokentype::IIMAGE3D},
{"uimage3D", yytokentype::UIMAGE3D},
{"image2Drect", yytokentype::IMAGE2DRECT},
{"iimage2Drect", yytokentype::IIMAGE2DRECT},
{"uimage2Drect", yytokentype::UIMAGE2DRECT},
{"imagecube", yytokentype::IMAGECUBE},
{"iimagecube", yytokentype::IIMAGECUBE},
{"uimagecube", yytokentype::UIMAGECUBE},
{"imagebuffer", yytokentype::IMAGEBUFFER},
{"iimagebuffer", yytokentype::IIMAGEBUFFER},
{"uimagebuffer", yytokentype::UIMAGEBUFFER},
{"image1Darray", yytokentype::IMAGE1DARRAY},
{"iimage1Darray", yytokentype::IIMAGE1DARRAY},
{"uimage1Darray", yytokentype::UIMAGE1DARRAY},
{"image2Darray", yytokentype::IMAGE2DARRAY},
{"iimage2Darray", yytokentype::IIMAGE2DARRAY},
{"uimage2Darray", yytokentype::UIMAGE2DARRAY},
{"imagecubearray", yytokentype::IMAGECUBEARRAY},
{"iimagecubearray", yytokentype::IIMAGECUBEARRAY},
{"uimagecubearray", yytokentype::UIMAGECUBEARRAY},
{"image2Dms", yytokentype::IMAGE2DMS},
{"iimage2Dms", yytokentype::IIMAGE2DMS},
{"uimage2Dms", yytokentype::UIMAGE2DMS},
{"image2Dmsarray", yytokentype::IMAGE2DMSARRAY},
{"iimage2Dmsarray", yytokentype::IIMAGE2DMSARRAY},
{"uimage2Dmsarray", yytokentype::UIMAGE2DMSARRAY},
{"f16image1D", yytokentype::F16IMAGE1D},
{"f16image2D", yytokentype::F16IMAGE2D},
{"f16image3D", yytokentype::F16IMAGE3D},
{"f16image2Drect", yytokentype::F16IMAGE2DRECT},
{"f16imagecube", yytokentype::F16IMAGECUBE},
{"f16image1Darray", yytokentype::F16IMAGE1DARRAY},
{"f16image2Darray", yytokentype::F16IMAGE2DARRAY},
{"f16imagecubearray", yytokentype::F16IMAGECUBEARRAY},
{"f16imagebuffer", yytokentype::F16IMAGEBUFFER},
{"f16image2Dms", yytokentype::F16IMAGE2DMS},
{"f16image2Dmsarray", yytokentype::F16IMAGE2DMSARRAY},
{"i64image1D", yytokentype::I64IMAGE1D},
{"u64image1D", yytokentype::U64IMAGE1D},
{"i64image2D", yytokentype::I64IMAGE2D},
{"u64image2D", yytokentype::U64IMAGE2D},
{"i64image3D", yytokentype::I64IMAGE3D},
{"u64image3D", yytokentype::U64IMAGE3D},
{"i64image2Drect", yytokentype::I64IMAGE2DRECT},
{"u64image2Drect", yytokentype::U64IMAGE2DRECT},
{"i64imagecube", yytokentype::I64IMAGECUBE},
{"u64imagecube", yytokentype::U64IMAGECUBE},
{"i64imagebuffer", yytokentype::I64IMAGEBUFFER},
{"u64imagebuffer", yytokentype::U64IMAGEBUFFER},
{"i64image1Darray", yytokentype::I64IMAGE1DARRAY},
{"u64image1Darray", yytokentype::U64IMAGE1DARRAY},
{"i64image2Darray", yytokentype::I64IMAGE2DARRAY},
{"u64image2Darray", yytokentype::U64IMAGE2DARRAY},
{"i64imagecubearray", yytokentype::I64IMAGECUBEARRAY},
{"u64imagecubearray", yytokentype::U64IMAGECUBEARRAY},
{"i64image2Dms", yytokentype::I64IMAGE2DMS},
{"u64image2Dms", yytokentype::U64IMAGE2DMS},
{"i64image2Dmsarray", yytokentype::I64IMAGE2DMSARRAY},
{"u64image2Dmsarray", yytokentype::U64IMAGE2DMSARRAY},
{"texturecubearray", yytokentype::TEXTURECUBEARRAY},
{"itexturecubearray", yytokentype::ITEXTURECUBEARRAY},
{"utexturecubearray", yytokentype::UTEXTURECUBEARRAY},
{"texture1D", yytokentype::TEXTURE1D},
{"itexture1D", yytokentype::ITEXTURE1D},
{"utexture1D", yytokentype::UTEXTURE1D},
{"texture1Darray", yytokentype::TEXTURE1DARRAY},
{"itexture1Darray", yytokentype::ITEXTURE1DARRAY},
{"utexture1Darray", yytokentype::UTEXTURE1DARRAY},
{"texture2Drect", yytokentype::TEXTURE2DRECT},
{"itexture2Drect", yytokentype::ITEXTURE2DRECT},
{"utexture2Drect", yytokentype::UTEXTURE2DRECT},
{"texturebuffer", yytokentype::TEXTUREBUFFER},
{"itexturebuffer", yytokentype::ITEXTUREBUFFER},
{"utexturebuffer", yytokentype::UTEXTUREBUFFER},
{"texture2Dms", yytokentype::TEXTURE2DMS},
{"itexture2Dms", yytokentype::ITEXTURE2DMS},
{"utexture2Dms", yytokentype::UTEXTURE2DMS},
{"texture2Dmsarray", yytokentype::TEXTURE2DMSARRAY},
{"itexture2Dmsarray", yytokentype::ITEXTURE2DMSARRAY},
{"utexture2Dmsarray", yytokentype::UTEXTURE2DMSARRAY},
{"f16texture1D", yytokentype::F16TEXTURE1D},
{"f16texture2D", yytokentype::F16TEXTURE2D},
{"f16texture3D", yytokentype::F16TEXTURE3D},
{"f16texture2Drect", yytokentype::F16TEXTURE2DRECT},
{"f16texturecube", yytokentype::F16TEXTURECUBE},
{"f16texture1Darray", yytokentype::F16TEXTURE1DARRAY},
{"f16texture2Darray", yytokentype::F16TEXTURE2DARRAY},
{"f16texturecubearray", yytokentype::F16TEXTURECUBEARRAY},
{"f16texturebuffer", yytokentype::F16TEXTUREBUFFER},
{"f16texture2Dms", yytokentype::F16TEXTURE2DMS},
{"f16texture2Dmsarray", yytokentype::F16TEXTURE2DMSARRAY},
{"subpassinput", yytokentype::SUBPASSINPUT},
{"subpassinputms", yytokentype::SUBPASSINPUTMS},
{"isubpassinput", yytokentype::ISUBPASSINPUT},
{"isubpassinputms", yytokentype::ISUBPASSINPUTMS},
{"usubpassinput", yytokentype::USUBPASSINPUT},
{"usubpassinputms", yytokentype::USUBPASSINPUTMS},
{"f16subpassinput", yytokentype::F16SUBPASSINPUT},
{"f16subpassinputms", yytokentype::F16SUBPASSINPUTMS},
{ "spirv_instruction", yytokentype::SPIRV_INSTRUCTION },
{ "spirv_execution_mode", yytokentype::SPIRV_EXECUTION_MODE },
{ "spirv_execution_mode_id", yytokentype::SPIRV_EXECUTION_MODE_ID },
{ "spirv_decorate", yytokentype::SPIRV_DECORATE },
{ "spirv_decorate_id", yytokentype::SPIRV_DECORATE_ID },
{ "spirv_decorate_string", yytokentype::SPIRV_DECORATE_STRING },
{ "spirv_class", yytokentype::SPIRV_TYPE },
{ "spirv_storage_class", yytokentype::SPIRV_STORAGE_CLASS },
{ "spirv_by_reference", yytokentype::SPIRV_BY_REFERENCE },
{ "spirv_literal", yytokentype::SPIRV_LITERAL },
{"<<", yytokentype::LEFT_OP},
{">>", yytokentype::RIGHT_OP},
{"++", yytokentype::INC_OP},
{"--", yytokentype::DEC_OP},
{"<=", yytokentype::LE_OP},
{">=", yytokentype::GE_OP},
{"==", yytokentype::EQ_OP},
{"!=", yytokentype::NE_OP},
{"&&", yytokentype::AND_OP},
{"||", yytokentype::OR_OP},
{"^", yytokentype::XOR_OP},
{"*=", yytokentype::MUL_ASSIGN},
{"/=", yytokentype::DIV_ASSIGN},
{"+=", yytokentype::ADD_ASSIGN},
{"%=", yytokentype::MOD_ASSIGN},
{"=", yytokentype::LEFT_ASSIGN},
{"=", yytokentype::RIGHT_ASSIGN},
{"&=", yytokentype::AND_ASSIGN},
{"^=", yytokentype::XOR_ASSIGN},
{"|=", yytokentype::OR_ASSIGN},
{"-=", yytokentype::SUB_ASSIGN},
//{"string_literal", yytokentype::STRING_LITERAL},
{"(", yytokentype::LEFT_PAREN},
{")", yytokentype::RIGHT_PAREN},
{"[", yytokentype::LEFT_BRACKET},
{"]", yytokentype::RIGHT_BRACKET},
{"{", yytokentype::LEFT_BRACE},
{"}", yytokentype::RIGHT_BRACE},
{".", yytokentype::DOT},
{",", yytokentype::COMMA},
{":", yytokentype::COLON},
{"=", yytokentype::EQUAL},
{";", yytokentype::SEMICOLON},
{"!", yytokentype::BANG},
{"-", yytokentype::DASH},
{"~", yytokentype::TILDE},
{"+", yytokentype::PLUS},
{"*", yytokentype::STAR},
{"/", yytokentype::SLASH},
{"%", yytokentype::PERCENT},
{"<", yytokentype::LEFT_ANGLE},
{">", yytokentype::RIGHT_ANGLE},
{"|", yytokentype::VERTICAL_BAR},
{"^", yytokentype::CARET},
{"&", yytokentype::AMPERSAND},
{"?", yytokentype::QUESTION},
{"invariant", yytokentype::INVARIANT},
{"high_precision", yytokentype::HIGH_PRECISION},
{"medium_precision", yytokentype::MEDIUM_PRECISION},
{"low_precision", yytokentype::LOW_PRECISION},
{"precision", yytokentype::PRECISION},
{"packed", yytokentype::PACKED},
{"resource", yytokentype::RESOURCE},
{"superp", yytokentype::SUPERP},
//{"floatconstant", yytokentype::FLOATCONSTANT},
//{"intconstant", yytokentype::INTCONSTANT},
//{"uintconstant", yytokentype::UINTCONSTANT},
//{"boolconstant", yytokentype::BOOLCONSTANT},
//{"identifier", yytokentype::IDENTIFIER},
{"type_name", yytokentype::TYPE_NAME},
{"centroid", yytokentype::CENTROID},
{"in", yytokentype::IN_},
{"out", yytokentype::OUT_},
{"inout", yytokentype::INOUT},
{"struct", yytokentype::STRUCT},
{"void", yytokentype::VOID_},
{"while", yytokentype::WHILE},
{"break", yytokentype::BREAK},
{"continue", yytokentype::CONTINUE},
{"do", yytokentype::DO},
{"else", yytokentype::ELSE},
{"for", yytokentype::FOR},
{"if", yytokentype::IF},
{"discard", yytokentype::DISCARD},
{"return", yytokentype::RETURN},
{"switch", yytokentype::SWITCH},
{"case", yytokentype::CASE},
{"default", yytokentype::DEFAULT},
{"terminate_invocation", yytokentype::TERMINATE_INVOCATION},
{"terminate_ray", yytokentype::TERMINATE_RAY},
{"ignore_intersection", yytokentype::IGNORE_INTERSECTION},
{"uniform", yytokentype::UNIFORM},
{"shared", yytokentype::SHARED},
{"buffer", yytokentype::BUFFER},
{"flat", yytokentype::FLAT},
{"smooth", yytokentype::SMOOTH},
{"layout", yytokentype::LAYOUT},
{"doubleconstant", yytokentype::DOUBLECONSTANT},
{"int16constant", yytokentype::INT16CONSTANT},
{"uint16constant", yytokentype::UINT16CONSTANT},
{"float16constant", yytokentype::FLOAT16CONSTANT},
{"int32constant", yytokentype::INT32CONSTANT},
{"uint32constant", yytokentype::UINT32CONSTANT},
{"int64constant", yytokentype::INT64CONSTANT},
{"uint64constant", yytokentype::UINT64CONSTANT},
{"subroutine", yytokentype::SUBROUTINE},
{"demote", yytokentype::DEMOTE},
{"payloadnv", yytokentype::PAYLOADNV},
{"payloadinnv", yytokentype::PAYLOADINNV},
{"hitattrnv", yytokentype::HITATTRNV},
{"calldatanv", yytokentype::CALLDATANV},
{"calldatainnv", yytokentype::CALLDATAINNV},
{"payloadext", yytokentype::PAYLOADEXT},
{"payloadinext", yytokentype::PAYLOADINEXT},
{"hitattrext", yytokentype::HITATTREXT},
{"calldataext", yytokentype::CALLDATAEXT},
{"calldatainext", yytokentype::CALLDATAINEXT},
{"patch", yytokentype::PATCH},
{"sample", yytokentype::SAMPLE},
{"nonuniform", yytokentype::NONUNIFORM},
{"coherent", yytokentype::COHERENT},
{"volatile", yytokentype::VOLATILE},
{"restrict", yytokentype::RESTRICT},
{"readonly", yytokentype::READONLY},
{"writeonly", yytokentype::WRITEONLY},
{"devicecoherent", yytokentype::DEVICECOHERENT},
{"queuefamilycoherent", yytokentype::QUEUEFAMILYCOHERENT},
{"workgroupcoherent", yytokentype::WORKGROUPCOHERENT},
{"subgroupcoherent", yytokentype::SUBGROUPCOHERENT},
{"nonprivate", yytokentype::NONPRIVATE},
{"shadercallcoherent", yytokentype::SHADERCALLCOHERENT},
{"noperspective", yytokentype::NOPERSPECTIVE},
{"explicitinterpamd", yytokentype::EXPLICITINTERPAMD},
{"pervertexnv", yytokentype::PERVERTEXNV},
{"perprimitivenv", yytokentype::PERPRIMITIVENV},
{"perviewnv", yytokentype::PERVIEWNV},
{"pertasknv", yytokentype::PERTASKNV},
{"precise", yytokentype::PRECISE},
        };

        Error GLSLTokenizer::Init(std::string_view source, EShLanguage shader_type_)
        {
            tokens.clear();
            shader_type = shader_type_;

            auto glslang_pool_allocator = std::make_unique<glslang::TPoolAllocator>();
            glslang::SetThreadPoolAllocator(glslang_pool_allocator.get());

            EShLanguage language = shader_type;
            TInfoSink infoSink;
            glslang::SpvVersion spvVersion;
            spvVersion.openGl = 110;
            bool forwardCompatible = true;
            EShMessages messages = EShMsgDefault;
            bool parsingBuiltIns = true;

            auto symbol_table = std::make_unique<glslang::TSymbolTable>();
            glslang::TIntermediate intermediate(language, glsl_version, glsl_profile);

            // Create Parse Context
            auto parseContext = glslang::TParseContext
            (
                *symbol_table,
                intermediate,
                parsingBuiltIns,
                glsl_version,
                glsl_profile,
                spvVersion,
                shader_type,
                infoSink,
                forwardCompatible,
                messages
            );

            glslang::TShader::ForbidIncluder fincluder;
            glslang::TPpContext ppContext(parseContext, "", fincluder);
            glslang::TScanContext scanContext(parseContext);
            parseContext.setScanContext(&scanContext);
            parseContext.setPpContext(&ppContext);
            parseContext.initializeExtensionBehavior();

            const char* builtInShaders[2] = { 0 };
            size_t builtInLengths[2] = { 0 };
            builtInShaders[0] = source.data();
            builtInLengths[0] = source.length();
            glslang::TInputScanner input(1, builtInShaders, builtInLengths);

            symbol_table->push();

            ppContext.setInput(input, false);
            parseContext.setScanner(&input);

            do
            {
                YYSTYPE yType{};
                ::TParserToken parserToken{ yType };
                auto token = static_cast<yytokentype>(scanContext.tokenize(&ppContext, (glslang::TParserToken&)(parserToken)));

                if (token == yytokentype::YYEOF)
                {
                    break;
                }

                auto line = yType.lex.loc.line;
                auto s = std::string{};
                if (auto glsl_string = glsl_token_type_to_string.find(token); glsl_string != std::end(glsl_token_type_to_string))
                {
                    s = glsl_string->second;
                }
                else
                {
                    switch (token)
                    {
                    case FLOATCONSTANT:
                    {
                        s = std::to_string(yType.lex.d);
                        break;
                    }
                    case INTCONSTANT:
                    {
                        s = std::to_string(yType.lex.i64);
                        break;
                    }
                    case UINTCONSTANT:
                    {
                        s = std::to_string(yType.lex.u64);
                        break;
                    }
                    case BOOLCONSTANT:
                    {
                        s = yType.lex.b == true ? "true" : "false";
                        break;
                    }
                    default:
                    {
                        s = yType.lex.string->c_str();
                    }
                    }
                }
                //DEBUG("line ({}) Token: {} Symbol: {} | String {}", yType.lex.loc.line, token, yType.lex.symbol, s);

                Token t{ token, s, static_cast<std::size_t>(line) };
                tokens.emplace_back(t);
            } while (true);

            return NoError();
        }

        Error GLSLTokenizer::Init(std::vector<Token> tokens_, EShLanguage shader_type_)
        {
            shader_type = shader_type_;
            tokens = std::move(tokens_);
            return NoError();
        }

        bool GLSLTokenizer::HasTokenAtIndex(GLSLTokenizerIterator& i)
        {
            auto index = i.index;
            if (index >= tokens.size())
            {
                return false;
            }
            return true;
        }

        Token& GLSLTokenizer::GetTokenAtIndex(GLSLTokenizerIterator& i)
        {
            auto index = i.index;
            if (index >= tokens.size())
            {
                PANIC("Getting token out of bounds");
            }
            return tokens[index];
        }

        Error GLSLTokenizer::InsertTokenAtIndex(std::size_t i, yytokentype type)
        {
            if (auto found = glsl_token_type_to_string.find(type); found != std::end(glsl_token_type_to_string))
            {
                auto& [_, s] = *found;
                Token token{ type, s };
                tokens.insert(std::begin(tokens) + i, token);
            }
            else
            {
                return StringError(fmt::format("Unable to insert token of type {}", type));
            }
            return NoError();
        }

        Error GLSLTokenizer::InsertTokenAtIndex(std::size_t i, std::string_view s)
        {
            if (auto found = glsl_token_string_to_type.find(s); found != std::end(glsl_token_string_to_type))
            {
                auto& [_, type] = *found;
                Token token{ type, s };
                tokens.insert(std::begin(tokens) + i, token);
            }
            else
            {
                return StringError(fmt::format("Unable to insert token of type {}", s));
            }
            return NoError();
        }

        Error GLSLTokenizer::InsertCustomTypeAtIndex(std::size_t i, yytokentype type, std::string_view s)
        {
            Token token{ type, std::string(s) };
            tokens.insert(std::begin(tokens) + i, token);
            return NoError();
        }

        Error GLSLTokenizer::InsertLiteralAtIndex(std::size_t i, std::string_view s)
        {
            return InsertCustomTypeAtIndex(i, yytokentype::STRING_LITERAL, s);
        }

        Error GLSLTokenizer::InsertTokensAtIndexVec(GLSLTokenizerIterator& iter, std::vector<Token> tokens)
        {
            for (auto i = 0; i < tokens.size(); i++)
            {
                const auto& token = tokens[iter.index];
                if (auto err = InsertTokenAtIndex(iter.index, token.type))
                {
                    llvm::consumeError(std::move(err));
                    PanicIfError(InsertCustomTypeAtIndex(iter.index, token.type, token.s));
                }
                iter.index++;
            }
            return NoError();
        }

        Error GLSLTokenizer::InsertTokensAtIndexVec(GLSLTokenizerIterator& iter, std::vector<std::string> tokens)
        {
            for (auto j = 0; j < tokens.size(); j++)
            {
                const auto& token = tokens[j];
                if (auto err = InsertTokenAtIndex(iter.index, token))
                {
                    llvm::consumeError(std::move(err));
                    PanicIfError(InsertLiteralAtIndex(iter.index, token));
                }
                iter.index++;
            }
            return NoError();
        }

        void GLSLTokenizer::EraseToken(int index)
        {
            tokens[index].type = yytokentype::YYUNDEF;
            tokens[index].s.clear();
        }

        Error GLSLTokenizer::RemoveTokenAtIndex(GLSLTokenizerIterator& iter)
        {
            tokens.erase(std::begin(tokens) + iter.index, std::begin(tokens) + iter.index + 1);
            iter.index--;

            return NoError();
        }

        Error GLSLTokenizer::RemoveLine(GLSLTokenizerIterator& iter)
        {
            auto index = iter.index;
            auto line = iter.line;
            auto begin = index;
            auto end = index;
            for (auto i = index; i >= 0; i--)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                begin = i;
            }
            begin = std::max(begin, 0);

            // check going forward
            for (auto i = index + 1; i < tokens.size(); i++)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    //iter.index = i - 1;
                    iter.line = token.line;
                    break;
                }
                end = i;
            }
            iter.index = begin;

            tokens.erase(std::begin(tokens) + begin, std::begin(tokens) + end);
            return NoError();
        }

        Expected<std::string> GLSLTokenizer::GetLine(const GLSLTokenizerIterator& iter) const
        {
            auto index = iter.index;
            auto line = iter.line;
            auto begin = index;
            auto end = index;
            for (auto i = index; i >= 0; i--)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                begin = i;
            }
            begin = std::max(begin, 0);

            // check going forward
            for (auto i = index + 1; i < tokens.size(); i++)
            {
                end = i;
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
            }

            std::vector<Token> tokens2;
            tokens2.reserve(end - begin);
            for (auto i = begin; i < end; i++)
            {
                tokens2.emplace_back(tokens[i]);
            }
            auto str = CovertTokensToString(tokens2);

            return str;
        }

        std::optional<GLSLTokenizerIterator> GLSLTokenizer::CheckContainsTokenInLine(GLSLTokenizerIterator& iter, yytokentype type)
        {
            auto index = iter.index;
            auto line = iter.line;
            for (auto i = index; i >= 0; --i)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                if (token.type == type)
                {
                    return GLSLTokenizerIterator{ i, line };
                }
            }
            // check going forward
            for (auto i = index + 1; i < tokens.size(); i++)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                if (token.type == type)
                {
                    return GLSLTokenizerIterator{ i, line };
                }
            }
            return std::nullopt;
        }

        std::optional<GLSLTokenizerIterator> GLSLTokenizer::CheckContainsTokenInLine(GLSLTokenizerIterator& iter, std::string_view s)
        {
            auto index = iter.index;
            auto line = iter.line;
            for (auto i = index; i >= 0; --i)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                if (token.s == s)
                {
                    return GLSLTokenizerIterator{ i, line };
                }
            }
            // check going forward
            for (auto i = index + 1; i < tokens.size(); i++)
            {
                const auto& token = tokens[i];
                if (token.line != line)
                {
                    break;
                }
                if (token.s == s)
                {
                    return GLSLTokenizerIterator{ i, line };
                }
            }
            return std::nullopt;
        }

        std::string GLSLTokenizer::CovertTokensToString(const std::vector<Token>& tokens)
        {
            std::stringstream ss;
            auto current_line = 0;
            if (tokens.size() != 0)
            {
                current_line = tokens[0].line;
            }
            for (auto i = 0; i < tokens.size(); i++)
            {
                auto& t = tokens[i];
                if (t.type == yytokentype::YYUNDEF)
                {
                    continue;
                }
                ss << t.s;
                if (i != tokens.size() - 1 && tokens[i + 1].line != current_line)
                {
                    current_line = tokens[i + 1].line;
                    ss << "\n";
                }
                else
                {
                    ss << " ";
                }
            }
            return ss.str();
        }

        std::string GLSLTokenizer::ConvertToString()
        {
            std::stringstream ss;
            ss << Raysterizer::MiddleWare::ShaderConverter::shader_version_450 << "\n";

            ss << CovertTokensToString(tokens);
            return ss.str();
        }

        std::string GLSLTokenizer::ConvertToStringWithoutHeader()
        {
            std::stringstream ss;

            ss << CovertTokensToString(tokens);
            return ss.str();
        }

        Error GLSLAnalyzer::Init(std::string_view source_, EShLanguage shader_type_)
        {
            if (initialized)
            {
                return NoError();
            }
            initialized = true;

            /*
            CallOnce
            {
                glslang::InitializeProcess();
            };
            */

            PanicIfError(tokenizer.Init(source_, shader_type_));

            // Parse any extensions and add them
            auto source_lines = Util::SplitString(source_, "\n");
            for (const auto& s : source_lines)
            {
                if (s.find("#extension") == 0)
                {
                    PanicIfError(tokenizer.InsertLiteralAtIndex(0, s));
                }
            }

            Raysterizer::MiddleWare::ShaderConverter shader_converter{};
            shader_converter.AdjustOpenGLToSPIRVVersion450(tokenizer);
            shader_type = shader_type_;
            source = tokenizer.ConvertToString();

            program = std::make_unique<glslang::TProgram>();
            uniforms.clear();
            uniform_blocks.clear();
            pipeline_inputs.clear();
            pipeline_outputs.clear();

            auto shader_source = std::string(source);
            auto shader_source_c_str = shader_source.c_str();

            shader = std::make_unique<glslang::TShader>(shader_type);
            //program = glslang::Tprogram();
            //https://github.com/dfranx/SHADERed/blob/d87082145899dcc48670b8153c6969fcfa447269/src/SHADERed/Objects/ShaderCompiler.cpp
            shader->setStrings(&shader_source_c_str, 1);

            auto client = EShClient::EShClientOpenGL;
            int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
            glslang::EShTargetClientVersion ClientVersion = glslang::EShTargetOpenGL_450;
            glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetLanguageVersion::EShTargetSpv_1_5;

            shader->setEnvInput(glslang::EShSourceGlsl, shader_type, client, ClientInputSemanticsVersion);
            shader->setEnvClient(client, ClientVersion);
            shader->setEnvTarget(glslang::EShTargetSpv, TargetVersion);
            shader->setAutoMapBindings(true);
            shader->setAutoMapLocations(true);

            TBuiltInResource Resources = DefaultTBuiltInResource;
            //EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgAST | EShMsgDebugInfo);
            //EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgAST | EShMsgDebugInfo);
            //EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgAST | EShMsgDebugInfo);
            EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgAST | EShMsgDebugInfo);

            DirStackFileIncluder Includer;

            /*
            std::string PreprocessedGLSL;
            if (!shader->preprocess(&Resources, glsl_version, glsl_profile, false, false, messages, &PreprocessedGLSL, Includer))
            {
                std::cout << "GLSL Preprocessing Failed for: " << std::endl;
                fmt::print("{}\n", shader_source_c_str);
                std::cout << shader->getInfoLog() << std::endl;
                std::cout << shader->getInfoDebugLog() << std::endl;
                PANIC("Shader error");
            }

            shader_source_c_str = PreprocessedGLSL.c_str();
            */
            shader->setStrings(&shader_source_c_str, 1);

            if (!shader->parse(&Resources, 110, false, messages, Includer))
            {
                std::cout << "GLSL Parsing Failed for: " << std::endl;
                std::cout << shader_source_c_str;
                std::cout << shader->getInfoLog() << std::endl;
                std::cout << shader->getInfoDebugLog() << std::endl;
                if (1)
                {
                    {
                        auto f = std::ofstream("debug2.glsl", std::ios::ate);
                        f << shader_source;
                    }
                }
                PANIC("Shader error");
            }

            program->addShader(shader.get());

            if (!program->link(messages))
            {
                std::cout << "GLSL Linking Failed for: " << std::endl;
                fmt::print("{}\n", shader_source_c_str);
                std::cout << program->getInfoLog() << std::endl;
                std::cout << program->getInfoDebugLog() << std::endl;
                if (1)
                {
                    {
                        auto f = std::ofstream("debug2.glsl", std::ios::ate);
                        f << shader_source;
                    }
                }
                PANIC("Shader error");
            }

            const auto& intermediate = program->getIntermediate(shader_type);
            constexpr auto reflection_opts =
                EShReflectionOptions::EShReflectionStrictArraySuffix |
                EShReflectionOptions::EShReflectionBasicArraySuffix |
                EShReflectionOptions::EShReflectionIntermediateIO |
                EShReflectionOptions::EShReflectionSeparateBuffers |
                EShReflectionOptions::EShReflectionAllBlockVariables | 
                EShReflectionOptions::EShReflectionUnwrapIOBlocks |
                EShReflectionOptions::EShReflectionAllIOVariables;
            program->buildReflection(reflection_opts);

#ifndef NDEBUG
            //program->dumpReflection();
#endif
            /*
            auto sink = TInfoSink();
            auto t = glslang::TOutputTraverser(sink);
            intermediate->getTreeRoot()->traverse(&t);
            fmt::print("{}\n{}\n", sink.info.c_str(), sink.debug.c_str());
            Util::WaitUntilKeyPress();
            */

            for (auto i = 0; i < program->getNumLiveUniformVariables(); i++)
            {
                const auto& uniform = program->getUniform(i);
                /*
                fmt::print("\n{}\n", i);
                fmt::print("{}\n", uniform.name);
                fmt::print("{}\n", uniform.getType()->getCompleteString());
                */
                const glslang::TType* attribute_type = uniform.getType();
                const auto& qualifier = attribute_type->getQualifier();
                /*
                if (qualifier.hasLayout())
                {
                    fmt::print("qualifier.layoutLocation: {}\n", qualifier.layoutLocation);
                }
                if (qualifier.hasBinding())
                {
                    fmt::print("qualifier.layoutBinding: {}\n", qualifier.layoutBinding);
                }
                if (qualifier.hasSet())
                {
                    fmt::print("qualifier.layoutSet: {}\n", qualifier.layoutSet);
                }
                fmt::print("members: {}\n", uniform.numMembers);
                fmt::print("blockIndex: {}\n", uniform.counterIndex);
                fmt::print("bufferOffset: {}\n", uniform.offset);
                fmt::print("arraySize: {}\n", uniform.size);
                */

                auto uniform_name = uniform.name;
                if (auto found = uniform_name.find("["); found != std::string::npos)
                {
                    uniform_name.erase(found);
                }
                auto u = Uniform(uniform, i);
                auto [inserted_pair, success] = uniforms.try_emplace(uniform_name, std::move(u));
                if (!success)
                {
                    //PANIC("Uniform {} already exists", uniform_name);
                }
                // Repeats occurs for arrays
                // ex Light clights[8]; -> [0]...[8], but all with clights
                /*
                if (!success)
                {
                    PANIC("Uniform block already exists {}\n", uniform_name);
                }
                */
            }

            // find any additional inactive uniforms...
            tokenizer.IterateMut([&](GLSLTokenizerIterator& iter, Token& token)
                {
                    if (token.type == UNIFORM && token.s == "uniform")
                    {
                        const auto& next_token = tokenizer.GetTokenAtIndex(GLSLTokenizerIterator{ iter.index + 1 });
                        if (next_token.type == SAMPLER2D && next_token.s == "sampler2D")
                        {
                            // assume only one entry...
                            const auto& next_next_token = tokenizer.GetTokenAtIndex(GLSLTokenizerIterator{ iter.index + 2 });
                            if (next_next_token.type == IDENTIFIER)
                            {
                                const auto& uniform_name = next_next_token.s;

                                glslang::TSampler glsl_sampler{ glslang::TBasicType::EbtSampler, glslang::TSamplerDim::Esd2D, false, false, false, false, true, false };
                                auto glsl_type = std::make_unique<glslang::TType>(glsl_sampler, glslang::TStorageQualifier::EvqUniform);
                                glslang::TObjectReflection uniform(uniform_name, *glsl_type, 0, 0, 0, 0);
                                uniform.name = uniform_name;
                                uniform.offset = -1;
                                uniform.glDefineType = GL_SAMPLER_2D;
                                uniform.size = 1;
                                uniform.index = -1;
                                uniform.counterIndex = -1;
                                uniform.numMembers = -1;
                                uniform.arrayStride = 1;
                                uniform.topLevelArraySize = 1;
                                uniform.topLevelArrayStride = 1;
                                uniform.stages = shader_type == EShLanguage::EShLangVertex ? EShLanguageMask::EShLangVertexMask : EShLanguageMask::EShLangFragmentMask;

                                auto u = Uniform(uniform, -1);
                                u.SetOwnGLSLType(std::move(glsl_type));
                                auto [inserted_pair, success] = uniforms.try_emplace(uniform_name, std::move(u));
                            }
                        }
                    }

                    return true;
                });

#ifndef NDEBUG
            for (const auto& [n, u] : uniforms)
            {
                uniforms_debug[n] = &u;
            }
#endif

            for (auto i = 0; i < program->getNumLiveUniformBlocks(); i++)
            {
                const auto& uniform_block = program->getUniformBlock(i);
                /*
                uniform_block.dump();
                uniform_block.glDefineType;
                fmt::print("\n{}\n", i);
                fmt::print("{}\n", uniform_block.name);
                fmt::print("{}\n", uniform_block.getType()->getCompleteString());
                */
                const glslang::TType* attribute_type = uniform_block.getType();
                const auto* struct_type = attribute_type->getStruct();
                const auto& qualifier = attribute_type->getQualifier();
                /*
                if (qualifier.hasLayout())
                {
                    fmt::print("qualifier.layoutLocation: {}\n", qualifier.layoutLocation);
                }
                if (qualifier.hasBinding())
                {
                    fmt::print("qualifier.layoutBinding: {}\n", qualifier.layoutBinding);
                }
                if (qualifier.hasSet())
                {
                    fmt::print("qualifier.layoutSet: {}\n", qualifier.layoutSet);
                }
                fmt::print("members: {}\n", uniform_block.numMembers);
                fmt::print("blockIndex: {}\n", uniform_block.counterIndex);
                fmt::print("bufferOffset: {}\n", uniform_block.offset);
                fmt::print("arraySize: {}\n", uniform_block.size);
                */
                const auto& structure = *attribute_type->getStruct();
                for (const auto& member : structure)
                {
                    const auto* member_type = member.type;
                    const auto& field_name = member_type->getFieldName();
                    if (auto found = uniforms.find(field_name); found != std::end(uniforms))
                    {
                        auto& [_, uniform] = *found;
                        if (uniform.GetGLSLType()->getBasicType() != member_type->getBasicType())
                        {
                            // uniform must be updated to be a struct
                            if (member_type->getBasicType() != EbtStruct)
                            {
                                PANIC("Assuming struct!");
                            }
                            uniform.SetGLSLType(member_type);
                        }
                        uniform.SetParentUniformBlockName(uniform_block.name);
                    }
                }

                auto ub = UniformBlock(uniform_block, i);
                auto [_, success] = uniform_blocks.try_emplace(uniform_block.name, std::move(ub));
                if (!success)
                {
                    PANIC("Uniform block already exists {}\n", uniform_block.name);
                }
            }

            phmap::flat_hash_set<std::string> pipeline_input_structs;

            for (auto i = 0; i < program->getNumPipeInputs(); i++)
            {
                const auto& attribute = program->getPipeInput(i);
                const auto* attribute_type = attribute.getType();
                const auto& qualifier = attribute_type->getQualifier();

                if (qualifier.storage >= EvqVertexId)
                {
                    continue;
                }

                std::string_view name = attribute.name;

                /*
                 * Sometimes it gets defined as "VertexData.clipPos", when the output is defined as a struct
                 *
                 * out VertexData {
                 *    float4 pos;
                 *    float4 colors_0;
                 *    float4 colors_1;
                 *    float3 tex0;
                 *    float4 clipPos;
                 *    float clipDist0;
                 *    float clipDist1;
                 * } vs;
                 *
                 * When actually, it should be "clipPos"
                 */

                bool is_part_of_struct = false;
                const std::string_view period = ".";
                if (auto found = name.find(period); found != std::string::npos)
                {
                    auto struct_name = name.substr(0, found);
                    name = name.substr(found + period.length());
                    is_part_of_struct = true;

                    // TODO: This is hacky af because it edits the entire source code...
                    if (!pipeline_input_structs.contains(struct_name))
                    {
                        pipeline_input_structs.insert(std::string(struct_name));

                        /*
                         *
                         * struct XBRTable
                         * {
                         *     vec2 texCoord;
                         *     vec4 t1;
                         *     vec4 t2;
                         *     vec4 t3;
                         *     vec4 t4;
                         *     vec4 t5;
                         *     vec4 t6;
                         *     vec4 t7;
                         * };
                         *
                         * out XBRTable xbrTable;
                         */

                         // OR

                         /*
                         * VARYING_LOCATION(0) out VertexData {
                         *   float4 pos;
                         *   float4 colors_0;
                         *   float4 colors_1;
                         *   float3 tex0;
                         *   float4 clipPos;
                         *   float clipDist0;
                         *   float clipDist1;
                         * } vs;
                         *
                         * ->
                         *
                         * float4 pos;
                         * float4 colors_0;
                         * float4 colors_1;
                         * float3 tex0;
                         * float4 clipPos;
                         * float clipDist0;
                         * float clipDist1;
                         *
                         * and any occurance of vs is deleted
                         *
                         * ex,
                         * vs.colors_0 = o.colors_0;
                         *
                         * ->
                         *
                         * colors_0 = o.colors_0;
                         */

                        auto found_struct_name = false;
                        auto found_left_braces = false;
                        auto found_right_braces = false;
                        auto found_main = false;
                        GLSLTokenizerIterator semicolon_iter;
                        std::string struct_string_literal_name{};
                        std::string object_name{};
                        flat_hash_map<std::size_t, GLSLTokenizerIterator> lines_to_delete;
                        tokenizer.IterateMut([&](GLSLTokenizerIterator& iter, Token& token)
                            {
                                auto& s = token.s;
                                auto type = token.type;
                                if (!found_struct_name && s == struct_name)
                                {
                                    // out XBRTable xbrTable;
                                    GLSLTokenizerIterator start_iter = iter.Clone();
                                    GLSLTokenizerIterator end_iter = iter.Clone();
                                    tokenizer.IterateStartPrevMut(start_iter, [&](GLSLTokenizerIterator& i2, Token& token)
                                        {
                                            if (i2.line != iter.line)
                                            {
                                                return false;
                                            }
                                            return true;
                                        });
                                    // out XBRTable xbrTable;
                                    start_iter.index += 1;
                                    tokenizer.IterateStartMut(end_iter, [&](GLSLTokenizerIterator& i3, Token& token)
                                        {
                                            if (i3.line != iter.line)
                                            {
                                                return false;
                                            }
                                            return true;
                                        });

                                    if (tokenizer.GetTokenAtIndex(start_iter).s == "in")
                                    {
                                        // XBRTable xbrTable;
                                        auto start_index = start_iter.index + 1;
                                        auto end_index = end_iter.index - 1;
                                        std::vector<Token> tokens;
                                        tokens.reserve((end_index - start_index));
                                        for (auto j = start_index; j < end_index; j++)
                                        {
                                            GLSLTokenizerIterator temp{ j };
                                            auto& token = tokenizer.GetTokenAtIndex(temp);
                                            tokens.emplace_back(token);
                                        }

                                        auto tokens_string = GLSLTokenizer::CovertTokensToString(tokens);
                                        auto a = PipelineInput(attribute, i, is_part_of_struct);
                                        a.SetOverrideDescription(tokens_string);

                                        auto [_, success] = pipeline_inputs.try_emplace(tokens_string, std::move(a));
                                        if (!success)
                                        {
                                            PANIC("Uniform block already exists {}\n", tokens_string);
                                        }

                                        //return false;
                                    }

                                    found_struct_name = true;
                                    //lines_to_delete.insert({ token.line, iter });
                                    PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
                                    return true;
                                }

                                return true;
                            }
                        );
                    }
                }
                else
                {
                    auto a = PipelineInput(attribute, i);
                    auto [_, success] = pipeline_inputs.try_emplace(attribute.name, std::move(a));
                    if (!success)
                    {
                        PANIC("Uniform block already exists {}\n", attribute.name);
                    }
                }
            }

            for (auto& [name, pipeline_input] : pipeline_inputs)
            {
                const auto& glsl_type = pipeline_input.GetGLSLType();
                const auto& qualifier = glsl_type->getQualifier();

                auto index = pipeline_input.GetIndex();

                if (qualifier.hasLayout() && qualifier.hasLocation())
                {
                    if (index != qualifier.layoutLocation)
                    {
                        // override
                        index = qualifier.layoutLocation;
                    }
                }

                auto [_, success] = index_to_pipeline_input.try_emplace(index, &pipeline_input);
                if (!success)
                {
                    PANIC("Insert same index");
                }
            }

            phmap::flat_hash_set<std::string> pipeline_output_structs;
            
            for (auto i = 0; i < program->getNumPipeOutputs(); i++)
            {
                const auto& attribute = program->getPipeOutput(i);
                const auto* attribute_type = attribute.getType();
                const auto& qualifier = attribute_type->getQualifier();

                if (qualifier.storage >= EvqVertexId)
                {
                    continue;
                }

                std::string_view name = attribute.name;

                /*
                 * Sometimes it gets defined as "VertexData.clipPos", when the output is defined as a struct
                 *
                 * out VertexData {
                 *    float4 pos;
                 *    float4 colors_0;
                 *    float4 colors_1;
                 *    float3 tex0;
                 *    float4 clipPos;
                 *    float clipDist0;
                 *    float clipDist1;
                 * } vs;
                 *
                 * When actually, it should be "clipPos"
                 */

                bool is_part_of_struct = false;
                const std::string_view period = ".";
                if (auto found = name.find(period); found != std::string::npos)
                {
                    auto struct_name = name.substr(0, found);
                    name = name.substr(found + period.length());
                    is_part_of_struct = true;

                    // TODO: This is hacky af because it edits the entire source code...
                    if (!pipeline_output_structs.contains(struct_name))
                    {
                        pipeline_output_structs.insert(std::string(struct_name));

                        /*
                         *
                         * struct XBRTable
                         * {
                         *     vec2 texCoord;
                         *     vec4 t1;
                         *     vec4 t2;
                         *     vec4 t3;
                         *     vec4 t4;
                         *     vec4 t5;
                         *     vec4 t6;
                         *     vec4 t7;
                         * };
                         *
                         * out XBRTable xbrTable;
                         */

                         // OR

                         /*
                         * VARYING_LOCATION(0) out VertexData {
                         *   float4 pos;
                         *   float4 colors_0;
                         *   float4 colors_1;
                         *   float3 tex0;
                         *   float4 clipPos;
                         *   float clipDist0;
                         *   float clipDist1;
                         * } vs;
                         *
                         * ->
                         *
                         * float4 pos;
                         * float4 colors_0;
                         * float4 colors_1;
                         * float3 tex0;
                         * float4 clipPos;
                         * float clipDist0;
                         * float clipDist1;
                         *
                         * and any occurance of vs is deleted
                         *
                         * ex,
                         * vs.colors_0 = o.colors_0;
                         *
                         * ->
                         *
                         * colors_0 = o.colors_0;
                         */

                        auto found_struct_name = false;
                        auto found_left_braces = false;
                        auto found_right_braces = false;
                        auto found_main = false;
                        GLSLTokenizerIterator semicolon_iter;
                        std::string struct_string_literal_name{};
                        std::string object_name{};
                        flat_hash_map<std::size_t, GLSLTokenizerIterator> lines_to_delete;
                        tokenizer.IterateMut([&](GLSLTokenizerIterator& iter, Token& token)
                            {
                                auto& s = token.s;
                                auto type = token.type;
                                if (!found_struct_name && s == struct_name)
                                {
                                    // out XBRTable xbrTable;
                                    GLSLTokenizerIterator start_iter = iter.Clone();
                                    GLSLTokenizerIterator end_iter = iter.Clone();
                                    tokenizer.IterateStartPrevMut(start_iter, [&](GLSLTokenizerIterator& i2, Token& token)
                                        {
                                            if (i2.line != iter.line)
                                            {
                                                return false;
                                            }
                                            return true;
                                        });
                                    // out XBRTable xbrTable;
                                    start_iter.index += 1;
                                    tokenizer.IterateStartMut(end_iter, [&](GLSLTokenizerIterator& i3, Token& token)
                                        {
                                            if (i3.line != iter.line)
                                            {
                                                return false;
                                            }
                                            return true;
                                        });

                                    if (tokenizer.GetTokenAtIndex(start_iter).s == "out")
                                    {
                                        // XBRTable xbrTable;
                                        auto start_index = start_iter.index + 1;
                                        auto end_index = end_iter.index - 1;
                                        std::vector<Token> tokens;
                                        tokens.reserve((end_index - start_index));
                                        for (auto j = start_index; j < end_index; j++)
                                        {
                                            GLSLTokenizerIterator temp{ j };
                                            auto& token = tokenizer.GetTokenAtIndex(temp);
                                            tokens.emplace_back(token);
                                        }

                                        auto tokens_string = GLSLTokenizer::CovertTokensToString(tokens);
                                        auto a = PipelineOutput(attribute, i, is_part_of_struct);
                                        a.SetOverrideDescription(tokens_string);

                                        auto [_, success] = pipeline_outputs.try_emplace(tokens_string, std::move(a));
                                        if (!success)
                                        {
                                            PANIC("Uniform block already exists {}\n", tokens_string);
                                        }

                                        //return false;
                                    }

                                    found_struct_name = true;
                                    //lines_to_delete.insert({ token.line, iter });
                                    PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
                                    return true;
                                }
                                if (token.type == yytokentype::VOID_)
                                {
                                    auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ iter.index + 1 });
                                    if (next_token.type == yytokentype::IDENTIFIER && next_token.s == "main")
                                    {
                                        found_main = true;
                                        return false;
                                    }
                                }

                                if (found_struct_name && !found_left_braces && !found_main)
                                {
                                    if (type == yytokentype::LEFT_BRACE)
                                    {
                                        found_left_braces = true;
                                        //lines_to_delete.insert({ token.line, iter });
                                        PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
                                        return true;
                                    }
                                }
                                if (found_left_braces && !found_right_braces && !found_main)
                                {
                                    if (type == yytokentype::RIGHT_BRACE)
                                    {
                                        found_right_braces = true;
                                        //lines_to_delete.insert({ token.line, iter });

                                        // Get next token which is the object name
                                        // "vs", in this case

                                        if (iter.index + 1 < tokenizer.GetTokens().size())
                                        {
                                            auto& next_token = tokenizer.GetTokenAtIndex(GLSLTokenizerIterator{ iter.index + 1 });
                                            if (next_token.type != yytokentype::IDENTIFIER)
                                            {
                                                PANIC("Expected next token to be identifier {}", next_token.s);
                                            }
                                            object_name = next_token.s;

                                            PanicIfError(tokenizer.RemoveTokenAtIndex(iter));

                                            auto iter2 = iter.Clone();
                                            tokenizer.IterateStartMut(iter2, [&](GLSLTokenizerIterator& iter2, Token& token)
                                                {
                                                    if (token.type == yytokentype::STRING_LITERAL)
                                                    {
                                                        struct_string_literal_name = token.s;
                                                    }
                                                    if (token.type == yytokentype::SEMICOLON)
                                                    {
                                                        lines_to_delete.insert({ token.line, iter });
                                                        return false;
                                                    }
                                                    return true;
                                                });
                                        }
                                    }
                                }

                                if (!struct_string_literal_name.empty())
                                {
                                    if (token.type == yytokentype::STRING_LITERAL && token.s == struct_string_literal_name)
                                    {
                                        PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
                                        auto next_iter = GLSLTokenizerIterator{ iter.index + 1 };
                                        auto& next_token = tokenizer.GetTokenAtIndex(next_iter);
                                        if (next_token.type == yytokentype::DOT)
                                        {
                                            PanicIfError(tokenizer.RemoveTokenAtIndex(next_iter));
                                        }
                                    }
                                }
                                if (!object_name.empty())
                                {
                                    if (token.type == yytokentype::IDENTIFIER && token.s == object_name)
                                    {
                                        PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
                                        auto next_iter = GLSLTokenizerIterator{ iter.index + 1 };
                                        auto& next_token = tokenizer.GetTokenAtIndex(next_iter);
                                        if (next_token.type == yytokentype::DOT)
                                        {
                                            PanicIfError(tokenizer.RemoveTokenAtIndex(next_iter));
                                        }
                                    }
                                }

                                return true;
                            }
                        );
                    }
                }
                else
                {
                    auto a = PipelineOutput(attribute, i, is_part_of_struct);

                    auto [_, success] = pipeline_outputs.try_emplace(name, std::move(a));
                    if (!success)
                    {
                        PANIC("Uniform block already exists {}\n", name);
                    }
                }
            }

            spv::SpvBuildLogger logger;
            glslang::SpvOptions spv_options{};
            spv_options.disableOptimizer = false;
            spv_options.disassemble = true;
            spv_options.generateDebugInfo = true;
            spv_options.validate = true;
            glslang::GlslangToSpv(*program->getIntermediate(shader_type), spirv, &logger, &spv_options);
            //printf("%s", logger.getAllMessages().c_str());
            //glslang::OutputSpvHex(spirv, GetBinaryName((EShLanguage)stage), variableName);
            //glslang::OutputSpvBin(spirv, GetBinaryName((EShLanguage)stage));

            //spv::Disassemble(std::cout, spirv);

            /*
            spirv_cross::CompilerGLSL glsl(spirv);
            spirv_cross::CompilerGLSL::Options options;
            options.vulkan_semantics = true;
            glsl.set_common_options(options);
            source = glsl.compile();
            PanicIfError(tokenizer.Init(source, shader_type));

            shader_converter.AdjustOpenGLToSPIRVVersion450(tokenizer);
            source = tokenizer.ConvertToString();
            */

            return NoError();
        }

        BaseGLSLInfo::BaseGLSLInfo(std::string_view name_, const TObjectReflection& object_reflection_, std::size_t index_) :
            object_reflection(object_reflection_),
            glsl_type(object_reflection_.getType()),
            name(name_),
            gl_type(object_reflection_.glDefineType),
            element_size(object_reflection_.size),
            offset(object_reflection_.offset),
            index(index_)
        {
            const auto& qualifier = glsl_type->getQualifier();
            /*
            if (qualifier.hasLayout())
            {
                fmt::print("qualifier.layoutLocation: {}\n", qualifier.layoutLocation);
            }
            if (qualifier.hasBinding())
            {
                fmt::print("qualifier.layoutBinding: {}\n", qualifier.layoutBinding);
            }
            if (qualifier.hasSet())
            {
                fmt::print("qualifier.layoutSet: {}\n", qualifier.layoutSet);
            }
            */

            if (qualifier.hasSet())
            {
                set = qualifier.layoutSet;
            }
            if (qualifier.hasLayout())
            {
                location = qualifier.layoutLocation;
            }
            if (qualifier.hasBinding())
            {
                binding = qualifier.layoutBinding;
            }

            //auto own_glsl_type = std::unique_ptr<glslang::TType>(glsl_type->clone());
            //SetOwnGLSLType(std::move(own_glsl_type));
        }

        namespace
        {
            std::string ConvertToStringWithNecessaryInfoHelper(const glslang::TObjectReflection& object_reflection, const glslang::TType* glsl_type, std::optional<uint32_t> set = {}, std::optional<uint32_t> binding = {}, std::optional<std::size_t> override_vector_size = {}, std::optional<glslang::TBasicType> override_glsl_type = {}, std::optional<uint32_t> override_location = {}, std::size_t depth = 0, bool is_part_of_struct = false, bool force_no_qualifiers = false)
            {
                std::stringstream ss{};

                auto glsl_basic_type = glsl_type->getBasicType();
                if (override_glsl_type)
                {
                    glsl_basic_type = *override_glsl_type;
                }

                const auto& sampler = glsl_type->getSampler();

                auto AppendOut = [&](const auto& str)
                {
                    ss << str;
                };

                auto AppendOutFmt = [&](auto&& format_string, auto&&... args)
                {
                    AppendOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
                };

                if(!is_part_of_struct && !force_no_qualifiers)
                {
                    const auto& qualifier = glsl_type->getQualifier();

                    std::vector<std::string> qualifiers;
                    if (set)
                    {
                        auto dummy_set = fmt::format("set = {}", *set);
                        qualifiers.emplace_back(dummy_set);
                    }
                    if (binding)
                    {
                        auto dummy_binding = fmt::format("binding = {}", *binding);
                        qualifiers.emplace_back(dummy_binding);
                    }

                    const auto& storage = qualifier.storage;
                    switch (storage)
                    {
                    case EvqIn:
                        AppendOut("in ");
                        break;
                    case EvqOut:
                        AppendOut("out ");
                        break;
                    }

                    //if (override_location && !qualifier.hasLocation())
                    if (override_location)
                    {
                        qualifiers.emplace_back(fmt::format("location = {}", *override_location));
                    }

                    if (qualifier.flat)
                    {
                        AppendOut("flat ");
                    }
                    if (qualifier.hasLayout())
                    {
                        if (!override_location && qualifier.hasLocation())
                        {
                            qualifiers.emplace_back(fmt::format("location = {}", qualifier.layoutLocation));
                        }
                        if (qualifier.hasMatrix())
                        {
                            qualifiers.emplace_back(TQualifier::getLayoutMatrixString(qualifier.layoutMatrix));
                        }
                        if (qualifier.hasPacking())
                        {
                            qualifiers.emplace_back(TQualifier::getLayoutPackingString(qualifier.layoutPacking));
                        }
                        if (qualifier.hasFormat())
                        {
                            qualifiers.emplace_back(TQualifier::getLayoutFormatString(qualifier.layoutFormat));
                        }
                    }

                    if (qualifiers.size() > 0)
                    {
                        AppendOut("layout(");
                        for (auto i = 0; i < qualifiers.size(); i++)
                        {
                            const auto& q = qualifiers[i];
                            AppendOutFmt("{}", q);
                            if (i != qualifiers.size() - 1)
                            {
                                AppendOutFmt(", ");
                            }
                        }
                        AppendOut(") ");
                    }
                }
                
                if (!is_part_of_struct && !force_no_qualifiers)
                {
                    AppendOut(glsl_type->getStorageQualifierString());
                    AppendOut(" ");
                }

                auto name = object_reflection.name;
                if (auto found_access_index = name.find("["); found_access_index != std::string::npos)
                {
                    name.erase(found_access_index);
                }
                // VertexData.colors_1 -> colors_1
                // occurs when vertex output is a struct
                else if (auto found_period_index = name.find("."); found_period_index != std::string::npos)
                {
                    name.erase(0, found_period_index + 1);
                }

                if (glsl_type->isMatrix())
                {
                    AppendOutFmt("mat{cols}x{rows} ", 
                                 "cols"_a = glsl_type->getMatrixCols(),
                                 "rows"_a = glsl_type->getMatrixRows()
                    );
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_type->isVector())
                {
                    std::string basic_type = "";
                    switch (glsl_basic_type)
                    {
                    case EbtInt8:
                    {
                        basic_type = "int8_t";
                        break;
                    }
                    case EbtUint8:
                    {
                        basic_type = "uint8_t";
                        break;
                    }
                    case EbtInt:
                    {
                        basic_type = "ivec";
                        break;
                    }
                    case EbtUint:
                    {
                        basic_type = "uvec";
                        break;
                    }
                    case EbtFloat:
                    {
                        basic_type = "vec";
                        break;
                    }
                    default:
                    {
                        PANIC("Unsupported!");
                        break;
                    }
                    }
                    
                    auto vector_size = override_vector_size ? *override_vector_size : glsl_type->getVectorSize();

                    AppendOutFmt("{basic_type}{n} ",
                                 "basic_type"_a = basic_type,
                                 "n"_a = vector_size
                    );
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_type->isStruct())
                {
                    if (!is_part_of_struct && !force_no_qualifiers)
                    { 
                        AppendOutFmt("{} ", name);
                        AppendOut("\n{\n");

                        const auto& structure = *glsl_type->getStruct();
                        for (const auto& member : structure)
                        {
                            const auto* member_type = member.type;
                            auto loc = member.loc;
                            auto s = member.loc.string;
                            auto member_type_str = ConvertToStringWithNecessaryInfoHelper(object_reflection, member_type, set, binding, {}, {}, {}, depth + 1, true);
                            const auto& field_name = member_type->getFieldName();

                            constexpr auto tab_string = "  ";
                            for (auto i = 0; i < depth + 1; i++)
                            {
                                AppendOut(tab_string);
                            }

                            // check for array access
                            if (auto found_array_access = member_type_str.find("["); found_array_access != std::string::npos)
                            {
                                auto member_type_begin = member_type_str.substr(0, found_array_access);
                                auto member_type_access_array = member_type_str.substr(found_array_access);
                                AppendOutFmt(
                                    "{member_type_begin}{field_name}{member_type_access_array};\n",
                                    "member_type_begin"_a = member_type_begin,
                                    "field_name"_a = field_name,
                                    "member_type_access_array"_a = member_type_access_array);
                            }
                            else
                            {
                                AppendOutFmt(
                                    "{member_type_str}{field_name};\n",
                                    "member_type_str"_a = member_type_str,
                                    "field_name"_a = field_name);
                            }

                            /*
                            const auto& field_name = member_type->getFieldName();
                            const auto& type_name = member_type->getTypeName();
                            AppendOut(fmt::format(
                                "{field_name} {type_name};\n",
                                "field_name"_a = field_name,
                                "type_name"_a = type_name));
                            */
                        }

                        AppendOut("}");
                    }
                    else
                    {
                        const auto& type_name = glsl_type->getTypeName();
                        AppendOutFmt("{} ", type_name);

                        if (force_no_qualifiers)
                        {
                            const auto& field_name = glsl_type->getFieldName();
                            AppendOutFmt("{}", field_name);
                        }
                    }
                }
                else if (glsl_basic_type == EbtSampler && sampler.isCombined())
                {
                    std::string array_modifier = "";
                    if (sampler.isArrayed())
                    {
                        array_modifier = "Array";
                    }

                    switch (sampler.dim)
                    {
                    case Esd1D:
                    case Esd2D:
                    case Esd3D:
                    {
                        AppendOutFmt("sampler{n}D{array_modifier} ",
                                     "n"_a = sampler.dim,
                                     "array_modifier"_a = array_modifier);
                        break;
                    }
                    case EsdCube:
                    {
                        AppendOutFmt("samplerCube ");
                        break;
                    }
                    case EsdRect:
                    {
                        AppendOutFmt("sampler2DRect ");
                        break;
                    }
                    case EsdBuffer:
                    {
                        AppendOutFmt("samplerBuffer ");
                        break;
                    }
                    case EsdSubpass:
                    {
                        AppendOutFmt("samplerInput ");
                        break;
                    }
                    default:
                        PANIC("Unexpected enum");
                        break;
                    }

                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_basic_type == EbtInt)
                {
                    AppendOut("int ");
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_basic_type == EbtUint)
                {
                    AppendOut("uint ");
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_basic_type == EbtFloat)
                {
                    AppendOut("float ");
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else if (glsl_basic_type == EbtBool)
                {
                    AppendOut("bool ");
                    if (!is_part_of_struct)
                    {
                        AppendOutFmt("{}", name);
                    }
                }
                else
                {
                    PANIC("Not supported");
                }

                if (glsl_type->isArray())
                {
                    const auto& array_sizes = glsl_type->getArraySizes();
                    for (auto i = 0; i < array_sizes->getNumDims(); i++)
                    {
                        int size = array_sizes->getDimSize(i);
                        if (size == UnsizedArraySize && i == 0 && array_sizes->isVariablyIndexed())
                        {
                            AppendOut("[]");
                        }
                        else
                        {
                            if (size == UnsizedArraySize)
                            {
                                if (i == 0)
                                {
                                    AppendOutFmt("[{}]", array_sizes->getImplicitSize());
                                }
                            }
                            else
                            {
                                AppendOutFmt("[{}]", array_sizes->getDimSize(i));
                            }
                        }
                    }
                    //AppendOut(" ");
                }

                return ss.str();
            }
        }

        std::string BaseGLSLInfo::GetInferredDefinition(uint32_t set, uint32_t binding, bool use_override_size, bool use_override_glsl_type) const
        {
            if (!override_description.empty())
            {
                return override_description;
            }
            std::optional<std::size_t> override_vector_size_input = std::nullopt;
            if (use_override_size)
            {
                override_vector_size_input = override_vector_size;
            }
            std::optional<glslang::TBasicType> override_glsl_type_input = std::nullopt;
            if (use_override_glsl_type)
            {
                override_glsl_type_input = override_glsl_type;
            }
            return ConvertToStringWithNecessaryInfoHelper(object_reflection, glsl_type, set, binding, override_vector_size_input, override_glsl_type_input);
        }

        std::string BaseGLSLInfo::GetInferredDefinitionWithQualifiers(bool use_override_size, bool use_override_glsl_type, std::optional<uint32_t> override_location) const
        {
            if (!override_description.empty())
            {
                return override_description;
            }
            std::optional<std::size_t> override_vector_size_input = std::nullopt;
            if (use_override_size)
            {
                override_vector_size_input = override_vector_size;
            }
            std::optional<glslang::TBasicType> override_glsl_type_input = std::nullopt;
            if (use_override_glsl_type)
            {
                override_glsl_type_input = override_glsl_type;
            }
            return ConvertToStringWithNecessaryInfoHelper(object_reflection, glsl_type, std::nullopt, std::nullopt, override_vector_size_input, override_glsl_type_input, override_location, 0, false, false);
        }

        std::string BaseGLSLInfo::GetInferredDefinitionWithoutQualifiers(bool use_override_size, bool use_override_glsl_type) const
        {
            if (!override_description.empty())
            {
                return override_description;
            }
            std::optional<std::size_t> override_vector_size_input = std::nullopt;
            if (use_override_size)
            {
                override_vector_size_input = override_vector_size;
            }
            std::optional<glslang::TBasicType> override_glsl_type_input = std::nullopt;
            if (use_override_glsl_type)
            {
                override_glsl_type_input = override_glsl_type;
            }
            return ConvertToStringWithNecessaryInfoHelper(object_reflection, glsl_type, std::nullopt, std::nullopt, override_vector_size_input, override_glsl_type_input, {}, 0, false, true);
        }

        std::string BaseGLSLInfo::UpdateCastMember(glslang::TBasicType glsl_basic_type, bool use_override_size) const
        {
            // we want to cast
            // vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
            // ->
            // vec4 aPos = vec4(InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos));
            auto override_glsl_type = GetOverrideGLSLType();
            auto override_vector_size = GetOverrideVectorSize();
            std::string basic_type = "";
            switch (glsl_basic_type)
            {
            case glslang::TBasicType::EbtInt8:
            {
                basic_type = "int8_t";
                break;
            }
            case glslang::TBasicType::EbtUint8:
            {
                basic_type = "uint8_t";
                break;
            }
            case glslang::TBasicType::EbtInt:
            {
                basic_type = "int";
                break;
            }
            case glslang::TBasicType::EbtUint:
            {
                basic_type = "uint";
                break;
            }
            case glslang::TBasicType::EbtFloat:
            {
                basic_type = "float";
                break;
            }
            default:
            {
                PANIC("Unsupported!");
                break;
            }
            }

            auto vector_size = glsl_type->getVectorSize();
            if (use_override_size)
            {
                if (override_vector_size)
                {
                    vector_size = *override_vector_size;
                }
            }

            std::string cast_member;
            if (glsl_type->isScalar())
            {
                cast_member = fmt::format("raysterizer_cast_{basic_type}{n}",
                    "basic_type"_a = basic_type,
                    "n"_a = 1);
            }
            else if (glsl_type->isVector())
            {
                cast_member = fmt::format("raysterizer_cast_{basic_type}{n}",
                    "basic_type"_a = basic_type,
                    "n"_a = vector_size);
            }
            else if(glsl_type->isMatrix())
            {
                auto rows = glsl_type->getMatrixRows();
                auto cols = glsl_type->getMatrixCols();
                cast_member = fmt::format("raysterizer_cast_{basic_type}{rows}x{cols}",
                    "basic_type"_a = basic_type,
                    "rows"_a = rows,
                    "cols"_a = cols);
            }

            return cast_member;
        }

        void BaseGLSLInfo::SetOverrideDescription(std::string override_description_)
        {
            override_description = std::move(override_description_);
        }

        const std::string& BaseGLSLInfo::GetOverrideDescription() const
        {
            return override_description;
        }

        Uniform::Uniform(const TObjectReflection& object_reflection_, std::size_t index_) :
            BaseGLSLInfo(object_reflection_.name, object_reflection_, index_)
        {
        }

        UniformBlock::UniformBlock(const TObjectReflection& object_reflection_, std::size_t index_) :
            BaseGLSLInfo(object_reflection_.name, object_reflection_, index_)
        {
        }

        PipelineInput::PipelineInput(const TObjectReflection& object_reflection_, std::size_t index_, bool is_part_of_struct_) :
            BaseGLSLInfo(object_reflection_.name, object_reflection_, index_), is_part_of_struct(is_part_of_struct_)
        {
        }

        PipelineOutput::PipelineOutput(const TObjectReflection& object_reflection_, std::size_t index_, bool is_part_of_struct_) :
            BaseGLSLInfo(object_reflection_.name, object_reflection_, index_), is_part_of_struct(is_part_of_struct_)
        {
        }
    }
}