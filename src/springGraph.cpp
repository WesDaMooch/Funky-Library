#include "../include/springGraph.hpp"

void SpringGraph::simulate(
	std::vector<SpringGraph::Node>& nodes,
	const std::vector<SpringGraph::Edge> edges,
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
