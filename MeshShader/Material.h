#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include <cstdint>
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "ColorControl.h"
#include "Const.h"
#include "Utility.h"

class Texture;
class Material;

/*
* MTL File Fields
# Ka Ambient
# Kd Diffuse
# Ks Spectral
# Ke Emissive
# Ni optical_density / index of refraction
# Ns Specular Exponent
# d  Dissolve / Transparency
# illum 2
Illumination    Properties that are turned on in the
 model           Property Editor

 0		Color on and Ambient off
 1		Color on and Ambient on
 2		Highlight on
 3		Reflection on and Ray trace on
 4		Transparency: Glass on
		Reflection: Ray trace on
 5		Reflection: Fresnel on and Ray trace on
 6		Transparency: Refraction on
		Reflection: Fresnel off and Ray trace on
 7		Transparency: Refraction on
		Reflection: Fresnel on and Ray trace on
 8		Reflection on and Ray trace off
 9		Transparency: Glass on
		Reflection: Ray trace off
 10		Casts shadows onto invisible surfaces

# name of texture
# Map_Kd
*/

typedef struct MaterialProperties MaterialProperties;
struct alignas(16) MaterialProperties
{
	// The Material properties must be aligned to a 16-byte boundary.
	// To guarantee alignment, the MaterialProperties structure will be allocated in aligned memory.
	MaterialProperties(
		const DirectX::XMFLOAT4 emissive = { 0, 0, 0, 1 },
		const DirectX::XMFLOAT4 specular = { 1, 1, 1, 1 },
		const float specularPower = 128.0f,
		const FLOAT reflectance = 0.4f,
		const FLOAT ambient = 0.2f,
		const FLOAT opacity = 1.0f,
		const DirectX::XMFLOAT4 diffuse = { 1, 1, 1, 1 },
		const FLOAT indexOfRefraction = 0.0f,
		const FLOAT bumpIntensity = 1.0f,
		const FLOAT alphaThreshold = 0.1f,
		const FLOAT intensity = 1.0f
	)
		: Emissive(emissive)
		, Ambient(ambient)
		, Reflectance(reflectance)
		, Diffuse(diffuse)
		, Specular(specular)
		, Opacity(opacity)
		, SpecularPower(specularPower)
		, IndexOfRefraction(indexOfRefraction)
		, BumpIntensity(bumpIntensity)
		, HasAmbientTexture(false)
		, HasEmissiveTexture(false)
		, HasDiffuseTexture(false)
		, HasSpecularTexture(false)
		, HasSpecularPowerTexture(false)
		, HasNormalTexture(false)
		, HasBumpTexture(false)
		, HasOpacityTexture(false)
		, Intensity(intensity)
	{}

	DirectX::XMFLOAT4 Diffuse;
	//------------------------------------ ( 16 bytes )
	DirectX::XMFLOAT4 Specular;
	//------------------------------------ ( 16 bytes )
	DirectX::XMFLOAT4 Emissive;
	//------------------------------------ ( 16 bytes )
	FLOAT Ambient;
	//------------------------------------ ( 16 bytes )
	FLOAT Reflectance;
	FLOAT Intensity;
	//------------------------------------ ( 4 bytes )
	float Opacity;                       // If Opacity < 1, then the material is transparent.
	float SpecularPower;
	float IndexOfRefraction;             // For transparent materials, IOR > 0.
	float BumpIntensity;                 // When using bump textures (height maps) we need
	// to scale the height values so the normals are visible.
//------------------------------------ ( 16 bytes )
	uint32_t HasAmbientTexture;
	uint32_t HasEmissiveTexture;
	uint32_t HasDiffuseTexture;
	uint32_t HasSpecularTexture;
	//------------------------------------ ( 16 bytes )
	uint32_t HasSpecularPowerTexture;
	uint32_t HasNormalTexture;
	uint32_t HasBumpTexture;
	uint32_t HasOpacityTexture;
	//------------------------------------ ( 16 bytes )
	// Total:                              ( 16 * 8 = 128 bytes )
};

class Material
{
public:
	// These are the texture slots that can be bound to the material.
	enum class TextureType
	{
		Ambient,
		Emissive,
		Diffuse,
		Specular,
		SpecularPower,
		Normal,
		Bump,
		Opacity,
		NumTypes,
	};
	Material(std::string materialName);
	Material(std::string materialName, const struct MaterialProperties& materialProperties)
		: m_MaterialProperties(materialProperties), materialName(materialName)
	{
		m_Textures = {};
		textureFileName = "";
	}
//	Material(const Material& copy)
//		: m_MaterialProperties(copy.m_MaterialProperties)
//		, m_Textures(copy.m_Textures)
//		, textureFileName(copy.textureFileName)
//	{}

	~Material() = default;
	std::string materialName;
	std::string& MaterialName() { return materialName; }
	static UINT FindMaterialIndex(const std::string& materialName);
	static std::string MakeMaterial(std::string colorName);

