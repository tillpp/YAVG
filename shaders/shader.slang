
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
    float3 pos;
};

[shader("vertex")]
VertexOutput vertMain(VSInput input,uint vid : SV_VertexID,uint iid :SV_InstanceID) {
    VertexOutput output;
    output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition , 1.0))));
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    output.pos = input.inPosition;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;

    float3 dx = ddx(inVert.pos);
    float3 dy = ddy(inVert.pos);
    float3 normal = normalize(cross(dx, dy));

    float invsqrt2 = 0.70710678118;
    if (normal.y > invsqrt2) {
        color *= 0.5;
    } else if (normal.y < -invsqrt2) {
        color *= 1.0;
    } else if (normal.x < -invsqrt2 || normal.x > invsqrt2){
        color *= 0.6;
    } else {
        color *= 0.8;
    }

    //return float4(color,1);
    return texture.Sample(inVert.fragTexCoord);
}