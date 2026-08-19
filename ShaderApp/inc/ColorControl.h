#pragma once
#include <algorithm>
#include <ctime>
#include <DirectXMath.h>
#include <iostream>
#include <map>
#include <random>
#include <stdio.h>
#include <string>
#include <wrl.h>
#include "Utility.h"

extern const std::map<std::string, UINT> colorMap;

class ColorControl
{
protected:
	uint64_t frameToChangeColor = 0;
	DirectX::XMFLOAT4 displayColor;
	DirectX::XMFLOAT4 mixColor;
	DirectX::XMFLOAT4 targetColor;
	boolean colorChanging = false;
	std::string colorName = "WHITE";
public:
	static std::mt19937 gen; // Seed with time
	static std::uniform_int_distribution<> colordist;
	static std::uniform_int_distribution<> framedist;

	ColorControl(std::string color = "WHITE") : colorName(color)
	{
		frameToChangeColor = framedist(gen);
//		colordist.param(std::uniform_int_distribution<int>::param_type(0, colorMap.size() - 1));
		displayColor = FindColor(colorName);
		targetColor = displayColor;
	}
	ColorControl(DirectX::XMFLOAT4 color) : displayColor(color), targetColor(color)
	{
		UINT rgb = FRGB2RGB(color);
		colorName = "RGB_0x00" + std::to_string((rgb >> 16) & 0xFF) + std::to_string((rgb >> 8) & 0xFF) + std::to_string(rgb & 0xFF);
	}
	std::string ColorName() { return colorName; }
	boolean IsColorChanging() { return colorChanging; }
	DirectX::XMFLOAT4 CurrentColor() { return (colorChanging) ? mixColor : displayColor; }
	void SetColor(std::string colorname)
	{
		colorName = colorname;
		displayColor = FindColor(colorname);
		targetColor = displayColor;
		colorChanging = false;
	}
	void SetColor(DirectX::XMFLOAT4 color)
	{
		displayColor = color;
		targetColor = color;
		colorChanging = false;
	}
	void GradientColorTo(DirectX::XMFLOAT4 color)
	{
		mixColor = displayColor;
		targetColor = color;
		colorChanging = true;
	}
	void GradientColorTo(std::string colorName) { GradientColorTo(FindColor(colorName)); }
	boolean IsColorEqual(DirectX::XMFLOAT4 color)
	{
		return
			(std::abs(color.x - targetColor.x) < 0.045f) &&
			(std::abs(color.y - targetColor.y) < 0.045f) &&
			(std::abs(color.z - targetColor.z) < 0.045f);
	}
	boolean StepGradientColor(uint64_t frame)
	{
		if (colorChanging)
		{
			mixColor.x += (targetColor.x - mixColor.x) * 0.03f;
			mixColor.y += (targetColor.y - mixColor.y) * 0.03f;
			mixColor.z += (targetColor.z - mixColor.z) * 0.03f;
			if (IsColorEqual(mixColor))
			{
				displayColor = targetColor;
				colorChanging = false;
				NextColorChangeFrame(frame);
			}
			return true;
		}
		return false;
	}
	boolean TimeToChangeColor(uint64_t frame)
	{
		if (frame >= frameToChangeColor)
		{
			UINT randomColor = colordist(gen);
			auto random_it = colorMap.begin();
			std::advance(random_it, randomColor);
			GradientColorTo(random_it->first);
			return true;
		}
		return false;
	}
	// start change to another color in random number of frames between 90 and 300
	void NextColorChangeFrame(uint64_t frame)
	{
		frameToChangeColor = frame + (uint64_t)framedist(gen);
	}
};
