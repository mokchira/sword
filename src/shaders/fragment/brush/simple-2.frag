#version 460

#include "stroke.glsl"
#include "sdf.glsl"
#include "ubo.glsl"

layout(location = 0) out vec4 outColor;

struct PaintSample
{
    float x;
    float y;
};

layout(set = 0, binding = 2) uniform sampleArray
{
    int count;
    int null0;
    int null1;
    int null2;
    PaintSample samples[16];
} samples;

#define WIDTH 1600
#define HEIGHT 1600

vec4 over(vec4 A, vec4 B)
{
    float alpha = A.a + B.a * ( 1.0 - A.a );
    vec3 res = (A.rgb + B.rgb * ( 1.0 - A.a));
    return vec4(res, alpha);
}

void main()
{
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    for (int i = 0; i < samples.count; i++)
    {
        vec4 thisColor = vec4(0.);
        float x = samples.samples[i].x;
        float y = samples.samples[i].y;
        vec2 mCoords = vec2(x, y);
        thisColor += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .0025, 0.005);
        thisColor *= vec4(.8, .2, .5, 1);
        color = over(thisColor, color);
    }
//    if (samples.count == 1)
//    {
//        color.b = 1;
//    }
	outColor = color;
}
