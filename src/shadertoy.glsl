@vs vs
layout(location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}

@end

@fs fs
layout(std140,binding=1) uniform frag {
    uniform vec4      iResolution;              // viewport resolution (in pixels)
    uniform vec4      iChannelTime;             // channel playback time (in seconds)
    uniform vec4      iChannelResolution[10];   // channel resolution (in pixels)
    uniform vec4      iMouse;                   // mouse pixel coords. xy: current (if MLB down), zw: click
    uniform vec4      iDate;                    // (year, month, day, time in seconds)
    uniform float     iSampleRate;              // sound sample rate (i.e., 44100)
    uniform float     iTime;                    // shader playback time (in seconds)
    uniform float     iTimeDelta;               // render time (in seconds)
    uniform float     iFrameRate;               // shader frame rate
    uniform int       iFrame;                   // shader playback frame
};


layout(binding=2) uniform texture2D iChannel0;
layout(binding=3) uniform sampler smp0;
layout(binding=4) uniform texture2D iChannel1;
layout(binding=5) uniform sampler smp1;
layout(binding=6) uniform texture2D iChannel2;
layout(binding=7) uniform sampler smp2;
layout(binding=8) uniform texture2D iChannel3;
layout(binding=9) uniform sampler smp3;
layout(binding=10) uniform texture2D iChannel4;
layout(binding=11) uniform sampler smp4;
layout(binding=12) uniform texture2D iChannel5;
layout(binding=13) uniform sampler smp5;
layout(binding=14) uniform texture2D iChannel6;
layout(binding=15) uniform sampler smp6;
layout(binding=16) uniform texture2D iChannel7;
layout(binding=17) uniform sampler smp7;
layout(binding=18) uniform texture2D iChannel8;
layout(binding=19) uniform sampler smp8;
layout(binding=20) uniform texture2D iChannel9;
layout(binding=21) uniform sampler smp9;

@include remote.glsl.in

layout(location = 0) out vec4 outColor;

void main()
{   vec2 fragCoord = gl_FragCoord.xy;
#if !SOKOL_GLSL
    // hlsl msl 的 y 轴需要翻转才能与 glsl 一致
    fragCoord.y = iResolution.y - fragCoord.y;
#endif
    mainImage(outColor, fragCoord);
}
@end
@program simple vs fs
