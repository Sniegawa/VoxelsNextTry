#version 460

in vec2 UV;

uniform sampler2D u_texture;

out vec4 Color;

void main()
{
	Color = texture(u_texture,UV);
}
