#pragma once

#include "Globals.h"
#include "Math/LCG.h"
#include <list>
#include "Timer.h"

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleEditor;
class ModuleMeshImporter;
class ModuleTextureImporter;
class ModuleBlast;
struct SDL_Surface;
struct SDL_Cursor;

class Application
{

public:

	enum EnGineCursors
	{
		ENGINE_CURSOR_ADD, ENGINE_CURSOR_ARROW, ENGINE_CURSOR_IBEAM, ENGINE_CURSOR_WAIT, ENGINE_CURSOR_WAITARROW, ENGINE_CURSOR_HAND, ENGINE_CURSOR_NO
	};

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();
	LCG& RandomNumber();
	void CapFPS(int max_fps);
	void SetCustomCursor(EnGineCursors cursor_type);

	enum EngineState {
		OnPlay, OnPause, OnStop
	};

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;
	ModuleEditor* editor;
	ModuleMeshImporter* mesh_importer;
	ModuleTextureImporter* texture_importer;
	ModuleBlast* blast;

private:

	Timer	ms_timer;
	Timer	fps_timer;
	Timer	module_ms_timer;
	float	dt;
	std::list<Module*> list_modules;
	LCG*	random = nullptr;
	
	//Variables needed to show ms and fps graphs
	int	num_fps;
	int	last_frame_ms;
	int	last_fps;
	int frames;
	int capped_ms;
	SDL_Cursor* cursor;
	EngineState state = EngineState::OnStop;

	SDL_Surface* cursor_add;

};

// Give App pointer access everywhere
extern Application* App;