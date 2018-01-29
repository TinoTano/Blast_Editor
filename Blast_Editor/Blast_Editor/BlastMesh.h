#pragma once

#include <string>
#include "Math/AABB.h"

class ComponentTransform;
class Material;

typedef unsigned int uint;

namespace Nv
{
	namespace Blast
	{
		class Mesh;
	}
}

class BlastMesh
{
public:

	BlastMesh();
	~BlastMesh();

	void LoadToMemory();
	void UnloadFromMemory();

	void CreateVerticesFromData();

	void SetExplosionDisplacement(float displacement);

private:
	//Must be call after setting the values to the mesh!!!
	void InitializeMesh();

public:
	uint id_indices; // id in VRAM
	uint num_indices;
	uint* indices;

	uint id_vertices_data; // id in VRAM
	uint num_vertices;

	uint id_normals;
	float* normals;

	uint id_colors;
	float* colors;

	uint id_texture_coords;
	float* texture_coords;
	
	float* vertices_data = nullptr;
	float* vertices = nullptr; //copy of the vertices for animation and aabb enclose

	AABB box;

	uint id_vao = 0;

	ComponentTransform* mesh_transform;
	Material* material;
	Material* interior_material;

	float3 base_displacement;
	int chunk_id;
	int chunk_depth;
	Nv::Blast::Mesh* blast_mesh;
	bool is_root;
	std::vector<BlastMesh*> childs;
	std::string name;
};

