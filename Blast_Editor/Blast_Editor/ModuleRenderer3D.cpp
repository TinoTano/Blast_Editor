#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "OpenGL.h"
#include "ModuleEditor.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ModuleCamera3D.h"
#include "BlastMesh.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Shader.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "glew-2.1.0/libx86/glew32.lib")

ModuleRenderer3D::ModuleRenderer3D(Application* app) : Module(app)
{
	use_vsync = true;
	is_using_depth_test = false;
	is_using_cull_test = false;
	is_using_fog = false;
	testing_light = false;
	name = "Renderer";
	editor_camera = nullptr;
	game_camera = nullptr;
	use_skybox = true;
	lights_count = 0;
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	CONSOLE_DEBUG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);
	if(context == NULL)
	{
		CONSOLE_DEBUG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	glewInit();
	
	if(ret == true)
	{
		//Use Vsync
		if(use_vsync && SDL_GL_SetSwapInterval(1) < 0)
			CONSOLE_DEBUG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		App->camera->CreateEditorCamera();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			CONSOLE_DEBUG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			CONSOLE_DEBUG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glClearDepth(1.0f);
		
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_DEBUG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize clear color
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			CONSOLE_DEBUG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//lights[0].Active(true);
		is_using_depth_test = true;
		is_using_cull_test = true;

		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_DEBUG("Error initializing OpenGL! %s\n", gluErrorString(error));
		}

		shader_program = new ShaderProgram();

		Shader* default_vert = new Shader();
		default_vert->SetShaderType(Shader::ShaderType::ST_VERTEX);

		std::string shader_text =
			"#version 330 core\n"
			"layout(location = 0) in vec3 position;\n"
			"layout(location = 1) in vec2 texCoord;\n"
			"layout(location = 2) in vec3 normals;\n"
			"layout(location = 3) in vec4 color;\n\n"
			"out vec4 ourColor;\n"
			"out vec3 Normal;\n"
			"out vec2 TexCoord;\n\n"
			"uniform mat4 Model;\n"
			"uniform mat4 view;\n"
			"uniform mat4 projection;\n\n"
			"void main()\n"
			"{ \n"
			"	gl_Position = projection * view * Model * vec4(position, 1.0f);\n"
			"	ourColor = color;\n"
			"	TexCoord = texCoord.xy;\n"
			"}";

		default_vert->SetContent(shader_text);

		Shader* default_frag = new Shader();
		default_frag->SetShaderType(Shader::ShaderType::ST_FRAGMENT);

		std::string shader_text2 =
			"#version 330 core\n"
			"in vec4 ourColor;\n"
			"in vec3 Normal;\n"
			"in vec2 TexCoord;\n\n"
			"out vec4 color;\n\n"
			"uniform bool has_material_color;\n"
			"uniform vec4 material_color;\n"
			"uniform bool has_texture;\n"
			"uniform sampler2D ourTexture;\n\n"
			"void main()\n"
			"{\n"
			"	if(has_texture)\n"
			"		color = texture(ourTexture, TexCoord);\n"
			"	else if(has_material_color)\n"
			"		color = material_color;\n"
			"	else\n"
			"		color = ourColor;\n"
			"}";

		default_frag->SetContent(shader_text2);

		shader_program->SetShaders(default_vert, default_frag);
		shader_program->LinkShaderProgram();
	}
	
	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	ms_timer.Start();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	if (editor_camera != nullptr)
	{
		DrawEditorScene();
	}

	//Assert polygon mode is fill before render gui
	GLint polygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	App->editor->DrawEditor();

	//reset polygonmode to previous one
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	SDL_GL_SwapWindow(App->window->window);
	return UPDATE_CONTINUE;
}

void ModuleRenderer3D::DrawEditorScene()
{
	if (use_skybox)
	{
		//glDisable(GL_DEPTH_TEST);
		//App->scene->DrawSkyBox(editor_camera->camera_frustum.pos);
		//glEnable(GL_DEPTH_TEST);
	}

	DrawSceneGameObjects(editor_camera, true);
}

