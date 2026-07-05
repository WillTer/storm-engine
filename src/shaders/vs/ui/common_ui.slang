#include "ui/common_ui.h"
#include "ubo_types.h"

[[vk::binding(0, 1)]]
ConstantBuffer<UBOVertex> ubo: register(b0, space1);

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.tex_coord = input.tex_coord;
    output.diffuse   = input.diffuse;
    output.position  = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 1.0));
    return output;
}
