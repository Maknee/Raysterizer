#ifndef COMMON_H
#define COMMON_H

#extension GL_EXT_control_flow_attributes : require
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_shader_clock : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

////////// Definitions //////////////////////

/*
layout(set = 0, binding = 1, rgba16f) restrict uniform image2D image;
//layout(set = 0, binding = 2, rgba16f) restrict uniform image2D rough_image;
//layout(set = 0, binding = 3, rgba16f) restrict uniform image2D normal_image;

layout(set = 0, binding = 2) uniform Info
{
  mat4 projection_view;
  mat4 projection_view_inverse;
  vec4 clear_color;
  float z_near;
  float z_far;

  float aperture;
  float focus_distance;
  float heatmap_scale;
  uint total_number_of_samples;
  uint number_of_samples;
  uint number_of_bounces;
  uint random_seed;
  bool has_sky;
  bool show_heatmap;
}
raysterizer_info;

#define INSTANCE_CUSTOM_INDEX_MATERIAL_UNUSED -1
#define INSTANCE_CUSTOM_INDEX_MATERIAL_START_INDEX 0

const uint MaterialLambertian = 0;
const uint MaterialMetallic = 1;
const uint MaterialDielectric = 2;
const uint MaterialIsotropic = 3;
const uint MaterialDiffuseLight = 4;

struct Material
{
  vec4 highlight_color;

  vec4 Diffuse;
  uint highlight;
  float Fuzziness;
  float RefractionIndex;
  uint MaterialModel;
};

layout(set = 0, binding = 3) uniform MaterialOverrides
{
  Material m;
}
material_overrides[];

struct DrawCallState
{
  int material_index;
};
*/

#ifdef RASTERIZATION_CODE

//VertexRasterizationHeader

// TEST

layout(location = 15) out vec4 FS_OUT_P;

//VertexRasterizationHeaderEnd

void VertexRasterizationCustomCode()
{
  FS_OUT_P = gl_Position;
}

//FragmentRasterizationHeader

layout(location = 0) out vec4 FS_OUT_GBuffer1; // RGB: Albedo, A: Metallic
layout(location = 1) out vec4 FS_OUT_GBuffer2; // RG: Normal, BA: Motion Vector
layout(location = 2) out vec4 FS_OUT_GBuffer3; // R: Roughness, G: Curvature, B: Mesh ID, A: Linear Z

layout(location = 15) in vec4 FS_OUT_P;
//layout(location = 11) in vec3 FS_OUT_N;

layout(set = 0, binding = 0) buffer RaysterizerOutColors
{
  vec4 raysterizer_out_colors[];
};

layout(set = 0, binding = 1) uniform sampler2D FS_IN_PrevCSPos;

layout(push_constant) uniform constants
{
  vec4 highlight_color;
  float roughness;
  float metallic;
  float unused1;
  float unused2;
} PushConstants;


// ------------------------------------------------------------------------
// FUNCTIONS --------------------------------------------------------------
// ------------------------------------------------------------------------

// A simple utility to convert a float to a 2-component octohedral representation
vec2 direction_to_octohedral(vec3 normal)
{
    vec2 p = normal.xy * (1.0f / dot(abs(normal), vec3(1.0f)));
    return normal.z > 0.0f ? p : (1.0f - abs(p.yx)) * (step(0.0f, p) * 2.0f - vec2(1.0f));
}

// ------------------------------------------------------------------------

vec2 compute_motion_vector(vec2 prev_pos, vec4 current_pos)
{
    // Perspective division, covert clip space positions to NDC.
    vec2 current = (current_pos.xy / current_pos.w);
    vec2 prev = prev_pos.xy;

    // Remap to [0, 1] range
    current = current * 0.5 + 0.5;
    prev = prev * 0.5 + 0.5;

    // Calculate velocity (current -> prev)
    return (prev - current);
}

//FragmentRasterizationHeaderEnd

