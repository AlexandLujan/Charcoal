//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "MyShaderCommon.hlsli"

ConstantBuffer<Constants> Globals : register(b0);
StructuredBuffer<MaterialBuffer> material : register(t4);
StructuredBuffer<Light> Lights : register(t6);

float4 LightingEffect(Light light, MaterialBuffer mat, float4 surfacePoint, float4 surfaceNormal, float4 eye)
{
    if ((light.status & 0x01) == 0) return float4(0, 0, 0, 1);// light is off
        
    float4 surfaceColor = float4(0, 0, 0, 1); //
    float4 light_to_surface_vector = normalize(surfacePoint - light.positionW);
    float cosine_beam = dot(light_to_surface_vector, light.directionW);
    if (cosine_beam < 0) return float4(0, 0, 0, mat.Transparency); // beam going in the wrong direction
        
//  if (cosine_beam < Spread())
//     return float4(0, 0, 0, mat.Transparency); // too much spread to be effective
//intensity *= cosine_beam;
// get hit and reflection angle of light at surface
// r = d−2(d⋅n)n
// reflection vector = incoming vector - 2* DOT(incoming, surface) * surfactNormal
    
    float4 surface_to_light_vector = normalize(light.positionW - surfacePoint); // d
    float cosine_surface_to_light = dot(surfaceNormal, surface_to_light_vector); // dot(n d)
    if (cosine_surface_to_light < 0) return float4(0, 0, 0, mat.Transparency); // pointed in wrong direction
        
// dot(d,n)*n * 2
    float4 d_dot_n_n = normalize(cosine_surface_to_light * 2.0f * surfaceNormal);
    float4 reflection_vector = normalize(surface_to_light_vector - d_dot_n_n); // d - 2*(Dot(d,n))*n
    float4 eye_hit_vector = normalize(eye - surfacePoint);
// diffuse is the dot product of incoming light and surface normal * factor
    float cosine_reflection_to_eye = dot(eye_hit_vector, reflection_vector);
    if (cosine_reflection_to_eye < 0) return float4(0, 0, 0, mat.Transparency);
    float k1 = 1.0f;
    float k2 = 1.0f;
    float4 diffuse = light.color * (cosine_surface_to_light * k1) * light.intensity * 0.20;
    surfaceColor += mat.Emissive * (diffuse * mat.Intensity * 0.5);
    float4 specular = light.color * pow(cosine_reflection_to_eye, mat.SpecularPower) * k2;
    surfaceColor += specular;
    surfaceColor.x = saturate(surfaceColor.x);
    surfaceColor.y = saturate(surfaceColor.y);
    surfaceColor.z = saturate(surfaceColor.z);
    surfaceColor.w = mat.Transparency;
    return surfaceColor;
}


float4 CalculateLighting(Light light,
                            MaterialBuffer mat,
                            float3 location,
                            float4 materielDiffuse,
                            float4 viewDir,
                            float4 normal)
{
    float4 lightDir = normalize(light.positionW);
    float3 reflectDir = reflect(-lightDir.xyz, normal.xyz);
    float spec = pow(max(dot(reflectDir, location.xyz), 0.0), mat.SpecularPower);
    float4 specular = spec * mat.Specular;
    // Diffuse + Specular Calculation
    float diff = max(dot(normal, lightDir), 0.0);  
    float4 diffuse = diff * light.color * light.intensity;
    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal.xyz, lightDir.xyz));
    float3 halfAngle = normalize(lightDir.xyz + viewDir.xyz);
    float blinnTerm = saturate(dot(normal.xyz, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, mat.Reflectance);
    float4 finalColor = Globals.ambient + ((cosAngle + blinnTerm + 0.1) * mat.Emissive /*light.color */ ) + specular;
    finalColor.w = mat.Transparency;
    return finalColor;
}

