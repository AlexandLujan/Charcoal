#include "pch.h"
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include "Camera.h"
#include "CameraPage.h"
#include "ContentPage.h"
#include "Object.h"
#include "ObjectPage.h"
#include "ShaderAppDlg.h"

void CShaderAppDlg::ReadJson(string filename)
{
	m_ContentLoaded = false;
	BuildJSON(filename);
//		BuildTemplates(templates);
	BuildViewPoint(viewpoint);
	BuildObjects(objects);
	ObjectActions(setup);
	BuildCameras(cameras);
	BuildLights(lights);
	//		BuildActions(actions);
	controlBoard->ResetObjectNameList();
	bool haveName = false;
	for (auto& [key, value] : ObjectList)
	{
		auto& k = key;
		if (not haveName)
		{
			objectName = key;
			haveName = true;
		}
		controlBoard->UpdateAddObjectName(key);
	}
	controlBoard->SetSelectedObject(0);

	shared_ptr<Object> o = ObjectList[objectName]; // GetSelectedObject();
	if (o != nullptr)
	{
		UpdateObjectRotation(o);
		UpdateObjectPositionBoxes(o);
	}

	controlBoard->ResetCameraNameList();
	haveName = false;
	for (auto& [key, value] : CameraList)
	{
		auto& k = key;
		if (not haveName)
		{
			cameraName = key;
			haveName = true;
		}
		controlBoard->UpdateAddCameraName(key);
	}
	controlBoard->SetSelectedCamera(0);


	std::shared_ptr<Camera> c = CameraList[cameraName];
	if (c != nullptr)
	{
		controlBoard->UpdateCameraPosition(c->Position());
		controlBoard->UpdateCameraTarget(c->Target());
		pCurrentCamera = c;
	}



	controlBoard->ResetLightingNameList();
	haveName = false;
	for (auto& [key, value] : LightingList)
	{
		auto& k = key;
		if (not haveName)
		{
			LightingName = key;
			haveName = true;
		}
		controlBoard->UpdateAddLightingName(key);
	}
	controlBoard->SetSelectedLighting(0);

	shared_ptr <Light> l = LightList[LightingName];
	if (l != nullptr)
	{
		controlBoard->UpdateLightPosition(l->Position());
		controlBoard->UpdateLightTarget(l->Target());
		controlBoard->UpdateLightColor(l->Color());
		controlBoard->UpdateLightAmbientIntensity(l->Ambient());
		controlBoard->UpdateLightType(l->LightType());
		controlBoard->UpdateLightStatus(l->Status());
		currentLight = l;

	}

	m_ContentLoaded = true;
//	controlBoard->UpdateLightAmbientIntensity(ambient_intensity);
//	controlBoard->UpdateLightingAmbientColor(ambient);
//	needs_update = true;
}


void CShaderAppDlg::BuildJSON(string filename)
{
	ifstream file(filename);
	if (!file.is_open()) return;
	document = json::parse(file);
	file.close();
	viewpoint = document["ViewPoint"];
//	templates = document["Templates"];
	objects = document["Objects"];
	setup = document["Set Up"];
	cameras = document["Cameras"];
	lights = document["Lights"];
	actions = document["Actions"];
}

void CShaderAppDlg::BuildViewPoint(json& viewpoint)
{
	float x = viewpoint["eye_x"];
	float y = viewpoint["eye_y"];
	float z = viewpoint["eye_z"];
	eyeLocation = { x,y,z,1 };
}
void CShaderAppDlg::BuildTemplates(json& templates)
{
	for (json::const_iterator it = templates.cbegin(); it != templates.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		string type = element["type"];
		if (type == "CIRCLE") TemplateList[name] = BuildCircle(element, name);
		if (type == "SQUARE") TemplateList[name] = BuildSquare(element, name);
		if (type == "RECTANGLE") TemplateList[name] = BuildRectangle(element, name);
		if (type == "POLYGON") TemplateList[name] = BuildPolygon(element, name);
		if (type == "TEXT") TemplateList[name] = BuildText(element, name);
		if (type == "MESH") TemplateList[name] = BuildMesh(element, name);
		if (type == "TEMPLATE_REF") TemplateList[name] = BuildTemplateReference(element, name);
		if (type == "FRACTAL") TemplateList[name] = BuildFractal(element, name);
		if (type == "BOX") TemplateList[name] = BuildBox(element, name);
	}
}

