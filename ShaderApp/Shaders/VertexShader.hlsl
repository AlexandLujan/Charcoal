cbuffer MVPBuffer : register(b0)
{
    matrix MVP;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Color    : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float3 Color    : COLOR;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.Position = mul(float4(input.Position, 1.0f), MVP);
    output.Normal = input.Normal;
    output.Color = input.Color;
    return output;
}