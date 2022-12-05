#version 460

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTex;

layout(location = 0) out vec4 outFragColor;
layout(set = 2, binding = 0) uniform sampler2DArray tex;

layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
	float texture_index;
} PushConstants;

void main() 
{
	outFragColor = vec4(inColor,1.0f);

	vec3 color = texture(tex, vec3(inTex.xy, PushConstants.texture_index)).xyz;
	//vec3 color = texture(tex, vec3(inTex.xy, 5.0)).xyz;
	outFragColor = vec4(color,1.0f);
}
