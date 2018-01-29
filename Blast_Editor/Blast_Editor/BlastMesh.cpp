#include "BlastMesh.h"
#include "OpenGL.h"
#include "Application.h"
#include "ModuleMeshImporter.h"
#include "ModuleRenderer3D.h"
#include "ComponentTransform.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringMesh.h"

BlastMesh::BlastMesh()
{
	id_vertices_data = 0;
	num_indices = 0;
	indices = nullptr;
	blast_mesh = nullptr;
	is_root = false;

	id_indices = 0;
	num_vertices = 0;
	name = "";
	mesh_transform = new ComponentTransform();
}

BlastMesh::~BlastMesh()
{
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(vertices_data);
	RELEASE_ARRAY(vertices);

	UnloadFromMemory();
	blast_mesh->release();
}

void BlastMesh::LoadToMemory()
{
	if (id_vertices_data == 0)
	{
		glGenBuffers(1, &id_vertices_data);
		glBindBuffer(GL_ARRAY_BUFFER, id_vertices_data);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*num_vertices * 12, vertices_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &id_indices);
		glBindBuffer(GL_ARRAY_BUFFER, id_indices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uint)*num_indices, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		InitializeMesh();
	}
}

void BlastMesh::UnloadFromMemory()
{
	glDeleteBuffers(1, &id_vertices_data);

	id_vertices_data = 0;
}

void BlastMesh::CreateVerticesFromData()
{
	if (vertices == nullptr && vertices_data != nullptr)
	{
		vertices = new float[num_vertices * 3];

		for (int i = 0; i < num_vertices; ++i)
		{
			memcpy(vertices + i * 3, vertices_data + i * 12, sizeof(float) * 3);
		}
	}
}

void BlastMesh::SetExplosionDisplacement(float displacement)
{
	float3 new_pos = base_displacement * displacement;
	mesh_transform->SetPosition(new_pos);
}

void BlastMesh::InitializeMesh()
{
	id_vao = App->renderer3D->GenVertexArrayObject();

	App->renderer3D->BindVertexArrayObject(id_vao);

	App->renderer3D->BindArrayBuffer(id_vertices_data);

	//vertices
	App->renderer3D->SetVertexAttributePointer(0, 3, 12, 0);
	App->renderer3D->EnableVertexAttributeArray(0);
	//texture coords
	App->renderer3D->SetVertexAttributePointer(1, 2, 12, 3);
	App->renderer3D->EnableVertexAttributeArray(1);
	//normals
	App->renderer3D->SetVertexAttributePointer(2, 3, 12, 5);
	App->renderer3D->EnableVertexAttributeArray(2);
	//colors
	App->renderer3D->SetVertexAttributePointer(3, 4, 12, 8);
	App->renderer3D->EnableVertexAttributeArray(3);
	
	App->renderer3D->BindElementArrayBuffer(id_indices);

	App->renderer3D->UnbindVertexArrayObject();
}
