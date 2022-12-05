#version 460

#extension GL_GOOGLE_include_directive : require

#include "raysterizer/shaders/source/other/common.glsl"

// ------------------------------------------------------------------------
// INPUTS -----------------------------------------------------------------
// ------------------------------------------------------------------------

layout(location = 0) in vec2 inUV;

// ------------------------------------------------------------------------
// OUTPUTS ----------------------------------------------------------------
// ------------------------------------------------------------------------

layout(location = 0) out vec4 FS_OUT_Color;

// ------------------------------------------------------------------------
// DESCRIPTOR SETS --------------------------------------------------------
// ------------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D s_Color;

// ------------------------------------------------------------------------
// PUSH CONSTANTS ---------------------------------------------------------
// ------------------------------------------------------------------------

#define VISUALIZATION_TYPE_FINAL 0
#define VISUALIZATION_TYPE_SHADOWS 1
#define VISUALIZATION_TYPE_AMBIENT_OCCLUSION 2
#define VISUALIZATION_TYPE_REFLECTIONS 3
#define VISUALIZATION_TYPE_GLOBAL_ILLUIMINATION 4
#define VISUALIZATION_TYPE_GROUND_TRUTH 5
#define VISUALIZATION_TYPE_NORMALS 6

layout(push_constant) uniform PushConstants
{
    int   single_channel;
    float exposure;
    int visualization_type;
}
u_PushConstants;

// ------------------------------------------------------------------------
// FUNCTIONS --------------------------------------------------------------
// ------------------------------------------------------------------------

vec3 aces_film(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

// ------------------------------------------------------------------------
// MAIN -------------------------------------------------------------------
// ------------------------------------------------------------------------

void main()
{
    if (u_PushConstants.single_channel == 1)
    {
        FS_OUT_Color = vec4(texture(s_Color, inUV).rrr, 1.0);
    }
    else
    {
        vec3 color;
        if (u_PushConstants.visualization_type == VISUALIZATION_TYPE_NORMALS)
        {
            vec3 normal = octohedral_to_direction(texture(s_Color, inUV, 0.0).rg);
            color = normal;
        }
        else
        {
            color = texture(s_Color, inUV).rgb;
        }

        // Apply exposure
        color *= u_PushConstants.exposure;

        // HDR tonemap and gamma correct
        color = aces_film(color);
        color = pow(color, vec3(1.0 / 2.2));

        FS_OUT_Color = vec4(color, 1.0);
    }
}

// ------------------------------------------------------------------------