void CShaderAppDlg::BuildObjects(json& objects)
{
	for (json::const_iterator it = objects.cbegin(); it != objects.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		string type = element["type"];
		if (type == "CIRCLE") ObjectList[name] = BuildCircle(element, name);
		if (type == "SQUARE") ObjectList[name] = BuildSquare(element, name);
		if (type == "RECTANGLE") ObjectList[name] = BuildRectangle(element, name);
		if (type == "POLYGON") ObjectList[name] = BuildPolygon(element, name);
		if (type == "TEXT") ObjectList[name] = BuildText(element, name);
		if (type == "MESH") ObjectList[name] = BuildMesh(element, name);
		if (type == "TEMPLATE_REF") ObjectList[name] = BuildTemplateReference(element, name);
		if (type == "OBJ_FILE") ObjectList[name] = BuildOBJFileMesh(element, name);
		if (type == "FRACTAL") ObjectList[name] = BuildFractal(element, name);
		if (type == "BOX") ObjectList[name] = BuildBox(element, name);
		if (type == "TUNNEL") ObjectList[name] = BuildTunnel(element, name);
	}
}

void CShaderAppDlg::BuildCameras(json& cameras)
{
	for (json::const_iterator it = cameras.cbegin(); it != cameras.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		json position = element["position"];
		json lookat = element["lookat"];
		float fov = element["fov"];
		XMFLOAT4 eye = { (FLOAT)position["x"] * CAMERA_SCALE_FACTOR, (FLOAT)position["y"] * CAMERA_SCALE_FACTOR, (FLOAT)position["z"] * CAMERA_SCALE_FACTOR,1 };
		XMFLOAT4 look = { (FLOAT)lookat["x"] * CAMERA_SCALE_FACTOR,   (FLOAT)lookat["y"] * CAMERA_SCALE_FACTOR,   (FLOAT)lookat["z"] * CAMERA_SCALE_FACTOR,1 };
		shared_ptr<Camera> c = make_shared<Camera>(name);
		c->Position() = { (int)eye.x,  (int)eye.y,  (int)eye.z };
		c->Target() = { (int)look.x, (int)look.y, (int)look.z };
		XMFLOAT4 floating_eye = { (FLOAT)eye.x,  (FLOAT)eye.y,  (FLOAT)eye.z, 0 };
		XMFLOAT4 floating_look = { (FLOAT)look.x, (FLOAT)look.y, (FLOAT)look.z,0 };
		FXMVECTOR eyeBall = XMLoadFloat4(&floating_eye);
		FXMVECTOR look_here = XMLoadFloat4(&floating_look);
		c->set_LookAt(eyeBall, look_here, { 0,1,0,0 });
		CameraList[name] = c;
	}
}

void CShaderAppDlg::BuildLights(json& lights)
{
	ambient = lights["Ambient"];
	json fixtures = lights["Fixtures"];

	for (json::const_iterator it = fixtures.cbegin(); it != fixtures.cend(); it++)
	{
		json fixture = *it;
		string fixture_name = fixture["name"];
		string fixture_type = (string)fixture.value("type", "default");
		string fixture_color = fixture.value("color", "WHITE");
		FLOAT fixture_intensity = fixture.value("intensity", 1.0f);
		float fixture_spread = cosf(DegreesToRadians(90.0f - (float)fixture.value("spread", 5.0f)));
		string fixture_status = fixture.value("status", "OFF");
		json position = fixture["position"];
		XMFLOAT4 Position = { position["x"] * CAMERA_SCALE_FACTOR,position["y"] * CAMERA_SCALE_FACTOR,position["z"] * CAMERA_SCALE_FACTOR,1 };
		json target = fixture["target"];
		XMFLOAT4 The_Target = { target.value("x",0.0f) * CAMERA_SCALE_FACTOR,target.value("y", 0.0f) * CAMERA_SCALE_FACTOR,target.value("z", 0.0f) * CAMERA_SCALE_FACTOR,1 };
		if (fixture_type == "PointLight")
		{
			shared_ptr<PointLight> pl = make_shared<PointLight>(Position, The_Target, fixture_color, ambient);
			(fixture_status == "ON") ? pl->TurnOn() : pl->TurnOff();
			RegisterLight(fixture_name, pl);
		}
		else if (fixture_type == "SpotLight")
		{
			shared_ptr<SpotLight> sl = make_shared<SpotLight>(Position, The_Target, fixture_color, ambient, fixture_spread);
			(fixture_status == "ON") ? sl->TurnOn() : sl->TurnOff();
			sl->Target() = The_Target;
			RegisterLight(fixture_name, sl);
		}
		else if (fixture_type == "DirectionalLight")
		{
			shared_ptr<DirectionalLight> dl = make_shared<DirectionalLight>(Position, The_Target, fixture_color, ambient);
			(fixture_status == "ON") ? dl->TurnOn() : dl->TurnOff();
			RegisterLight(fixture_name, dl);
		}
	}
}

