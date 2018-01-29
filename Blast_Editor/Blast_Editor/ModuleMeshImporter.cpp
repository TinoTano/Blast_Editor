#include "ModuleMeshImporter.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "BlastMesh.h"
#include "ModuleCamera3D.h"
#include "ModuleTextureImporter.h"
#include "Material.h"
#include "ModuleRenderer3D.h"
#include "Math/OBB.h"
#include "Texture.h"
#include "ModuleBlast.h"

#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#include "Assimp/include/cimport.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

ModuleMeshImporter::ModuleMeshImporter(Application* app) : Module(app)
{
}

ModuleMeshImporter::~ModuleMeshImporter()
{
}

bool ModuleMeshImporter::Init()
{
	name = "Mesh_Importer";

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	stream.callback = &Callback;
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleMeshImporter::Start()
{
	//CreatePrimitives();

	return true;
}

bool ModuleMeshImporter::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

void ModuleMeshImporter::ImportMesh(const char* path)
{
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr && scene->HasMeshes())
	{
		aiNode* root_node = scene->mRootNode;
		AABB camera_pos(float3::zero, float3::zero);
		LoadMeshNode(nullptr, root_node, *scene, path, camera_pos);
		//Focus the camera on the mesh
		App->camera->can_update = true;
		//App->camera->FocusOnObject(camera_pos);
		App->camera->can_update = false;
		CONSOLE_DEBUG("Object succesfully loaded from, %s", path);
		aiReleaseImport(scene);
	}
	else
	{
		CONSOLE_ERROR("Cannot load object from %s", path);
	}
}

