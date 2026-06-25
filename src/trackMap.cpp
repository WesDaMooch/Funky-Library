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


void TrackMap::FruchtermanReingold(const std::vector<int>& group_ids)
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


	// Find connected node groups
	GroupConnectedNodes();


	for (const auto& group : groups)
	{
		if (group.ids.size() <= 1)
			continue;
		
		// Run layout
		FruchtermanReingold(group.ids);
	}


	// Pack groups
	// Largest group to smalles packing algo
	Pack();

	// Scale
	//CentreAndScale(2000, 2000, nodes);
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


void TrackMap::CentreAndScaleGroup(unsigned int width, unsigned int height, const std::vector<int>& group)
{
	// Find current dimensions
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::lowest();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::lowest();

	for (const int id : group)
	{
		const Node& node = nodes[nodeIndex[id]];

		if (node.pos.x < x_min)
			x_min = node.pos.x;

		if (node.pos.x > x_max)
			x_max = node.pos.x;

		if (node.pos.y < y_min)
			y_min = node.pos.y;

		if (node.pos.y > y_max)
			y_max = node.pos.y;
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

	for (const int id : group)
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
}


void TrackMap::Centre(const std::vector<int>& group_ids)
{
	// Find current dimensions
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::lowest();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::lowest();

	for (const int id : group_ids)
	{
		const Node& node = nodes[nodeIndex[id]];

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

	for (const int id : group_ids)
	{
		Node& node = nodes[nodeIndex[id]];
		node.pos = node.pos - offset;
	}
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

		//groups.emplace_back(std::move(group));
		
		Group g;
		g.ids = std::move(group_ids);
		groups.emplace_back(std::move(g));
	}
}


// Based on James Bremner packing around a centre point.
void TrackMap::Pack()
{
	if (groups.empty())
		return;

	for (auto& group : groups)
	{
		// Position group at 0,0
		Centre(group.ids);

		// Generate group bounding box and find area
		Vec minPos;
		Vec maxPos;
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

			group.rect = { minPos, maxPos };

			double width = minPos.x - maxPos.x;
			double height = minPos.y - maxPos.y;
			group.area = width * height;
		}
	}



	// Divide area into four quadrants 

	// Sort rects into order of decreasing area
	// and assigned to the quadrants in round robin fashion
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

	for (const auto& group : groups)
	{
		// line
		
		ImVec2 min = camera.ToScreenPos(group.rect.first.x, group.rect.first.y);
		ImVec2 max = camera.ToScreenPos(group.rect.second.x, group.rect.second.y);

		drawList->AddRect(
			min,
			max,
			IM_COL32_WHITE
		);
	}

	return info;
}