void CShaderAppDlg::RegisterLight(
	const std::string& name,
	const std::shared_ptr<Light>& light)
{
	if (!light)
		return;

	LightList[name] = light;

	if (auto directionalLight =
		std::dynamic_pointer_cast<DirectionalLight>(light))
	{
		DirectionalLightList[name] = directionalLight;

		if (m_LightingPSO)
		{
			m_LightingPSO->SetDirectionalLights(
				DirectionalLightList
			);
		}
	}
	else if (auto spotLight =
		std::dynamic_pointer_cast<SpotLight>(light))
	{
		SpotLightList[name] = spotLight;
	}
	else if (auto pointLight =
		std::dynamic_pointer_cast<PointLight>(light))
	{
		PointLightList[name] = pointLight;
	}
}

void CShaderAppDlg::BuildActions(json& actions) {}

// Specific Item Factories

shared_ptr<Object> CShaderAppDlg::BuildCircle(json& element, string name)
{
	string circle_name = element["name"];
	XMFLOAT4 position = { 0,0,0,1 };
	string rgb = element.value("color", "WHITE");
	float diameter = element["diameter"];
	boolean isVisible = true;
	shared_ptr<gCircle> c = make_shared<gCircle>(circle_name, position, diameter, rgb);
	return c;
}

shared_ptr<Object> CShaderAppDlg::BuildSquare(json& element, string name)
{
	string square_name = element["name"];
	string rgb = element.value("color", "WHITE");
	float width = element["width"];
	float half_width = width / 2.0f;

	XMFLOAT4 c0 = { -half_width, half_width,0,1 };
	XMFLOAT4 c1 = { half_width, half_width,0,1 };
	XMFLOAT4 c2 = { half_width,-half_width,0,1 };
	XMFLOAT4 c3 = { -half_width,-half_width,0,1 };
	XMFLOAT4 c4 = { -half_width, half_width,0,1 };
	vector<XMFLOAT4> coords = { c0,c1,c2,c3,c4 };
	shared_ptr<gPolygon> s = make_shared<gPolygon>(square_name, coords, rgb);
	coords.erase(coords.begin(), coords.end());
	coords.clear();
	return s;
}

shared_ptr<Object> CShaderAppDlg::BuildRectangle(json& element, string name)
{
	string rectangle_name = element["name"];
	string rgb = element.value("color", "WHITE");
	float width = element["width"];
	float height = element["height"];
	float half_width = width / 2.0f;
	float half_height = height / 2.0f;

	XMFLOAT4 c0 = { -half_width, half_height,0,1 };
	XMFLOAT4 c1 = { half_width, half_height,0,1 };
	XMFLOAT4 c2 = { half_width,-half_height,0,1 };
	XMFLOAT4 c3 = { -half_width,-half_height,0,1 };
	XMFLOAT4 c4 = { -half_width, half_height,0,1 };
	vector<XMFLOAT4> coords = { c0,c1,c2,c3,c4 };
	shared_ptr<gPolygon> r = make_shared <gPolygon>(rectangle_name, coords, rgb);
	coords.erase(coords.begin(), coords.end());
	coords.clear();
	return r;
}
shared_ptr<Object> CShaderAppDlg::BuildPolygon(json& element, string name)
{
	string polygon_name = element["name"];
	string rgb = element.value("color", "WHITE");
	json coords = element["coords"];
	vector <XMFLOAT4> here;
	for (json::const_iterator it = coords.cbegin(); it != coords.cend(); it++)
	{
		json coord = *it;
		XMFLOAT4 c = { coord["x"],coord["y"],coord["z"],1.0f };
		here.push_back(c);
	}
	shared_ptr <gPolygon> p = make_shared<gPolygon>(polygon_name, here, rgb);
	here.erase(here.begin(), here.end());
	here.clear();
	return p;
}

uint32_t FindFontFamily(string ff)
{
	if (ff == "ROMAN")  return FF_ROMAN;	       /* Times Roman, Century Schoolbook, etc. */
	if (ff == "SWISS")  return FF_SWISS;		   /* Helvetica, Swiss, etc. */
	if (ff == "MODERN") return FF_MODERN;		   /* Pica, Elite, Courier, etc. */
	if (ff == "SCRIPT") return FF_SCRIPT;          /* Cursive, etc. */
	if (ff == "DECORATIVE") return FF_DECORATIVE;  /* Old English, etc. */
	return FF_DONTCARE;
}

