#pragma once

#include "Module.h"
#include <vector>
#include "Nvidia/Blast/Include/extensions/authoring/NvBlastExtAuthoringTypes.h"
#include "Nvidia/PhysX/Include/PxPhysicsAPI.h"
#include "Math/float3.h"
#include <string>

typedef unsigned int uint;
class Material;
class BlastMesh;
class ComponentTransform;

namespace Nv
{
	namespace Blast
	{
		class FractureTool;
		class TkFramework;
		class TkAsset;
		class Mesh;
		class VoronoiSitesGenerator;
		struct AuthoringResult;
	}
}

class RandomGenerator : public Nv::Blast::RandomGeneratorBase
{
public:
	float getRandomValue();
	void seed(int32_t seed);
};

class ModuleBlast :
	public Module
{
public:

	enum FractureType
	{
		VoronoiUniform, VoronoiClustered, VoronoiRadial, VoronoiInSphere, VoronoiRemove, Slice
	};

	ModuleBlast(Application* app);
	virtual ~ModuleBlast();

	bool Init();
	bool CleanUp();

	void CreateBlastMesh(float* vertices, uint num_vertices, float* normals, float* text_coords, uint* indices, uint num_indices, std::string mesh_name,
		float3 mesh_pos, float3 mesh_rot, float3 mesh_scale, ComponentTransform* parent_transform);
	void UpdateMesh();
	void SetCurrentMesh(Nv::Blast::Mesh* current);
	void RemoveChildChunks(BlastMesh* mesh);
	void ApplyFracture();
	void ResetMesh();
	void OptimizeChunks();
	bool RemoveInSphereFracture();

	void SetFractureType(FractureType type);
	FractureType GetFractureType() const;
	void SetCellCounts(int counts);
	int GetCellCounts() const;
	void SetReplaceChunk(bool replace);
	bool GetReplaceChunk() const;
	void SetRemoveIslands(bool remove);
	bool GetRemoveIslands() const;

	void SetClusterCount(int count);
	int GetClusterCount() const;
	void SetClusterRadius(float radius);
	float GetClusterRadius() const;

	void SetRadialCenter(float3 center);
	float3 GetRadialCenter() const;
	void SetRadialNormal(float3 normal);
	float3 GetRadialNormal() const;
	void SetRadialRadius(float radius);
	float GetRadialRadius() const;
	void SetRadialAngularSteps(int steps);
	int GetRadialAngularSteps() const;
	void SetRadialSteps(int steps);
	int GetRadialSteps() const;
	void SetRadialAngleOffset(float offset);
	float GetRadialAngleOffset() const;
	void SetRadialVariability(float variability);
	float GetRadialVariability() const;

	void SetSphereCenter(float3 center);
	float3 GetSphereCenter() const;
	void SetSphereRadius(float radius);
	float GetSphereRadius() const;

	void SetRemoveSphereCenter(float3 center);
	float3 GetRemoveSphereCenter() const;
	void SetRemoveSphereRadius(float radius);
	float GetRemoveSphereRadius() const;
	void SetRemoveSphereEraserProbability(float probability);
	float GetRemoveSphereEraserProbability() const;

	void SetSlicingAngleVariation(float variation);
	float GetSlicingAngleVariation() const;
	void SetSlicingNoiseAmplitude(float amplitude);
	float GetSlicingNoiseAmplitude() const;
	void SetSlicingNoiseFrequency(float frequency);
	float GetSlicingNoiseFrequency() const;
	void SetSlicingNoiseOctaveNumber(uint number);
	uint GetSlicingNoiseOctaveNumber() const;
	void SetSlicingOffsetVariations(float variations);
	float GetSlicingOffsetVariations() const;
	void SetSlicingSurfaceResolution(uint resolution);
	uint GetSlicingSurfaceResolution() const;
	void SetSlicingX(int x);
	int GetSlicingX() const;
	void SetSlicingY(int y);
	int GetSlicingY() const;
	void SetSlicingZ(int z);
	int GetSlicingZ() const;

	void CreateBlastFile();

private:
	void DeleteChilds(BlastMesh* mesh);
	bool ApplyVoronoiFracture();
	bool ApplyVoronoiClusteredFracture();
	bool ApplyVoronoiRadialFracture();
	bool ApplyVoronoiInSphereFracture();
	bool ApplySliceFracture();

private:
	physx::PxFoundation* physx_foundation;
	physx::PxPhysics* physx_physics;
	physx::PxCooking* cooking;

	Nv::Blast::FractureTool* fract_tool;
	Nv::Blast::VoronoiSitesGenerator* voronoi_sites_generator;
	Nv::Blast::AuthoringResult* authoring_result;
	RandomGenerator* blast_random_generator;

	physx::PxDefaultErrorCallback gDefaultErrorCallback;
	physx::PxDefaultAllocator gDefaultAllocatorCallback;

	Material* mesh_material;
	Material* interior_mesh_material;
	FractureType fracture_type;
	int cell_counts;

	int cluster_count;
	float cluster_radius;

	float3 radial_center;
	float3 radial_normal;
	float radial_radius;
	int radial_angular_steps;
	int radial_steps;
	float radial_angle_offset;
	float radial_variability;

	float3 sphere_center;
	float sphere_radius;

	float3 remove_sphere_center;
	float remove_sphere_radius;
	float remove_sphere_eraser_probability;

	float slicing_angle_variation;
	float slicing_noise_amplitude;
	float slicing_noise_frquency;
	uint slicing_noise_octave_number;
	float slicing_offset_variations;
	uint slicing_surface_resolution;
	int slicing_x_slices;
	int slicing_y_slices;
	int slicing_z_slices;

	bool replace_chunk;
	bool remove_islands;
	bool new_mesh;

public:
	std::vector<BlastMesh*> blast_meshes;
	BlastMesh* current_selected_mesh;
	float explosion_amount;
	const char* fracture_names[];
};

