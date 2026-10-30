#include "stdafx.h"
#include <afx.h>
#include <afxstr.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstringt.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgicommon.h>
#include <dxgiformat.h>
#include <exception>
#include <iostream>
#include <memory>
#include <queue>
#include <speechapi_cxx.h>
#include <speechapi_cxx_enums.h>
#include <speechapi_cxx_recognition_result.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_speech_recognizer.h>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "Actor.h"
#include "Camera.h"
#include "d3d12.h"
#include "d3d12sdklayers.h"
#include "d3dx12.h"
#include "DXImage.h"
#include "DXImageHelper.h"
#include "MeshRender.h"
#include "Model.h"
#include "scripting.h"
#include "steptimer.h"
#include "Utility.h"
#include "Win32Application.h"
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Microsoft::CognitiveServices::Speech;
using namespace std;

extern std::unordered_map<std::string, std::shared_ptr<Actor>> ActorList;

void MeshRender::OnKeyDown(UINT8 key)
{
	if (key == ' ')
	{
	// Start recognition (blocks until a single phrase is spoken)
		auto result = speechRecognizer->RecognizeOnceAsync().get();

		// Check the result status
		if (result->Reason == ResultReason::RecognizedSpeech)
		{
			ParseVoiceCommand(result->Text);
//			cout << "RECOGNIZED: " <<  << "\n";
		}
		else if (result->Reason == ResultReason::NoMatch)
		{
			cout << "NOMATCH: Speech could not be recognized.\n";
		}
		else if (result->Reason == ResultReason::Canceled)
		{
			auto cancellation = CancellationDetails::FromResult(result);
			cout << "CANCELED: Reason=" << (int)cancellation->Reason << "\n";
			if (cancellation->Reason == CancellationReason::Error)
			{
				cout << "CANCELED: ErrorCode=" << (int)cancellation->ErrorCode << "\n";
				cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << "\n";
			}
		}
	}
}

string toLower(string& s)
{
	for (char& c : s)
	{
		c = std::tolower(static_cast<unsigned char>(c));
	}
	return s;
}
int ParseTextNumbers(string n)
{
	return 0;
}

bool MeshRender::wordIsKeyWord(string word)
{
	for (auto it = componentKeywords.begin(); it != componentKeywords.end(); it++)
	{
		string s = *it;
		if (s == toLower(word)) return true;
	}
	return false;
}

bool MeshRender::wordIsCommand(string word)
{
	for (auto it = commandKeywords.begin(); it != commandKeywords.end(); it++)
	{
		string s = *it;
		if (s == toLower(word)) return true;
	}
	return false;
}

