﻿#version 450 core

uniform sampler2D currentColor;
uniform sampler2D previousColor;
uniform sampler2D velocityTexture;
uniform sampler2D currentDepth;

uniform int ScreenWidth;
uniform int ScreenHeight;
uniform int frameCount;

in  vec2 screenPosition;
out vec4 outColor;

vec3 RGB2YCoCgR(vec3 rgbColor)
{
    vec3 YCoCgRColor;

    YCoCgRColor.y = rgbColor.r - rgbColor.b;
    float temp = rgbColor.b + YCoCgRColor.y / 2;
    YCoCgRColor.z = rgbColor.g - temp;
    YCoCgRColor.x = temp + YCoCgRColor.z / 2;

    return YCoCgRColor;
}

vec3 YCoCgR2RGB(vec3 YCoCgRColor)
{
    vec3 rgbColor;

    float temp = YCoCgRColor.x - YCoCgRColor.z / 2;
    rgbColor.g = YCoCgRColor.z + temp;
    rgbColor.b = temp - YCoCgRColor.y / 2;
    rgbColor.r = rgbColor.b + YCoCgRColor.y;

    return rgbColor;
}

float Luminance(vec3 color)
{
    //return 0.25 * color.r + 0.5 * color.g + 0.25 * color.b;
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    
}

vec3 ToneMap(vec3 color)
{
    return color / (1 + Luminance(color));
}

vec3 UnToneMap(vec3 color)
{
    return color / (1 - Luminance(color));
}

vec3 clipAABB(vec3 nowColor, vec3 preColor)
{
    vec3 aabbMin = nowColor, aabbMax = nowColor;
    vec2 deltaRes = vec2(1.0 / ScreenWidth, 1.0 / ScreenHeight);
    vec3 m1 = vec3(0), m2 = vec3(0);

    for(int i=-1;i<=1;++i)
    {
        for(int j=-1;j<=1;++j)
        {
            vec2 newUV = screenPosition + deltaRes * vec2(i, j);
            vec3 C = RGB2YCoCgR(texture(currentColor, newUV).rgb);
            m1 += C;
            m2 += C * C;
        }
    }

    // Variance clip
    const int N = 9;
    const float VarianceClipGamma = 1.0f;
    vec3 mu = m1 / N;
    vec3 sigma = sqrt(abs(m2 / N - mu * mu));
    aabbMin = mu - VarianceClipGamma * sigma;
    aabbMax = mu + VarianceClipGamma * sigma;

    // clip to center
    vec3 p_clip = 0.5 * (aabbMax + aabbMin);
    vec3 e_clip = 0.5 * (aabbMax - aabbMin);

    vec3 v_clip = preColor - p_clip;
    vec3 v_unit = v_clip.xyz / e_clip;
    vec3 a_unit = abs(v_unit);
    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

    if (ma_unit > 1.0)
        return p_clip + v_clip / ma_unit;
    else
        return preColor;
}


vec2 getClosestOffset()
{
    vec2 deltaRes = vec2(1.0 / ScreenWidth, 1.0 / ScreenHeight);
    float closestDepth = 1.0f;
    vec2 closestUV = screenPosition;

    for(int i=-1;i<=1;++i)
    {
        for(int j=-1;j<=1;++j)
        {
            vec2 newUV = screenPosition + deltaRes * vec2(i, j);

            float depth = texture2D(currentDepth, newUV).a;

            if(depth < closestDepth)
            {
                closestDepth = depth;
                closestUV = newUV;
            }
        }
    }

    return closestUV;
}

void main()
{
    vec3 nowColor = texture(currentColor, screenPosition).rgb;

    if(frameCount == 0)
    {
        outColor = vec4(nowColor, 1.0);
        return;
    }

    // 周围3x3内距离最近的速度向量
    vec2 velocity = texture(velocityTexture, getClosestOffset()).rg;
    vec2 offsetUV = clamp(screenPosition - velocity, 0, 1);
    vec3 preColor = texture(previousColor, offsetUV).rgb;

    // 只有jitter
    //vec3 preColor = texture(previousColor, screenPosition).rgb;

    // tonemap and clip 
    nowColor = RGB2YCoCgR(nowColor);
    preColor = RGB2YCoCgR(preColor);

    preColor = clipAABB(nowColor, preColor);

    //preColor = UnToneMap(YCoCgR2RGB(preColor));
    //nowColor = UnToneMap(YCoCgR2RGB(nowColor));
    preColor = YCoCgR2RGB(preColor);
    nowColor = YCoCgR2RGB(nowColor);

    // 混合
    float c = 0.05;
    outColor = vec4((c * nowColor + (1-c) * preColor), 1.0);
}