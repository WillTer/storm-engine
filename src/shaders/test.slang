struct VSInput {
    float3 Position: TEXCOORD0;
    float2 UV: TEXCOORD1;
};

struct VSOutput {
    float4 Position: SV_Position;
    float4 Color: TEXCOORD0;
};

[shader("vertex")]
VSOutput main_vs(VSInput input, uint VertexIndex: SV_VertexID)
{
    VSOutput output;
    output.Color    = float4(input.UV, 1.0, 1.0); // TODO
    output.Position = float4(input.Position, 1.0);
    return output;
}

[shader("pixel")]
float4 main_ps(VSOutput input) : SV_Target0
{
    return input.Color;
}