void FragmentRasterizationCustomCode()
{

  FS_OUT_GBuffer1 = vec4(0.0, 0.0, 0.0, 0.0);
  FS_OUT_GBuffer2 = vec4(0.0, 0.0, 0.0, 0.0);
  FS_OUT_GBuffer3 = vec4(0.0, 0.0, 0.0, 0.0);

#ifdef RAYSTERIZER_COLOR
  FS_OUT_GBuffer1 = vec4(RAYSTERIZER_COLOR.xyz, 0.0);
  raysterizer_out_colors[gl_PrimitiveID] = vec4(FS_OUT_GBuffer1.xyz, 0.0f);
#endif
  if (PushConstants.highlight_color.w > 0.0)
  {
    FS_OUT_GBuffer1.xyz = PushConstants.highlight_color.xyz;
  }
  //FS_OUT_GBuffer1.xyz = vec3(1.0f, 0.0f, 0.0f);
  FS_OUT_GBuffer1.w = PushConstants.metallic; // metallic

#ifdef RAYSTERIZER_NORMAL
  FS_OUT_GBuffer2.xy = direction_to_octohedral(RAYSTERIZER_NORMAL.xyz);
#else
  vec3 fdx = dFdx(FS_OUT_P.xyz);
  vec3 fdy = dFdy(FS_OUT_P.xyz);
  vec3 FS_OUT_N = normalize(cross(fdx, fdy));
  FS_OUT_GBuffer2.xy = direction_to_octohedral(FS_OUT_N.xyz);
#endif

  //vec2 PREV_CS_POS = textureLod(FS_IN_PrevCSPos, gl_FragCoord.xy / textureSize(FS_IN_PrevCSPos, 0), 0.0).rg;
  vec2 PREV_CS_POS = texelFetch(FS_IN_PrevCSPos, ivec2(gl_FragCoord.xy), 0).rg;
  FS_OUT_GBuffer2.zw = compute_motion_vector(PREV_CS_POS, FS_OUT_P);

  FS_OUT_GBuffer3.rg = vec2(FS_OUT_P.xy / FS_OUT_P.w);
  FS_OUT_GBuffer3.b = PushConstants.roughness;
  FS_OUT_GBuffer3.a = gl_FragCoord.z / gl_FragCoord.w;
}

//TessellationControlRasterizationHeader

layout (vertices = 3) out;

layout(set = 0, binding = 0) buffer RaysterizerOutColors
{
  vec4 raysterizer_out_colors[];
};

//TessellationControlRasterizationHeaderEnd

void TessellationControlRasterizationCustomCode()
{
  if (gl_InvocationID == 0)
  {
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
  }

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
#ifdef RAYSTERIZER_COLOR
  raysterizer_out_colors[gl_PrimitiveID] = vec4(RAYSTERIZER_COLOR.xyz, 0.0f);
#endif
}

//TessellationEvaluationRasterizationHeader

layout (triangles, fractional_odd_spacing, cw) in;

layout(location = 10) out vec4 FS_OUT_P;
layout(location = 11) out vec3 FS_OUT_N;

//TessellationEvaluationRasterizationHeaderEnd

void TessellationEvaluationRasterizationCustomCode()
{
  gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                (gl_TessCoord.y * gl_in[1].gl_Position) +
                (gl_TessCoord.z * gl_in[2].gl_Position);

  FS_OUT_P = gl_Position;

  FS_OUT_N = cross(
    normalize(vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position)),
    normalize(vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position))
  );

}

#endif

///////////////////////////////////

#ifdef RT_SHADER

layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

#ifdef RAYGEN_PAYLOAD
#define RAY_LOAD_TYPE rayPayloadEXT
#else
#define RAY_LOAD_TYPE rayPayloadInEXT
#endif

struct Payload
{
  vec4 color_and_distance; // rgb + t
  vec4 scatter_direction;  // xyz + w (is scatter needed)
  uint random_seed;
  uint depth;
};

