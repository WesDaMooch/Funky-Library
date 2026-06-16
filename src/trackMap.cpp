#include "../include/trackMap.hpp"


void TrackMap::bake(std::vector<TrackNode> trackNodeList)
{
	trackNodes = std::move(trackNodeList);


	// TODO: Bake map
	int i = 0;
	for (TrackNode& tn : trackNodes)
	{
		tn.pos = ImVec2(i * 200, i * 100);
		i++;
	}
}


void TrackMap::render()
{
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
	

	for (TrackNode tn : trackNodes)
	{
		drawList->AddCircle(
			ImVec2(tn.pos.x + camera.pos.x, tn.pos.y + camera.pos.y), 
			30.f, 
			tn.col, 
			16);

		
	}
}





/*
void trackMap::simulate(
	std::vector<trackMap::Node>& nodes,
	const std::vector<trackMap::Edge> edges,
	float dt)
{
	const float repulsion = 5000.0f;
	const float spring_k = 0.1f;
	const float rest_length = 100.0f;
	const float damping = 0.9f;

	std::vector<ImVec2> forces{};
	forces.reserve(nodes.size());

	// Repulsion
	for (int i = 0; i < nodes.size(); i++)
	{
		for (int j = i + 1; j < nodes.size(); j++)
		{
			ImVec2 delta;
			delta.x = nodes[i].pos.x - nodes[j].pos.x;
			delta.y = nodes[i].pos.y - nodes[j].pos.x;

			float distance = GetLength(delta) + 0.1f;
			ImVec2 direction = Normalize(delta);
			float force = repulsion / (distance * distance);

			ImVec2 f;
			f.x = direction.x * force;
			f.y = direction.y * force;

			forces[i].x += f.x;
			forces[i].y += f.y;

			forces[j].x -= f.x;
			forces[j].y -= f.y;
		}
	}

	// Springs
	for (const Edge& e : edges) 
	{
		ImVec2 delta;
		delta.x = nodes[e.b].pos.x - nodes[e.a].pos.x;
		delta.y = nodes[e.b].pos.y - nodes[e.a].pos.y;

		float distance = GetLength(delta) + 0.1f;
		ImVec2 direction = Normalize(delta);
		float force = spring_k * (distance - rest_length);

		ImVec2 f;
		f.x = direction.x * force;
		f.y = direction.y * force;

		forces[e.a].x += f.x;
		forces[e.a].y += f.y;

		forces[e.b].x -= f.x;
		forces[e.b].y -= f.y;
	}

	// Intergrate
	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i].vel.x += forces[i].x * dt;
		nodes[i].vel.y += forces[i].y * dt;

		nodes[i].vel.x *= damping;
		nodes[i].vel.y *= damping;

		nodes[i].pos.x += nodes[i].vel.x * dt;
		nodes[i].pos.y += nodes[i].vel.y * dt;
	}
}
*/