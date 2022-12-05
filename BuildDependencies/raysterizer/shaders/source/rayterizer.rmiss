#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#define RT_SHADER
#include "common.h"

void main()
{
	if (raysterizer_info.has_sky)
	{
		// Sky color
		const float t = 0.5*(normalize(gl_WorldRayDirectionEXT).y + 1);
		const vec3 skyColor = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t);

		payload.color_and_distance = vec4(skyColor, -1);
	}
	else
	{
		payload.color_and_distance = vec4(0, 0, 0, -1);
	}
}