#include "../include/trackMap.hpp"

TrackMap::Vec& TrackMap::Vec::operator+=(const Vec& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

TrackMap::Vec& TrackMap::Vec::operator-=(const Vec& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

TrackMap::Vec& TrackMap::Vec::operator*=(double scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

TrackMap::Vec& TrackMap::Vec::operator/=(double scalar)
{
	x /= scalar;
	y /= scalar;
	return *this;
}

TrackMap::Vec operator+(TrackMap::Vec lhs, const TrackMap::Vec& rhs)
{
	lhs += rhs;
	return lhs;
}

TrackMap::Vec operator-(TrackMap::Vec lhs, const TrackMap::Vec& rhs)
{
	lhs -= rhs;
	return lhs;
}

TrackMap::Vec operator*(TrackMap::Vec lhs, double scalar)
{
	lhs *= scalar;
	return lhs;
}

TrackMap::Vec operator*(double scalar, TrackMap::Vec rhs)
{
	rhs *= scalar;
	return rhs;
}

TrackMap::Vec operator/(TrackMap::Vec lhs, double scalar)
{
	lhs /= scalar;
	return lhs;
}

// Layout
void TrackMap::FruchtermanReingold()
{
	double temperature = 10 * std::sqrt(nodes.size());
	double kSquared = k * k;

	std::vector<Vec> forces(nodes.size());

	for (int iteration = 0; iteration < iterations; iteration++)
	{
		std::fill(forces.begin(), forces.end(), Vec{});

		// Repulsion force between vertice pairs
		for (size_t i = 0; i < nodes.size(); i++)
		{
			for (size_t j = i + 1; j < nodes.size(); j++)
			{
				Vec delta = nodes[i].pos - nodes[j].pos;

				double distance = delta.norm();

				// Handle distance small distance
				// TODO: add random jitters
				if (distance < 1e-6)
					continue;

				// > 1000.0: not worth computing
				//if (distance > maxDistance)
				//	continue;

				double repulsion = kSquared / distance;

				forces[i] += delta / distance * repulsion;
				forces[j] -= delta / distance * repulsion;
			}
		}

		// Attraction force between edges
		for (const Edge& e : edges)
		{
			int a = nodeIndex[e.A_id];
			int b = nodeIndex[e.B_id];

			Vec delta = nodes[a].pos - nodes[b].pos;

			double distance = delta.norm();

			if (distance == 0.0)
				continue;

			double attraction = distance * distance / k;

			forces[a] -= delta / distance * attraction;
			forces[b] += delta / distance * attraction;
		}


		// Max movement capped by current temperature
		for (size_t i = 0; i < nodes.size(); i++)
		{
			double forcesNorm = forces[i].norm();

			// < 1.0: not worth computing
			if (forcesNorm < 1.0)
				continue;

			double cappedForcesNorm = std::min(forcesNorm, temperature);
			Vec cappedForces = forces[i] / forcesNorm * cappedForcesNorm;

			nodes[i].pos += cappedForces;
		}

		temperature *= 0.95;
	}
}


// Recursive depth first search
void TrackMap::Dfs(
	int statrt_id,
	const std::unordered_map<int, std::vector<int>>& adj,
	std::unordered_set<int>& visited,
	std::vector<int>& group_ids)
{
	if (visited.find(statrt_id) != visited.end())
		return;

	visited.insert(statrt_id);
	group_ids.emplace_back(statrt_id);

	auto it = adj.find(statrt_id);

	if (it == adj.end())
		return;

	for (int neighbour : it->second)
		Dfs(neighbour, adj, visited, group_ids);
}


void TrackMap::GroupConnectedNodes()
{
	// Create adjacent list
	std::unordered_map<int, std::vector<int>> adj;
	for (const auto& e : edges)
	{
		adj[e.A_id].emplace_back(e.B_id);
		adj[e.B_id].emplace_back(e.A_id);
	}

	std::unordered_set<int> visited;

	for (const auto& node : nodes)
	{
		if (visited.find(node.id) != visited.end())
			continue;

		std::vector<int> group_ids;
		Dfs(node.id, adj, visited, group_ids);

		Group g;
		g.ids = std::move(group_ids);
		groups.emplace_back(std::move(g));
	}
}


void TrackMap::GroupRadialPull()
{
	if (groups.empty())
		return;

	const int radialPullIterations = 5000;
	const Vec origin = { 0,0 };

	std::vector<Vec> prePullPos;
	prePullPos.reserve(groups.size());

	for (const auto& group : groups)
		prePullPos.emplace_back(group.pos);

	for (int i = 0; i < radialPullIterations; i++)
	{
		// Pull groups towards the origin
		for (auto& group : groups)
		{
			// works
			//Vec toOrigin = origin - group.pos;
			//group.pos += toOrigin * stepAmount; //0.05

			Vec toOrigin = origin - group.pos;
			double distance = toOrigin.norm();

			if (distance > 0.0)
			{
				Vec direction = toOrigin / distance;

				double step = group.radius * 0.055;
				group.pos += direction * step;
			}
		}

		//Resolve collisions between groups
		for (size_t a = 0; a < groups.size(); a++)
		{
			for (size_t b = a + 1; b < groups.size(); b++)
			{
				Vec delta = groups[b].pos - groups[a].pos;
				double distance = delta.norm();

				if (distance == 0.0)
				{
					delta = { 1.0, 0.0 };
					distance = 1.0;
				}

				double minDistance = groups[a].radius + groups[b].radius;

				if (distance < minDistance)
				{
					// Larger groups are 'heavier'
					double weightA = groups[a].radius;
					double weightB = groups[b].radius;

					double totalWeight = weightA + weightB;

					double moveA = weightB / totalWeight;
					double moveB = weightA / totalWeight;

					double overlap = minDistance - distance;
					Vec normal = delta / distance;

					groups[a].pos.x -= normal.x * overlap * moveA;
					groups[a].pos.y -= normal.y * overlap * moveA;
					groups[b].pos.x += normal.x * overlap * moveB;
					groups[b].pos.y += normal.y * overlap * moveB;
				}
			}
		}
	}

	// Update node positions in group
	for (size_t i = 0; i < groups.size(); i++)
	{
		const auto& group = groups[i];
		for (const size_t id : group.ids)
		{
			Node& n = nodes[nodeIndex[id]];
			n.pos += group.pos - prePullPos[i];
		}
	}
}


void TrackMap::Circle(std::vector<Node>& nodes)
{
	double angle = 2.0 * M_PI / nodes.size();
	for (size_t i = 0; i < nodes.size(); i++)
	{
		nodes[i].pos.x = cos(i * angle);
		nodes[i].pos.y = sin(i * angle);
	}
}


void TrackMap::CentreAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes)
{
	// Find current dimensions
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::lowest();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::lowest();

	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].pos.x < x_min)
			x_min = nodes[i].pos.x;

		if (nodes[i].pos.x > x_max)
			x_max = nodes[i].pos.x;

		if (nodes[i].pos.y < y_min)
			y_min = nodes[i].pos.y;

		if (nodes[i].pos.y > y_max)
			y_max = nodes[i].pos.y;

	}

	// 0,0 centre
	double cur_width = x_max - x_min;
	double cur_height = y_max - y_min;

	// compute scale factor (0.9: keep some margin)
	double x_scale = width / cur_width;
	double y_scale = height / cur_height;
	double scale = 0.9 * (x_scale < y_scale ? x_scale : y_scale);

	// compute offset and apply it to every position
	Vec centre = { x_max + x_min, y_max + y_min };
	Vec offset = centre / 2.0 * scale;

	for (auto& node : nodes)
		node.pos = node.pos * scale - offset;
}


