#include "ubo_types.h"

[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<UBOFragment> fs_ubo: register(b0, space3);

[shader("fragment")]
float4 main(float2 tex_coord: TEXCOORD0, float2 tex_coord_next: TEXCOORD1) : SV_Target0
{
    float4 tex1  = Texture.Sample(tex_coord);
    float4 tex2  = Texture.Sample(tex_coord_next);
    float4 color = (1 - fs_ubo.diffuse.a) * tex1 + fs_ubo.diffuse.a * tex2;
    color.a      = 1.0F;
    return color;
}
