#version 330

in vec3 vcolor;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(vcolor, 1.0);
}