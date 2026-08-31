Texture2D SceneTexture : register(t0);
Texture2D BloomTexture : register(t1);

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

    const float bloomStrength = 2.0f;

    // Composite the original HDR scene with the blurred bloom texture.
    float3 hdrColor =
        sceneColor + bloomColor * bloomStrength;

    // Exposure adjustment.
    const float exposure = 0.25f;
    hdrColor *= exposure;

    // Reinhard tone mapping.
    float3 mapped =
        hdrColor / (hdrColor + 1.0f);

    // Linear -> sRGB.
    float3 low =
        12.92f * mapped;

    float3 high =
        1.055f * pow(
            mapped,
            1.0f / 2.4f
        ) - 0.055f;

    float3 useHigh =
        step(
            0.0031308f,
            mapped
        );

    float3 srgb =
        lerp(
            low,
            high,
            useHigh
        );

    return float4(
        srgb,
        1.0f
    );
}