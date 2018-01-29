#include "Material.h"
#include "Texture.h"
#include "Application.h"
#include "ModuleTextureImporter.h"
#include <ctime>
#include "OpenGL.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "ModuleRenderer3D.h"

Material::Material()
{
	wireframe = false;
	two_sided = false;
	shading_model = 0;
	blend_mode = 0;
	opacity = 1;
	shininess = 0;
	shininess_strength = 1;
	refraction = 1;
	reflectivity = 0;
	bump_scaling = 1;

	diffuse_color = { 0.6f,0.6f,0.0f };

	

	SetDefaultShaders();
}

Material::~Material()
{

}

void Material::LoadToMemory(uint program)
{
	bool has_tex = false;
	if (diffuse_texture != nullptr && diffuse_texture->GetID() != 0)
	{
		glBindTexture(GL_TEXTURE_2D, diffuse_texture->GetID());
		has_tex = true;
	}

	bool has_mat_color = false;
	if (diffuse_color.a != 0.0f)
	{
		App->renderer3D->SetUniformVector4(program, "material_color", float4(diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a));
		has_mat_color = true;
	}

	App->renderer3D->SetUniformBool(program, "has_texture", has_tex);
	App->renderer3D->SetUniformBool(program, "has_material_color", has_mat_color);
	
}

void Material::UnloadFromMemory()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Material::SetDiffuseTexture(Texture * diffuse)
{
	diffuse_texture = diffuse;
}

void Material::SetSpecularTexture(Texture * specular)
{
	specular_texture = specular;
}

void Material::SetAmbientTexture(Texture * ambient)
{
	ambient_texture = ambient;
}

void Material::SetEmissiveTexture(Texture * emissive)
{
	emissive_texture = emissive;
}

void Material::SetHeightMapTexture(Texture * heightmap)
{
	heightmap_texture = heightmap;
}

void Material::SetNormalMapTexture(Texture * normalmap)
{
	normalmap_texture = normalmap;
}

void Material::SetShininessTexture(Texture * shininess)
{
	shininess_texture = shininess;
}

void Material::SetOpacityTexture(Texture * opacity)
{
	opacity_texture = opacity;
}

void Material::SetDisplacementTexture(Texture * displacement)
{
	displacement_texture = displacement;
}

void Material::SetLightMapTexture(Texture * ligthmap)
{
	lightmap_texture = ligthmap;
}

void Material::SetReflectionTexture(Texture * refection)
{
	reflection_texture = refection;
}

Texture * Material::GetDiffuseTexture() const
{
	return diffuse_texture;
}

Texture * Material::GetSpecularTexture() const
{
	return specular_texture;
}

Texture * Material::GetAmbientTexture() const
{
	return ambient_texture;
}

Texture * Material::GetEmissiveTexture() const
{
	return emissive_texture;
}

Texture * Material::GetHeightMapTexture() const
{
	return heightmap_texture;
}

Texture * Material::GetNormalMapTexture() const
{
	return normalmap_texture;
}

Texture * Material::GetShininessTexture() const
{
	return shininess_texture;
}

Texture * Material::GetOpacityTexture() const
{
	return opacity_texture;
}

Texture * Material::GetDisplacementTexture() const
{
	return displacement_texture;
}

Texture * Material::GetLightMapTexture() const
{
	return lightmap_texture;
}

Texture * Material::GetReflectionTexture() const
{
	return reflection_texture;
}

void Material::SetDiffuseColor(float r, float g, float b)
{
	diffuse_color.r = r;
	diffuse_color.g = g;
	diffuse_color.b = b;
	diffuse_color.a = 1.0f;
}

void Material::SetSpecularColor(float r, float g, float b)
{
	specular_color.r = r;
	specular_color.g = g;
	specular_color.b = b;
	specular_color.a = 1.0f;
}

void Material::SetAmbientColor(float r, float g, float b)
{
	ambient_color.r = r;
	ambient_color.g = g;
	ambient_color.b = b;
	ambient_color.a = 1.0f;
}

void Material::SetEmissiveColor(float r, float g, float b)
{
	emissive_color.r = r;
	emissive_color.g = g;
	emissive_color.b = b;
	emissive_color.a = 1.0f;
}

void Material::SetTransparentColor(float r, float g, float b)
{
	transparent_color.r = r;
	transparent_color.g = g;
	transparent_color.b = b;
	transparent_color.a = 1.0f;
}

void Material::SetReflectiveColor(float r, float g, float b)
{
	reflective_color.r = r;
	reflective_color.g = g;
	reflective_color.b = b;
	transparent_color.a = 1.0f;
}

Color Material::GetDiffuseColor() const
{
	return diffuse_color;
}

Color Material::GetSpecularColor() const
{
	return specular_color;
}

Color Material::GetAmbientColor() const
{
	return ambient_color;
}

