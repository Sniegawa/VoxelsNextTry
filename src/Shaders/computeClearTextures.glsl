#version 460

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;
layout(r32f,    binding = 1) uniform image2D depthTex;

uniform ivec2 iResolution;

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= iResolution.x || pixel.y >= iResolution.y) return;

    imageStore(imgOutput, pixel, vec4(0.0, 0.0, 0.0, 1.0));
    imageStore(depthTex, pixel, vec4(1e30));
}
