#ifndef CAST_H
#define CAST_H

#extension GL_EXT_control_flow_attributes : require
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_GOOGLE_include_directive : require

//#define float16_t
#define float16_t2 f16vec2
#define float16_t3 f16vec3
#define float16_t4 f16vec4
//#define int8_t
#define int8_t2 i8vec2
#define int8_t3 i8vec3
#define int8_t4 i8vec4
//#define int16_t
#define int16_t2 i16vec2
#define int16_t3 i16vec3
#define int16_t4 i16vec4
//#define uint8_t
#define uint8_t2 u8vec2
#define uint8_t3 u8vec3
#define uint8_t4 u8vec4
//#define uint16_t
#define uint16_t2 u16vec2
#define uint16_t3 u16vec3
#define uint16_t4 u16vec4

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define float2x2 mat2
#define float3x3 mat3
#define float4x4 mat4

vec2 raysterizer_cast_float2(float v)
{
  return vec2(v, 0);
}

vec2 raysterizer_cast_float2(vec2 v)
{
  return v;
}

////////////////////////////////

vec3 raysterizer_cast_float3(float v)
{
  return vec3(v, 0, 0);
}

vec3 raysterizer_cast_float3(vec2 v)
{
  return vec3(v, 0);
}

vec3 raysterizer_cast_float3(vec3 v)
{
  return v;
}

////////////////////////////////

vec4 raysterizer_cast_float4(float v)
{
  return vec4(v, 0, 0, 0);
}

vec4 raysterizer_cast_float4(vec2 v)
{
  return vec4(v, 0, 0);
}

vec4 raysterizer_cast_float4(vec3 v)
{
  return vec4(v, 0);
}

vec4 raysterizer_cast_float4(vec4 v)
{
  return v;
}

////////////////////////////////

uint8_t2 raysterizer_cast_uint8_t2(vec2 v)
{
  return uint8_t2(uint8_t(v.x * 255.0), uint8_t(v.y * 255.0));
}

uint8_t2 raysterizer_cast_uint8_t2(uint8_t2 v)
{
  return v;
}

uint8_t3 raysterizer_cast_uint8_t3(vec3 v)
{
  return uint8_t3(uint8_t(v.x * 255.0), uint8_t(v.y * 255.0), uint8_t(v.z * 255.0));
}

uint8_t3 raysterizer_cast_uint8_t3(uint8_t3 v)
{
  return v;
}

uint8_t4 raysterizer_cast_uint8_t4(vec4 v)
{
  return uint8_t4(uint8_t(v.x * 255.0), uint8_t(v.y * 255.0), uint8_t(v.z * 255.0), uint8_t(v.w * 255.0));
}

uint8_t4 raysterizer_cast_uint8_t4(uint8_t4 v)
{
  return v;
}

////////////////////////////////

mat3x3 raysterizer_cast_float3x3(mat3x3 v)
{
  return v;
}

mat4x4 raysterizer_cast_float4x4(mat4x4 v)
{
  return v;
}



#define RAYSTERIZER_CAST(cast_to, val) raysterizer_to_vec4()

#endif