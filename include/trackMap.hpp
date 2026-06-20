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
		float zoomVelocity = 0.0f;
		
		void SetZoom(float z, float dt)
		{
			zoomVelocity += z;
			zoomVelocity = std::clamp(zoomVelocity, -4.5f, 4.5f); // Max velocity
			zoom *= std::exp(zoomVelocity * dt);
			zoom = std::clamp(zoom, 0.1f, 15.0f); // Max & min zoom
			zoomVelocity *= 0.85f; // Dampening
		}
		
		ImVec2 ToScreenPos(double x, double y)
		{
			return ImVec2(
				(float)(x * zoom + pos.x),
				(float)(y * zoom + pos.y)
			);
		}
	};

	void bake(std::vector<Node> newNodes, std::vector<Edge> newEdges);
	void circle(std::vector<Node>& nodes);
	void centreAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes);
	void render();

	int iterations = 500;
	double k = 1.4;
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