#define RT_GENERIC 0
#define RT_SHADOW_TRACE 1
#define RT_SHADOW_INTERNAL 2

#define MAIN_PAYLOAD 0
layout(location = MAIN_PAYLOAD) RAY_LOAD_TYPE Payload payload;

#define SHADOW_PAYLOAD 1
layout(location = SHADOW_PAYLOAD) RAY_LOAD_TYPE bool shadow;

/////

#define OTHER_DEF

#ifdef OTHER_DEF

// Polynomial approximation by Christophe Schlick
float Schlick(const float cosine, const float refractionIndex)
{
  float r0 = (1 - refractionIndex) / (1 + refractionIndex);
  r0 *= r0;
  return r0 + (1 - r0) * pow(1 - cosine, 5);
}

// Lambertian
Payload ScatterLambertian(const Material m, const vec3 direction, const vec3 normal, const vec2 texCoord, const float t, uint depth, inout uint seed)
{
  const bool isScattered = dot(direction, normal) < 0;
  const vec4 texColor = vec4(1);
  const vec4 colorAndDistance = vec4(m.Diffuse.rgb * texColor.rgb, t);
  const vec4 scatter = vec4(normal + RandomInUnitSphere(seed), isScattered ? 1 : 0);

  return Payload(colorAndDistance, scatter, seed, depth);
}

// Metallic
Payload ScatterMetallic(const Material m, const vec3 direction, const vec3 normal, const vec2 texCoord, const float t, uint depth, inout uint seed)
{
  const vec3 reflected = reflect(direction, normal);
  const bool isScattered = dot(reflected, normal) > 0;

  const vec4 texColor = vec4(0);
  const vec4 colorAndDistance = vec4(m.Diffuse.rgb * texColor.rgb, t);
  const vec4 scatter = vec4(reflected + m.Fuzziness * RandomInUnitSphere(seed), isScattered ? 1 : 0);

  return Payload(colorAndDistance, scatter, seed, depth);
}

// Dielectric
Payload ScatterDieletric(const Material m, const vec3 direction, const vec3 normal, const vec2 texCoord, const float t, uint depth, inout uint seed)
{
  const float dot = dot(direction, normal);
  const vec3 outwardNormal = dot > 0 ? -normal : normal;
  const float niOverNt = dot > 0 ? m.RefractionIndex : 1 / m.RefractionIndex;
  const float cosine = dot > 0 ? m.RefractionIndex * dot : -dot;

  const vec3 refracted = refract(direction, outwardNormal, niOverNt);
  const float reflectProb = refracted != vec3(0) ? Schlick(cosine, m.RefractionIndex) : 1;

  const vec4 texColor = vec4(1);

  return RandomFloat(seed) < reflectProb
             ? Payload(vec4(texColor.rgb, t), vec4(reflect(direction, normal), 1), seed, depth)
             : Payload(vec4(texColor.rgb, t), vec4(refracted, 1), seed, depth);
}

// Diffuse Light
Payload ScatterDiffuseLight(const Material m, const float t, uint depth, inout uint seed)
{
  const vec4 colorAndDistance = vec4(m.Diffuse.rgb, t);
  const vec4 scatter = vec4(1, 0, 0, 0);

  return Payload(colorAndDistance, scatter, seed, depth);
}

Payload Scatter(const Material m, const vec3 direction, const vec3 normal, const vec2 texCoord, const float t, uint depth, inout uint seed)
{
  const vec3 normDirection = normalize(direction);

  switch (m.MaterialModel)
  {
  case MaterialLambertian:
    return ScatterLambertian(m, normDirection, normal, texCoord, t, depth, seed);
  case MaterialMetallic:
    return ScatterMetallic(m, normDirection, normal, texCoord, t, depth, seed);
  case MaterialDielectric:
    return ScatterDieletric(m, normDirection, normal, texCoord, t, depth, seed);
  case MaterialDiffuseLight:
    return ScatterDiffuseLight(m, t, depth, seed);
  }
}
#endif

