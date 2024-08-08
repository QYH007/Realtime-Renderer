#version 410 core

out vec4 color;

void main()
{
    float depth = gl_FragCoord.z;
    float depth_2 = depth*depth;
    color = vec4(depth, depth_2, 0.0, 1.0);
}
