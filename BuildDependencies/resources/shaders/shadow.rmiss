#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#include "common.h"

void main()
{
    payload.direct_color = vec3(1.0, 1.0, 1.0);
   //shadow = false;
}