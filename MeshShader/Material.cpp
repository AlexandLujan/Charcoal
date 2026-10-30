#include "stdafx.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <Windows.h>
#include "Material.h"
#include "utility.h"

using namespace DirectX;
using namespace std;

unordered_map<std::string, std::shared_ptr<Material>> MaterialsMap;
std::string ToUpper(const std::string& str)
{
	std::string result = str;
	transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}
#include <afxstr.h>
#include <tchar.h>
#include <cstringt.h>
void DumpMaterialList(string objectName)
{
	int offset = 0;
	CString st;
	st.Format(_T("OBJECT %s\r\n"), objectName.c_str());
	OutputDebugString(st);
	for (auto it = MaterialsMap.begin(); it != MaterialsMap.end(); ++it)
	{
		const auto& [name, mat] = *it;
		if (mat == nullptr)
		{
			st.Format(_T("index %3d ERROR in Material %s\r\n"), offset, name.c_str());
			OutputDebugString(st);
			offset++;
			continue;
		}
		st.Format(_T("%3d (index %3d) Material %s\r\n\tEmissive: %6.4f,%6.4f,%6.4f\r\n\tDiffuse: %6.4f,%6.4f,%6.4f\r\n\tAmbient: %6.4f Transparent: %4s\r\n"),
					 offset, Material::FindMaterialIndex(name), name.c_str(),
					 mat->Emissive().x, mat->Emissive().y, mat->Emissive().z,
					 mat->Diffuse().x, mat->Diffuse().y, mat->Diffuse().z,
					 mat->Ambient(), mat->IsTransparent() ? "YES" : "NO");
		OutputDebugString(st);
		offset++;
	}
}

UINT Material::FindMaterialIndex(const std::string& materialName)
{
	auto it = MaterialsMap.find(materialName);
	if (it != MaterialsMap.end())
	{
		UINT offset = (UINT)distance(MaterialsMap.begin(), it);
#if 0
		CString st;
		st.Format(_T("Material %s : INDEX %d\r\n"), materialName.c_str(), offset);
		OutputDebugString(st);
#endif
		return offset;
	}
	string newMaterial = MakeMaterial(materialName);

	return FindMaterialIndex(newMaterial); // Should be index for WHITE
}

struct MaterialProperties Zero = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		0.0f,0.0f };

Material MaterialZero("ZERO", Zero);

std::mt19937 Material::gen; // Seed with time
std::uniform_int_distribution<> Material::colordist;
extern const std::map<std::string, UINT> colorMap;

string Material::RandomMaterial()
{
	::MaterialProperties Random = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f };
	string colorName;
	do
	{
		colordist.param(std::uniform_int_distribution<int>::param_type(0, (int)colorMap.size() - 1));
		UINT randomColor = colordist(gen);
		auto random_it = colorMap.begin();
		std::advance(random_it, randomColor);
		string colorName = ToUpper(random_it->first);
		if (MaterialsMap.contains(colorName))
		{
			return colorName;
		}
		Random.Emissive = FindColor(colorName);
		MaterialsMap[colorName] = make_shared<Material>(colorName, Random);
	} while (colorName.contains("BLACK"));
	return colorName;
}

string Material::MakeMaterial(string colorName)
{
	::MaterialProperties Base = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f };

	if (MaterialsMap.contains(colorName))
	{
		return colorName;
	}
	Base.Emissive = FindColor(colorName);
	MaterialsMap[colorName] = make_shared<Material>(colorName, Base);
	return colorName;
}

Material::Material(string materialName) : materialName(materialName)
{
//	if (MaterialsMap.contains(materialName))
//	{
//		m_MaterialProperties = MaterialsMap[materialName]->MaterialProperties();
//		textureFileName = MaterialsMap[materialName]->TextureFileName();
//	}
//	else
//	{
	::MaterialProperties Random = {
								{ 1.0f, 0.0f, 0.0f, 1.0f },
								{ 1.0f, 1.0f, 1.0f, 1.0f },
									128.0f, 0.1f };
	MaterialsMap[materialName] = make_shared<Material>(materialName, Random);
	m_MaterialProperties = MaterialsMap[materialName]->MaterialProperties();
	m_Textures = {};
	textureFileName = "";
//	}
}