void ModuleRenderer3D::DrawDebugCube(BlastMesh * mesh, ComponentCamera * active_camera)
{
	//if (mesh->GetMesh() != nullptr)
	//{
	//	AABB aabb = mesh->GetMesh()->box;
	//	float3 size = aabb.Size();
	//	float3 pos = aabb.CenterPoint();
	//	Quat rot = Quat::identity;
	//	float4x4 trans = float4x4::FromTRS(pos, rot, size);

	//	ShaderProgram* program = default_program;

	//	UseShaderProgram(program->GetProgramID());

	//	SetUniformMatrix(program->GetProgramID(), "view", active_camera->GetViewMatrix());
	//	SetUniformMatrix(program->GetProgramID(), "projection", active_camera->GetProjectionMatrix());
	//	SetUniformMatrix(program->GetProgramID(), "Model", trans.Transposed().ptr());

	//	SetUniformBool(program->GetProgramID(), "has_texture", false);
	//	SetUniformBool(program->GetProgramID(), "has_material_color", true);
	//	SetUniformVector4(program->GetProgramID(), "material_color", float4(1.0f, 0.5f, 0.0f, 1.0f));

	//	Mesh* cube = App->resources->GetMesh("PrimitiveCube");
	//	if (cube->id_indices == 0) cube->LoadToMemory();

	//	//set wireframe before render 
	//	GLint polygonMode;
	//	glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
	//	SetWireframeMode();

	//	BindVertexArrayObject(cube->id_vao);
	//	glDrawElements(GL_TRIANGLES, cube->num_indices, GL_UNSIGNED_INT, NULL);
	//	UnbindVertexArrayObject();

	//	//restore previous polygon mode
	//	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
	//}
}

void ModuleRenderer3D::DrawSceneGameObjects(ComponentCamera* active_camera, bool is_editor_camera)
{
	int i = 0;
	for (std::list<BlastMesh*>::iterator it = dynamic_mesh_to_draw.begin(); it != dynamic_mesh_to_draw.end(); it++)
	{
		if ((*it)->is_root) continue;
		DrawMesh(*it, active_camera);
		
		/*if ((*it)->GetGameObject()->IsSelected())
		{
			DrawDebugCube(*it, active_camera);
		}*/
	}
}

void ModuleRenderer3D::DrawMesh(BlastMesh * mesh, ComponentCamera* active_camera)
{
	if (mesh == nullptr) return;
	if (mesh->id_indices == 0) mesh->LoadToMemory();
	
	Material* material = mesh->material;
	Material* interior_material = mesh->interior_material;

	uint program = shader_program->GetProgramID();
	UseShaderProgram(program);

	SetUniformMatrix(program, "view", active_camera->GetViewMatrix());
	SetUniformMatrix(program, "projection", active_camera->GetProjectionMatrix());
	SetUniformMatrix(program, "Model", mesh->mesh_transform->GetMatrix().Transposed().ptr());
	BindVertexArrayObject(mesh->id_vao);

	if (material != nullptr)
	{
		material->LoadToMemory(program);
		glDrawElements(GL_TRIANGLES, material->indice_count, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * material->first_indice));
	}

	if (interior_material != nullptr)
	{
		interior_material->LoadToMemory(program);
		glDrawElements(GL_TRIANGLES, interior_material->indice_count, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * interior_material->first_indice));
	}
	
	UnbindVertexArrayObject();
}

void ModuleRenderer3D::AddMeshToDraw(BlastMesh * mesh)
{
	dynamic_mesh_to_draw.push_back(mesh);
}

void ModuleRenderer3D::RemoveMeshToDraw(BlastMesh * mesh)
{
	dynamic_mesh_to_draw.remove(mesh);
}

void ModuleRenderer3D::SetExplosionDisplacement(float displacement)
{
	for (std::list<BlastMesh*>::iterator it = dynamic_mesh_to_draw.begin(); it != dynamic_mesh_to_draw.end(); it++)
	{
		(*it)->SetExplosionDisplacement(displacement);
	}
}

