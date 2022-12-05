#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#include "common.h"


// Based on Simple Sky Shader by robobo1221, see
// https://www.shadertoy.com/view/MsVSWt

vec3 skyMix(vec3 sunTone, vec3 skyTone, vec3 scatterTone, float scatterFactor, float powFactor)
{
    light_direction = normalize(light_direction);
    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);
    float y = abs(gl_WorldRayDirectionEXT.y + 1.5) / 3.0;

    float sun = 1.0 - distance(rayDir, light_direction);
    sun = clamp(sun, 0.0, 2.0);

    float glow = sun;
    glow = clamp(glow, 0.0, 1.0);

    sun = pow(sun, powFactor);
    sun *= 1000.0;
    sun = clamp(sun, 0.0, 16.0);

    glow = pow(glow, 6.0) * 1.0;
    glow = pow(glow, y);
    glow = clamp(glow, 0.0, 1.0);

    sun *= pow(dot(y, y), 1.0 / 1.65);

    glow *= pow(dot(y, y), 1.0 / 2.0);

    sun += glow;

    vec3 sunColor = sunTone * sun;

    float atmosphere = sqrt(1.0 - y);

    float scatter = pow(4 - light_direction.y, 1.0 / 15.0);
    scatter = 1.0 - clamp(scatter, 0.8, 1.0);

    vec3 scatterColor = mix(vec3(1.0), scatterTone * 1.5, scatter);
    vec3 skyScatter = mix(skyTone, vec3(scatterColor), atmosphere / scatterFactor);

    return sunColor + skyScatter;
}

void main()
{
    //payload.hit_value = vec3(0.0, 0.0, 0.2);
    //StorePayloadValue(rayterizer_info.clear_color.rgb);

    vec3 res = skyMix(vec3(1.0, 0.6, 0.05), vec3(0.2, 0.4, 0.8), vec3(1.0, 0.3, 0.0), 1.3, 80);

    payload.direct_color = res;
    payload.rough_value = vec4(res, 0);
    payload.depth = 10000.f;
}