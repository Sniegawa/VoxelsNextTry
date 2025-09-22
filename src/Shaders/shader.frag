#version 460

in vec2 UV;

uniform sampler2D u_texture;

out vec4 Color;

void main()
{
	Color = vec4(UV,0.0f,1.0f);//texture(u_texture,UV);	
}
