#include "ui/font_normal.h"
#include "ubo_types.h"

[[vk::binding(0, 1)]]
ConstantBuffer<FontUBOVertex> ubo: register(b0, space1);

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
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.tex_coord = get_tex_coords(input.index);
    output.position  = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 0.0, 1.0));
    return output;
}
