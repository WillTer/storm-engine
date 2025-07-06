#include "font_normal.h"
#include "ubo_types.h"

// Vertex shader sets
[[vk::binding(0, 1)]]
ConstantBuffer<FontUBOVertex> ubo: register(b0, space1);

// Fragment shader sets
[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<UBOFragment> fs_ubo: register(b0, space3);

float2 get_tex_coords(uint index)
{
    switch (index) {
    case 0: return float2(ubo.tex_coords.x, ubo.tex_coords.y);
    case 1: return float2(ubo.tex_coords.z, ubo.tex_coords.y);
    case 2: return float2(ubo.tex_coords.z, ubo.tex_coords.w);
    case 3: return float2(ubo.tex_coords.x, ubo.tex_coords.w);
    }

    return float2(0.0F, 0.0F);
}

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.tex_coord      = get_tex_coords(input.index);
    output.position       = mul(mul(ubo.view_proj, ubo.model), input.position);
    return output;
}

[shader("fragment")]
float4 main_fs(float2 tex_coord: TEXCOORD0) : SV_Target0
{
    return fs_ubo.color * Texture.Sample(tex_coord);
}
