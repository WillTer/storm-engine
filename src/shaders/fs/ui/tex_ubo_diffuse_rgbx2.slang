#include "ubo_types.h"

[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<UBOFragment> fs_ubo: register(b0, space3);

[shader("fragment")]
float4 main(float2 tex_coord: TEXCOORD0) : SV_Target0
{
    float4 tex_color = Texture.Sample(tex_coord);
    // Same as modulate2x from fx
    float3 final_color = fs_ubo.diffuse.rgb * tex_color.rgb * 2;
    float  final_alpha = fs_ubo.diffuse.a * tex_color.a;
    return float4(final_color, final_alpha);
}
