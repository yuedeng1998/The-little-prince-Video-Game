#version 330 

layout(location = 0) in vec4 in_position;

out vec2 uv;

void main()
{
   // gl_Position = in_position;
gl_Position = vec4(in_position.xyz, 1.0);
    // Convert to the [0, 1] range of UV coordinate
    uv = (in_position.xy + vec2(1.05, 1.05)) / 2.1;
    
}