void ModuleRenderer3D::ResetRender()
{
	dynamic_mesh_to_draw.clear();
	debug_primitive_to_draw.clear();
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	CONSOLE_DEBUG("Destroying 3D Renderer");
	SDL_GL_DeleteContext(context);
	rendering_cameras.clear();
	return true;
}


void ModuleRenderer3D::OnResize(int width, int height, ComponentCamera* camera)
{
	float ratio = (float)width / (float)height;
	camera->SetAspectRatio(ratio);
	glViewport(0, 0, width, height);
}

void ModuleRenderer3D::SetWireframeMode()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void ModuleRenderer3D::SetActiveDepthTest(bool active)
{
	is_using_depth_test = active;

	if (active)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void ModuleRenderer3D::SetActiveCullTest(bool active)
{
	is_using_cull_test = active;

	if (active)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

void ModuleRenderer3D::SetActiveFog(bool active)
{
	is_using_fog = active;

	if (active)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}
}

bool ModuleRenderer3D::GetActiveDepthTest() const
{
	return is_using_depth_test;
}

bool ModuleRenderer3D::GetActiveCullTest() const
{
	return is_using_cull_test;
}

bool ModuleRenderer3D::GetActiveFog() const
{
	return is_using_fog;
}

void ModuleRenderer3D::EnableTestLight()
{
	lights[7].ref = GL_LIGHT7;
	lights[7].Active(true);
	for (uint i = 1; i < lights_count - 1; ++i)
		lights[i].Active(false);
	testing_light = true;
}

void ModuleRenderer3D::DisableTestLight()
{
	lights[7].Active(false);
	for (uint i = 1; i < lights_count - 1; ++i)
		lights[i].Active(true);
	lights[7].ref = 0;
	testing_light = false;
}

void ModuleRenderer3D::ActiveSkybox(bool active)
{
	use_skybox = active;
}

bool ModuleRenderer3D::IsUsingSkybox() const
{
	return use_skybox;
}

void ModuleRenderer3D::BindArrayBuffer(uint id) const
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error bind array buffer: %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::BindElementArrayBuffer(uint id) const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error bind buffer: %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::UnbindArraybuffer() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error unbind array buffer: %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::UnbindElementArrayBuffer() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR ("Error unbind buffer: %s\n", gluErrorString(error));
	}
}

// ------------- Shaders -------------------------

uint ModuleRenderer3D::GenVertexArrayObject() const
{
	uint ret = 0;
	glGenVertexArrays(1, (GLuint*)&ret);
	return ret;
}

void ModuleRenderer3D::BindVertexArrayObject(uint id) const
{
	glBindVertexArray(id);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error bind vertex array buffer: %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::UnbindVertexArrayObject() const
{
	glBindVertexArray(0);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error unbind array buffer: %s\n", gluErrorString(error));
	}
}

uint ModuleRenderer3D::CreateVertexShader(const char * source)
{
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &source, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		CONSOLE_ERROR("Shader compilation error:\n %s", infoLog);
		glDeleteShader(vertexShader);
		return 0;
	}
	return vertexShader;
}

uint ModuleRenderer3D::CreateFragmentShader(const char * source)
{
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &source, NULL);
	glCompileShader(fragmentShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		CONSOLE_ERROR("Shader compilation error:\n %s", infoLog);
		glDeleteShader(fragmentShader);
		return 0;
	}
	return fragmentShader;
}

void ModuleRenderer3D::DeleteShader(uint shader_id)
{
	if (shader_id != 0)
	{
		glDeleteShader(shader_id);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_ERROR("Error deleting shader %s\n", gluErrorString(error));
		}
	}
}

uint ModuleRenderer3D::GetProgramBinary(uint program_id, uint buff_size, char * buff) const
{
	uint ret = 0;

	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	GLint *binaryFormats = new GLint[formats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);

	glGetProgramBinary(program_id, buff_size, (GLsizei*)&ret, (GLenum*)binaryFormats, buff);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error getting shader program %d binary! %s\n", program_id, gluErrorString(error));
	}

	RELEASE_ARRAY(binaryFormats);

	return ret;
}

