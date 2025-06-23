struct VSInput {
    float3 Position: TEXCOORD0;
    float2 TexCoord: TEXCOORD1;
};

struct VSOutput {
    float2 TexCoord: TEXCOORD0;
    float4 Position: SV_Position;
};

[[vk::binding(0, 2)]]
Sampler2D Sampler: register(t0, space2) : register(s0, space2);

[shader("vertex")]
VSOutput main_vs(VSInput input, uint VertexIndex: SV_VertexID)
{
    VSOutput output;
    output.TexCoord = input.TexCoord;
    output.Position = float4(input.Position, 1.0);
    return output;
}

[shader("fragment")]
float4 main_fs(float2 TexCoord: TEXCOORD0) : SV_Target0
{
    return Sampler.Sample(TexCoord);
}
