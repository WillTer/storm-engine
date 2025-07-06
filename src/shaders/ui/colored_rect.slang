#include "colored_rect.h"
#include "ubo_types.h"

// Vertex shader sets
[[vk::binding(0, 1)]]
ConstantBuffer<UBOVertex> ubo: register(b0, space1);

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.position = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 0.0, 1.0));
    output.color    = input.color;
    return output;
}

[shader("fragment")]
float4 main_fs(float4 color: TEXCOORD0) : SV_Target0
{
    return color;
}