void TrackMap::Bake(std::vector<Node> newNodes, std::vector<Edge> newEdges)
{
	nodes = std::move(newNodes);
	edges = std::move(newEdges);

	if (nodes.empty())
		return;

	// Position nodes in a circle
	Circle(nodes);
	
	// Generate node index lookup table 
	nodeIndex.clear();

	for (int i = 0; i < nodes.size(); i++)
		nodeIndex[nodes[i].id] = i;

	// Run force directed layout
	FruchtermanReingold();
	
	scale = (int)(nodes.size() * 200);
	CentreAndScale(scale, scale, nodes);

	// Find connected node groups
	groups.clear();
	GroupConnectedNodes();

	// Find group position and radius
	const double margin = 10.0;

	for (auto& group : groups)
	{
		constexpr double minDouble = -std::numeric_limits<double>::max();
		constexpr double maxDouble = std::numeric_limits<double>::max();

		Vec minPos = { maxDouble, maxDouble };
		Vec maxPos = { minDouble, minDouble };
		for (size_t i = 0; i < group.ids.size(); i++)
		{
			const int id = group.ids[i];
			const Node& node = nodes[nodeIndex[id]];

			if (node.pos.x > maxPos.x)
				maxPos.x = node.pos.x;

			if (node.pos.x < minPos.x)
				minPos.x = node.pos.x;

			if (node.pos.y > maxPos.y)
				maxPos.y = node.pos.y;

			if (node.pos.y < minPos.y)
				minPos.y = node.pos.y;
		}

		// Add margin
		minPos.x -= margin;
		minPos.y -= margin;
		maxPos.x += margin;
		maxPos.y += margin;

		double width = maxPos.x - minPos.x;
		double height = maxPos.y - minPos.y;

		group.pos = (minPos + maxPos) * 0.5f;
		group.radius = 0.5f * std::sqrt(width * width + height * height);
	}
	
	// Pull groups towards the centre origin
	GroupRadialPull();
}