float4 main(VertexOut input) : SV_TARGET
{
    float3 viewDir = normalize(Globals.camera_direction.xyz);
    float3 normal = normalize(input.Normal);
    float4 surfaceColor = { 0, 0, 0, 1 };
    // start with surface color under ambient light
    float ambient = Globals.ambient * 1.0;
    surfaceColor = material[input.Color].Emissive * ambient;

   // Iterate over all active light sources passed in the buffer
    uint lightCount, stride;
    Lights.GetDimensions(lightCount, stride);
 //   for (uint i = 0; i < lightCount; i++)
    {
    surfaceColor += LightingEffect(Lights[0], material[input.Color], float4(input.PositionVS, 1), float4(normal, 0), Globals.camera_position);
    surfaceColor += LightingEffect(Lights[1], material[input.Color], float4(input.PositionVS, 1), float4(normal, 0), Globals.camera_position);
    surfaceColor += LightingEffect(Lights[2], material[input.Color], float4(input.PositionVS, 1), float4(normal, 0), Globals.camera_position);
    surfaceColor += LightingEffect(Lights[3], material[input.Color], float4(input.PositionVS, 1), float4(normal, 0), Globals.camera_position);
       //surfaceColor += CalculateLighting(Lights[i], 
       //                                     material[input.Color],
       //                                     input.PositionVS,
       //                                     material[input.Color].Emissive,
       //                                     Globals.camera_direction,
       //                                     float4(input.Normal, 0.0));
    surfaceColor.x = saturate(surfaceColor.x);
    surfaceColor.y = saturate(surfaceColor.y);
    surfaceColor.z = saturate(surfaceColor.z);
 //   surfaceColor = material[input.Color].Emissive;
    }
    return float4(surfaceColor.xyz, material[input.Color].Transparency);
 //   return surfaceColor;
}
/*
float4 main(VertexOut input) : SV_TARGET
{
    float4 lightColor = float4(0.2, 0.2, 0.2, 1.0);
    float4 lightDir = -normalize(float4(10, -10, 10 ,0));
    float4 surfaceColor = materialProperty[input.Color].Emissive;
    float amb = 0.1; //    materialProperty[input.Color].Ambient;
    float4 ambient = float4(amb,amb,amb,1);
    float3 normal = normalize(input.Normal);
    float3 reflectDir = reflect(-lightDir.xyz, normal);
    float spec = pow(max(dot(reflectDir, input.PositionVS), 0.0), materialProperty[input.Color].SpecularPower);
    //float4 specular = / y*lightColor * m*m/ spec * materialProperty[input.Color].Specular;
    
    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal, lightDir.xyz));
    float3 viewDir = -normalize(input.PositionVS);
    float3 halfAngle = normalize(lightDir.xyz + viewDir);
    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, materialProperty[input.Color].Reflectance.x);
    float4 finalColor = ambient + ((cosAngle + blinnTerm + 0.1) * surfaceColor) + specular;
//    float4 finalColor = ambient + surfaceColor;
    finalColor.a = materialProperty[input.Color].Transparency;
 //   finalColor.x = saturate(finalColor.x);
 //   finalColor.y = saturate(finalColor.y);
//    finalColor.z = saturate(finalColor.z);
//    if (finalColor.a < 1.0)
//    {
//        finalColor = float4(1, 0, 0, 0.8);
//    }
    return finalColor;
}
*/

