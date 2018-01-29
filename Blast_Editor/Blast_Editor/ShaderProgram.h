#pragma once

class Shader;

typedef unsigned int uint;

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	void SetVertexShader(Shader* shader);
	void SetFragmentShader(Shader* shader);
	void SetShaders(Shader* vert, Shader* frag);

	Shader* GetVertexShader()const;
	Shader* GetFragmentShader()const;

	void UseProgram();
	void LinkShaderProgram();
	uint GetProgramID() const;

	void LoadToMemory();
	void UnloadFromMemory();

private:
	Shader* vertex_shader;
	Shader* fragment_shader;

	uint program_id;
};

