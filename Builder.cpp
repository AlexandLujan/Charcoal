#include "stdafx.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include "Actor.h"
#include "Camera.h"
#include "Const.h"
#include "DXImage.h"
#include "Light.h"
#include "nlohmann/json.hpp"
#include "Object.h"
#include "Scripting.h"
#include "utility.h"

using namespace std;
using namespace DirectX;
using json = nlohmann::json;
void InitializeMaterialMap();

void DXImage::ReadJson(wstring filename)
{
	string jsonFile = wstring_to_string_mb(filename);

	InitializeMaterialMap();

	BuildJSON(jsonFile);
	BuildObjects(objects);
	BuildActors(actors);
	ObjectSetUps(setup);
	BuildCameras(cameras);
	BuildLights(lights);
	BuildScript(script);
}

void DXImage::BuildJSON(string filename)
{
	ifstream file(filename);
	if (!file.is_open()) return;
	document = json::parse(file);
	file.close();
	objects = document["Objects"];
	actors = document["Actors"];
	setup = document["Set Up"];
	cameras = document["Cameras"];
	lights = document["Lights"];
	script = document["Script"];
}

void DXImage::BuildActors(json& actors)
{
	for (json::const_iterator it = actors.cbegin(); it != actors.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		string object = element["object"];
		XMFLOAT3 position = { element["position"]["x"], element["position"]["y"], element["position"]["z"] };
		XMFLOAT3 rotation = { element["rotation"]["x"], element["rotation"]["y"], element["rotation"]["z"] };
		XMFLOAT3 scale = { element["scale"]["x"], element["scale"]["y"], element["scale"]["z"] };

		// find type in Object List and add to correct Actor Type
		// text
		// mesh
		// multiple mesh
		// assembly
		// all actors have position, rotation, scale, material, visibility
		if ((object == "t38") or (object == "T38"))
		{
			shared_ptr<Actor> a = make_shared<T38>(name);
			a->RotateTo(rotation.x, rotation.y, rotation.z);
			a->ScaleTo(scale.x, scale.y, scale.z);
			a->MoveTo(position.x, position.y, position.z);
			ActorList[name] = a;
		}
		if ((object == "C130") or (object == "c130"))
		{
			shared_ptr<Actor> a = make_shared<C130>(name);
			a->RotateTo(rotation.x, rotation.y, rotation.z);
			a->ScaleTo(scale.x, scale.y, scale.z);
			a->MoveTo(position.x, position.y, position.z);
			ActorList[name] = a;
		}
		// plus specific properties for each type
/*		if (type == "TEXT") {}
		if (type == "MESH") {}
		if (type == "FRACTAL") {}
		if (type == "BOX") {}
		if (type == "TUNNEL") {}
		if (type == "STARSHIP") {}
*/
	}
}

void DXImage::BuildObjects(json& objects)
{
	for (json::const_iterator it = objects.cbegin(); it != objects.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		string type = element["type"];
		if (type == "TEXT") ObjectList[name] = BuildText(element, name);
		if (type == "MESH") ObjectList[name] = BuildMesh(element, name);
		if (type == "OBJ_FILE") ObjectList[name] = BuildOBJFileMesh(element, name);
		if (type == "FRACTAL") ObjectList[name] = BuildFractal(element, name);
		if (type == "BOX") ObjectList[name] = BuildBox(element, name);
		if (type == "TUNNEL") ObjectList[name] = BuildTunnel(element, name);
		if (type == "STARSHIP") ObjectList[name] = BuildStarShip(element, name);
	}
}

void DXImage::BuildCameras(json& cameras)
{
	for (json::const_iterator it = cameras.cbegin(); it != cameras.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		json position = element["position"];
		json lookat = element["lookat"];
		float fov = element["fov"];
		FXMVECTOR eye = XMVector3Normalize({ (FLOAT)position["x"], (FLOAT)position["y"], (FLOAT)position["z"] });
		FXMVECTOR look = XMVector3Normalize({ (FLOAT)lookat["x"], (FLOAT)lookat["y"],   (FLOAT)lookat["z"] });
		shared_ptr<Camera> c = make_shared<Camera>(name);
		c->set_LookAt(eye, look, { 0,1,0 });
		CameraList[name] = c;
	}
}

FLOAT Light::Nothing = 0;
XMFLOAT4 Light::Nothing4 = { 0,0,0,0 };

