#include "Shader.h"
#include "Application.h"
#include "ModuleRenderer3D.h"

Shader::Shader()
{
	shader_id = 0;
	shader_type = ST_NULL;
}

Shader::~Shader()
{
}

void Shader::SetShaderType(ShaderType type)
{
	shader_type = type;
}

Shader::ShaderType Shader::GetShaderType() const
{
	return shader_type;
}

void Shader::SetID(uint id)
{
	shader_id = id;
}

uint Shader::GetID() const
{
	return shader_id;
}

void Shader::SetContent(std::string content)
{
	shader_text = content;
	UpdateShader();
}

std::string Shader::GetContent() const
{
	return shader_text;
}

void Shader::LoadToMemory()
{
}

void Shader::UnloadFromMemory()
{
}

void Shader::UpdateShader()
{
	//if ()
		//App->resources->OnShaderUpdate(this);
	CompileShader();
}

bool Shader::CompileShader()
{
	bool ret = false;
	switch (shader_type)
	{
	case ST_VERTEX:
		shader_id = App->renderer3D->CreateVertexShader(shader_text.c_str());
		break;
	case ST_FRAGMENT:
		shader_id = App->renderer3D->CreateFragmentShader(shader_text.c_str());
		break;
	}
	if (shader_id != 0)
	{
		CONSOLE_DEBUG("Shader compilation Success :)");
		ret = true;
	}
	return ret;
}
