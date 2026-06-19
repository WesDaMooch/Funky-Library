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


void TrackMap::bake(std::vector<Node> newNodes, std::vector<Edge> newEdges)
{
	nodes = std::move(newNodes);
	edges = std::move(newEdges);

	if (nodes.empty())
		return;
	
	// Position nodes in a circle
	circle(nodes);
	
	// Generate node index lookup table 

	nodeIndex.clear();

	for (int i = 0; i < nodes.size(); ++i)
		nodeIndex[nodes[i].id] = i;

	// Fruchterman-Reingold layout
	// TODO: Use grid variant layout for disconnected node groups


	temperature = 10 * std::sqrt(nodes.size());
	double kSquared = k * k;

	for (int iteration = 0; iteration < iterations; iteration++)
	{
		std::vector<Vec> mvmts_;
		mvmts_.resize(nodes.size());

		// Repulsion force between vertice pairs
		for (int v_id = 0; v_id < nodes.size(); v_id++) 
		{
			for (int other_id = v_id + 1; other_id < nodes.size(); other_id++) 
			{
				if (v_id == other_id) 
					continue;
				
				Vec delta = nodes[v_id].pos - nodes[other_id].pos;

				double distance = delta.norm();

				// Handle distance small distance
				//if (distance < 0.001)
				//	continue;

				// > 1000.0: not worth computing
				if (distance > 1000.0)
					continue;

				double repulsion = kSquared / distance;

				mvmts_[v_id] += delta / distance * repulsion;
				mvmts_[other_id] -= delta / distance * repulsion;
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

			mvmts_[a] -= delta / distance * attraction;
			mvmts_[b] += delta / distance * attraction;
		}

		// Max movement capped by current temperature
		for (int v_id = 0; v_id < nodes.size(); v_id++)
		{
			double mvmt_norm = mvmts_[v_id].norm();

			// < 1.0: not worth computing
			if (mvmt_norm < 1.0)
				continue;

			double capped_mvmt_norm = std::min(mvmt_norm, temperature);
			Vec capped_mvmt = mvmts_[v_id] / mvmt_norm * capped_mvmt_norm;

			nodes[v_id].pos += capped_mvmt;
		}

		// Cool down fast until we reach 1.5, then stay at low temperature
		if (temperature > 1.5)
			temperature *= 0.85;
		else
			temperature = 1.5;
	}

	centerAndScale(1920, 1080, nodes);
}

void TrackMap::circle(std::vector<Node>& nodes)
{
	double angle = 2.0 * M_PI / nodes.size();
	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i].pos.x = cos(i * angle);
		nodes[i].pos.y = sin(i * angle);
	}
}

void TrackMap::centerAndScale(unsigned int width, unsigned int height, std::vector<Node>& nodes)
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


void TrackMap::render()
{
	bool open = true;
	if (ImGui::Begin("Map Settings", &open))
	{
		if (ImGui::InputDouble("k", &k))
			bake(nodes, edges);
		

		if (ImGui::InputInt("iter", &iterations))
			bake(nodes, edges);
		
		//ImGui::InputDouble("X Pos", &nodes[0].pos.x);
		//ImGui::SameLine();
		//ImGui::InputDouble("Y Pos", &nodes[0].pos.y);

		ImGui::End();
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 mousePos(io.MousePos.x, io.MousePos.y);

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


	// Mouse zoom
	//float verticalWheel = io.MouseWheel;
	//float horizontalWheel = io.MouseWheelH;
	
	// Draw nodes
	for (const Node& n : nodes)
	{
		drawList->AddCircle(
			ImVec2((float)n.pos.x + camera.pos.x, (float)n.pos.y + camera.pos.y), 
			4.f, 
			n.col, 
			16);
	}

	// Draw edges
	for (const Edge& e : edges)
	{
		const Node& a = nodes[nodeIndex.at(e.A_id)];
		const Node& b = nodes[nodeIndex.at(e.B_id)];

		drawList->AddLine(
			ImVec2((float)a.pos.x + camera.pos.x, (float)a.pos.y + camera.pos.y),
			ImVec2((float)b.pos.x + camera.pos.x, (float)b.pos.y + camera.pos.y),
			IM_COL32_WHITE,
			0.5f);
	}
}

