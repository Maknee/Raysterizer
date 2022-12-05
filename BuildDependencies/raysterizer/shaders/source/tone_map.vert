#version 460

layout(location = 0) out vec2 FS_IN_TexCoord;

void main()
{
    FS_IN_TexCoord = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    gl_Position = vec4((FS_IN_TexCoord * 2.0) - vec2(1.0), 0.0, 1.0);
}

