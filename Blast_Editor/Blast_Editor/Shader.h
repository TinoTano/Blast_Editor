#pragma once

#include <string>

typedef unsigned int uint;

class Shader
{
public:
	enum ShaderType
	{
		ST_NULL,
		ST_VERTEX,
		ST_FRAGMENT,
	};

	Shader();
	~Shader();

	void SetShaderType(ShaderType type);
	ShaderType GetShaderType() const;

	void SetID(uint id);
	uint GetID() const;

	void SetContent(std::string content);
	std::string GetContent() const;

	void LoadToMemory();
	void UnloadFromMemory();

	void UpdateShader();

private:
	bool CompileShader();

private:
	uint shader_id;
	ShaderType shader_type;
	std::string shader_text;
};

