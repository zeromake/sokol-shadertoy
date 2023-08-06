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

// The MIT License
// Copyright © 2019 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Distance to an oriented box.

// List of some other 2D distances: https://www.shadertoy.com/playlist/MXdSRf
//
// and iquilezles.org/articles/distfunctions2d


float sdOrientedBox( in vec2 p, in vec2 a, in vec2 b, float th )
{
    float l = length(b-a);
    vec2  d = (b-a)/l;
    vec2  q = p-(a+b)*0.5;
          q = mat2(d.x,-d.y,d.y,d.x)*q;
          q = abs(q)-vec2(l*0.5,th);
    return length(max(q,0.0)) + min(max(q.x,q.y),0.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // normalized pixel coordinates
    vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;
    vec2 m = (2.0*iMouse.xy-iResolution.xy)/iResolution.y;
    p *= 1.4;
    m *= 1.4;

    // animation
    vec2 v1 = cos( iTime*0.5 + vec2(0.0,1.00) + 0.0 );
	vec2 v2 = cos( iTime*0.5 + vec2(0.0,3.00) + 1.5 );
    float th = 0.3*(0.5+0.5*cos(iTime*1.1+1.0));

	float d = sdOrientedBox( p, v1, v2, th );

    // distance
    vec3 col = vec3(1.0) - sign(d)*vec3(0.1,0.4,0.7);
	col *= 1.0 - exp(-6.0*abs(d));
	col *= 0.8 + 0.2*cos(120.0*d);
	col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.015,abs(d)) );

    if( iMouse.z>0.001 )
    {
    d = sdOrientedBox(m, v1, v2, th);
    col = mix(col, vec3(1.0,1.0,0.0), 1.0-smoothstep(0.0, 0.005, abs(length(p-m)-abs(d))-0.0025));
    col = mix(col, vec3(1.0,1.0,0.0), 1.0-smoothstep(0.0, 0.005, length(p-m)-0.015));
    }


	fragColor = vec4(col, 1.0);
}


layout(location = 0) out vec4 outColor;

void main()
{   vec2 fragCoord = gl_FragCoord.xy;
#if SOKOL_HLSL
    fragCoord.y = iResolution.y - fragCoord.y;
#endif
    mainImage(outColor, fragCoord);
}
@end
@program simple vs fs