shared_ptr<Object> CShaderAppDlg::BuildText(json& element, string name)
{
	XMFLOAT4 coord = { 0,0,0,1 };
	string fontname = element["fontname"];
	string fontfamily = element["fontfamily"];
	uint32_t fontFamily = FindFontFamily(fontfamily); // FF_SWISS;
	int fontSize = element["fontsize"];
	string vOrigin = element.value("vOrigin", "Left");
	string hOrigin = element.value("hOrigin", "Top");
	string rgb = element.value("color", "WHITE");
	json lines = element["text"];
	vector<string> theText;
	for (json::const_iterator it = lines.cbegin(); it != lines.cend(); it++)
	{
		string line = *it;
		theText.push_back(line);
	}
	shared_ptr<TextMesh> t = make_shared<TextMesh>(name, theText, fontname, fontFamily, fontSize, rgb, hOrigin, vOrigin);
	theText.erase(theText.begin(), theText.end());
	theText.clear();
	return t;
}
ImageJustify GetImageJustification(string justification)
{
	if (justification == "FLIP_IMAGE") return FLIP_IMAGE;
	if (justification == "CENTER_IMAGE") return CENTER_IMAGE;
	if (justification == "CENTER_FLIP_IMAGE") return CENTER_FLIP_IMAGE;
	if (justification == "BOTTOM_IMAGE") return BOTTOM_IMAGE;
	if (justification == "BOTTOM_FLIP_IMAGE") return BOTTOM_FLIP_IMAGE;
	if (justification == "TOP_IMAGE") return TOP_IMAGE;
	if (justification == "TOP_FLIP_IMAGE") return TOP_FLIP_IMAGE;
	return NOTHING;
}

VertexExchange GetVertexExchange(string exchange)
{
	if (exchange == "XZY") return XZY;
	if (exchange == "YXZ") return YXZ;
	if (exchange == "YZX") return YZX;
	if (exchange == "ZXY") return ZXY;
	if (exchange == "ZYX") return ZYX;
	return XYZ;
}
shared_ptr<Object> CShaderAppDlg::BuildMesh(json& element, string name)
{
	string objName = element["name"];
	string filename = element["filename"];
	string color = element.value("color", "WHITE");
	float scale = element.value("scale", 1.0f);
//	g_Object* mesh = new MeshObject(name, filename, scale, FindColor(color), exchange, justification);
//	return mesh;
	return nullptr;
}

shared_ptr<Object> CShaderAppDlg::BuildOBJFileMesh(json& element, string name)
{
	string objName = element["name"];
	string filename = element["filename"];
	string color = element.value("color", "WHITE");
	float scale = element.value("scale", 1.0f);
	string reverseYZ = element.value("vertex exchange", "XYZ");
	VertexExchange exchange = GetVertexExchange(reverseYZ);
	ImageJustify justification = GetImageJustification(element.value("justification", "NOTHING"));
	shared_ptr<MeshObject> mesh = make_shared<OBJ_File_Mesh>(name, filename, scale, color, exchange, justification);
	return mesh;
}

shared_ptr<Object> CShaderAppDlg::BuildTemplateReference(json& element, string name)
{
	string objName = element["name"];
	string templatename = element["template"];
	string color = element.value("color", "WHITE");
	return make_shared <TemplateRef>(objName, templatename, color);
}

