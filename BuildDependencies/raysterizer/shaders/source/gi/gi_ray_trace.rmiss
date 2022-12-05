#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "raysterizer/shaders/source/other/common.glsl"

// ------------------------------------------------------------------------
// DESCRIPTOR SETS --------------------------------------------------------
// ------------------------------------------------------------------------

layout(set = 3, binding = 0) uniform samplerCube s_Cubemap;
layout(set = 3, binding = 1) uniform sampler2D s_IrradianceSH;
layout(set = 3, binding = 2) uniform samplerCube s_Prefiltered;
layout(set = 3, binding = 3) uniform sampler2D s_BRDF;


// ------------------------------------------------------------------------
// PAYLOADS ---------------------------------------------------------------
// ------------------------------------------------------------------------

layout(location = 0) rayPayloadInEXT GIPayload p_Payload;

// ------------------------------------------------------------------------
// MAIN -------------------------------------------------------------------
// ------------------------------------------------------------------------

void main()
{
    p_Payload.L = textureLod(s_Cubemap, gl_WorldRayDirectionEXT, 0.0f).rgb;
    //p_Payload.L = vec3(0.0);
}

// ------------------------------------------------------------------------