#include "ModuleBlast.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoring.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringFractureTool.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringMesh.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringCollisionBuilder.h"
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringBondGenerator.h"
#include "Nvidia/Blast/Include/toolkit/NvBlastTkFramework.h"
#include "Nvidia/Blast/Include/extensions/physx/NvBlastExtPxAsset.h"
#include "Nvidia/Blast/Include/extensions/serialization/NvBlastExtPxSerialization.h"
#include "Nvidia/Blast/Include/extensions/serialization/NvBlastExtSerialization.h"
#include "Nvidia/Blast/Include/extensions/serialization/NvBlastExtTkSerialization.h"
#include "BlastMesh.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ComponentTransform.h"
#include "Material.h"
#include "ComponentTransform.h"
#include <fstream>

#pragma comment (lib, "Nvidia/Blast/lib/NvBlastDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastExtAuthoringDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastExtPxSerializationDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastExtSerializationDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastTkDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastGlobalsDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastExtPhysXDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/Blast/lib/NvBlastExtTkSerializationDEBUG_x86.lib")

#pragma comment (lib, "Nvidia/PhysX/libx86/Debug/PhysX3DEBUG_x86.lib")
#pragma comment (lib, "Nvidia/PhysX/libx86/Debug/PhysX3CommonDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/PhysX/libx86/Debug/PxFoundationDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/PhysX/libx86/Debug/PhysX3CookingDEBUG_x86.lib")
#pragma comment (lib, "Nvidia/PhysX/libx86/Debug/PhysX3ExtensionsDEBUG.lib")


ModuleBlast::ModuleBlast(Application* app) : Module(app)
{
	name = "Blast";
	fract_tool = nullptr;
	voronoi_sites_generator = nullptr;
	blast_random_generator = nullptr;
	authoring_result = nullptr;
	current_selected_mesh = nullptr;
	explosion_amount = 0;
	cell_counts = 10;
	cluster_count = 5;
	cluster_radius = 100;
	radial_center = float3::zero;
	radial_normal = float3::zero;
	radial_radius = 100;
	radial_angular_steps = 6;
	radial_steps = 5;
	radial_angle_offset = 0;
	radial_variability = 0;
	sphere_center = float3::zero;
	sphere_radius = 20;
	remove_sphere_center = float3::zero;
	remove_sphere_radius = 20;
	remove_sphere_eraser_probability = 1;
	slicing_angle_variation = 0;
	slicing_noise_amplitude = 0;
	slicing_noise_frquency = 1;
	slicing_noise_octave_number = 1;
	slicing_offset_variations = 0;
	slicing_surface_resolution = 1;
	slicing_x_slices = 2;
	slicing_y_slices = 2;
	slicing_z_slices = 2;
	replace_chunk = false;
	remove_islands = true;
	new_mesh = false;

	interior_mesh_material = new Material();
	interior_mesh_material->SetDiffuseColor(1,1,1);
}

ModuleBlast::~ModuleBlast()
{
}

bool ModuleBlast::Init()
{
	bool ret = true;

	physx_foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!physx_foundation)
	{
		ret = false;
	}
	else
	{
		physx_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *physx_foundation, physx::PxTolerancesScale(), true);
		if (!physx_physics)
		{
			ret = false;
		}
		else
		{
			cooking = PxCreateCooking(PX_PHYSICS_VERSION, *physx_foundation, physx::PxCookingParams(physx::PxTolerancesScale()));
			if (!cooking)
			{
				ret = false;
			}
		}
	}

	fract_tool = NvBlastExtAuthoringCreateFractureTool();
	fract_tool->setInteriorMaterialId(MATERIAL_INTERIOR);
	blast_random_generator = new RandomGenerator();
	fract_tool->setRemoveIslands(remove_islands);
	fracture_type = VoronoiUniform;

	return ret;
}

bool ModuleBlast::CleanUp()
{
	fract_tool->release();
	if (voronoi_sites_generator)
	{
		voronoi_sites_generator->release();
	}
	delete blast_random_generator;
	blast_random_generator = nullptr;
	for (std::vector<BlastMesh*>::iterator it = blast_meshes.begin(); it != blast_meshes.end(); it++)
	{
		(*it)->blast_mesh->release();
		delete *it;
		*it = nullptr;
		it = blast_meshes.erase(it);
	}

	return true;
}