void DXImage::BuildLights(json& lights)
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
		XMFLOAT4 Position = { position["x"],position["y"],position["z"],1 };
		json target = fixture["target"];
		XMFLOAT4 The_Target = { target.value("x",0.0f),target.value("y", 0.0f),target.value("z", 0.0f),1 };
		if (fixture_type == "PointLight")
		{
			shared_ptr<PointLight> pl = make_shared<PointLight>(Position, The_Target, fixture_color, ambient);
			(fixture_status == "ON") ? pl->TurnOn() : pl->TurnOff();
			PointLightList[fixture_name] = pl;
		}
		else if (fixture_type == "SpotLight")
		{
			shared_ptr<SpotLight> sl = make_shared<SpotLight>(Position, The_Target, fixture_color, ambient, fixture_spread);
			(fixture_status == "ON") ? sl->TurnOn() : sl->TurnOff();
			sl->Target() = The_Target;
			SpotLightList[fixture_name] = sl;
		}
		else if (fixture_type == "DirectionalLight")
		{
			shared_ptr<DirectionalLight> dl = make_shared<DirectionalLight>(Position, The_Target, fixture_color, ambient);
			(fixture_status == "ON") ? dl->TurnOn() : dl->TurnOff();
			DirectionalLightList[fixture_name] = dl;
		}
	}
}

void DXImage::BuildScript(json& script)
{
#if 0
	"Script": {
		"movie filename": "tunnel.mp4",
			"start frame" : 0,
			"end frame" : 300,
			"fps" : 30,
			"width" : 1920,
			"height" : 1080,
			"objects" : [] ,
			"cameras" : [] ,
			"lights" : []
	}
#endif
	movie_filename = script.value("movie filename", "");
	movie_start_frame = script.value("start frame", 0);
	movie_end_frame = script.value("end frame", 0);
	movie_fps = script.value("fps", 30.0);
	movie_width = script.value("width", 1920);
	movie_height = script.value("height", 1080);
	json actor_script = script["actors"];
	BuildActorScript(actor_script);
	json camera_script = script["cameras"];
	cameraScripts.clear();
	BuildCameraScript(camera_script);
	json lights_script = script["lights"];
	if (movie_start_frame != 0)
	{
		StartVideoOutput(movie_width, movie_height, movie_fps, movie_filename);
		video_recording_in_progress = false;
		video_output_started = true;
		video_output_finished = false;
	}
}

void DXImage::BuildCameraScript(json& camera_script)
{
	for (json::const_iterator it = camera_script.cbegin(); it != camera_script.cend(); it++)
	{
		json element = *it;
		string name = element["camera name"];
		json schedule = element["camera script"];
		vector<shared_ptr<ScriptSchedule>> sked = BuildSchedule(schedule);
		shared_ptr<CameraScript> cs = make_shared<CameraScript>(name);
		uint64_t start_frame = sked[0]->scheduled_frame;
		uint64_t end_frame = sked[sked.size() - 1]->scheduled_frame;
		cs->LoadSchedule(start_frame, end_frame, sked);
		cameraScripts.push_back(cs);
	}
}

void DXImage::BuildActorScript(json& actor_script)
{
	for (json::const_iterator it = actor_script.cbegin(); it != actor_script.cend(); it++)
	{
		json element = *it;
		string name = element["actor name"];
		json schedule = element["actor script"];
		vector<shared_ptr<ScriptSchedule>> sked = BuildSchedule(schedule);
		shared_ptr<ActorScript> os = make_shared<ActorScript>(name);
		uint64_t start_frame = sked[0]->scheduled_frame;
		uint64_t end_frame = sked[sked.size() - 1]->scheduled_frame;
		os->LoadSchedule(start_frame, end_frame, sked);
		actorScripts.push_back(os);
	}
}

