#version 460

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTex;

layout(location = 0) out vec4 outFragColor;
layout(set = 2, binding = 0) uniform sampler2D tex;

void main() 
{
	outFragColor = vec4(inColor,1.0f);

	vec3 color = texture(tex, inTex).xyz;
	outFragColor = vec4(color,1.0f);
}
