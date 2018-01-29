#pragma once

#include "Module.h"
#include <list>
#include <string>

union SDL_Event;
class BlastMesh;

struct ImFont;

class ModuleEditor :
	public Module
{
public:
	ModuleEditor(Application* app);
	~ModuleEditor();

	bool Init();
	update_status PreUpdate(float delta_time);
	update_status Update(float dt);
	bool DrawEditor();
	void DrawChunkTree(BlastMesh* mesh);
	bool CleanUp();

	void HandleInput(SDL_Event* event);
	void OpenBrowserPage(const char* url);

private:
	ImFont* font = nullptr;
	bool open_options;
};