	static std::mt19937 gen; // Seed with time
	static std::uniform_int_distribution<> colordist;
	static std::string RandomMaterial();
	static BOOL DoesMaterialExist(std::string name);
	static void AddMaterial(const std::string& name, struct MaterialProperties& materialProperties);

	std::string& TextureFileName() { return textureFileName; }
	FLOAT& Ambient() { return m_MaterialProperties.Ambient; }
	DirectX::XMFLOAT4& Diffuse() { return m_MaterialProperties.Diffuse; }
	DirectX::XMFLOAT4& Specular() { return m_MaterialProperties.Specular; }
	DirectX::XMFLOAT4& Emissive() { return m_MaterialProperties.Emissive; }
	FLOAT& Reflectance() { return m_MaterialProperties.Reflectance; }
	FLOAT& Intensity() { return m_MaterialProperties.Intensity; }
	FLOAT& IndexOfRefraction() { return m_MaterialProperties.IndexOfRefraction; }
	FLOAT& SpecularPower() { return m_MaterialProperties.SpecularPower; }
	FLOAT& Opacity() { return m_MaterialProperties.Opacity; }                       // If Opacity < 1, then the material is transparent.
	FLOAT& BumpIntensity() { return m_MaterialProperties.BumpIntensity; }                 // When using bump textures (height maps) we need
/*
	const DirectX::XMFLOAT4& GetAmbientColor() const;
	void                     SetAmbientColor(const DirectX::XMFLOAT4& ambient);

	const DirectX::XMFLOAT4& GetDiffuseColor() const;
	void                     SetDiffuseColor(const DirectX::XMFLOAT4& diffuse);

	const DirectX::XMFLOAT4& GetEmissiveColor() const;
	void                     SetEmissiveColor(const DirectX::XMFLOAT4& emissive);

	const DirectX::XMFLOAT4& GetSpecularColor() const;
	void                     SetSpecularColor(const DirectX::XMFLOAT4& specular);

	float GetSpecularPower() const;
	void  SetSpecularPower(float specularPower);

	const DirectX::XMFLOAT4& GetReflectance() const;
	void                     SetReflectance(const DirectX::XMFLOAT4& reflectance);

	const float GetOpacity() const;
	void        SetOpacity(float opacity);

	float GetIndexOfRefraction() const;
	void  SetIndexOfRefraction(float indexOfRefraction);
	// When using bump maps, we can adjust the "intensity" of the normals generated
	// from the bump maps. We can even inverse the normals by using a negative intensity.
	// Default bump intensity is 1.0 and a value of 0 will remove the bump effect altogether.
	float GetBumpIntensity() const;
	void  SetBumpIntensity(float bumpIntensity);
*/

	std::shared_ptr<Texture> GetTexture(TextureType ID) const;
	void                     SetTexture(TextureType type, std::shared_ptr<Texture> texture);

	// This material defines a transparent material
	// if the opacity value is < 1, or there is an opacity map, or the diffuse texture has an alpha channel.
	bool IsTransparent() const;

	MaterialProperties& MaterialProperties() { return m_MaterialProperties; }
//	void SetMaterialProperties(const MaterialProperties& materialProperties) { m_MaterialProperties = materialProperties; }

	static struct MaterialProperties& findMaterial(std::string name);


protected:
private:
	using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;
	// A unique pointer with a custom allocator/deallocator to ensure alignment.
//	using MaterialPropertiesPtr = std::unique_ptr<MaterialProperties, void (*)(MaterialProperties*)>;

	struct MaterialProperties	m_MaterialProperties;
	TextureMap					m_Textures;
	std::string					textureFileName;
};

/*
	// Define some interesting materials.
	static const MaterialProperties Zero;
	static const MaterialProperties Red;
	static const MaterialProperties Green;
	static const MaterialProperties Blue;
	static const MaterialProperties Cyan;
	static const MaterialProperties Magenta;
	static const MaterialProperties Yellow;
	static const MaterialProperties White;
	static const MaterialProperties WhiteDiffuse;
	static const MaterialProperties Black;
	static const MaterialProperties Emerald;
	static const MaterialProperties Jade;
	static const MaterialProperties Obsidian;
	static const MaterialProperties Pearl;
	static const MaterialProperties Ruby;
	static const MaterialProperties Turquoise;
	static const MaterialProperties Brass;
	static const MaterialProperties Bronze;
	static const MaterialProperties Chrome;
	static const MaterialProperties Copper;
	static const MaterialProperties Gold;
	static const MaterialProperties Silver;
	static const MaterialProperties BlackPlastic;
	static const MaterialProperties CyanPlastic;
	static const MaterialProperties GreenPlastic;
	static const MaterialProperties RedPlastic;
	static const MaterialProperties WhitePlastic;
	static const MaterialProperties YellowPlastic;
	static const MaterialProperties BlackRubber;
	static const MaterialProperties CyanRubber;
	static const MaterialProperties GreenRubber;
	static const MaterialProperties RedRubber;
	static const MaterialProperties WhiteRubber;
	static const MaterialProperties YellowRubber;
*/
