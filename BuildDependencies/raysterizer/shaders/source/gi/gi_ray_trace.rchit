#version 460

#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#define RAY_TRACING
#define RAY_TRACING_RCHIT
#include "raysterizer/shaders/source/other/brdf.glsl"
#include "raysterizer/shaders/source/other/scene_descriptor_set.glsl"
#include "raysterizer/shaders/source/other/ray_query.glsl"
#include "raysterizer/shaders/source/gi/gi_common.glsl"
#define RAY_THROUGHPUT
//#define SAMPLE_SKY_LIGHT
#include "raysterizer/shaders/source/other/lighting.glsl"

// ------------------------------------------------------------------------
// PAYLOADS ---------------------------------------------------------------
// ------------------------------------------------------------------------

layout(location = 0) rayPayloadInEXT GIPayload p_Payload;

// ------------------------------------------------------------------------
// HIT ATTRIBUTE ----------------------------------------------------------
// ------------------------------------------------------------------------

hitAttributeEXT vec2 hit_attribs;

// ------------------------------------------------------------------------
// DESCRIPTOR SETS --------------------------------------------------------
// ------------------------------------------------------------------------

layout(set = 2, binding = 0) uniform PerFrameUBO
{
    mat4  view_inverse;
    mat4  proj_inverse;
    mat4  view_proj_inverse;
    mat4  prev_view_proj;
    mat4  view_proj;
    vec4  cam_pos;
    vec4  near_far;
    vec4  current_prev_jitter;
    Light light;
}
ubo;

layout(set = 3, binding = 0) uniform samplerCube s_Cubemap;
layout(set = 3, binding = 1) uniform sampler2D s_IrradianceSH;
layout(set = 3, binding = 2) uniform samplerCube s_Prefiltered;
layout(set = 3, binding = 3) uniform sampler2D s_BRDF;

layout(set = 4, binding = 0) uniform sampler2D s_Irradiance;
layout(set = 4, binding = 1) uniform sampler2D s_Depth;
layout(set = 4, binding = 2, scalar) uniform DDGIUBO
{
    DDGIUniforms ddgi;
};

// ------------------------------------------------------------------------
// PUSH CONSTANTS ---------------------------------------------------------
// ------------------------------------------------------------------------

layout(push_constant) uniform PushConstants
{
    mat4  random_orientation;
    uint  num_frames;
    uint  infinite_bounces;
    float gi_intensity;
}
u_PushConstants;

// ------------------------------------------------------------------------
// FUNCTIONS --------------------------------------------------------------
// ------------------------------------------------------------------------

vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// ----------------------------------------------------------------------------

vec3 indirect_lighting(vec3 Wo, vec3 N, vec3 P, vec3 F0, vec3 diffuse_color, float roughness, float metallic)
{
    vec3 F = fresnel_schlick_roughness(max(dot(N, Wo), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 sample_irr = sample_irradiance(ddgi, P, N, Wo, s_Irradiance, s_Depth);
    //return sample_irr;
    return u_PushConstants.gi_intensity * kD * diffuse_color * sample_irr;
}

// ------------------------------------------------------------------------
// MAIN -------------------------------------------------------------------
// ------------------------------------------------------------------------

void main()
{
    InstanceData instance_data = instance_datas[gl_InstanceCustomIndexEXT];
    Triangle triangle = fetch_triangle(instance_data, gl_PrimitiveID);
    Material material = materials[uint(instance_data.material_index)];

    const vec3 barycentrics = vec3(1.0 - hit_attribs.x - hit_attribs.y, hit_attribs.x, hit_attribs.y);

    Vertex vertex = interpolated_vertex(triangle, barycentrics);

    transform_vertex(instance_data, vertex);

    const vec3  albedo    = fetch_albedo(material, vertex.tex_coord.xy).rgb;
    const float roughness = fetch_roughness(material, vertex.tex_coord.xy);
    const float metallic  = fetch_metallic(material, vertex.tex_coord.xy);

    const vec3 N  = fetch_normal(material, vertex.tangent.xyz, vertex.tangent.xyz, vertex.normal.xyz, vertex.tex_coord.xy);
    const vec3 Wo = -gl_WorldRayDirectionEXT;
    const vec3 R  = reflect(-Wo, N);

    const vec3 F0        = mix(vec3(0.04f), albedo, metallic);
    const vec3 c_diffuse = mix(albedo * (vec3(1.0f) - F0), vec3(0.0f), metallic);

    vec3 Lo = vec3(0.0f);

    //Lo += direct_lighting(ubo.light, Wo, N, vertex.position.xyz, F0, c_diffuse, roughness, p_Payload.T, next_vec2(p_Payload.rng), s_Cubemap);
    Lo += direct_lighting(ubo.light, Wo, N, vertex.position.xyz, F0, c_diffuse, roughness, p_Payload.T);

    if (u_PushConstants.infinite_bounces == 1)
    {
        Lo += indirect_lighting(Wo, N, vertex.position.xyz, F0, c_diffuse, roughness, metallic);
    }

    //Lo = triangle.v0.position.xyz;
    p_Payload.L            = Lo;
    p_Payload.hit_distance = gl_RayTminEXT + gl_HitTEXT;
}

// ------------------------------------------------------------------------