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

	// Fruchterman-Reingold layout
	
	// Generate node index lookup table 

	nodeIndex.clear();

	for (int i = 0; i < nodes.size(); ++i)
		nodeIndex[nodes[i].id] = i;

	temperature = 10 * std::sqrt(nodes.size());

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

	/*
	static const int areaSize = 100;

	// Position nodes randomly
	for (Node& n : nodes)
	{
		n.pos.x = (float)(rand() % areaSize);
		n.pos.y = (float)(rand() % areaSize);
	}

	const float area = 500; //500.0f * 500.0f;

	// Ideal node spacing
	const float k = sqrtf(area / nodes.size());

	float temperature = 200.0f;

	constexpr int iterations = 500;

	for (int iter = 0; iter < iterations; ++iter)
	{
		std::vector<ImVec2> disp(nodes.size(), ImVec2(0, 0));

		//--------------------------------------------------
		// Repulsion
		//--------------------------------------------------

		for (size_t v = 0; v < nodes.size(); ++v)
		{
			for (size_t u = v + 1; u < nodes.size(); ++u)
			{
				ImVec2 delta(
					nodes[v].pos.x - nodes[u].pos.x,
					nodes[v].pos.y - nodes[u].pos.y
				);

				float dist = std::max(length(delta), 0.01f);

				ImVec2 dir = normalize(delta);

				float force = (k * k) / dist;

				disp[v].x += dir.x * force;
				disp[v].y += dir.y * force;

				disp[u].x -= dir.x * force;
				disp[u].y -= dir.y * force;
			}
		}

		//--------------------------------------------------
		// Attraction
		//--------------------------------------------------

		for (const Edge& e : edges)
		{
			Node* a = findNode(e.A_id);
			Node* b = findNode(e.B_id);

			if (!a || !b)
				continue;

			size_t ai = a - nodes.data();
			size_t bi = b - nodes.data();

			ImVec2 delta(
				a->pos.x - b->pos.x,
				a->pos.y - b->pos.y
			);

			float dist = std::max(length(delta), 0.01f);

			ImVec2 dir = normalize(delta);

			float force = (dist * dist) / k;

			disp[ai].x -= dir.x * force;
			disp[ai].y -= dir.y * force;

			disp[bi].x += dir.x * force;
			disp[bi].y += dir.y * force;
		}

		//--------------------------------------------------
		// Move nodes
		//--------------------------------------------------

		for (size_t i = 0; i < nodes.size(); ++i)
		{
			float dispLen = length(disp[i]);

			if (dispLen < 0.001f)
				continue;

			ImVec2 dir = normalize(disp[i]);

			float moveAmount =
				std::min(dispLen, temperature);

			nodes[i].pos.x += dir.x * moveAmount;
			nodes[i].pos.y += dir.y * moveAmount;
		}

		//--------------------------------------------------
		// Cool
		//--------------------------------------------------

		temperature *= 0.95f;
	}
	*/



	/*
	static const int maxIterations = 100;

	static const float baseStringStength = 0.05f;
	static const float desiredSpringLength = 150.0f;

	nodes = std::move(newNodes);
	edges = std::move(newEdges);

	// Position nodes randomly
	for (Node& n : nodes)
	{
		n.pos.x = (float)(rand() % 500);
		n.pos.y = (float)(rand() % 500);
	}

	for (int iteration = 0; iteration < maxIterations; iteration++)
	{
		std::vector<ImVec2> forces(nodes.size(), ImVec2(0, 0));

		// Repulsion
		for (size_t i = 0; i < nodes.size(); i++)
		{
			for (size_t j = i + 1; j < nodes.size(); j++)
			{
				ImVec2 delta(nodes[j].pos.x - nodes[i].pos.x, nodes[j].pos.y - nodes[i].pos.y);

				float distSq =delta.x * delta.x + delta.y * delta.y;
				distSq = std::max(distSq, 1.0f);

				float force = 50000.0f / distSq;
				float dist = sqrtf(distSq);

				ImVec2 dir(delta.x / dist,delta.y / dist);

				forces[i].x -= dir.x * force;
				forces[i].y -= dir.y * force;

				forces[j].x += dir.x * force;
				forces[j].y += dir.y * force;
			}
		}

		// Spring attraction
		for (const Edge& e : edges)
		{
			Node* a = findNode(e.A_id);
			Node* b = findNode(e.B_id);

			if (!a || !b)
				continue;

			size_t ai = a - nodes.data();
			size_t bi = b - nodes.data();

			ImVec2 delta(b->pos.x - a->pos.x,b->pos.y - a->pos.y);

			float dist =std::sqrtf(delta.x * delta.x + delta.y * delta.y);
			dist = std::max(dist, 1.0f);

			ImVec2 dir(delta.x / dist,delta.y / dist);

			float force = (dist - desiredSpringLength) * baseStringStength;

			forces[ai].x += dir.x * force;
			forces[ai].y += dir.y * force;

			forces[bi].x -= dir.x * force;
			forces[bi].y -= dir.y * force;
		}

		// Apply forces
		for (size_t i = 0; i < nodes.size(); i++)
		{
			nodes[i].pos.x += forces[i].x;
			nodes[i].pos.y += forces[i].y;
		}
	}
	*/
	


	/*
	// Circle layout
	float radius = 500.0f;
	float step = 2.0f * 3.1415f / nodes.size();

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		float a = i * step;

		nodes[i].pos = ImVec2(
			cosf(a) * radius,
			sinf(a) * radius);
	}
	*/
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

	for (int v_id = 0; v_id < nodes.size(); v_id++)
	{
		if (nodes[v_id].pos.x < x_min)
			x_min = nodes[v_id].pos.x;

		if (nodes[v_id].pos.x > x_max)
			x_max = nodes[v_id].pos.x;

		if (nodes[v_id].pos.y < y_min)
			y_min = nodes[v_id].pos.y;

		if (nodes[v_id].pos.y > y_max)
			y_max = nodes[v_id].pos.y;

	}

	double cur_width = x_max - x_min;
	double cur_height = y_max - y_min;

	// Compute scale factor (0.9: keep some margin)
	double x_scale = width / cur_width;
	double y_scale = height / cur_height;
	double scale = 0.9 * (x_scale < y_scale ? x_scale : y_scale);

	// Compute offset and apply it to every position
	Vec center = { x_max + x_min, y_max + y_min };
	Vec offset = center / 2.0 * scale;

	// TODO: center at the middle of window

	for (int v_id = 0; v_id < nodes.size(); v_id++)
	{
		nodes[v_id].pos.x = nodes[v_id].pos.x * scale - offset.x;
		nodes[v_id].pos.y = nodes[v_id].pos.y * scale - offset.y;
	}
}


void TrackMap::render()
{
	bool open = true;
	if (ImGui::Begin("Map Settings", &open))
	{
		if (ImGui::InputDouble("k", &k))
		{
			kSquared = k * k;

			bake(nodes, edges);
		}

		if (ImGui::InputInt("iter", &iterations))
		{
			bake(nodes, edges);
		}
		
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
			n.baseRadius, 
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
			1.0f);
	}
}