void ModuleBlast::CreateBlastMesh(float* vertices, uint num_vertices, float* normals, float* texture_coords, uint* indices, uint num_indices, std::string mesh_name,
	float3 mesh_pos, float3 mesh_rot, float3 mesh_scale, ComponentTransform* parent_transform)
{
	Nv::Blast::Mesh* mesh = NvBlastExtAuthoringCreateMesh((physx::PxVec3*)vertices, (physx::PxVec3*)normals, (physx::PxVec2*)texture_coords,
		num_vertices, indices, num_indices);
	SetCurrentMesh(mesh);
	new_mesh = true;
	UpdateMesh();
	current_selected_mesh = blast_meshes[0];
	SetCurrentMesh(blast_meshes[0]->blast_mesh);
	blast_meshes.back()->name = mesh_name;
	blast_meshes.back()->mesh_transform->SetParentTransform(parent_transform);
	blast_meshes.back()->mesh_transform->SetPosition(mesh_pos);
	blast_meshes.back()->mesh_transform->SetRotation(mesh_rot);
	blast_meshes.back()->mesh_transform->SetScale(mesh_scale);
}

void ModuleBlast::UpdateMesh()
{
	Nv::Blast::BlastBondGenerator* bond_generator = NvBlastExtAuthoringCreateBondGenerator(cooking, &physx_physics->getPhysicsInsertionCallback());
	Nv::Blast::ConvexMeshBuilder* collision_builder = NvBlastExtAuthoringCreateConvexMeshBuilder(cooking, &physx_physics->getPhysicsInsertionCallback());
	Nv::Blast::CollisionParams param;
	param.maximumNumberOfHulls = 1;
	param.voxelGridResolution = 0;
	authoring_result = NvBlastExtAuthoringProcessFracture(*fract_tool, *bond_generator, *collision_builder, param);
	bond_generator->release();
	collision_builder->release();
	
	for (int i = 0; i < authoring_result->chunkCount; i++)
	{
		if (!blast_meshes.empty() && !new_mesh && i == 0) continue;

		BlastMesh* mesh = new BlastMesh();
		Nv::Blast::Mesh* m = fract_tool->createChunkMesh(i);
		mesh->blast_mesh = m;
		mesh->chunk_depth = fract_tool->getChunkDepth(i);
		mesh->chunk_id = i;

		float* pos = authoring_result->chunkDescs[i].centroid;
		float x = pos[0];
		float y = pos[1];
		float z = pos[2];

		int triangle_count = authoring_result->geometryOffset[i + 1] - authoring_result->geometryOffset[i];
		mesh->num_vertices = triangle_count * 3;
		mesh->vertices_data = new float[mesh->num_vertices * 12];
		mesh->num_indices = triangle_count * 3;
		mesh->indices = new uint[mesh->num_indices];
		int vertex_index = 0;
		int indice_index = 0;
		int start_triangle = authoring_result->geometryOffset[i];
		int end_triangle = authoring_result->geometryOffset[i + 1];
		int inter_mat = fract_tool->getInteriorMaterialId();
		std::vector<int> inter_mat_indices;

		for (int j = start_triangle; j < end_triangle; j++)
		{
			Nv::Blast::Triangle& tr = authoring_result->geometry[j];
			for (int vi = 0; vi < 3; vi++, vertex_index++)
			{
				auto& vertex = (&tr.a)[vi];
				mesh->vertices_data[vertex_index * 12 + 0] = vertex.p.x;
				mesh->vertices_data[vertex_index * 12 + 1] = vertex.p.y;
				mesh->vertices_data[vertex_index * 12 + 2] = vertex.p.z;
				mesh->vertices_data[vertex_index * 12 + 3] = vertex.uv->x;
				mesh->vertices_data[vertex_index * 12 + 4] = vertex.uv->y;
				mesh->vertices_data[vertex_index * 12 + 5] = vertex.n.x;
				mesh->vertices_data[vertex_index * 12 + 6] = vertex.n.y;
				mesh->vertices_data[vertex_index * 12 + 7] = vertex.n.z;
				mesh->vertices_data[vertex_index * 12 + 8] = 1;
				mesh->vertices_data[vertex_index * 12 + 9] = 1;
				mesh->vertices_data[vertex_index * 12 + 10] = 1;
				mesh->vertices_data[vertex_index * 12 + 11] = 1;

				if (tr.materialId != inter_mat)
				{
					mesh->indices[indice_index] = vertex_index;
					indice_index++;
				}
				else
				{
					inter_mat_indices.push_back(vertex_index);
				}
			}
		}

		int tmp_indices = indice_index;
		for (auto& index : inter_mat_indices)
		{
			mesh->indices[indice_index] = index;
			indice_index++;
		}

		if (!new_mesh)
		{
			mesh->interior_material = new Material();
			mesh->interior_material->SetDiffuseColor(1, 1, 1);
			mesh->interior_material->indice_count = inter_mat_indices.size();
			mesh->interior_material->first_indice = tmp_indices;
		}

		mesh->CreateVerticesFromData();
		mesh->LoadToMemory();
		//mesh->material = mesh_material;
		mesh->material = new Material();
		mesh->material->indice_count = tmp_indices;
		mesh->material->first_indice = 0;
		App->renderer3D->AddMeshToDraw(mesh);
		if (i == 0)
		{
			blast_meshes.push_back(mesh);
		}
		if (current_selected_mesh && !new_mesh)
		{
			mesh->chunk_depth += current_selected_mesh->chunk_depth;
			current_selected_mesh->childs.push_back(mesh);
			mesh->base_displacement = float3(x, y, z);
			mesh->mesh_transform->SetPosition(/*current_selected_mesh->mesh_transform->GetGlobalPosition()*/mesh->base_displacement * explosion_amount);
			current_selected_mesh->is_root = true;
		}
		new_mesh = false;
	}
}

