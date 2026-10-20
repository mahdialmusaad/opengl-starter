#version 330

in vec2 texture_position;
uniform sampler2D our_texture;

out vec4 final_colour;

void main()
{
	final_colour = texture(our_texture, texture_position);
}
