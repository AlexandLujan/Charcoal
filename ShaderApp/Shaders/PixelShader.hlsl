cbuffer DirectionalLightBuffer : register(b1)
{
    float4 LightDirection;
    float4 LightColor;
    
    float  Ambient;
    float  DiffuseIntensity;
    float2 Padding;
}

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float3 Color    : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 L = normalize(-LightDirection.xyz);

    // float3 lightDirection =
        // normalize(float3(-0.5f, -1.0f, 0.5f));

    float diffuse = max(0.0f, dot(N, L)) * DiffuseIntensity;

    // float ambient = 0.15f;

    float lighting =
        Ambient + diffuse;

    float3 finalColor =
        input.Color * LightColor.rgb * lighting;

    return float4(finalColor, 1.0f);
}