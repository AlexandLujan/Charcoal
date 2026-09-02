// clang-format off
struct PixelShaderInput
{
    float4 PositionVS  : POSITION;
    float3 NormalVS    : NORMAL;
    float3 TangentVS   : TANGENT;
    float3 BitangentVS : BITANGENT;
    float2 TexCoord    : TEXCOORD;
};

struct Material
{
    float4 Diffuse;
    //------------------------------------ ( 16 bytes )
    float4 Specular;
    //------------------------------------ ( 16 bytes )
    float4 Emissive;
    //------------------------------------ ( 16 bytes )
    float4 Ambient;
    //------------------------------------ ( 16 bytes )
    float4 Reflectance;
    //------------------------------------ ( 16 bytes )
    float Opacity;            // If Opacity < 1, then the material is transparent.
    float SpecularPower;
    float IndexOfRefraction;  // For transparent materials, IOR > 0.
    float BumpIntensity;      // When using bump textures (height maps) we need
                              // to scale the height values so the normals are visible.
    //------------------------------------ ( 16 bytes )
    bool  HasAmbientTexture;
    bool  HasEmissiveTexture;
    bool  HasDiffuseTexture;
    bool  HasSpecularTexture;
    //------------------------------------ ( 16 bytes )
    bool  HasSpecularPowerTexture;
    bool  HasNormalTexture;
    bool  HasBumpTexture;
    bool  HasOpacityTexture;
    //------------------------------------ ( 16 bytes )
    bool HasRoughnessTexture;
    bool RoughnessPadding0;
    bool RoughnessPadding1;
    bool RoughnessPadding2;
    //------------------------------------ ( 16 bytes )
    bool HasAmbientOcclusionTexture;
    bool AmbientOcclusionPadding0;
    bool AmbientOcclusionPadding1;
    bool AmbientOcclusionPadding2;
    //------------------------------------ ( 16 bytes )
    // Total:                              ( 16 * 10 = 160 bytes )
};

#if ENABLE_LIGHTING
struct PointLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float  Ambient;
    float  DiffuseIntensity;
    float  SpecularIntensity;

    float  ConstantAttenuation;
    float  LinearAttenuation;
    float  QuadraticAttenuation;
    float2 Padding;
    //----------------------------------- (32 byte boundary)
    // Total:                              16 * 3 + 32 = 80 bytes
};

struct SpotLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float  Ambient;
    float  DiffuseIntensity;
    float  SpecularIntensity;
    float  SpotAngle;
    float  ConstantAttenuation;
    float  LinearAttenuation;
    float  QuadraticAttenuation;
    float  Padding;
    //----------------------------------- (32 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

struct DirectionalLight
{
    float4 DirectionWS;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float Ambient;
    float DiffuseIntensity;
    float SpecularIntensity;
    float Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
    uint NumDirectionalLights;
    uint Padding;
    
    float4 AmbientLightColor;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
};

ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

StructuredBuffer<PointLight> PointLights : register( t0 );
StructuredBuffer<SpotLight> SpotLights : register( t1 );
StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 );
#endif // ENABLE_LIGHTING

ConstantBuffer<Material> MaterialCB : register(b0, space1);

// Textures
Texture2D AmbientTexture              : register( t3 );
Texture2D EmissiveTexture             : register( t4 );
Texture2D DiffuseTexture              : register( t5 );
Texture2D SpecularTexture             : register( t6 );
Texture2D SpecularPowerTexture        : register( t7 );
Texture2D NormalTexture               : register( t8 );
Texture2D BumpTexture                 : register( t9 );
Texture2D OpacityTexture              : register( t10 );
Texture2D RoughnessTexture            : register( t11 );
Texture2D AmbientOcclusionTexture     : register( t12 );

SamplerState TextureSampler    : register(s0);

