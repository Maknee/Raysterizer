#version 450

layout(location = 0) out vec4 FS_OUT_Color;

layout(location = 0) in vec3 FS_IN_WorldPos;

// ------------------------------------------------------------------
// DESCRIPTOR SETS --------------------------------------------------
// ------------------------------------------------------------------

layout(set = 0, binding = 0) uniform samplerCube s_Cubemap;
layout(set = 0, binding = 1) uniform sampler2D s_IrradianceSH;
layout(set = 0, binding = 2) uniform samplerCube s_Prefiltered;
layout(set = 0, binding = 3) uniform sampler2D s_BRDF;

// ------------------------------------------------------------------
// MAIN -------------------------------------------------------------
// ------------------------------------------------------------------

void main()
{
    vec3 env_color = texture(s_Cubemap, FS_IN_WorldPos).rgb;
    FS_OUT_Color   = vec4(env_color, 1.0f);
}

// ------------------------------------------------------------------