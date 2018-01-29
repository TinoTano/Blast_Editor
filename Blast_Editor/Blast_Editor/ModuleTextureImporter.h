#pragma once
#include "Module.h"

class Texture;

class ModuleTextureImporter :
	public Module
{
public:
	ModuleTextureImporter(Application* app);
	~ModuleTextureImporter();

	bool CleanUp();

	//Retuns the library path if created or an empty string
	Texture* ImportTexture(const char* path);
};