void ModuleMeshImporter::LoadMeshNode(ComponentTransform* parent_transform, aiNode * node, const aiScene & scene, const char * path, AABB& camera_pos)
{
	ComponentTransform* transform = nullptr;

	if (node->mNumMeshes < 1)
	{
		std::string s_node_name(node->mName.C_Str());
		aiVector3D pos;
		aiQuaternion quat;
		aiVector3D scale;
		if (s_node_name.find("$AssimpFbx$_PreRotation") != std::string::npos || s_node_name.find("$AssimpFbx$_Rotation") != std::string::npos ||
			s_node_name.find("$AssimpFbx$_PostRotation") != std::string::npos || s_node_name.find("$AssimpFbx$_Scaling") != std::string::npos ||
			s_node_name.find("$AssimpFbx$_Translation") != std::string::npos)
		{
			GetDummyTransform(*node, pos, quat, scale);
			aiVector3D n_pos;
			aiQuaternion n_quat;
			aiVector3D n_scale;
			node->mTransformation.Decompose(n_scale, n_quat, n_pos);
			pos += n_pos;
			quat = quat * n_quat;
			scale = n_scale;
		}
		else
		{
			node->mTransformation.Decompose(scale, quat, pos);
		}

		transform = new ComponentTransform();
		if (parent_transform)
		{
			transform->SetParentTransform(parent_transform);
		}
		math::Quat math_quat(quat.x, quat.y, quat.z, quat.w);
		float3 rotation = math::RadToDeg(math_quat.ToEulerXYZ());
		transform->SetPosition({ pos.x, pos.y, pos.z });
		transform->SetRotation(rotation);
		transform->SetScale({ scale.x, scale.y, scale.z });
	}
	else
	{
		for (int i = 0; i < node->mNumMeshes; i++)
		{
			bool mesh_created = true; //If node have more than 1 mesh and last mesh returned false, we need to reset the return for the new mesh.

			aiMesh* ai_mesh = scene.mMeshes[node->mMeshes[i]];

			// -------------- Mesh --------------------

			uint num_vertices = ai_mesh->mNumVertices;
			float* vertices = new float[num_vertices * 3];
			memcpy(vertices, ai_mesh->mVertices, sizeof(float) * num_vertices * 3);
			CONSOLE_DEBUG("New mesh ""%s"" with %d vertices", node->mName.C_Str(), num_vertices);

			uint num_indices = 0;
			uint* indices = nullptr;
			if (ai_mesh->HasFaces())
			{
				num_indices = ai_mesh->mNumFaces * 3;
				indices = new uint[num_indices]; // assume each face is a triangle
				CONSOLE_DEBUG("New mesh ""%s"" with %d indices.", node->mName.C_Str(), num_indices);
				for (uint j = 0; j < ai_mesh->mNumFaces; ++j)
				{
					if (ai_mesh->mFaces[j].mNumIndices != 3)
					{
						CONSOLE_DEBUG("WARNING, geometry face %d with != 3 indices! Not imported", j);
						mesh_created = false;
					}
					else
					{
						memcpy(&indices[j * 3], ai_mesh->mFaces[j].mIndices, 3 * sizeof(uint));
					}
				}
				CONSOLE_DEBUG("New mesh ""%s"" with %d triangles.", node->mName.C_Str(), num_indices / 3);
			}

			if (!mesh_created) continue;

			float* normals = nullptr;
			if (ai_mesh->HasNormals())
			{
				normals = new float[num_vertices * 3];
				memcpy(normals, ai_mesh->mNormals, sizeof(float) * num_vertices * 3);
				CONSOLE_DEBUG("Mesh ""%s"" has Normals", node->mName.C_Str());
			}

			float* colors = nullptr;
			if (ai_mesh->HasVertexColors(0))
			{
				colors = new float[num_vertices * 4];
				memcpy(colors, ai_mesh->mColors[0], sizeof(float) * num_vertices * 4);
				CONSOLE_DEBUG("Mesh ""%s"" has Colors", node->mName.C_Str());
			}

			float* texture_coords = nullptr;
			if (ai_mesh->HasTextureCoords(0))
			{
				int coords_num = num_vertices * 2;
				texture_coords = new float[coords_num];
				for (int i = 0, j = 0; i < coords_num; i+=2, j++)
				{
					texture_coords[i] = ai_mesh->mTextureCoords[0][j].x;
					texture_coords[i+1] = ai_mesh->mTextureCoords[0][j].y;
				}
				CONSOLE_DEBUG("Mesh ""%s"" has UVs", node->mName.C_Str());
			}

			Material* material = nullptr;
			if (scene.HasMaterials())
			{
				aiMaterial* ai_mat = scene.mMaterials[ai_mesh->mMaterialIndex];

				if (ai_mat != nullptr)
				{
					aiString mat_name;
					ai_mat->Get(AI_MATKEY_NAME, mat_name);
					if (material == nullptr)
					{
						material = new Material();
						LoadMaterial(*material, *ai_mat);
					}
				}
			}

			material = material;

			aiVector3D assimp_position;
			aiQuaternion assimp_rotation;
			aiVector3D assimp_scale;
			node->mTransformation.Decompose(assimp_scale, assimp_rotation, assimp_position);
			math::Quat math_quat(assimp_rotation.x, assimp_rotation.y, assimp_rotation.z, assimp_rotation.w);
			float3 rotation = math::RadToDeg(math_quat.ToEulerXYZ());
			float3 pos(assimp_position.x, assimp_position.y, assimp_position.z);
			float3 scale(assimp_scale.x, assimp_scale.y, assimp_scale.z);

			/*box.SetNegativeInfinity();
			box.Enclose((float3*)vertices, num_vertices);
			math::OBB obb = box.Transform(mesh_transform->GetMatrix());
			box = obb.MinimalEnclosingAABB();

			if (box.minPoint.x < camera_pos.minPoint.x) camera_pos.minPoint.x = box.minPoint.x;
			if (box.minPoint.y < camera_pos.minPoint.y) camera_pos.minPoint.y = box.minPoint.y;
			if (box.minPoint.z < camera_pos.minPoint.z) camera_pos.minPoint.z = box.minPoint.z;
			if (box.maxPoint.x > camera_pos.maxPoint.x) camera_pos.maxPoint.x = box.maxPoint.x;
			if (box.maxPoint.y > camera_pos.maxPoint.y) camera_pos.maxPoint.y = box.maxPoint.y;
			if (box.maxPoint.z > camera_pos.maxPoint.z) camera_pos.maxPoint.z = box.maxPoint.z;*/

			std::string name = node->mName.C_Str();
			if (i > 0) name += "_" + std::to_string(i);
			App->blast->CreateBlastMesh(vertices, num_vertices, normals, texture_coords, indices, num_indices, name, pos, rotation, scale, parent_transform);
		}
		transform = parent_transform;
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		LoadMeshNode(transform, node->mChildren[i], scene, path, camera_pos);
	}
}

