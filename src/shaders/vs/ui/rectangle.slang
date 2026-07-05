#include "ui/rectangle.h"
#include "ubo_types.h"

[[vk::binding(0, 1)]]
ConstantBuffer<UBOVertex> ubo: register(b0, space1);

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.color    = input.color;
    output.position = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 0.0, 1.0));
    return output;
}