XMFLOAT3 BuildFloat3(json& coords)
{
	return { (float)coords.value("x",0.0f),
			 (float)coords.value("y",0.0f),
			 (float)coords.value("z",0.0f) };
}
ScriptActionCode ParseScriptAction(string action)
{
	if (action == "Bank Left")			return Action_BankLeft;
	if (action == "Bank Right")			return Action_BankRight;
	if (action == "Elevator Climb")		return Action_ElevatorClimb;
	if (action == "Elevator Dive")		return Action_ElevatorDive;
	if (action == "Flaps Down")			return Action_FlapsDown;
	if (action == "Flaps Up")			return Action_FlapsUp;
	if (action == "Front Canopy Close")	return Action_FrontCanopyClose;
	if (action == "Front Canopy Open")	return Action_FrontCanopyOpen;
	if (action == "Gear Down")			return Action_GearDown;
	if (action == "Gear Up")			return Action_GearUp;
	if (action == "Rear Canopy Close")	return Action_RearCanopyClose;
	if (action == "Rear Canopy Open")	return Action_RearCanopyOpen;
	if (action == "Rudder Left")		return Action_RudderLeft;
	if (action == "Rudder Right")		return Action_RudderRight;
	if (action == "Speed Brake Extend")	return Action_SpeedBrakeExtend;
	if (action == "Speed Brake Retract")return Action_SpeedBrakeRetract;
	return NoAction;
}
vector<shared_ptr<ScriptSchedule>> DXImage::BuildSchedule(json& a_schedule)
{
	vector<shared_ptr<ScriptSchedule>> schedule;
	for (json::const_iterator it = a_schedule.cbegin(); it != a_schedule.cend(); it++)
	{
		json element = *it;
		uint64_t frame = element["frame"];
		XMFLOAT3 location = BuildFloat3(element["location"]);
		XMFLOAT3 rotation = BuildFloat3(element["rotation"]);
		XMFLOAT3 target = BuildFloat3(element["target"]);
		string parameter = element.value("parameter", "");
		string actiontext = element.value("action", "No Action");
		ScriptActionCode action = ParseScriptAction(actiontext);
		shared_ptr<ScriptSchedule> s = make_shared<ScriptSchedule>(frame, location, rotation, target, action, parameter);
		schedule.push_back(s);
	}
	return schedule;
}

// Specific Item Factories

uint32_t FindFontFamily(string ff)
{
	if (ff == "ROMAN")  return FF_ROMAN;	       /* Times Roman, Century Schoolbook, etc. */
	if (ff == "SWISS")  return FF_SWISS;		   /* Helvetica, Swiss, etc. */
	if (ff == "MODERN") return FF_MODERN;		   /* Pica, Elite, Courier, etc. */
	if (ff == "SCRIPT") return FF_SCRIPT;          /* Cursive, etc. */
	if (ff == "DECORATIVE") return FF_DECORATIVE;  /* Old English, etc. */
	return FF_DONTCARE;
}

