#pragma once
#include "Module.h"
#include "Globals.h"
#include "Light.h"
#include <list>

class BlastMesh;
class Primitive;
class ComponentCamera;
class ShaderProgram;

#define MAX_LIGHTS 8

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app);
	~ModuleRenderer3D();

	bool Init();
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnResize(int width, int height, ComponentCamera* camera);
	void SetWireframeMode();

	void SetActiveDepthTest(bool active);
	void SetActiveCullTest(bool active);
	void SetActiveFog(bool active);

	bool GetActiveDepthTest() const;
	bool GetActiveCullTest() const;
	bool GetActiveFog() const;

	void EnableTestLight();
	void DisableTestLight();

	void ActiveSkybox(bool active);
	bool IsUsingSkybox()const;

	void AddMeshToDraw(BlastMesh* mesh);
	void RemoveMeshToDraw(BlastMesh* mesh);
	void SetExplosionDisplacement(float displacement);
	void ResetRender();

	void BindArrayBuffer(uint id) const;
	void BindElementArrayBuffer(uint id) const;

	void UnbindArraybuffer() const;
	void UnbindElementArrayBuffer() const;

	//Shaders
	uint GenVertexArrayObject() const;
	void BindVertexArrayObject(uint id) const;
	void UnbindVertexArrayObject() const;

	uint CreateVertexShader(const char* source);
	uint CreateFragmentShader(const char* source);
	void DeleteShader(uint shader_id);

	uint GetProgramBinary(uint program_id, uint buff_size, char* buff) const;
	int GetProgramSize(uint program_id) const;
	void LoadProgramFromBinary(uint program_id, uint buff_size, const char* buff);

	void EnableVertexAttributeArray(uint id);
	void DisableVertexAttributeArray(uint id);
	void SetVertexAttributePointer(uint id, uint element_size, uint elements_gap, uint infogap);

	void UseShaderProgram(uint id);
	
	void SetUniformBool(uint program, const char* name, bool data);
	void SetUniformFloat(uint program, const char* name, float data);
	void SetUniformVector4(uint program, const char* name, float4 data);
	void SetUniformMatrix(uint program, const char* name, float* data);

	uint CreateShaderProgram();
	void AttachShaderToProgram(uint program_id, uint shader_id);
	bool LinkProgram(uint program_id);
	void DeleteProgram(uint program_id);

private:
	void DrawSceneGameObjects(ComponentCamera* active_camera, bool is_editor_camera);
	void DrawMesh(BlastMesh* mesh, ComponentCamera* active_camera);
	void DrawEditorScene();
	void DrawDebugCube(BlastMesh* mesh, ComponentCamera* active_camera);

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	std::list<ComponentCamera*> rendering_cameras;
	ComponentCamera* editor_camera;
	ComponentCamera* game_camera;

private:
	bool use_vsync;
	bool is_using_depth_test;
	bool is_using_cull_test;
	bool is_using_fog;

	bool testing_light;
	bool use_skybox;

	int lights_count;

	std::list<BlastMesh*> dynamic_mesh_to_draw;
	std::list<Primitive*> debug_primitive_to_draw;
	ShaderProgram* shader_program = nullptr;
};