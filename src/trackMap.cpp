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




void TrackMap::GroupFruchtermanReingold(const std::vector<int>& group_ids)
{
	// Fruchterman-Reingold layout


	// Generate group id lookup table
	std::unordered_map<int, size_t> groupIndex;

	for (size_t i = 0; i < group_ids.size(); ++i)
		groupIndex[group_ids[i]] = i;


	//double temperature = 100.0;
	double temperature = 10 * std::sqrt(group_ids.size());
	double kSquared = k * k;


	// Generate set to find if edges are in the group
	std::unordered_set<int> groupSet(group_ids.begin(), group_ids.end());
	

	// Run layout
	std::vector<Vec> forces(group_ids.size());

	for (int iteration = 0; iteration < iterations; iteration++)
	{
		std::fill(forces.begin(), forces.end(), Vec{});

		// Repulsion force between vertice pairs
		for (size_t i = 0; i < group_ids.size(); i++)
		{
			for (size_t j = i + 1; j < group_ids.size(); j++)
			{
				Node& a = nodes[nodeIndex[group_ids[i]]];
				Node& b = nodes[nodeIndex[group_ids[j]]];

				Vec delta = a.pos - b.pos;

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
			if (groupSet.find(e.A_id) == groupSet.end() ||
				groupSet.find(e.B_id) == groupSet.end())
			{
				continue;
			}

			Node& nodeA = nodes[nodeIndex[e.A_id]];
			Node& nodeB = nodes[nodeIndex[e.B_id]];

			Vec delta = nodeA.pos - nodeB.pos;

			double distance = delta.norm();

			if (distance == 0.0)
				continue;

			// double attraction = distance
			// double attraction = std::log(distance)
			double attraction = distance * distance / k;

			forces[groupIndex.at(e.A_id)] -= delta / distance * attraction;
			forces[groupIndex.at(e.B_id)] += delta / distance * attraction;
		}

		// Max movement capped by current temperature
		for (size_t v_id = 0; v_id < group_ids.size(); v_id++)
		{
			double forceNorm = forces[v_id].norm();

			// < 1e-6: not worth computing
			if (forceNorm < 1e-6)
				continue;

			double cappedForceNorm = std::min(forceNorm, temperature);
			Vec cappedForce = forces[v_id] / forceNorm * cappedForceNorm;

			nodes[nodeIndex[group_ids[v_id]]].pos += cappedForce;
		}

		temperature *= 0.95;
	}
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
	// ++i??
	for (int i = 0; i < nodes.size(); ++i)
		nodeIndex[nodes[i].id] = i;

	FruchtermanReingold();

	//Centre(nodes);

	int scale = nodes.size() * 200;
	CentreAndScale(scale, scale, nodes);

	// Find connected node groups
	groups2.clear();
	GroupConnectedNodes();


	const double margin = 10.0;

	for (auto& group : groups2)
	{
		// Generate group bounding box and find area
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
		
	GroupRadialPull();

	/*
	// Run layout
	for (const auto& group : groups)
	{
		if (group.ids.size() <= 1)
			continue;
		
		// Run layout
		GroupFruchtermanReingold(group.ids);

		int scale = group.ids.size() * 80;
		CentreAndScaleGroup(scale, scale, group.ids);
	}

	// Pack groups
	const double margin = 20.0;

	for (auto& group : groups)
	{
		// Position group at 0,0
		//Centre(group.ids);

		// Generate group bounding box and find area
		Vec minPos, maxPos;
		for (size_t i = 0; i < group.ids.size(); i++)
		{
			const int id = group.ids[i];
			const Node& node = nodes[nodeIndex[id]];

			if (node.pos.x > maxPos.x)
				maxPos.x = node.pos.x;
			else if (node.pos.x < minPos.x)
				minPos.x = node.pos.x;

			if (node.pos.y > maxPos.y)
				maxPos.y = node.pos.y;
			else if (node.pos.y < minPos.y)
				minPos.y = node.pos.y;
		}

		// Add margin
		minPos.x -= margin;
		minPos.y -= margin;
		maxPos.x += margin;
		maxPos.y += margin;

		group.rect = { minPos, maxPos }; // TODO: remove after debug?
		group.width = std::abs(maxPos.x - minPos.x);
		group.height = std::abs(maxPos.y - minPos.y);
		group.area = group.width * group.height;
	}

	// Sort groups by area
	std::sort(groups.begin(), groups.end(),
		[](const Group& a, const Group& b)
		{
			return a.area > b.area;
		});


	// Divide area into four quadrants 
	// and assigned to the quadrants in round robin fashion
	std::vector<std::vector<size_t>> quadGroups(4);	// store group index

	for (size_t i = 0; i < groups.size(); ++i)
	{
		size_t quad = i % 4;
		quadGroups[quad].emplace_back(i);
	}

	// Pack groups in each quadrant
	for (size_t i = 0; i < quadGroups.size(); i++)
	{
		const auto& quad = quadGroups[i];

		Pack(quad);
		Rotate(quad, (int)i);
	}

	////
	for (const auto& quad : quadGroups)
	{
		Pack(quad);

		// Rotate

	}
	////
	*/

	// Scale
	//int scale = nodes.size() * 100;
	//CentreAndScale(scale, scale, nodes);
	
}


void TrackMap::GroupRadialPull()
{
	if (groups2.empty())
		return;

	const int radialPullIterations = 100; // TODO: play with this number
	const Vec origin = { 0,0 };

	std::vector<Vec> prePullPos;
	prePullPos.reserve(groups2.size());

	for (const auto& group : groups2)
		prePullPos.emplace_back(group.pos);

	for (int i = 0; i < radialPullIterations; i++)
	{
		// Pull groups towards the origin
		for (auto& group : groups2)
		{
			Vec toOrigin = origin - group.pos;
			group.pos += toOrigin * 0.05;
		}

		//Resolve collisions between groups
		for (size_t a = 0; a < groups2.size(); a++)
		{
			for (size_t b = a + 1; b < groups2.size(); b++)
			{
				Vec delta = groups2[b].pos - groups2[a].pos;
				double distance = delta.norm();

				if (distance == 0.0)
				{
					delta = { 1.0, 0.0 };
					distance = 1.0;
				}

				double minDistance = groups2[a].radius + groups2[b].radius;

				if (distance < minDistance)
				{
					double overlap = minDistance - distance;
					Vec normal = delta / distance;

					groups2[a].pos.x -= normal.x * overlap * 0.5;
					groups2[a].pos.y -= normal.y * overlap * 0.5;
					groups2[b].pos.x += normal.x * overlap * 0.5;
					groups2[b].pos.y += normal.y * overlap * 0.5;
				}
			}
		}
	}

	// Update node positions in group
	for (size_t i = 0; i < groups2.size(); i++)
	{
		const auto& group = groups2[i];
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
	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i].pos.x = cos(i * angle);
		nodes[i].pos.y = sin(i * angle);
	}
}


void TrackMap::CentreAndScaleGroup(unsigned int width, unsigned int height, const std::vector<int>& group_ids)
{
	// Find current dimensions
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::lowest();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::lowest();

	for (const size_t id : group_ids)
	{
		const Node& n = nodes[nodeIndex[id]];

		if (n.pos.x < x_min)
			x_min = n.pos.x;

		if (n.pos.x > x_max)
			x_max = n.pos.x;

		if (n.pos.y < y_min)
			y_min = n.pos.y;

		if (n.pos.y > y_max)
			y_max = n.pos.y;
	}

	double cur_width = x_max - x_min;
	double cur_height = y_max - y_min;

	// compute scale factor (0.9: keep some margin)
	double x_scale = width / cur_width;
	double y_scale = height / cur_height;
	double scale = 0.9 * (x_scale < y_scale ? x_scale : y_scale);

	// compute offset and apply it to every position
	Vec centre = { x_max + x_min, y_max + y_min };
	Vec offset = centre / 2.0 * scale;

	for (const int id : group_ids)
	{
		Node& node = nodes[nodeIndex[id]];
		node.pos = node.pos * scale - offset;
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

	/* Screen centre
	double cur_width = x_max - x_min;
	double cur_height = y_max - y_min;

	// Compute scale factor (0.9: keep some margin)
	double x_scale = width / cur_width;
	double y_scale = height / cur_height;
	double scale = 0.9 * (x_scale < y_scale ? x_scale : y_scale);

	// Compute offset and apply it to every position
	Vec min = { x_min, y_min };
	Vec offset = { (width - cur_width * scale) * 0.5, (height - cur_height * scale) * 0.5 };

	for (Node& n : nodes)
	{
		Vec pos = n.pos;
		pos -= min;
		pos *= scale;
		pos += offset;
		n.pos = pos;
	}
	*/
}


void TrackMap::Centre(std::vector<Node>& nodes)
{
	// Find current dimensions
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::lowest();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::lowest();

	for (const auto& node : nodes)
	{
		if (node.pos.x < x_min)
			x_min = node.pos.x;

		if (node.pos.x > x_max)
			x_max = node.pos.x;

		if (node.pos.y < y_min)
			y_min = node.pos.y;

		if (node.pos.y > y_max)
			y_max = node.pos.y;
	}
	// compute offset and apply it to every position
	Vec centre = { x_max + x_min, y_max + y_min };
	Vec offset = centre / 2.0;

	for (auto& node : nodes)
		node.pos = node.pos - offset;
}


// Recursive depth first search
void Dfs(
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

		Group2 g;
		g.ids = std::move(group_ids);
		groups2.emplace_back(std::move(g));
	}
}


// Based on James Bremner packing around a centre point.
void TrackMap::Pack(const std::vector<size_t>& group_idxs)
{
	if (group_idxs.empty())
		return;

	double totalArea = 0.0;

	for (size_t idx : group_idxs)
		totalArea += groups[idx].area;

	double targetWidth = std::sqrt(totalArea);

	double x = 0.0;
	double y = 0.0;
	double rowHeight = 0.0;

	for (size_t idx : group_idxs)
	{
		Group& g = groups[idx];

		double width = g.rect.second.x - g.rect.first.x;
		double height = g.rect.second.y - g.rect.first.y;

		if (x + width > targetWidth)
		{
			x = 0.0;
			y += rowHeight;
			rowHeight = 0.0;
		}

		Vec offset{
			x - g.rect.first.x,
			y - g.rect.first.y
		};

		// Move all nodes in the group
		for (int id : g.ids)
		{
			nodes[nodeIndex[id]].pos += offset;
		}

		// Update stored bounding box
		g.rect.first += offset;
		g.rect.second += offset;

		x += width;

		if (height > rowHeight)
			rowHeight = height;
	}
}

void TrackMap::Rotate(const std::vector<size_t>& group_idxs, int quadrant)
{
	if (group_idxs.empty() || quadrant >= 3 || quadrant < 0)
		return;

	// 0: top left, 1: top right, 2: bottom left, 3: bottm right (no rotation)

	for (size_t idx : group_idxs)
	{
		Group& group = groups[idx];

		for (size_t id : group.ids)
		{
			Vec& pos = nodes[nodeIndex[id]].pos;

			double x = pos.x;
			double y = pos.y;

			switch (quadrant)
			{
			case 0: // 270
				pos.x = y;
				pos.y = -x;
				break;

			case 1: // 180
				pos.x = -x;
				pos.y = -y;
				break;

			case 2: // 90
				pos.x = -y;
				pos.y = x;
				break;
			}
		}
	}
}


TrackMap::Info TrackMap::Render()
{
	bool dirty = false;

	if (ImGui::InputDouble("k", &k))
		dirty = true;

	if (ImGui::InputInt("iter", &iterations))
		dirty = true;

	if (dirty)
		Bake(nodes, edges);
	
	Info info;

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 mousePos = { io.MousePos.x, io.MousePos.y };

	// Mouse zoom
	float wheel = io.MouseWheel;

	float oldZoom = camera.zoom;
	ImVec2 worldUnderMouse = { (mousePos.x - camera.pos.x) / oldZoom, (mousePos.y - camera.pos.y) / oldZoom };

	camera.SetZoom(wheel, io.DeltaTime);

	camera.pos.x = mousePos.x - worldUnderMouse.x * camera.zoom;
	camera.pos.y = mousePos.y - worldUnderMouse.y * camera.zoom;


	// Mouse drag
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		// Save mouse position
		mousePosOnLeftClick = mousePos;
		cameraPosOnLeftClick = camera.pos;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		float dx = mousePos.x - mousePosOnLeftClick.x;
		float dy = mousePos.y - mousePosOnLeftClick.y;

		camera.pos = ImVec2(cameraPosOnLeftClick.x + dx, cameraPosOnLeftClick.y + dy);
	}

	// Draw edges
	float radius = baseRadius * camera.zoom;
	float lineWidth = 0.5f * camera.zoom;
	float arrowLength = 4.0f * camera.zoom;
	float arrowWidth = 4.0f * camera.zoom;

	for (const Edge& e : edges)
	{
		const Node& a = nodes[nodeIndex.at(e.A_id)];
		const Node& b = nodes[nodeIndex.at(e.B_id)];

		ImVec2 start = camera.ToScreenPos(a.pos.x, a.pos.y);
		ImVec2 end = camera.ToScreenPos(b.pos.x, b.pos.y);
		ImVec2 dir = { end.x - start.x,end.y - start.y };
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

		if (len > 0.0f)
		{
			// Gap between node and endge
			dir.x /= len;
			dir.y /= len;

			float gap = (4.0f * camera.zoom) + radius;

			start.x += dir.x * gap;
			start.y += dir.y * gap;

			end.x -= dir.x * gap;
			end.y -= dir.y * gap;

			// Draw arrow
			ImVec2 perp = { -dir.y,dir.x };
			
			if (e.A_direction)
			{
				ImVec2 arrowLine1 = {
					start.x + dir.x * arrowLength + perp.x * arrowWidth,
					start.y + dir.y * arrowLength + perp.y * arrowWidth
				};

				ImVec2 arrowA2 = {
					start.x + dir.x * arrowLength - perp.x * arrowWidth,
					start.y + dir.y * arrowLength - perp.y * arrowWidth
				};

				drawList->AddLine(start, arrowLine1, IM_COL32_WHITE, lineWidth);
				drawList->AddLine(start, arrowA2, IM_COL32_WHITE, lineWidth);
			}

			if (e.B_direction)
			{
				ImVec2 arrowLine1 = {
					end.x - dir.x * arrowLength + perp.x * arrowWidth,
					end.y - dir.y * arrowLength + perp.y * arrowWidth
				};

				ImVec2 arrowLine2 = {
					end.x - dir.x * arrowLength - perp.x * arrowWidth,
					end.y - dir.y * arrowLength - perp.y * arrowWidth
				};

				drawList->AddLine(end, arrowLine1, IM_COL32_WHITE, lineWidth);
				drawList->AddLine(end, arrowLine2, IM_COL32_WHITE, lineWidth);
			}
			
		}

		// Edge line
		drawList->AddLine(
			start,
			end,
			IM_COL32_WHITE,
			lineWidth
		);
	}

	// Draw nodes
	for (const Node& n : nodes)
	{
		ImVec2 nodePos = camera.ToScreenPos(n.pos.x, n.pos.y);

		// Node hovered
		float dx = mousePos.x - nodePos.x;
		float dy = mousePos.y - nodePos.y;

		if ((dx * dx + dy * dy) <= (radius * radius))
		{
			info.hoveredId = n.id;
			info.pos = nodePos;
		}
		

		drawList->AddCircleFilled(
			nodePos,
			radius,
			n.col,
			16
		);

		
		// Outline
		drawList->AddCircle(
			nodePos,
			radius,
			IM_COL32_WHITE,
			16,
			0.5f * camera.zoom
		);
		
	}
	
	for (const auto& group : groups2)
	{
		ImVec2 pos = camera.ToScreenPos((float)group.pos.x, (float)group.pos.y);
		drawList->AddCircle(
			pos,
			(float)group.radius * camera.zoom,
			IM_COL32_WHITE,
			16,
			1.0f
		);
	}


	/*
	for (const auto& group : groups)
	{
		ImVec2 min = camera.ToScreenPos(group.rect.first.x, group.rect.first.y);
		ImVec2 max = camera.ToScreenPos(group.rect.second.x, group.rect.second.y);
		
		drawList->AddRect(
			min,
			max,
			IM_COL32_WHITE
		);

		// Debug
		drawList->AddText(
			camera.ToScreenPos(group.rect.second.x, group.rect.second.y),
			IM_COL32_WHITE,
			std::to_string(group.area).c_str()
		);
		
	}
	*/

	// Debug
	// Centre
	drawList->AddCircleFilled(camera.ToScreenPos(0, 0), 10.0f, IM_COL32_WHITE, 8);

	return info;
}

