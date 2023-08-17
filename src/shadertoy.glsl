@vs vs
layout(location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}

@end

@fs fs
layout(std140,binding=1) uniform frag {
    uniform vec4      iResolution;           // viewport resolution (in pixels)
    uniform vec4      iChannelTime;          // channel playback time (in seconds)
    uniform vec4      iChannelResolution[4]; // channel resolution (in pixels)
    uniform vec4      iMouse;               // mouse pixel coords. xy: current (if MLB down), zw: click
    uniform vec4      iDate;                 // (year, month, day, time in seconds)
    uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
    uniform float     iTime;                 // shader playback time (in seconds)
    uniform float     iTimeDelta;            // render time (in seconds)
    uniform float     iFrameRate;            // shader frame rate
    uniform int       iFrame;                // shader playback frame
};


layout(binding=2) uniform texture2D iChannel0;
layout(binding=3) uniform sampler smp0;
layout(binding=4) uniform texture2D iChannel1;
layout(binding=5) uniform sampler smp1;
layout(binding=6) uniform texture2D iChannel2;
layout(binding=7) uniform sampler smp2;
layout(binding=8) uniform texture2D iChannel3;
layout(binding=9) uniform sampler smp3;

@include remote.glsl.in

layout(location = 0) out vec4 outColor;

void main()
{   vec2 fragCoord = gl_FragCoord.xy;
#if !SOKOL_GLSL
    fragCoord.y = iResolution.y - fragCoord.y;
#endif
    mainImage(outColor, fragCoord);
}
@end
@program simple vs fs