float3 LinearToSRGB( float3 x )
{
    float3 result;
    for (int i = 0; i < 3; ++i)
    {
        // This is exactly the sRGB curve
        //result[i] = x[i] < 0.0031308 ? 12.92 * x[i] : 1.055 * pow(abs(x[i]), 1.0 / 2.4) - 0.055;
        // This is cheaper but nearly equivalent
        //result[i] = (x[i] < 0.0031308) ? (12.92 * x[i]) : (1.13005 * sqrt(abs(x[i] - 0.00228)) - 0.13448 * x[i] + 0.005719);
        if (x[i] < 0.0031308)
            result[i] = (12.92 * x[i]);
        else
            result[i] = (1.13005 * sqrt(abs(x[i] - 0.00228)) - 0.13448 * x[i] + 0.005719);
    }
    return result;
}
#if ENABLE_LIGHTING
float DoDiffuse( float3 N, float3 L )
{
    return max( 0, dot( N, L ) );
}

float DoSpecular( float3 V, float3 N, float3 L, float specularPower )
{
    float3 R = normalize( reflect( -L, N ) );
    float RdotV = max( 0, dot( R, V ) );

    return pow( RdotV, specularPower );
}

float DoAttenuation( float c, float l, float q, float d )
{
    return 1.0f / ( c + l * d + q * d * d );
}

float DoSpotCone( float3 spotDir, float3 L, float spotAngle )
{
    float minCos = cos( spotAngle );
    float maxCos = ( minCos + 1.0f ) / 2.0f;
    float cosAngle = dot( spotDir, -L );
    return smoothstep( minCos, maxCos, cosAngle );
}

LightResult DoPointLight( PointLight light, float3 V, float3 P, float3 N, float specularPower )
{
    LightResult result;
    float3 L = ( light.PositionVS.xyz - P );
    float d = length(L);
    d = max(d, 0.0001f);
    L = L / d;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    result.Diffuse = DoDiffuse( N, L ) * attenuation * light.DiffuseIntensity * light.Color;
    result.Specular = DoSpecular( V, N, L, specularPower ) * attenuation * light.SpecularIntensity * light.Color;
    result.Ambient = LightPropertiesCB.AmbientLightColor * light.Color * light.Ambient * attenuation;

    return result;
}

LightResult DoSpotLight( SpotLight light, float3 V, float3 P, float3 N, float specularPower )
{
    LightResult result;
    float3 L = ( light.PositionVS.xyz - P );
    float d = length(L);
    d = max(d, 0.0001f);
    L = L / d;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    float spotIntensity = DoSpotCone( light.DirectionVS.xyz, L, light.SpotAngle );

    result.Diffuse = DoDiffuse( N, L ) * attenuation * spotIntensity * light.DiffuseIntensity * light.Color;
    result.Specular = DoSpecular( V, N, L, specularPower ) * attenuation * spotIntensity * light.SpecularIntensity * light.Color;
    result.Ambient = LightPropertiesCB.AmbientLightColor * light.Color * light.Ambient * attenuation * spotIntensity;

    return result;
}

LightResult DoDirectionalLight( DirectionalLight light, float3 V, float3 P, float3 N, float specularPower )
{
    LightResult result;

    float3 L = normalize( -light.DirectionVS.xyz );

    result.Diffuse = light.Color * DoDiffuse( N, L ) * light.DiffuseIntensity;
    result.Specular = light.Color * DoSpecular( V, N, L, specularPower ) * light.SpecularIntensity;
    result.Ambient = LightPropertiesCB.AmbientLightColor * light.Color * light.Ambient;

    return result;
}

