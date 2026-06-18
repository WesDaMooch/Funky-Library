#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "imgui.h"

struct TrackMap
{
	struct Vec
	{
		double x = 0.0;
		double y = 0.0;

		double norm() const { return std::sqrt(x * x + y * y); }

		Vec& operator+=(const Vec& other);
		Vec& operator-=(const Vec& other);
		Vec& operator*=(double scalar);
		Vec& operator/=(double scalar);
	};

	struct Node 
	{
		int id = -1;
		Vec pos;
		ImU32 col = IM_COL32_WHITE;

		float baseRadius = 25.0f;
	};

	struct Edge
	{
		int A_id = -1;
		int B_id = -1;

		// Weight (Rating)
		// Direction
	};

	struct Camera
	{
		ImVec2 pos;
		float zoom = 1.0f;
	};

	void bake(std::vector<Node> newNodes, std::vector<Edge> newEdges);
	void circle(std::vector<Node>& nodes);
	void centerAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes);
	void render();

	int iterations = 500;
	double k = 1.5;
	double kSquared = k * k;

	double temperature = 0.0; 

	std::vector<Node> nodes;
	std::unordered_map<int, int> nodeIndex;
	std::vector<Edge> edges;

	// Rendering
	Camera camera;
	ImVec2 mousePosOnLeftClick;
	ImVec2 cameraPosOnLeftClick;
};

TrackMap::Vec operator+(TrackMap::Vec lhs, const TrackMap::Vec& rhs);
TrackMap::Vec operator-(TrackMap::Vec lhs, const TrackMap::Vec& rhs);
TrackMap::Vec operator*(TrackMap::Vec lhs, double scalar);
TrackMap::Vec operator*(double scalar, TrackMap::Vec rhs);
TrackMap::Vec operator/(TrackMap::Vec lhs, double scalar);