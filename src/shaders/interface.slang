struct VertexInput {
    float3 position: TEXCOORD0;
    float2 tex_coord: TEXCOORD1;
};

struct VertexOutput {
    float2 tex_coord: TEXCOORD0;
    float4 position: SV_Position;
};

struct UBO {
    float4x4 model;
    float4x4 view_proj;
};

[[vk::binding(0, 1)]]
ConstantBuffer<UBO> ubo: register(b0, space1);

[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.tex_coord = input.tex_coord;
    // output.position  = mul(mul(float4(input.position, 1.0), ubo.model), ubo.view_proj);
    output.position = mul(mul(ubo.view_proj, ubo.model), float4(input.position, 1.0));
    return output;
}

[shader("fragment")]
float4 main_fs(float2 TexCoord: TEXCOORD0) : SV_Target0
{
    return Texture.Sample(TexCoord);
}