/*

// lighting with multiple light sources
struct Light {
    float4 position;
    float4 color;
    float4 direction; // For spot/directional lights
    float intensity;
    float range;
};
StructuredBuffer<Light> Lights : register(t1);
ConstantBuffer<Material> Mat : register(b1);

struct PixelInput {
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
};

float3 CalculateLighting(Light light, float3 viewDir, float3 normal, float3 worldPos) {
    float3 lightDir = normalize(light.position - worldPos);
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Diffuse + Specular Calculation
    float3 diffuse = diff * light.color * light.intensity;
    return diffuse * Mat.diffuse;
}

float4 main(PixelInput input) : SV_Target {
    float3 result = float3(0.0f, 0.0f, 0.0f);
    float3 viewDir = normalize(cameraPos - input.worldPos);
    float3 normal = normalize(input.normal);
    
    // Iterate over all active light sources passed in the buffer
    uint lightCount, stride;
    Lights.GetDimensions(lightCount, stride);
    
    for (uint i = 0; i < lightCount; i++) {
        result += CalculateLighting(Lights[i], viewDir, normal, input.worldPos);
    }
    
    return float4(result, 1.0f);
}

    float shininess = materialProperty[input.Color].Reflectance.x;
    if (!Globals.DrawMeshlets)
    {
        shininess = 64.0;
    }

    float3 normal = normalize(input.Normal);
 //   float spec = pow(max(dot(reflectDir, input.viewDirection), 0.0), SpecularPower);
    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);

    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);
    float4 finalColor = ambient + ((cosAngle + blinnTerm + 0.1) * surfaceColor);

XMFLOAT3 Lighting::LightingEffect(XMFLOAT4 surfacePoint, XMFLOAT4 surfaceNormal, XMFLOAT4 eye, XMUINT3 triangle, bool RasterScan)
{
	if ((not RasterScan) and (not CanLightReachSurface(surfacePoint, triangle))) return { 0, 0, 0 };
	FLOAT intensity = Intensity();
	XMFLOAT4 light_to_surface_vector = VECTOR(surfacePoint, ScenePosition());
	FLOAT cosine_beam = DOT(light_to_surface_vector, Beam());
	if (cosine_beam < 0) return { 0,0,0 }; // beam going in the wrong direction
	if (cosine_beam < Spread()) return { 0,0,0 }; // too much spread to be effective
	//	intensity.x *= cosine_beam;
	//intensity.y *= cosine_beam;
	//intensity.z *= cosine_beam;
	// get hit and reflection angle of light at surface
	// r = d−2(d⋅n)n
	// reflection vector = incoming vector - 2* DOT(incoming, surface) * surfactNormal
	XMFLOAT4 surface_to_light_vector = VECTOR(ScenePosition(), surfacePoint); // d
	FLOAT cosine_surface_to_light = DOT(surfaceNormal, surface_to_light_vector); // dot(n d)
	if (cosine_surface_to_light < 0) return { 0,0,0 };  // pointed in wrong direction
	XMFLOAT4 d_dot_n_n = Normalize({ cosine_surface_to_light * surfaceNormal.x * 2.0f, // dot(d,n)*n * 2
									 cosine_surface_to_light * surfaceNormal.y * 2.0f,
									 cosine_surface_to_light * surfaceNormal.z * 2.0f, 0 });
	XMFLOAT4 reflection_vector = SUBTRACT(surface_to_light_vector, d_dot_n_n);// d - 2*(Dot(d,n))*n
	XMFLOAT4 eye_hit_vector = VECTOR(eye, surfacePoint);
	// diffuse is the dot product of incoming light and surface normal * factor
	float cosine_reflection_to_eye = DOT(eye_hit_vector, reflection_vector);
	if (cosine_reflection_to_eye < 0) return { 0,0,0 };
	float k1 = 1.0f;
	float k2 = 1.0f;
	float n = 1500.0f;
	float diffuse = intensity * (cosine_surface_to_light * k1);
	float spectral = intensity * powf(cosine_reflection_to_eye, n) * k2;

	float iRed = max(0, min(1.0f, RedIntensity() * (diffuse + spectral)));

	float iGreen = max(0, min(1.0f, GreenIntensity() * (diffuse + spectral)));
	float iBlue = max(0, min(1.0f, BlueIntensity() * (diffuse + spectral)));
	return { fabs(iRed), fabs(iGreen), fabs(iBlue) };
}


XMFLOAT3 PointLight::LightingEffect(XMFLOAT4 surfacePoint, XMFLOAT4 surfaceNormal, XMFLOAT4 eye, XMUINT3 triangle, bool RasterScan)
{
	if ((not RasterScan) and (not CanLightReachSurface(surfacePoint, triangle))) return { 0, 0, 0 };
	FLOAT intensity = Intensity();
	// get hit and reflection angle of light at surface
	// r = d−2(d⋅n)n
	// reflection vector = incoming vector - 2* DOT(incoming, surface) * surfactNormal
	XMFLOAT4 surface_to_light_vector = VECTOR(ScenePosition(), surfacePoint); // d
	FLOAT cosine_surface_to_light = DOT(surfaceNormal, surface_to_light_vector); // dot(n d)
	if (cosine_surface_to_light < 0) return { 0,0,0 };  // pointed in wrong direction
	XMFLOAT4 d_dot_n_n = Normalize({ cosine_surface_to_light * surfaceNormal.x * 2.0f, // dot(d,n)*n * 2
									 cosine_surface_to_light * surfaceNormal.y * 2.0f,
									 cosine_surface_to_light * surfaceNormal.z * 2.0f, 0 });
	XMFLOAT4 reflection_vector = SUBTRACT(surface_to_light_vector, d_dot_n_n);// d - 2*(Dot(d,n))*n
	XMFLOAT4 eye_hit_vector = VECTOR(eye, surfacePoint);
	// diffuse is the dot product of incoming light and surface normal * factor
	float cosine_reflection_to_eye = DOT(eye_hit_vector, reflection_vector);
	if (cosine_reflection_to_eye < 0) return { 0,0,0 };
	float k1 = 1.0f;
	float k2 = 1.0f;
	float n = 1500.0f;
	float diffuse = intensity * (cosine_surface_to_light * k1);
	float spectral = intensity * powf(cosine_reflection_to_eye, n) * k2;
	float iRed = max(0, min(1.0f, RedIntensity() * (diffuse + spectral)));
	float iGreen = max(0, min(1.0f, GreenIntensity() * (diffuse + spectral)));
	float iBlue = max(0, min(1.0f, BlueIntensity() * (diffuse + spectral)));
	return { fabs(iRed), fabs(iGreen), fabs(iBlue) };
}


*/