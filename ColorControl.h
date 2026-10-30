#pragma once
#include <cstdint>
#include <cstdlib>
#include <DirectXMath.h>
#include <iterator>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <windows.h>
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
	boolean transparent = false;
	boolean colorFixed = false; // if true, color will not change
	int changeColorSpeed = 1; // 1 = fast, 2 = medium, 3 = slow
public:
	static std::mt19937 gen; // Seed with time
	static std::uniform_int_distribution<> colordist;
	static std::uniform_int_distribution<> framedist;
	static std::uniform_int_distribution<> framedist_fast;
	static std::uniform_int_distribution<> framedist_medium;
	static std::uniform_int_distribution<> framedist_slow;

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
	boolean IsTransparent() { return transparent; }
	boolean& Transparent() { return transparent; }
	std::string ColorName() { return colorName; }
	boolean IsColorChanging() { return colorChanging; }
	boolean IsColorFixed() { return colorFixed; }
	boolean& ColorFixed() { return colorFixed; }
	int& ChangeColorSpeed() { return changeColorSpeed; }
	DirectX::XMFLOAT4 CurrentColor() { return (colorChanging) ? mixColor : displayColor; }
	DirectX::XMFLOAT4 SetRandomColor(boolean fixed = true)
	{
		UINT randomColor = colordist(gen);
		auto random_it = colorMap.begin();
		std::advance(random_it, randomColor);
		SetColor(random_it->first);
		colorChanging = false;
		colorFixed = fixed;
		return displayColor;
	}
	void SetColor(std::string colorname, boolean fixed = true)
	{
		colorName = colorname;
		displayColor = FindColor(colorname);
		targetColor = displayColor;
		colorChanging = false;
		colorFixed = fixed;
	}
	void SetColor(DirectX::XMFLOAT4 color, boolean fixed = true)
	{
		displayColor = color;
		targetColor = color;
		colorChanging = false;
		colorFixed = fixed;
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
		if ((frame >= frameToChangeColor) && (!colorFixed))
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
		switch (changeColorSpeed)
		{
			case 1: frameToChangeColor = frame + (uint64_t)framedist_fast(gen); break;
			case 2: frameToChangeColor = frame + (uint64_t)framedist_medium(gen); break;
			case 3: frameToChangeColor = frame + (uint64_t)framedist_slow(gen); break;
		}
	}
};