void ModuleBlast::SetCurrentMesh(Nv::Blast::Mesh * current)
{
	if (current_selected_mesh && current_selected_mesh->blast_mesh == current) return;
	fract_tool->setSourceMesh(current);
}

void ModuleBlast::RemoveChildChunks(BlastMesh * mesh)
{
	if (fract_tool->deleteAllChildrenOfChunk(0))
	{
		DeleteChilds(mesh);
		mesh->is_root = false;
	}
}

void ModuleBlast::ApplyFracture()
{
	if (voronoi_sites_generator) voronoi_sites_generator->release();
	voronoi_sites_generator = NvBlastExtAuthoringCreateVoronoiSitesGenerator(current_selected_mesh->blast_mesh, blast_random_generator);

	switch (fracture_type)
	{
	case ModuleBlast::VoronoiUniform:
		ApplyVoronoiFracture();
		break;
	case ModuleBlast::VoronoiClustered:
		ApplyVoronoiClusteredFracture();
		break;
	case ModuleBlast::VoronoiRadial:
		ApplyVoronoiRadialFracture();
		break;
	case ModuleBlast::VoronoiInSphere:
		ApplyVoronoiInSphereFracture();
		break;
	case ModuleBlast::VoronoiRemove:
		RemoveInSphereFracture();
		break;
	case ModuleBlast::Slice:
		ApplySliceFracture();
		break;
	default:
		break;
	}
}

void ModuleBlast::ResetMesh()
{
	fract_tool->reset();
	DeleteChilds(blast_meshes.front());
}

void ModuleBlast::OptimizeChunks()
{
	//fract_tool->uniteChunks()
}