void MeshRender::ParseVoiceCommand(std::string command)
{
	std::erase(command, '.');
	std::erase(command, ',');
	vector<string> parts = split(command, ' ');
	// actor name
	// component name
	// action
	bool stillWorking = true;
	VoiceCommandStates commandState = GatheringActorName;
	int partIndex = 0;
	string actorName = "";
	string componentName = "";
	string commandName = "";
	string parameter = "";
	string word = "";
	VoiceComponents theComponent = component_Not_Set;
	VoiceCommands theCommand = Command_Not_Set;
	while (stillWorking)
	{
		if (partIndex < parts.size()) word = parts[partIndex];
		switch (commandState)
		{
			case GatheringActorName:
				if (wordIsKeyWord(word))
				{
					// finished building actor name -- get pointer to actor
					commandState = GatheringComponentName;
				}
				else
				{
					if (actorName.size() > 0) actorName += " ";
					actorName += toLower(word);
					partIndex++;
				}
				break;
			case GatheringComponentName:
				if (partIndex >= parts.size())
				{
					// no more text, formulate the command and pass to the "OnUpdate" method
					stillWorking = false;
				}
				else if (wordIsCommand(word))
				{
					// finished building component
					// get code for the component
					commandState = GatheringCommand;
				}
				else
				{
					if (theComponent == component_Not_Set)
					{
						auto it = find(componentKeywords.begin(), componentKeywords.end(), word);
						if (it != componentKeywords.end())
						{
							int index = (int)(it - componentKeywords.begin());
							theComponent = componentID[index];
						}
					}
					if (componentName.size() > 0) componentName += " ";
					componentName += word;
					partIndex++;
				}
				break;
			case GatheringCommand:
				if (partIndex >= parts.size())
				{
					// no more text, formulate the command and pass to the "OnUpdate" method
					stillWorking = false;
				}
				else if (not wordIsCommand(word))
				{
					// done finding command
					// most commands are done at this time.
					// only "flaps down" might have a parameter (% of full flaps or "full")
					commandState = GatheringCommandParameters;
				}
				else
				{
					if (theCommand == Command_Not_Set)
					{
						auto it = find(commandKeywords.begin(), commandKeywords.end(), word);
						if (it != commandKeywords.end())
						{
							int index = (int)(it - commandKeywords.begin());
							theCommand = commandID[index];
						}
					}
					if (commandName.size() > 0) commandName += " ";
					commandName += word;
					partIndex++;
				}
				break;
			case GatheringCommandParameters:
				if (partIndex >= parts.size())
				{
					// no more text, formulate the command and pass to the "OnUpdate" method
					stillWorking = false;
				}
				else
				{
					parameter += word;
					partIndex++;
				}
				break;
		}
		if (partIndex >= parts.size()) break;
	}
	CString str;
	str.Format(_T("actor: %s command: %s action: %s parameter: %s\r\n"), actorName.c_str(), componentName.c_str(), commandName.c_str(), parameter.c_str());
	OutputDebugString(str);
	// assemble the command message
	if (ActorList.contains(actorName))
	{
		shared_ptr<Actor> actor = ActorList[actorName];
		if ((actor == nullptr) or (actorName.size() == 0))
		{
			str.Format(_T("actor : %s was not found\r\n"), actorName.c_str());
			OutputDebugString(str);
		}
		else if ((theComponent == component_Not_Set) or (theCommand == Command_Not_Set))
		{
			str.Format(_T("actor : %s component or command not heard properly\r\n"), actorName.c_str());
			OutputDebugString(str);
		}
		else
		{
			switch (theComponent)
			{
				case Component_Flaps:
					HandleFlaps(actor, theCommand, "50");
					break;
				case Component_Landing_Gear:
					HandleLandingGear(actor, theCommand);
					break;
				case Component_Front_Canopy:
					HandleFrontCanopy(actor, theCommand);
					break;
				case Component_Rear_Canopy:
					HandleRearCanopy(actor, theCommand);
					break;
				case Component_Both_Canopies:
					HandleFrontCanopy(actor, theCommand);
					HandleRearCanopy(actor, theCommand);
					break;
				case Component_Speed_Brakes:
					HandleSpeedBrakes(actor, theCommand);
					break;
				case Aircraft_Action_Aileron_Roll:
					break;
				case Aircraft_Action_Barrel_Roll:
					break;
				case Aircraft_Action_Climb:
					break;
				case Aircraft_Action_Dive:
					break;
				case Aircraft_Action_Bank:
					break;

			}
		}
	}
	else
	{
		str.Format(_T("actor : %s was not found\r\n"), actorName.c_str());
		OutputDebugString(str);
	}
}

void MeshRender::HandleFlaps(std::shared_ptr<Actor> actor, VoiceCommands cmd, string percentage)
{
	struct RemoteActionRequest request = { actor,ScriptActionCode::NoAction,percentage };
	request.actionCode = (cmd == Command_Up) ? Action_FrontCanopyOpen : Action_FrontCanopyClose;
	requestList.push(request);
}
void MeshRender::HandleFrontCanopy(std::shared_ptr<Actor> actor, VoiceCommands cmd)
{
	struct RemoteActionRequest request = { actor,ScriptActionCode::NoAction,"" };
	if ((cmd == Command_Open) or (cmd == Command_Up)) request.actionCode = Action_FrontCanopyOpen;
	if ((cmd == Command_Close) or (cmd == Command_Down)) request.actionCode = Action_FrontCanopyClose;
	requestList.push(request);
}
void MeshRender::HandleRearCanopy(std::shared_ptr<Actor> actor, VoiceCommands cmd)
{
	struct RemoteActionRequest request = { actor,ScriptActionCode::NoAction,"" };
	if ((cmd == Command_Open) or (cmd == Command_Up)) request.actionCode = Action_RearCanopyOpen;
	if ((cmd == Command_Close) or (cmd == Command_Down)) request.actionCode = Action_RearCanopyClose;
	requestList.push(request);
}
void MeshRender::HandleLandingGear(std::shared_ptr<Actor> actor, VoiceCommands cmd)
{
	struct RemoteActionRequest request = { actor,ScriptActionCode::NoAction,"" };
	request.actionCode = (cmd == Command_Up) ? Action_GearUp : Action_GearDown;
	requestList.push(request);
}
void MeshRender::HandleSpeedBrakes(std::shared_ptr<Actor> actor, VoiceCommands cmd)
{
	struct RemoteActionRequest request = { actor,ScriptActionCode::NoAction,"" };
	request.actionCode = (cmd == Command_Extend) ? Action_SpeedBrakeExtend : Action_SpeedBrakeRetract;
	requestList.push(request);
}
