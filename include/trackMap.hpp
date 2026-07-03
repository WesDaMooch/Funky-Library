#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include "imgui.h"

// Debug
#include <string>

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
	};

	struct Edge
	{
		int A_id = -1;
		int B_id = -1;

		// Weight (Rating)
		bool A_direction = true;
		bool B_direction = true;
	};

	struct Group
	{
		std::vector<int> ids;
		std::pair<Vec, Vec> rect; // TODO: replace with pos?
		double width = 0.0;
		double height = 0.0;
		double area = 0.0;
	};

	struct Group2
	{
		std::vector<int> ids;
		Vec pos;
		double radius = 0.0;
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

	struct Info
	{
		int hoveredId = -1;
		int clickedId = -1;
		ImVec2 pos = { 0.0f, 0.0f };
	};

	void Bake(std::vector<Node> newNodes, std::vector<Edge> newEdges);
	void Circle(std::vector<Node>& nodes);
	void CentreAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes);
	void CentreAndScaleGroup(unsigned int width, unsigned int height, const std::vector<int>& group_ids);
	void Centre(std::vector<Node>& nodes);
	void Rotate(const std::vector<size_t>& group_idxs, int quadrant);
	void GroupConnectedNodes();
	void Pack(const std::vector<size_t>& group_idxs);
	void FruchtermanReingold();
	void GroupFruchtermanReingold(const std::vector<int>& group_ids);
	void GroupRadialPull();
	// TODO: move rendering to main app
	Info Render();

	int iterations = 100;
	double k = 2.0;
	const float baseRadius = 8.0f;

	std::vector<Node> nodes;
	std::unordered_map<int, int> nodeIndex;
	std::vector<Edge> edges;

	std::vector<Group> groups;
	std::vector<Group2> groups2;

	//std::vector<std::pair<Vec, Vec>> groupRects;

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