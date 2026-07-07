#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

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
		//ImU32 col = IM_COL32_WHITE;
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
		Vec pos;
		double radius = 0.0;
	};

	void FruchtermanReingold();
	void Dfs(int statrt_id, const std::unordered_map<int, std::vector<int>>& adj, std::unordered_set<int>& visited, std::vector<int>& group_ids);
	void GroupConnectedNodes();
	void GroupRadialPull();
	void Circle(std::vector<Node>& nodes);
	void CentreAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes);
	void Bake(std::vector<Node> newNodes, std::vector<Edge> newEdges);

	int iterations = 200;
	double k = 1.6;

	int scale = 1;
	double stepAmount = 0.1;

	std::vector<Node> nodes;
	std::unordered_map<int, int> nodeIndex;
	std::vector<Edge> edges;

	std::vector<Group> groups;
};

TrackMap::Vec operator+(TrackMap::Vec lhs, const TrackMap::Vec& rhs);
TrackMap::Vec operator-(TrackMap::Vec lhs, const TrackMap::Vec& rhs);
TrackMap::Vec operator*(TrackMap::Vec lhs, double scalar);
TrackMap::Vec operator*(double scalar, TrackMap::Vec rhs);
TrackMap::Vec operator/(TrackMap::Vec lhs, double scalar);