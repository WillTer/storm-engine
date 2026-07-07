#include "ubo_types.h"

[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<UBOFragment> fs_ubo: register(b0, space3);

[shader("fragment")]
float4 main(float2 tex_coords: TEXCOORD0) : SV_Target0
{
    return fs_ubo.diffuse * Texture.Sample(tex_coords);
}
