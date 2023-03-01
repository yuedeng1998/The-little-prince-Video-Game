#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec2 offset;
//uniform offset;

// Output color
layout(location = 0) out  vec4 color;
//vec2 offset;

void main()
{
	//offset = vec2(0.0,1.0/2.0);
	color = vec4(fcolor, 1.0) * texture(sampler0, offset + vec2(texcoord.x, texcoord.y));
}