shared_ptr<Object> DXImage::BuildText(json& element, string name)
{
	XMFLOAT4 coord = { 0,0,0,1 };
	string fontname = element["fontname"];
	string fontfamily = element["fontfamily"];
	uint32_t fontFamily = FindFontFamily(fontfamily); // FF_SWISS;
	int fontSize = element["fontsize"];
	string vOrigin = element.value("vOrigin", "Top");
	string hOrigin = element.value("hOrigin", "Left");
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

shared_ptr<Object> DXImage::BuildMesh(json& element, string name)
{
	string objName = element["name"];
	string filename = element["filename"];
	string color = element.value("color", "WHITE");
	float scale = element.value("scale", 1.0f);
//	g_Object* mesh = new MeshObject(name, filename, scale, FindColor(color), exchange, justification);
//	return mesh;
	return nullptr;
}

shared_ptr<Object> DXImage::BuildOBJFileMesh(json& element, string name)
{
	string objName = element["name"];
	string filename = element["filename"];
	string directoryname = element["directory"];
	string color = element.value("color", "WHITE");
	float scale = element.value("scale", 1.0f);
	json meshNames = element["mesh_names"];
	string reverseYZ = element.value("vertex exchange", "XYZ");
	boolean woundCCW = element.value("woundCCW", true);
	VertexExchange exchange = GetVertexExchange(reverseYZ);
	ImageJustify justification = GetImageJustification(element.value("justification", "NOTHING"));
	json mirror = element["mirror"];
	XMUINT3 mirror_settings = { (UINT)mirror.value("x",0),
								(UINT)mirror.value("y",0),
								(UINT)mirror.value("z",0) };

	shared_ptr<OBJ_File_Mesh> mesh = make_shared<OBJ_File_Mesh>(name, meshNames, directoryname, filename, scale, color, exchange, justification, mirror_settings, woundCCW);
	return mesh;
}

//shared_ptr<Object> DXImage::BuildTemplateReference(json& element, string name)
//{
//	string objName = element["name"];
//	string templatename = element["template"];
//	string color = element.value("color", "WHITE");
//	return make_shared <TemplateRef>(objName, templatename, color);
//}

shared_ptr<Object> DXImage::BuildFractal(json& element, string name)
{
	string objName = element["name"];
	string type = element["fractal type"];
	int iterations = element["iterations"];
	string seed = element.value("seed", "random");
	json baseVertex = element["base_vertex"];
	vector<XMFLOAT3> vertices;
	vector<XMUINT3> indices;
	for (json::const_iterator it = baseVertex.cbegin(); it != baseVertex.cend(); it++)
	{
		float x = (*it)["x"];
		float y = (*it)["y"];
		float z = (*it)["z"];
		XMFLOAT4 color = FindColor((*it)["c"]);
		vertices.push_back(XMFLOAT3(x, y, z));
	}
	json baseIndex = element["base_index"];
	for (json::const_iterator it = baseIndex.cbegin(); it != baseIndex.cend(); it++)
	{
		uint16_t i0 = (*it)["0"];
		uint16_t i1 = (*it)["1"];
		uint16_t i2 = (*it)["2"];
		indices.push_back({ i0,i1,i2 });
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
	}
	else
	{
		uint32_t seed_value = std::stoul(seed);
		std::srand(seed_value);
	}
	shared_ptr<Fractal> f = make_shared<Fractal>(objName, type, iterations, vertices, indices);
	return f;
};

shared_ptr<Object> DXImage::BuildBox(json& element, string name)
{
	string box_name = element["name"];
	float width = element["width"];
	float height = element["height"];
	float depth = element["depth"];
//	json colors = element["color"];
	string front = element["front"];
	string back = element["back"];
	string top = element["top"];
	string bottom = element["bottom"];
	string left = element["left"];
	string right = element["right"];
	map<string, string> colorTable = { { "front", front },
										{ "back", back },
										{ "top", top },
										{ "bottom", bottom },
										{ "left", left },
										{ "right", right } };

	shared_ptr<Box> b = make_shared<Box>(box_name, width, height, depth, colorTable);
	colorTable.erase(colorTable.begin(), colorTable.end());
	colorTable.clear();
	return b;
}

shared_ptr<Object> DXImage::BuildTunnel(json& element, string name)
{
	string tunnel_name = element["name"];
	string tunnel_type = element["tunnel type"];
	if (tunnel_type == "ROUND") return BuildRoundTunnel(element, name);
	if (tunnel_type == "TRIANGLE") return BuildTriangleTunnel(element, name);
	return nullptr;
}

shared_ptr<Object> DXImage::BuildRoundTunnel(json& element, string name)
{
	string tunnel_name = element["name"];
	float diameter = element["diameter"];
	float length = element["length"];
	float segmentWidth = element["segment width"];
	float tilesPerSegment = element["segment tiles"];
	shared_ptr<RoundTunnel> rt = make_shared<RoundTunnel>(tunnel_name, diameter, length, segmentWidth, tilesPerSegment);
	return rt;
}

shared_ptr<Object> DXImage::BuildTriangleTunnel(json& element, string name)
{
	string tunnel_name = element["name"];
	float base = element["base"];
	float height = element["height"];
	float length = element["length"];
	float segmentWidth = element["segment width"];
	shared_ptr<TriangleTunnel> tt = make_shared<TriangleTunnel>(tunnel_name, base, height, length, segmentWidth);
	return tt;
}

shared_ptr<Object> DXImage::BuildStarShip(json& element, string name)
{
	string starship_name = element["name"];
	float base_cube = element["cube size"];
	float spire_height = element["spire height"];
	UINT pin_count = element["pin count"];
	string color = element.value("color", "WHITE");

	shared_ptr<StarShip> rt = make_shared<StarShip>(starship_name, color, base_cube, spire_height, pin_count);
	return rt;
}

void DXImage::ObjectSetUps(json& setup)
{
	for (json::const_iterator it = setup.cbegin(); it != setup.cend(); it++)
	{
		json element = *it;
		string name = element["name"];
		shared_ptr<Actor> o = ActorList[name];
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
}
