Texture2D SceneTexture : register(t0);
SamplerState SceneSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput IN) : SV_Target
{
    float3 color =
        SceneTexture.Sample(
            SceneSampler,
            IN.TexCoord
        ).rgb;

    // Perceived brightness.
    float brightness =
        dot(
            color,
            float3(
                0.2126f,
                0.7152f,
                0.0722f
            )
        );

    const float threshold = 0.20f;

    float3 brightColor =
        brightness > threshold
        ? color
        : float3(0.0f, 0.0f, 0.0f);

    return float4(brightColor, 1.0f);
}