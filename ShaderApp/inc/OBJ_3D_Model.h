#pragma once
#include <CommandList.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dx12lib.h>
#include <Material.h>
#include <Mesh.h>
#include <Scene.h>
#include <string>
#include <vector>
#include "d3dx12.h"

using namespace std;
namespace dx12lib
{
	using VertexCollection = vector<VertexPositionNormalTangentBitangentTexture>;
	using IndexCollection = vector<uint16_t>;


	typedef struct face face; // ???
	struct face
	{
		vector<uint32_t> vertexIndex;
		vector<uint32_t> textureIndex;
		vector<uint32_t> normalIndex;
	};
	typedef struct object object;
	struct object
	{
		string name;
		string material;
		string color;
		XMFLOAT4 mn;
		XMFLOAT4 mx;
		vector<face> faces;
	};

	class OBJ_3D_Model : public Scene
	{
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT3> textureVertices;
		vector<XMFLOAT3> vertexNormals;
		vector<object*> objects;
		object* currentObject = nullptr;
	protected:
		string objectName;
	public:
		OBJ_3D_Model(string name) :Scene(), objectName(name)
		{
			vertices.clear();
			textureVertices.clear();
			vertexNormals.clear();
			objects.clear();
			currentObject = nullptr;
		}
		~OBJ_3D_Model()
		{
			vertices.clear();
			textureVertices.clear();
			vertexNormals.clear();
			for (auto obj : objects)
			{
				delete obj;
			}
			objects.clear();
		}
		shared_ptr<Scene> CreateModel(string filename, shared_ptr<CommandList> commandList, float scale = 1.0f);
		int objReader(std::string filename);
		std::shared_ptr<Mesh> CreateMesh(string name, string color, const VertexCollection& vertices, const IndexCollection& indices, shared_ptr<CommandList> commandList, XMFLOAT4 mins, XMFLOAT4 maxs);
	};
}
