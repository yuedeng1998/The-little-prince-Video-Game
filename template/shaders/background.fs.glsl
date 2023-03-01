#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float dead_timer;

in vec2 uv;

layout(location = 0) out vec4 color;

void main()
{
	vec2 coord = uv;

    vec4 in_color = texture(screen_texture, coord);
    color = in_color;
}