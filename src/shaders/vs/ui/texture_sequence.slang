#include "ui/texture_sequence.h"
#include "ubo_types.h"

[[vk::binding(0, 1)]]
ConstantBuffer<AnimationUBOVertex> ubo: register(b0, space1);

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
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.tex_coord      = get_tex_coords(ubo.frame, input.index);
    output.tex_coord_next = get_tex_coords(ubo.frame + 1, input.index);
    output.position       = float4(input.position, 0.0, 1.0);
    return output;
}
