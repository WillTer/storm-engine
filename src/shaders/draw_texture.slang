struct VertexInput {
    float4 position_uv: TEXCOORD0;  // x,y - vertex position; z,w - texture coords
};

struct VertexOutput {
    float2 tex_coord: TEXCOORD0;
    float4 position: SV_Position;
};

// Vertex shader sets
[[vk::binding(0, 1)]]
ConstantBuffer<float4x4> view_proj: register(b0, space1);

// Fragment shader sets
[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<float4> diffuse: register(b0, space3);

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.tex_coord = input.position_uv.zw;
    output.position  = mul(view_proj, float4(input.position_uv.xy, 0.0, 1.0));
    return output;
}

[shader("fragment")]
float4 main_fs(float2 tex_coord: TEXCOORD0) : SV_Target0
{
    return diffuse * Texture.Sample(tex_coord);
}
