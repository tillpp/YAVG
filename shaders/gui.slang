
struct VSInput {
    float3 inPosition;
    float3 inColor;
    float2 inTexCoord;
};

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 color;
    float2 fragTexCoord;
};

[shader("vertex")]
VertexOutput vertMain(VSInput input,uint vid : SV_VertexID,uint iid :SV_InstanceID) {
    VertexOutput output;
    output.sv_position = float4(input.inPosition.xz ,0.0, 1.0);
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

//Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;

    return float4(color,1);
    //return texture.Sample(inVert.fragTexCoord);
}