LightResult DoLighting( float3 P, float3 N, float specularPower )
{
    uint i;

    // Lighting is performed in view space.
    float3 V = normalize( -P );

    LightResult totalResult = (LightResult)0;

    // Iterate point lights.
    for ( i = 0; i < LightPropertiesCB.NumPointLights; ++i )
    {
        LightResult result = DoPointLight( PointLights[i], V, P, N, specularPower );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    // Iterate spot lights.
    for ( i = 0; i < LightPropertiesCB.NumSpotLights; ++i )
    {
        LightResult result = DoSpotLight( SpotLights[i], V, P, N, specularPower );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    // Iterate directinal lights
    for (i = 0; i < LightPropertiesCB.NumDirectionalLights; ++i)
    {
        LightResult result = DoDirectionalLight( DirectionalLights[i], V, P, N, specularPower );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    // totalResult.Diffuse = saturate( totalResult.Diffuse );
    totalResult.Specular = saturate( totalResult.Specular );
    // totalResult.Ambient = saturate( totalResult.Ambient );

    return totalResult;
}
#endif // ENABLE_LIGHTING

float3 ExpandNormal( float3 n )
{
    return n * 2.0f - 1.0f;
}

float3 DoNormalMapping( float3x3 TBN, Texture2D tex, float2 uv )
{
    float3 N = tex.Sample( TextureSampler, uv ).xyz;
    N = ExpandNormal( N );

    // Transform normal from tangent space to view space.
    N = mul( N, TBN );
    return normalize(N);
}

float3 DoBumpMapping( float3x3 TBN, Texture2D tex, float2 uv, float bumpScale )
{
    // Sample the heightmap at the current texture coordinate.
    float height_00 = tex.Sample( TextureSampler, uv ).r * bumpScale;
    // Sample the heightmap in the U texture coordinate direction.
    float height_10 = tex.Sample( TextureSampler, uv, int2( 1, 0 ) ).r * bumpScale;
    // Sample the heightmap in the V texture coordinate direction.
    float height_01 = tex.Sample( TextureSampler, uv, int2( 0, 1 ) ).r * bumpScale;

    float3 p_00 = { 0, 0, height_00 };
    float3 p_10 = { 1, 0, height_10 };
    float3 p_01 = { 0, 1, height_01 };

    // normal = tangent x bitangent
    float3 tangent = normalize( p_10 - p_00 );
    float3 bitangent = normalize( p_01 - p_00 );

    float3 normal = cross( tangent, bitangent );

    // Transform normal from tangent space to view space.
    normal = mul( normal, TBN );

    return normal;
}

float3 DoNormalBumpMapping(
    float3x3 TBN,
    Texture2D normalTex,
    Texture2D bumpTex,
    float2 uv,
    float bumpScale
)
{
    float3 normalTS =
        ExpandNormal(
            normalTex.Sample(
                TextureSampler,
                uv
            ).xyz
        );

    float height00 =
        bumpTex.Sample(
            TextureSampler,
            uv
        ).r;

    float height10 =
        bumpTex.Sample(
            TextureSampler,
            uv,
            int2(1, 0)
        ).r;

    float height01 =
        bumpTex.Sample(
            TextureSampler,
            uv,
            int2(0, 1)
        ).r;

    float dU =
        (height10 - height00)
        * bumpScale;

    float dV =
        (height01 - height00)
        * bumpScale;

    float3 bumpTS =
        normalize(
            float3(
                -dU,
                -dV,
                1.0f
            )
        );

    float3 combinedTS =
        normalize(
            float3(
                normalTS.xy + bumpTS.xy,
                normalTS.z * bumpTS.z
            )
        );

    return normalize(
        mul(combinedTS, TBN)
    );
}

// If c is not black, then blend the color with the texture
// otherwise, replace the color with the texture.
float4 SampleTexture(Texture2D t, float2 uv, float4 c)
{
    if (any(c.rgb))
    {
        c *= t.Sample( TextureSampler, uv );
    }
    else
    {
        c = t.Sample( TextureSampler, uv );
    }

    return c;
}

float3 ComputeEmissiveGradient(float2 uv)
{
    float centerMask =
        1.0f - abs(uv.y * 2.0f - 1.0f);

    centerMask =
        pow(
            saturate(centerMask),
            0.75f
        );

    const float3 edgeColor =
        float3(
            1.00f,
            0.18f,
            0.02f
        );

    const float3 centerColor =
        float3(
            1.00f,
            0.72f,
            0.12f
        );

    return lerp(
        edgeColor,
        centerColor,
        centerMask
    );
}

float4 main( PixelShaderInput IN ): SV_Target
{
    Material material = MaterialCB;

    // By default, use the alpha component of the diffuse color.
    float  alpha    = material.Diffuse.a;
    if (material.HasOpacityTexture) 
    {
        alpha = OpacityTexture.Sample( TextureSampler, IN.TexCoord.xy ).r;
    }

#if ENABLE_DECAL
    if ( alpha < 0.1f )
    {
        discard; // Discard the pixel if it is below a certain threshold.
    }
#endif // ENABLE_DECAL

    float4 ambient = material.Ambient;
    float4 emissive = material.Emissive;
    float4 diffuse = material.Diffuse;
    float specularPower = material.SpecularPower;
    float2 uv = IN.TexCoord.xy;
    float ao = 1.0f;

    if (material.HasAmbientTexture)
    {
        ambient = SampleTexture( AmbientTexture, uv, ambient );
    }
    
    if (material.HasEmissiveTexture)
    {
        emissive = SampleTexture( EmissiveTexture, uv, emissive );
    }
    
    float3 emissiveBase =
        emissive.rgb;

    if (any(emissiveBase))
    {
        emissive.rgb =
        ComputeEmissiveGradient(uv);
    }
    
    if ( material.HasDiffuseTexture )
    {
        diffuse = SampleTexture( DiffuseTexture, uv, diffuse );
    }
    
    if (material.HasSpecularPowerTexture)
    {
        specularPower *= SpecularPowerTexture.Sample( TextureSampler, uv ).r;
    }
    
    if (material.HasRoughnessTexture)
    {
        float roughness = RoughnessTexture.Sample(TextureSampler, uv).r;

        specularPower = lerp(128.0f, 4.0f, saturate(roughness));
    }
    
    if (material.HasAmbientOcclusionTexture)
    {
        ao =
        AmbientOcclusionTexture.Sample(
            TextureSampler,
            uv
        ).r;
    }

    float3 N;
    float3 tangent = normalize(IN.TangentVS);
    float3 bitangent = normalize(IN.BitangentVS);
    float3 normal = normalize(IN.NormalVS);

    float3x3 TBN = float3x3(
    tangent,
    bitangent,
    normal
);

    if (material.HasNormalTexture &&
    material.HasBumpTexture)
    {
        N = DoNormalBumpMapping(
        TBN,
        NormalTexture,
        BumpTexture,
        uv,
        material.BumpIntensity
    );
    }
    else if (material.HasNormalTexture)
    {
        N = DoNormalMapping(
        TBN,
        NormalTexture,
        uv
    );
    }
    else if (material.HasBumpTexture)
    {
        float3x3 bumpTBN = float3x3(
        tangent,
        -bitangent,
        normal
    );

        N = DoBumpMapping(
        bumpTBN,
        BumpTexture,
        uv,
        material.BumpIntensity
    );
    }
    else
    {
        N = normal;
    }

    float shadow = 1;
    float4 specular = 0;
#if ENABLE_LIGHTING
    LightResult lit = DoLighting(
        IN.PositionVS.xyz,
        N,
        specularPower
    );

    diffuse *= lit.Diffuse;
    ambient *= lit.Ambient * ao;

    if (specularPower > 1.0f)
    {
        specular = material.Specular;

        if (material.HasSpecularTexture)
        {
            specular = SampleTexture(
                SpecularTexture,
                uv,
                specular
            );
        }

        specular *= lit.Specular;
    }
#else 
    shadow = -IN.NormalVS.z;
#endif // ENABLE_LIGHTING

    return float4( 
        (emissive + ambient + diffuse + specular).rgb * shadow, 
        alpha * material.Opacity 
    );
}