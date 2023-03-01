#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// uniform frame {width, height}
//		how many frames in the width, how many in the height
// uniform offset
//		add offset divided by width and height
// maybe another uniform to keep track of which state the prince is in?
//		so they know which row which frame correlates to?

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	// 
	float dimension_scale = 0.55;
	float x_offset = 100;
	float y_offset = 0;
	
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
