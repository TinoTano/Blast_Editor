#pragma once
#include "Module.h"
#include <string>
#include "Assimp\include\quaternion.h"
#include "Assimp\include\vector3.h"
#include "Math/AABB.h"

class Mesh;
struct aiNode;
struct aiScene;
struct aiMaterial;
class Material;
class Texture;
class ComponentTransform;

class ModuleMeshImporter :
	public Module
{
public:
	ModuleMeshImporter(Application* app);
	~ModuleMeshImporter();

	bool Init();
	bool Start();
	bool CleanUp();

	void ImportMesh(const char* path);

private:
	//Loads the model and returns the root gameobject
	void LoadMeshNode(ComponentTransform* parent_transform, aiNode* node, const aiScene& scene, const char* path, AABB& camera_pos);
	void GetDummyTransform(aiNode& node, aiVector3D& pos, aiQuaternion& rot, aiVector3D& scale);
	void LoadMaterial(Material& material, const aiMaterial& ai_material);
	Texture* CreateTexture(std::string mat_texture_path);

	void CreatePrimitives() const;
	void CreateBox() const;
	void CreatePlane() const;

};

void Callback(const char* message, char* c);