void ModuleMeshImporter::GetDummyTransform(aiNode & node, aiVector3D & pos, aiQuaternion & rot, aiVector3D & scale)
{
	if (node.mChildren)
	{
		std::string s_node_name(node.mName.C_Str());
		if (s_node_name.find("$AssimpFbx$_PreRotation") != std::string::npos || s_node_name.find("$AssimpFbx$_Rotation") != std::string::npos ||
			s_node_name.find("$AssimpFbx$_PostRotation") != std::string::npos || s_node_name.find("$AssimpFbx$_Scaling") != std::string::npos ||
			s_node_name.find("$AssimpFbx$_Translation") != std::string::npos)
		{
			aiVector3D node_pos;
			aiQuaternion node_quat;
			aiVector3D node_scale;
			node.mTransformation.Decompose(node_scale, node_quat, node_pos);
			pos += node_pos;
			rot = rot * node_quat;
			scale = aiVector3D(scale * node_scale);
			node = *node.mChildren[0];
			GetDummyTransform(node, pos, rot, scale);
		}
	}
}

void ModuleMeshImporter::LoadMaterial(Material & material, const aiMaterial& ai_material)
{
	aiColor3D diffuse;
	aiColor3D specular;
	aiColor3D ambient;
	aiColor3D emissive;
	aiColor3D transparent;
	aiColor3D reflective;
	aiString texture_path;
	bool wireframe = false;
	bool two_sided = false;
	int shading_model = 0;
	int blend_mode = 0;
	float opacity = 1;
	float shininess = 0;
	float shininess_strength = 1;
	float refraction = 1;
	float reflectivity = 0;
	float bump_scaling = 1;


	//COLOR
	ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	ai_material.Get(AI_MATKEY_COLOR_SPECULAR, specular);
	ai_material.Get(AI_MATKEY_COLOR_AMBIENT, ambient);
	ai_material.Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
	ai_material.Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
	ai_material.Get(AI_MATKEY_COLOR_REFLECTIVE, reflective);

	material.SetDiffuseColor(diffuse.r, diffuse.g, diffuse.b);
	material.SetSpecularColor(specular.r, specular.g, specular.b);
	material.SetAmbientColor(ambient.r, ambient.g, ambient.b);
	material.SetEmissiveColor(emissive.r, emissive.g, emissive.b);
	material.SetTransparentColor(transparent.r, transparent.g, transparent.b);
	material.SetReflectiveColor(reflective.r, reflective.g, reflective.b);

	//TEXTURES
	for (int i = aiTextureType_DIFFUSE; i < aiTextureType_UNKNOWN; i++)
	{
		for (int j = 0; j < ai_material.GetTextureCount((aiTextureType)i); j++)
		{
			ai_material.GetTexture((aiTextureType)i, j, &texture_path);
			Texture* texture = CreateTexture(texture_path.C_Str());
			material.SetDiffuseTexture(texture);
		}
	}

	//PROPERTIES
	ai_material.Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe);
	ai_material.Get(AI_MATKEY_TWOSIDED, two_sided);
	ai_material.Get(AI_MATKEY_SHADING_MODEL, shading_model);
	ai_material.Get(AI_MATKEY_BLEND_FUNC, blend_mode);
	ai_material.Get(AI_MATKEY_OPACITY, opacity);
	ai_material.Get(AI_MATKEY_SHININESS, shininess);
	ai_material.Get(AI_MATKEY_SHININESS_STRENGTH, shininess_strength);
	ai_material.Get(AI_MATKEY_REFRACTI, refraction);
	ai_material.Get(AI_MATKEY_COLOR_REFLECTIVE, reflectivity);
	ai_material.Get(AI_MATKEY_BUMPSCALING, bump_scaling);

	material.SetWireframe(wireframe);
	material.SetTwoSided(two_sided);
	material.SetShadingModel(shading_model);
	material.SetBlendMode(blend_mode);
	material.SetOpacity(opacity);
	material.SetShininess(shininess);
	material.SetShininessStrength(shininess_strength);
	material.SetRefraction(refraction);
	material.SetReflectivity(reflectivity);
	material.SetBumpScaling(bump_scaling);

}

Texture* ModuleMeshImporter::CreateTexture(std::string mat_texture_name)
{
	Texture* material_texture = nullptr;
	if (mat_texture_name.length() > 0)
	{
		std::string name;
		int size = mat_texture_name.find_last_of('\\');
		if (size != -1)
		{
			name = mat_texture_name.substr(size);
			for (std::string::iterator it = name.begin(); it != name.end(); it++)
			{
				if (*it != '\\') break;
				else name.erase(it);
			}
		}
		else
		{
			name = mat_texture_name;
		}
		
		material_texture = App->texture_importer->ImportTexture(name.c_str());
	}

	if (!material_texture) material_texture = new Texture();

	return material_texture;
}

void Callback(const char* message, char* c) {
	CONSOLE_DEBUG("ASSIMP: %s", message);
}
