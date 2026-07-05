[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[shader("fragment")]
float4 main(float2 tex_coord: TEXCOORD0, float4 diffuse: TEXCOORD1) : SV_Target0
{
    return diffuse * Texture.Sample(tex_coord);
}
