Texture2D SceneTexture  : register(t0);
Texture2D BloomTexture  : register(t1);

SamplerState SceneSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput IN) : SV_Target
{
    float3 sceneColor =
        SceneTexture.Sample(
            SceneSampler,
            IN.TexCoord
        ).rgb;

    float3 bloomColor =
        BloomTexture.Sample(
            SceneSampler,
            IN.TexCoord
        ).rgb;

    float3 hdrColor =
        sceneColor + bloomColor;

    float3 mapped =
        hdrColor / (hdrColor + 1.0f);

    return float4(mapped, 1.0f);
}