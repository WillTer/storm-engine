struct VertexInput {
    float4 position: TEXCOORD0;
    uint   index: SV_VertexID;
};

struct VertexOutput {
    float2 tex_coord: TEXCOORD0;
    float2 tex_coord_next: TEXCOORD1;
    float4 position: SV_Position;
};

struct UBO {
    int32_t frame;
    int32_t h_frames_count;
    int32_t v_frames_count;
    int16_t flip_h;
    int16_t flip_v;
};

// Vertex shader sets
[[vk::binding(0, 1)]]
ConstantBuffer<UBO> ubo: register(b0, space1);

// Fragment shader sets
[[vk::binding(0, 2)]]
Sampler2D Texture: register(t0, space2) : register(s0, space2);

[[vk::binding(0, 3)]]
ConstantBuffer<float4> diffuse: register(b0, space3);

float2 get_tex_coords(int32_t frame, uint index)
{
    const int32_t frame_count   = ubo.h_frames_count * ubo.v_frames_count;
    const int32_t current_frame = frame >= frame_count ? 0 : frame;

    // Position of the current frame
    const float fx = current_frame % ubo.h_frames_count;
    const float fy = current_frame / ubo.h_frames_count;

    // Frame grid size
    const float x_count = ubo.h_frames_count;
    const float y_count = ubo.v_frames_count;

    const float x_left   = ubo.flip_h == 1 ? (fx + 1) / x_count : fx / x_count;
    const float x_right  = ubo.flip_h == 1 ? fx / x_count : (fx + 1) / x_count;
    const float y_top    = ubo.flip_v == 1 ? (fy + 1) / y_count : fy / y_count;
    const float y_bottom = ubo.flip_v == 1 ? fy / y_count : (fy + 1) / y_count;

    switch (index) {
    case 0: return float2(x_left, y_top);
    case 1: return float2(x_right, y_top);
    case 2: return float2(x_right, y_bottom);
    case 3: return float2(x_left, y_bottom);
    }

    return float2(0.0F, 0.0F);
}

[shader("vertex")]
VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    output.tex_coord      = get_tex_coords(ubo.frame, input.index);
    output.tex_coord_next = get_tex_coords(ubo.frame + 1, input.index);
    output.position       = input.position;
    return output;
}

[shader("fragment")]
float4 main_fs(float2 tex_coord: TEXCOORD0, float2 tex_coord_next: TEXCOORD1) : SV_Target0
{
    float4 tex1  = Texture.Sample(tex_coord);
    float4 tex2  = Texture.Sample(tex_coord_next);
    float4 color = (1 - diffuse.w) * tex1 + diffuse.w * tex2;
    color.w      = 1.0F;
    return color;
}
