
struct VSInput {
    float3 inPosition;
    float3 inColor;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 color;
    float2 fragTexCoord;
};
struct PushConstants {
    float2 pos;
    float2 size;
};
[[vk::push_constant]] PushConstants pc;

[shader("vertex")]
VertexOutput vertMain(VSInput input,uint vid : SV_VertexID,uint iid :SV_InstanceID) {
    VertexOutput output;
    output.sv_position = float4(float3(input.inPosition.xz * pc.size.xy * 2 + pc.pos.xy*2 - float2(1), 0), 1.0);
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = texture.Sample(inVert.fragTexCoord).xyz; // inVert.color
    return float4(float3(1),color.r);
}