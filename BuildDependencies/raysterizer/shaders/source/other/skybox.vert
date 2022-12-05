#version 450

layout(location = 0) in vec3 VS_IN_Position;

// ------------------------------------------------------------------
// OUTPUT VARIABLES  ------------------------------------------------
// ------------------------------------------------------------------

layout(location = 0) out vec3 FS_IN_WorldPos;

// ------------------------------------------------------------------
// PUSH CONSTANTS ---------------------------------------------------
// ------------------------------------------------------------------

layout(push_constant) uniform PushConstants
{
    mat4 view_projection;
}
u_PushConstants;

// ------------------------------------------------------------------
// MAIN  ------------------------------------------------------------
// ------------------------------------------------------------------

void main()
{
    FS_IN_WorldPos = VS_IN_Position;

    //vec4 clipPos = u_PushConstants.view_projection * vec4(VS_IN_Position, 1.0);
    //gl_Position = clipPos;

    mat4 rotView = mat4(mat3(u_PushConstants.view_projection));
    vec4 clipPos = rotView * vec4(VS_IN_Position, 1.0);
    gl_Position = clipPos.xyww;
}