void InitializeMaterialMap()
{

// Material Definitions
// clang-format off
	const MaterialProperties White = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f
	};

	const MaterialProperties Red = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f
	};

	const MaterialProperties Green = {
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.0f
	};

	const MaterialProperties Blue = {
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.0f
	};

	const MaterialProperties Cyan = {
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.0f
	};

	const MaterialProperties Magenta = {
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f
	};

	const MaterialProperties Yellow = {
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		128.0f, 0.1f
	};

//	const MaterialProperties White = {
//		{ 1.0f, 1.0f, 1.0f, 1.0f },
//		{ 1.0f, 1.0f, 1.0f, 1.0f },
//		128.0f,
//		{ 0.1f, 0.1f, 0.1f, 1.0f }
//	};

	const MaterialProperties WhiteDiffuse = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		0.0f, 0.0f
	};

	const MaterialProperties Black = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		0.0f, 0.0f
	};

	const MaterialProperties Emerald = {
		{ 0.07568f, 0.61424f, 0.07568f, 1.0f },
		{ 0.633f, 0.727811f, 0.633f, 1.0f },
		76.8f, 0.02f
	};

	const MaterialProperties Jade = {
		{ 0.54f, 0.89f, 0.63f, 1.0f },
		{ 0.316228f, 0.316228f, 0.316228f, 1.0f },
		12.8f, 0.135f
	};

	const MaterialProperties Obsidian = {
		{ 0.18275f, 0.17f, 0.22525f, 1.0f },
		{ 0.332741f, 0.328634f, 0.346435f, 1.0f },
		38.4f, 0.05f
	};

	const MaterialProperties Pearl = {
		{ 1.0f, 0.829f, 0.829f, 1.0f },
		{ 0.296648f, 0.296648f, 0.296648f, 1.0f },
		11.264f, 0.25f
	};

	const MaterialProperties Ruby = {
		{ 0.61424f, 0.04136f, 0.04136f, 1.0f },
		{ 0.727811f, 0.626959f, 0.626959f, 1.0f },
		76.8f, 0.01745f
	};

	const MaterialProperties Turquoise = {
		{ 0.396f, 0.74151f, 0.69102f, 1.0f },
		{ 0.297254f, 0.30829f, 0.306678f, 1.0f },
		12.8f, 0.1f
	};

	const MaterialProperties Brass = {
		{ 0.780392f, 0.568627f, 0.113725f, 1.0f },
		{ 0.992157f, 0.941176f, 0.807843f, 1.0f },
		27.9f, 0.32f
	};

	const MaterialProperties Bronze = {
		{ 0.714f, 0.4284f, 0.18144f, 1.0f },
		{ 0.393548f, 0.271906f, 0.166721f, 1.0f },
		25.6f, 0.2125f
	};

	const MaterialProperties Chrome = {
		{ 0.4f, 0.4f, 0.4f, 1.0f },
		{ 0.774597f, 0.774597f, 0.774597f, 1.0f },
		76.8f, 0.25f
	};

	const MaterialProperties Aluminium = {
		{ 0.50754f, 0.50754f, 0.50754f, 1.0f },
		{ 0.774597f, 0.774597f, 0.774597f, 1.0f },
		14.8f, 0.25f
	};

	const MaterialProperties Copper = {
		{ 0.7038f, 0.27048f, 0.0828f, 1.0f },
		{ 0.256777f, 0.137622f, 0.086014f, 1.0f },
		12.8f, 0.19125f
	};

	const MaterialProperties Gold = {
		{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
		{ 0.628281f, 0.555802f, 0.366065f, 1.0f },
		51.2f, 0.24725f
	};

	const MaterialProperties Silver = {
		{ 0.50754f, 0.50754f, 0.50754f, 1.0f },
		{ 0.508273f, 0.508273f, 0.508273f, 1.0f },
		51.2f, 0.19225f
	};

	const MaterialProperties BlackPlastic = {
		{ 0.01f, 0.01f, 0.01f, 1.0f },
		{ 0.5f, 0.5f, 0.5f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties CyanPlastic = {
		{ 0.0f, 0.50980392f, 0.50980392f, 1.0f },
		{ 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties GreenPlastic = {
		{ 0.1f, 0.35f, 0.1f, 1.0f },
		{ 0.45f, 0.55f, 0.45f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties RedPlastic = {
		{ 0.5f, 0.0f, 0.0f, 1.0f },
		{ 0.7f, 0.6f, 0.6f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties WhitePlastic = {
		{ 0.55f, 0.55f, 0.55f, 1.0f },
		{ 0.7f, 0.7f, 0.7f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties YellowPlastic = {
		{ 0.5f, 0.5f, 0.0f, 1.0f },
		{ 0.6f, 0.6f, 0.5f, 1.0f },
		32.0f, 0.0f
	};

	const MaterialProperties BlackRubber = {
		{ 0.01f, 0.01f, 0.01f, 1.0f },
		{ 0.4f, 0.4f, 0.4f, 1.0f },
		10.0f, 0.02f
	};

	const MaterialProperties CyanRubber = {
		{ 0.4f, 0.5f, 0.5f, 1.0f },
		{ 0.04f, 0.7f, 0.7f, 1.0f },
		10.0f,0.05f
	};

	const MaterialProperties GreenRubber = {
		{ 0.4f, 0.5f, 0.4f, 1.0f },
		{ 0.04f, 0.7f, 0.04f, 1.0f },
		10.0f,0.05f
	};

	const MaterialProperties RedRubber = {
		{ 0.5f, 0.4f, 0.4f, 1.0f },
		{ 0.7f, 0.04f, 0.04f, 1.0f },
		10.0f, 0.05f
	};

	const MaterialProperties WhiteRubber = {
		{ 0.5f, 0.5f, 0.5f, 1.0f },
		{ 0.7f, 0.7f, 0.7f, 1.0f },
		10.0f, 0.05f
	};

	const MaterialProperties YellowRubber = {
		{ 0.5f, 0.5f, 0.4f, 1.0f },
		{ 0.7f, 0.7f, 0.04f, 1.0f },
		10.0f, 0.05f
	};

	MaterialsMap[ToUpper("Zero")] = make_shared<Material>("ZERO", Zero);
	MaterialsMap[ToUpper("Black")] = make_shared<Material>("Black", Black);
	MaterialsMap[ToUpper("BlackPlastic")] = make_shared<Material>("BlackPlastic", BlackPlastic);
	MaterialsMap[ToUpper("BlackRubber")] = make_shared<Material>("BlackRubber", BlackRubber);
	MaterialsMap[ToUpper("Blue")] = make_shared<Material>("Blue", Blue);
	MaterialsMap[ToUpper("Brass")] = make_shared<Material>("Brass", Brass);
	MaterialsMap[ToUpper("Bronze")] = make_shared<Material>("Bronze", Bronze);
	MaterialsMap[ToUpper("Chrome")] = make_shared<Material>("Chrome", Chrome);
	MaterialsMap[ToUpper("Copper")] = make_shared<Material>("Copper", Copper);
	MaterialsMap[ToUpper("Cyan")] = make_shared<Material>("Cyan", Cyan);
	MaterialsMap[ToUpper("CyanPlastic")] = make_shared<Material>("CyanPlastic", CyanPlastic);
	MaterialsMap[ToUpper("CyanRubber")] = make_shared<Material>("CyanRubber", CyanRubber);
	MaterialsMap[ToUpper("Emerald")] = make_shared<Material>("Emerald", Emerald);
	MaterialsMap[ToUpper("Gold")] = make_shared<Material>("Gold", Gold);
	MaterialsMap[ToUpper("Green")] = make_shared<Material>("Green", Green);
	MaterialsMap[ToUpper("GreenPlastic")] = make_shared<Material>("GreenPlastic", GreenPlastic);
	MaterialsMap[ToUpper("GreenRubber")] = make_shared<Material>("GreenRubber", GreenRubber);
	MaterialsMap[ToUpper("Jade")] = make_shared<Material>("Jade", Jade);
	MaterialsMap[ToUpper("Magenta")] = make_shared<Material>("Magenta", Magenta);
	MaterialsMap[ToUpper("Obsidian")] = make_shared<Material>("Obsidian", Obsidian);
	MaterialsMap[ToUpper("Pearl")] = make_shared<Material>("Pearl", Pearl);
	MaterialsMap[ToUpper("Red")] = make_shared<Material>("Red", Red);
	MaterialsMap[ToUpper("RedPlastic")] = make_shared<Material>("RedPlastic", RedPlastic);
	MaterialsMap[ToUpper("RedRubber")] = make_shared<Material>("RedRubber", RedRubber);
	MaterialsMap[ToUpper("Ruby")] = make_shared<Material>("Ruby", Ruby);
	MaterialsMap[ToUpper("Silver")] = make_shared<Material>("Silver", Silver);
	MaterialsMap[ToUpper("Turquoise")] = make_shared<Material>("Turquoise", Turquoise);
	MaterialsMap[ToUpper("White")] = make_shared<Material>("White", White);
	MaterialsMap[ToUpper("WhiteDiffuse")] = make_shared<Material>("WhiteDiffuse", WhiteDiffuse);
	MaterialsMap[ToUpper("WhitePlastic")] = make_shared<Material>("WhitePlastic", WhitePlastic);
	MaterialsMap[ToUpper("WhiteRubber")] = make_shared<Material>("WhiteRubber", WhiteRubber);
	MaterialsMap[ToUpper("Yellow")] = make_shared<Material>("Yellow", Yellow);
	MaterialsMap[ToUpper("YellowPlastic")] = make_shared<Material>("YellowPlastic", YellowPlastic);
	MaterialsMap[ToUpper("YellowRubber")] = make_shared<Material>("YellowRubber", YellowRubber);
}

void Material::AddMaterial(const std::string& name, struct MaterialProperties& materialProperties)
{
	MaterialsMap[ToUpper(name)] = make_shared<Material>(name, materialProperties);
//	DumpMaterialList(name);
}

std::shared_ptr<Texture> Material::GetTexture(TextureType ID) const
{
	TextureMap::const_iterator iter = m_Textures.find(ID);
	if (iter != m_Textures.end())
	{
		return iter->second;
	}

	return nullptr;
}

void Material::SetTexture(TextureType type, std::shared_ptr<Texture> texture)
{
	m_Textures[type] = texture;

	switch (type)
	{
		case TextureType::Ambient:
			{
				m_MaterialProperties.HasAmbientTexture = (texture != nullptr);
			}
			break;
		case TextureType::Emissive:
			{
				m_MaterialProperties.HasEmissiveTexture = (texture != nullptr);
			}
			break;
		case TextureType::Diffuse:
			{
				m_MaterialProperties.HasDiffuseTexture = (texture != nullptr);
			}
			break;
		case TextureType::Specular:
			{
				m_MaterialProperties.HasSpecularTexture = (texture != nullptr);
			}
			break;
		case TextureType::SpecularPower:
			{
				m_MaterialProperties.HasSpecularPowerTexture = (texture != nullptr);
			}
			break;
		case TextureType::Normal:
			{
				m_MaterialProperties.HasNormalTexture = (texture != nullptr);
			}
			break;
		case TextureType::Bump:
			{
				m_MaterialProperties.HasBumpTexture = (texture != nullptr);
			}
			break;
		case TextureType::Opacity:
			{
				m_MaterialProperties.HasOpacityTexture = (texture != nullptr);
			}
			break;
	}
}

bool Material::IsTransparent() const
{
	return (m_MaterialProperties.Opacity < 1.0f); // || m_MaterialProperties.HasOpacityTexture);
}

MaterialProperties& Material::findMaterial(string name)
{
	if (MaterialsMap.contains(ToUpper(name)))
	{
		return MaterialsMap[ToUpper(name)]->MaterialProperties();
	}
	// If the material is not found, add a new material with default properties (Zero) and return it.
	MaterialsMap[ToUpper(name)] = make_shared<Material>(name, Zero);
	return MaterialsMap[ToUpper(name)]->MaterialProperties();
}

BOOL Material::DoesMaterialExist(string name)
{
	return (MaterialsMap.contains(ToUpper(name)));
}