Color Material::GetEmissiveColor() const
{
	return emissive_color;
}

Color Material::GetTransparentColor() const
{
	return transparent_color;
}

Color Material::GetReflectiveColor() const
{
	return reflective_color;
}

void Material::SetWireframe(bool wireframe)
{
	this->wireframe = wireframe;
}

bool Material::IsWireFrame() const
{
	return wireframe;
}

void Material::SetTwoSided(bool two_sided)
{
	this->two_sided = two_sided;
}

bool Material::IsTwoSided() const
{
	return two_sided;
}

void Material::SetShadingModel(int shading_model)
{
	this->shading_model = shading_model;
}

int Material::GetShadingModel() const
{
	return shading_model;
}

void Material::SetBlendMode(int blend_mode)
{
	this->blend_mode = blend_mode;
}

int Material::GetBlendMode() const
{
	return blend_mode;
}

void Material::SetOpacity(float opacity)
{
	this->opacity = opacity;
}

float Material::GetOpacity() const
{
	return opacity;
}

void Material::SetShininess(float shininess)
{
	this->shininess = shininess;
}

float Material::GetShininess() const
{
	return shininess;
}

void Material::SetShininessStrength(float shininess_strength)
{
	this->shininess_strength = shininess_strength;
}

float Material::GetShininessStrength() const
{
	return shininess_strength;
}

void Material::SetRefraction(float refraction)
{
	this->refraction = refraction;
}

float Material::GetRefraction() const
{
	return refraction;
}

void Material::SetReflectivity(float reflectivity)
{
	this->reflectivity = reflectivity;
}

float Material::GetReflectivity() const
{
	return reflectivity;
}

void Material::SetBumpScaling(float bump_scaling)
{
	this->bump_scaling = bump_scaling;
}

float Material::GetBumpScaling() const
{
	return bump_scaling;
}

//ShaderProgram * Material::GetShaderProgram() const
//{
//	return shader_program;
//}

//void Material::SetVertexShader(Shader * vertex)
//{
//	if (vertex != shader_program->GetVertexShader())
//	{
//
//		ShaderProgram* prog = App->resources->GetShaderProgram(vertex, shader_program->GetFragmentShader());
//
//		if (prog != nullptr)
//		{
//			shader_program = prog;
//		}
//		else
//		{
//			prog = new ShaderProgram();
//			prog->SetFragmentShader(shader_program->GetFragmentShader());
//			prog->SetVertexShader(vertex);
//			prog->LinkShaderProgram();
//
//			App->resources->AddResource(prog);
//
//			shader_program = prog;
//		}
//
//		shader_program->IncreaseUsedCount();
//	}
//}
//
//void Material::SetFragmentShader(Shader * fragment)
//{
//	if (fragment != shader_program->GetFragmentShader())
//	{
//		shader_program->DecreaseUsedCount();
//
//		ShaderProgram* prog = App->resources->GetShaderProgram(shader_program->GetVertexShader(), fragment);
//
//		if (prog != nullptr)
//		{
//			shader_program = prog;
//		}
//		else
//		{
//			prog = new ShaderProgram();
//			prog->SetFragmentShader(fragment);
//			prog->SetVertexShader(shader_program->GetVertexShader());
//			prog->LinkShaderProgram();
//
//			App->resources->AddResource(prog);
//
//			shader_program = prog;
//		}
//
//		shader_program->IncreaseUsedCount();
//	}
//}
//
//void Material::SetShaders(Shader * vertex, Shader * fragment)
//{
//	ShaderProgram* prog = App->resources->GetShaderProgram(vertex, fragment);
//
//	if (prog != nullptr)
//	{
//		shader_program = prog;
//	}
//	else
//	{
//		prog = new ShaderProgram();
//		prog->SetFragmentShader(fragment);
//		prog->SetVertexShader(vertex);
//		prog->LinkShaderProgram();
//
//		App->resources->AddResource(prog);
//
//		shader_program = prog;
//	}
//
//	shader_program->IncreaseUsedCount();
//	
//}
//
//uint Material::GetShaderProgramID() const
//{
//	return shader_program->GetProgramID();
//}

uint Material::GetShaderProgramID() const
{
	return 0;
}

void Material::SetDefaultShaders()
{
	/*Shader* vert = App->resources->GetShader("default_vertex");
	Shader* frag = App->resources->GetShader("default_fragment");

	if (vert != nullptr && frag != nullptr)
	{
		ShaderProgram* prog = App->resources->GetShaderProgram(vert, frag);

		if (prog != nullptr)
		{
			shader_program = prog;
			prog->IncreaseUsedCount();
		}
		else
			CONSOLE_ERROR("Default Shader Program missing!");
	}
	else
		CONSOLE_ERROR("Default Shaders missing!");*/

	
}