bool ModuleBlast::ApplyVoronoiFracture()
{
	voronoi_sites_generator->uniformlyGenerateSitesInMesh(cell_counts);
	const physx::PxVec3* sites = nullptr;
	uint sites_count = voronoi_sites_generator->getVoronoiSites(sites);
	if (fract_tool->voronoiFracturing(current_selected_mesh->chunk_id, sites_count, sites, replace_chunk) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

bool ModuleBlast::ApplyVoronoiClusteredFracture()
{
	voronoi_sites_generator->clusteredSitesGeneration(cluster_count, cell_counts, cluster_radius);
	const physx::PxVec3* sites = nullptr;
	uint sites_count = voronoi_sites_generator->getVoronoiSites(sites);
	if (fract_tool->voronoiFracturing(current_selected_mesh->chunk_id, sites_count, sites, replace_chunk) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

bool ModuleBlast::ApplyVoronoiRadialFracture()
{
	physx::PxVec3 center(radial_center.x, radial_center.y, radial_center.z);
	physx::PxVec3 normal(radial_normal.x, radial_normal.y, radial_normal.z);
	voronoi_sites_generator->radialPattern(center, normal, radial_radius, radial_angular_steps, radial_steps, radial_angle_offset, radial_variability);
	const physx::PxVec3* sites = nullptr;
	uint sites_count = voronoi_sites_generator->getVoronoiSites(sites);
	if (fract_tool->voronoiFracturing(current_selected_mesh->chunk_id, sites_count, sites, replace_chunk) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

bool ModuleBlast::ApplyVoronoiInSphereFracture()
{
	physx::PxVec3 center(sphere_center.x, sphere_center.y, sphere_center.z);
	voronoi_sites_generator->generateInSphere(cell_counts, sphere_radius, center);
	const physx::PxVec3* sites = nullptr;
	uint sites_count = voronoi_sites_generator->getVoronoiSites(sites);
	if (fract_tool->voronoiFracturing(current_selected_mesh->chunk_id, sites_count, sites, replace_chunk) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

bool ModuleBlast::RemoveInSphereFracture()
{
	physx::PxVec3 center(sphere_center.x, sphere_center.y, sphere_center.z);
	voronoi_sites_generator->deleteInSphere(sphere_radius, center, remove_sphere_eraser_probability);
	const physx::PxVec3* sites = nullptr;
	uint sites_count = voronoi_sites_generator->getVoronoiSites(sites);
	if (fract_tool->voronoiFracturing(current_selected_mesh->chunk_id, sites_count, sites, replace_chunk) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

bool ModuleBlast::ApplySliceFracture()
{
	Nv::Blast::SlicingConfiguration config;
	config.angle_variations = slicing_angle_variation;
	config.noiseAmplitude = slicing_noise_amplitude;
	config.noiseFrequency = slicing_noise_frquency;
	config.noiseOctaveNumber = slicing_noise_octave_number;
	config.offset_variations = slicing_offset_variations;
	config.surfaceResolution = slicing_surface_resolution;
	config.x_slices = slicing_x_slices;
	config.y_slices = slicing_y_slices;
	config.z_slices = slicing_z_slices;

	if (fract_tool->slicing(current_selected_mesh->chunk_id, config, replace_chunk, blast_random_generator) == 0)
	{
		UpdateMesh();
		return true;
	}
	return false;
}

void ModuleBlast::DeleteChilds(BlastMesh * mesh)
{
	for (std::vector<BlastMesh*>::iterator it = mesh->childs.begin(); it != mesh->childs.end(); it = mesh->childs.erase(it))
	{
		DeleteChilds(*it);
		App->renderer3D->RemoveMeshToDraw(*it);
		delete *it;
		*it = nullptr;
	}
}

void ModuleBlast::SetFractureType(FractureType type)
{
	fracture_type = type;
}

ModuleBlast::FractureType ModuleBlast::GetFractureType() const
{
	return fracture_type;
}

void ModuleBlast::SetCellCounts(int counts)
{
	cell_counts = counts;
}

int ModuleBlast::GetCellCounts() const
{
	return cell_counts;
}

void ModuleBlast::SetReplaceChunk(bool replace)
{
	replace_chunk = replace;
}

bool ModuleBlast::GetReplaceChunk() const
{
	return replace_chunk;
}

void ModuleBlast::SetRemoveIslands(bool remove)
{
	remove_islands = remove;
}

bool ModuleBlast::GetRemoveIslands() const
{
	return remove_islands;
}

void ModuleBlast::SetClusterCount(int count)
{
	cluster_count = count;
}

int ModuleBlast::GetClusterCount() const
{
	return cluster_count;
}

void ModuleBlast::SetClusterRadius(float radius)
{
	cluster_radius = radius;
}

float ModuleBlast::GetClusterRadius() const
{
	return cluster_radius;
}

void ModuleBlast::SetRadialCenter(float3 center)
{
	radial_center = center;
}

float3 ModuleBlast::GetRadialCenter() const
{
	return radial_center;
}

void ModuleBlast::SetRadialNormal(float3 normal)
{
	radial_normal = normal;
}

float3 ModuleBlast::GetRadialNormal() const
{
	return radial_normal;
}

void ModuleBlast::SetRadialRadius(float radius)
{
	radial_radius = radius;
}

float ModuleBlast::GetRadialRadius() const
{
	return radial_radius;
}

void ModuleBlast::SetRadialAngularSteps(int steps)
{
	radial_angular_steps = steps;
}

int ModuleBlast::GetRadialAngularSteps() const
{
	return radial_angular_steps;
}

void ModuleBlast::SetRadialSteps(int steps)
{
	radial_steps = steps;
}

int ModuleBlast::GetRadialSteps() const
{
	return radial_steps;
}

void ModuleBlast::SetRadialAngleOffset(float offset)
{
	radial_angle_offset = offset;
}

float ModuleBlast::GetRadialAngleOffset() const
{
	return radial_angle_offset;
}

void ModuleBlast::SetRadialVariability(float variability)
{
	radial_variability = variability;
}

float ModuleBlast::GetRadialVariability() const
{
	return radial_variability;
}

void ModuleBlast::SetSphereCenter(float3 center)
{
	sphere_center = center;
}

float3 ModuleBlast::GetSphereCenter() const
{
	return sphere_center;
}

void ModuleBlast::SetSphereRadius(float radius)
{
	sphere_radius = radius;
}

float ModuleBlast::GetSphereRadius() const
{
	return sphere_radius;
}

void ModuleBlast::SetRemoveSphereCenter(float3 center)
{
	remove_sphere_center = center;
}

float3 ModuleBlast::GetRemoveSphereCenter() const
{
	return remove_sphere_center;
}

void ModuleBlast::SetRemoveSphereRadius(float radius)
{
	remove_sphere_radius = radius;
}

float ModuleBlast::GetRemoveSphereRadius() const
{
	return remove_sphere_radius;
}

void ModuleBlast::SetRemoveSphereEraserProbability(float probability)
{
	remove_sphere_eraser_probability = probability;
}

float ModuleBlast::GetRemoveSphereEraserProbability() const
{
	return remove_sphere_eraser_probability;
}

void ModuleBlast::SetSlicingAngleVariation(float variation)
{
	slicing_angle_variation = variation;
}

float ModuleBlast::GetSlicingAngleVariation() const
{
	return slicing_angle_variation;
}

void ModuleBlast::SetSlicingNoiseAmplitude(float amplitude)
{
	slicing_noise_amplitude = amplitude;
}

float ModuleBlast::GetSlicingNoiseAmplitude() const
{
	return slicing_noise_amplitude;
}

void ModuleBlast::SetSlicingNoiseFrequency(float frequency)
{
	slicing_noise_frquency = frequency;
}

float ModuleBlast::GetSlicingNoiseFrequency() const
{
	return slicing_noise_frquency;
}

void ModuleBlast::SetSlicingNoiseOctaveNumber(uint number)
{
	slicing_noise_octave_number = number;
}

uint ModuleBlast::GetSlicingNoiseOctaveNumber() const
{
	return slicing_noise_octave_number;
}

void ModuleBlast::SetSlicingOffsetVariations(float variations)
{
	slicing_offset_variations = variations;
}

float ModuleBlast::GetSlicingOffsetVariations() const
{
	return slicing_offset_variations;
}

void ModuleBlast::SetSlicingSurfaceResolution(uint resolution)
{
	slicing_surface_resolution = resolution;
}

uint ModuleBlast::GetSlicingSurfaceResolution() const
{
	return slicing_surface_resolution;
}

void ModuleBlast::SetSlicingX(int x)
{
	slicing_x_slices = x;
}

int ModuleBlast::GetSlicingX() const
{
	return slicing_x_slices;
}

void ModuleBlast::SetSlicingY(int y)
{
	slicing_y_slices = y;
}

int ModuleBlast::GetSlicingY() const
{
	return slicing_y_slices;
}

void ModuleBlast::SetSlicingZ(int z)
{
	slicing_z_slices = z;
}

int ModuleBlast::GetSlicingZ() const
{
	return slicing_z_slices;
}

void ModuleBlast::CreateBlastFile()
{
	Nv::Blast::TkFramework* framework = NvBlastTkFrameworkCreate();
	Nv::Blast::TkAssetDesc descriptor;
	descriptor.bondCount = authoring_result->bondCount;
	descriptor.bondDescs = authoring_result->bondDescs;
	descriptor.bondFlags = nullptr;
	descriptor.chunkCount = authoring_result->chunkCount;
	descriptor.chunkDescs = authoring_result->chunkDescs;
	Nv::Blast::ExtPxAsset* physicsAsset = Nv::Blast::ExtPxAsset::create(descriptor, authoring_result->physicsChunks, authoring_result->physicsSubchunks, *framework);
	void* buffer;
	Nv::Blast::ExtSerialization* serialization = NvBlastExtSerializationCreate();
	if (serialization != nullptr && physx_physics != nullptr && cooking != nullptr && framework != nullptr)
	{
		NvBlastExtTkSerializerLoadSet(*framework, *serialization);
		NvBlastExtPxSerializerLoadSet(*framework, *physx_physics, *cooking, *serialization);
		serialization->setSerializationEncoding(NVBLAST_FOURCC('C', 'P', 'N', 'B'));
	}
	const uint64_t buffer_size = NvBlastExtSerializationSerializeExtPxAssetIntoBuffer(buffer, *serialization, physicsAsset);
	if (buffer_size == 0)
	{
		int i = 0;
	}
	else
	{
		std::ofstream outfile("./ejjreve.bmesh", std::ofstream::binary);
		outfile.write((char*)buffer, buffer_size);
		outfile.close();
		gDefaultAllocatorCallback.deallocate(buffer);
	}
}


//Blast Random Generator
float RandomGenerator::getRandomValue()
{
	return App->RandomNumber().Float();
}

void RandomGenerator::seed(int32_t seed)
{
	App->RandomNumber().Seed(seed);
}