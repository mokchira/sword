#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform uboBuf 
{ 
	float time;
	float mouseX;
	float mouseY;
    int blur;
    float r;
    float g;
    float b;
    float a;
    float brushSize;
    int layerId;
} ubo;


layout (binding = 1) uniform sampler2D samplerColor[];

vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
	vec2 st = gl_FragCoord.xy / resolution;
	outColor = texture(samplerColor[ubo.layerId], st);
}
