#include "common_ui.h"
#include "ubo_types.h"

[[vk::binding(0, 1)]]
ConstantBuffer<UBOVertex> ubo: register(b0, space1);

[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.tex_coord = input.tex_coord;
    output.diffuse   = input.diffuse;
    output.position  = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 1.0));
    return output;
}

[shader("fragment")]
float4 main_fs(FragmentInput input) : SV_Target0
{
    return input.diffuse * Texture.Sample(input.tex_coord);
}
