#version 460

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 UV;

void main()
{
	gl_Position = vec4(a_pos,1.0);
	UV = a_uv;
}