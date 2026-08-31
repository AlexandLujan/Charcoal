Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput IN) : SV_Target
{
    uint width;
    uint height;
    InputTexture.GetDimensions(width, height);

    float2 texelSize =
        float2(
            1.0f / width,
            1.0f / height
        );

    float3 color =
        InputTexture.Sample(
            InputSampler,
            IN.TexCoord
        ).rgb * 0.227027f;

    color +=
        InputTexture.Sample(
            InputSampler,
            IN.TexCoord + float2(0.0f, texelSize.y * 1.384615f)
        ).rgb * 0.316216f;

    color +=
        InputTexture.Sample(
            InputSampler,
            IN.TexCoord - float2(0.0f, texelSize.y * 1.384615f)
        ).rgb * 0.316216f;

    color +=
        InputTexture.Sample(
            InputSampler,
            IN.TexCoord + float2(0.0f, texelSize.y * 3.230769f)
        ).rgb * 0.070270f;

    color +=
        InputTexture.Sample(
            InputSampler,
            IN.TexCoord - float2(0.0f, texelSize.y * 3.230769f)
        ).rgb * 0.070270f;

    return float4(color, 1.0f);
}