///////

vec2 dFdx(vec2 v)
{
  return v;
}

vec2 dFdy(vec2 v)
{
  return v;
}

vec3 dFdx(vec3 v)
{
  return v;
}

vec3 dFdy(vec3 v)
{
  return v;
}

vec4 dFdx(vec4 v)
{
  return v;
}

vec4 dFdy(vec4 v)
{
  return v;
}

#ifdef HIT_SHADER
hitAttributeEXT vec3 HIT_ATTRIBUTE;

vec2 InterpolateBarycentrics(vec2 a, vec2 b, vec2 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 InterpolateBarycentrics(vec3 a, vec3 b, vec3 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 InterpolateBarycentrics(vec4 a, vec4 b, vec4 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

ivec4 InterpolateBarycentrics(ivec4 a, ivec4 b, ivec4 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return ivec4(a * barycentrics.x + b * barycentrics.y + c * barycentrics.z);
}

uvec4 InterpolateBarycentrics(uvec4 a, uvec4 b, uvec4 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return uvec4(a * barycentrics.x + b * barycentrics.y + c * barycentrics.z);
}

mat4 InterpolateBarycentrics(mat4 a, mat4 b, mat4 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  //return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
  return a;
}

uint8_t4 InterpolateBarycentrics(uint8_t4 a, uint8_t4 b, uint8_t4 c)
{
  const vec3 barycentrics = vec3(1.0f - HIT_ATTRIBUTE.x - HIT_ATTRIBUTE.y, HIT_ATTRIBUTE.x, HIT_ATTRIBUTE.y);
  return uint8_t4(uvec4(a) * barycentrics.x + uvec4(b) * barycentrics.y + uvec4(c) * barycentrics.z);
}

#else

//

#endif

void StorePayloadValue(vec4 color)
{
  payload.color_and_distance = color;
}

void StorePayloadValue(vec3 color)
{
  payload.color_and_distance.rgb = color;
}

struct RaysterizerVertex
{
  vec3 position;
  vec3 normal;
  vec2 tex_coord;
};

#ifdef CUSTOM_CODE

void RayCustomCode()
{
  RaysterizerVertex v;
#ifdef RAY_POSITION
  v.position = RAY_POSITION.xyz;
#endif
#ifdef RAY_NORMAL
  v.normal = RAY_NORMAL.xyz;
#endif
#ifdef RAY_COLOR_TEXTURE_COORDINATES
  v.tex_coord = RAY_COLOR_TEXTURE_COORDINATES.xy;
#endif

  vec3 object_color = Raysterizer_FRAG_COLOR.xyz;
  StorePayloadValue(object_color);
  payload.color_and_distance.w = gl_HitTEXT;
  payload.scatter_direction.w = 0;

  Material material;
  material.Diffuse = vec4(object_color.xyz, 1.0f);
  material.Fuzziness = 0;
  material.RefractionIndex = 0;
  material.MaterialModel = 0;

  uint depth = payload.depth;
  int material_index = RAYSTERIZER_DRAW_CALL_STATE.material_index;
  if (material_index == INSTANCE_CUSTOM_INDEX_MATERIAL_UNUSED)
  {
  }
  else
  {
    material = material_overrides[nonuniformEXT(material_index)].m;
    if (depth == 0 && material.highlight > 0)
    {
      StorePayloadValue(material.highlight_color);
      return;
    }
    if (material.MaterialModel == 4)
    {
      StorePayloadValue(material.highlight_color);
      return;
    }
  }
  material.Diffuse = vec4(object_color.xyz, 1.0f);

  payload = Scatter(material, gl_WorldRayDirectionEXT, v.normal, v.tex_coord, gl_HitTEXT, depth, payload.random_seed);
}

#endif

#endif

#endif