shared_ptr<Object> CShaderAppDlg::BuildFractal(json& element, string name)
{
	string objName = element["name"];
	string type = element["fractal type"];
	int iterations = element["iterations"];
	string seed = element.value("seed", "random");
	json baseVertex = element["base_vertex"];
	vector<VertexType> vertices;
	vector<vertex_index> indices;
	for (json::const_iterator it = baseVertex.cbegin(); it != baseVertex.cend(); it++)
	{
		float x = (*it)["x"];
		float y = (*it)["y"];
		float z = (*it)["z"];
		XMFLOAT4 color = FindColor((*it)["c"]);
		vertices.push_back(make_pair(XMFLOAT4(x, y, z, 1), color));
	}
	json baseIndex = element["base_index"];
	for (json::const_iterator it = baseIndex.cbegin(); it != baseIndex.cend(); it++)
	{
		uint16_t i0 = (*it)["0"];
		uint16_t i1 = (*it)["1"];
		uint16_t i2 = (*it)["2"];
		indices.push_back({ { i0, i1, i2 }, {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}, {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF} });
	}
	string color_125 = element.value("color_125", "BLUE");
	string color_400 = element.value("color_400", "GREEN");
	string color_750 = element.value("color_750", "BROWN");
	string color_1000 = element.value("color_1000", "WHITE");
	vector<string> colors = { color_125,color_400,color_750,color_1000 };
	json scale = element.value("scale", json::object({ {"x",0},{"y",0},{"z",0} }));
	XMFLOAT3 scaleFactor = { scale["x"],scale["y"],scale["z"] };
	if (seed == "random")
	{
		uint32_t seed_value = static_cast<unsigned int>(time(0));
		std::srand(seed_value);
		controlBoard->UpdateContentSeedValue(seed_value);
	}
	else
	{
		uint32_t seed_value = std::stoul(seed);
		std::srand(seed_value);
		controlBoard->UpdateContentSeedValue(seed_value);
	}
	shared_ptr<Fractal> f = make_shared<Fractal>(objName, type, iterations, scaleFactor, vertices, indices, colors);
	return f;
};
shared_ptr<Object> CShaderAppDlg::BuildBox(json& element, string name)
{
	string box_name = element["name"];
	float width = element["width"];
	float height = element["height"];
	float depth = element["depth"];
//	json colors = element["color"];
	string llf = element["llf"];
	string rlf = element["rlf"];
	string ruf = element["ruf"];
	string luf = element["luf"];
	string llr = element["llr"];
	string rlr = element["rlr"];
	string rur = element["rur"];
	string lur = element["lur"];
	map<string, string> colorTable = { { "llf", llf },
										{ "rlf", rlf },
										{ "ruf", ruf },
										{ "luf", luf },
										{ "llr", llr },
										{ "rlr", rlr },
										{ "rur", rur },
										{ "lur", lur } };

	shared_ptr<Box> b = make_shared<Box>(box_name, width, height, depth, colorTable);
	colorTable.erase(colorTable.begin(), colorTable.end());
	colorTable.clear();
	return b;
}

shared_ptr<Object> CShaderAppDlg::BuildTunnel(json& element, string name)
{
	string tunnel_name = element["name"];
	string tunnel_type = element["tunnel type"];
	if (tunnel_type == "ROUND") return BuildRoundTunnel(element, name);
	if (tunnel_type == "TRIANGLE") return BuildTriangleTunnel(element, name);
	return nullptr;
}

shared_ptr<Object> CShaderAppDlg::BuildRoundTunnel(json& element, string name)
{
	/*
	string tunnel_name = element["name"];
	float diameter = element["diameter"];
	float length = element["length"];
	float segmentWidth = element["segment width"];
	float tilesPerSegment = element["segment tiles"];
	shared_ptr<RoundTunnel> rt = make_shared<RoundTunnel>(tunnel_name, diameter, length, segmentWidth, tilesPerSegment);
	*/
	return nullptr;
}

shared_ptr<Object> CShaderAppDlg::BuildTriangleTunnel(json& element, string name)
{
	/*
	string tunnel_name = element["name"];
	float base = element["base"];
	float height = element["height"];
	float length = element["length"];
	float segmentWidth = element["segment width"];
	shared_ptr<TriangleTunnel> tt = make_shared<TriangleTunnel>(tunnel_name, base, height, length, segmentWidth);
	*/
	return nullptr;
}

void CShaderAppDlg::ObjectActions(json& setup)
{
	for (json::const_iterator it = setup.cbegin(); it != setup.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		shared_ptr<Object> o = ObjectList[name];
		if (o == nullptr) continue;
		if (element.contains("moveto"))
		{
			json moveTo = element["moveto"];
			float x = moveTo["x"];
			float y = moveTo["y"];
			float z = moveTo["z"];
			o->MoveTo(x, y, z);
		}
		if (element.contains("moveby"))
		{
			json moveBy = element["moveby"];
			float x = moveBy["x"];
			float y = moveBy["y"];
			float z = moveBy["z"];
			o->MoveBy(x, y, z);
		}
		if (element.contains("rotateto"))
		{
			json rotateTo = element["rotateto"];
			float p = rotateTo["pitch"];
			float y = rotateTo["yaw"];
			float r = rotateTo["roll"];
			o->RotateTo(p, y, r);
		}
		if (element.contains("rotateby"))
		{
			json rotateBy = element["rotateby"];
			float p = rotateBy["pitch"];
			float y = rotateBy["yaw"];
			float r = rotateBy["roll"];
			o->RotateBy(p, y, r);
		}
		if (element.contains("scaleto"))
		{
			json scaleTo = element["scaleto"];
			float x = scaleTo["x"];
			float y = scaleTo["y"];
			float z = scaleTo["z"];
			o->ScaleTo(x, y, z);
		}
		if (element.contains("scaleby"))
		{
			json scaleBy = element["scaleby"];
			float x = scaleBy["x"];
			float y = scaleBy["y"];
			float z = scaleBy["z"];
			o->ScaleBy(x, y, z);
		}
	}
//	Refresh();
}

