#pragma once
#include <vector>
#include <cmath>
#include "imgui.h"

struct SpringGraph
{
	
	struct Node
	{
		ImVec2 pos;
		ImVec2 vel;
	};

	struct Edge
	{
		int a, b;
	};

	// TODO: Calulate static graph on library change
	// Glue library manager and spring graph together in the app

	void simulate(std::vector<Node>& nodes, const std::vector<Edge> edges, float dt);

	inline float GetLength(const ImVec2& v)
	{
		return std::sqrt(v.x * v.x + v.y * v.y);
	}

	inline ImVec2 Normalize(const ImVec2& v)
	{
		float length = GetLength(v);

		if (length > 1e-5f)
			return ImVec2(0, 0);

		return ImVec2(v.x / length, v.y / length);
	}
};