int ModuleRenderer3D::GetProgramSize(uint program_id) const
{
	int ret = 0;

	glGetProgramiv(program_id, GL_PROGRAM_BINARY_LENGTH, &ret);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error getting shader program %d size! %s\n", program_id, gluErrorString(error));

	}

	return ret;
}

void ModuleRenderer3D::LoadProgramFromBinary(uint program_id, uint buff_size, const char * buff)
{
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	GLint *binaryFormats = new GLint[formats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);

	glProgramBinary(program_id, (GLenum)binaryFormats, buff, buff_size);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error loading shader program binary %s\n", gluErrorString(error));

	}
}

void ModuleRenderer3D::EnableVertexAttributeArray(uint id)
{
	glEnableVertexAttribArray(id);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error enabling vertex attribute Pointer %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::DisableVertexAttributeArray(uint id)
{
	glDisableVertexAttribArray(id);

	//Check for error
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error disabling vertex attributePointer %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::SetVertexAttributePointer(uint id, uint element_size, uint elements_gap, uint infogap)
{
	glVertexAttribPointer(id, element_size, GL_FLOAT, GL_FALSE, elements_gap * sizeof(GLfloat), (void*)(infogap * sizeof(GLfloat)));
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error Setting vertex attributePointer %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::UseShaderProgram(uint id)
{
	glUseProgram(id);
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error at use shader program: %s\n", gluErrorString(error));
	}
}

void ModuleRenderer3D::SetUniformMatrix(uint program, const char * name, float * data)
{
	GLint modelLoc = glGetUniformLocation(program, name);
	if (modelLoc != -1)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, data);
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error Setting uniform matrix %s: %s\n", name, gluErrorString(error));
	}
}

void ModuleRenderer3D::SetUniformBool(uint program, const char * name, bool data)
{
	GLint modelLoc = glGetUniformLocation(program, name);
	if (modelLoc != -1)
		glUniform1i(modelLoc, data);
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error Setting uniform float %s: %s\n", name, gluErrorString(error));
	}
}

void ModuleRenderer3D::SetUniformFloat(uint program, const char * name, float data)
{
	GLint modelLoc = glGetUniformLocation(program, name);
	if (modelLoc != -1)
		glUniform1f(modelLoc, data);
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error Setting uniform float %s: %s\n", name, gluErrorString(error));
	}
}

void ModuleRenderer3D::SetUniformVector4(uint program, const char * name, float4 data)
{
	GLint modelLoc = glGetUniformLocation(program, name);
	if (modelLoc != -1)
		glUniform4f(modelLoc, data.x, data.y, data.z, data.w);
	GLenum error = glGetError();

	//Check for error
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error Setting uniform float %s: %s\n", name, gluErrorString(error));
	}
}

uint ModuleRenderer3D::CreateShaderProgram()
{
	uint ret = glCreateProgram();
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		CONSOLE_ERROR("Error creating shader program %s\n", gluErrorString(error));
	}
	return ret;
}

void ModuleRenderer3D::AttachShaderToProgram(uint program_id, uint shader_id)
{
	if (program_id != 0 && shader_id != 0)
	{
		glAttachShader(program_id, shader_id);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_ERROR("Error attaching shader %s\n", gluErrorString(error));
		}
	}
}

bool ModuleRenderer3D::LinkProgram(uint program_id)
{
	bool ret = true;

	if (program_id != 0)
	{
		glLinkProgram(program_id);

		GLint success;
		GLint valid;
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		glGetProgramiv(program_id, GL_VALIDATE_STATUS, &valid);
		if (!success || !valid) {
			GLchar infoLog[512];
			glGetProgramInfoLog(program_id, 512, NULL, infoLog);
			CONSOLE_ERROR("Shader link error: %s", infoLog);
			ret = false;
		}
	}
	else ret = false;

	return ret;
}

void ModuleRenderer3D::DeleteProgram(uint program_id)
{
	if (program_id != 0)
	{
		glDeleteProgram(program_id);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_ERROR("Error deleting shader program %s\n", gluErrorString(error));
		}
	}
}
// ------------------------------------------------
