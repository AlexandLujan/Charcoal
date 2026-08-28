Texture2D SceneTexture : register(t0);
SamplerState SceneSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput IN) : SV_Target
{
    float3 hdrColor =
        SceneTexture.Sample(
            SceneSampler,
            IN.TexCoord
        ).rgb;

    // Temporary simple tone mapping.
    float3 mapped =
        hdrColor / (hdrColor + 1.0f);

    return float4(mapped, 1.0f);
}