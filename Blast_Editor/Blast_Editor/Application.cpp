#include "Application.h"
#include "Timer.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleMeshImporter.h"
#include "ModuleTextureImporter.h"
#include "ModuleBlast.h"

Application::Application()
{
	frames = 0;
	last_frame_ms = -1;
	last_fps = 0;
	num_fps = 0;
	capped_ms = 1000 / 60;

	window = new ModuleWindow(this);
	input = new ModuleInput(this);
	renderer3D = new ModuleRenderer3D(this);
	camera = new ModuleCamera3D(this);
	editor = new ModuleEditor(this);
	mesh_importer = new ModuleMeshImporter(this);
	texture_importer = new ModuleTextureImporter(this);
	blast = new ModuleBlast(this);

	AddModule(window);
	AddModule(input);
	AddModule(renderer3D);
	AddModule(mesh_importer);
	AddModule(texture_importer);
	AddModule(camera);
	AddModule(editor);
	AddModule(blast);

	random = new math::LCG();
	cursor = nullptr;
	//cursor_add = SDL_LoadBMP(EDITOR_IMAGES_FOLDER"PlusArrow.bmp");

}

Application::~Application()
{
	window = nullptr;
	input = nullptr;
	renderer3D = nullptr;
	camera = nullptr;
	editor = nullptr;
	mesh_importer = nullptr;
	texture_importer = nullptr;

	std::list<Module*>::iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		RELEASE(*item);
		++item;
	}

	list_modules.clear();

	RELEASE(random);
	SDL_FreeCursor(cursor);
}

bool Application::Init()
{
	bool ret = true;

	// Call Init() in all modules
	std::list<Module*>::iterator item = list_modules.begin();

	while (item != list_modules.end() && ret == true)
	{
		ret = (*item)->Init();
		++item;
	}

	// After all Init calls we call Start() in all modules
	CONSOLE_DEBUG("-------------- Application Start --------------");
	item = list_modules.begin();

	while (item != list_modules.end() && ret == true)
	{
		ret = (*item)->Start();
		++item;
	}
	
	ms_timer.Start();
	fps_timer.Start();

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	frames++;
	num_fps++;

	if (fps_timer.Read() >= 1000)//in ms
	{
		fps_timer.Start();
		last_fps = num_fps;
		num_fps = 0;
	}

	last_frame_ms = ms_timer.Read();
	
	if (capped_ms > 0 && last_frame_ms < capped_ms)
	{
		SDL_Delay(capped_ms - last_frame_ms);
	}

}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	std::list<Module*>::iterator item = list_modules.begin();
	
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PreUpdate(dt);
		++item;
	}

	item = list_modules.begin();

	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->Update(dt);
		++item;
	}

	item = list_modules.begin();

	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PostUpdate(dt);
		++item;
	}
	FinishUpdate();
	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	for (std::list<Module*>::iterator it = list_modules.begin(); it != list_modules.end() && ret == true; ++it)
	{
		ret = (*it)->CleanUp();
	}

	return ret;
}

LCG & Application::RandomNumber()
{
	return *random;
}

void Application::CapFPS(int max_fps)
{
	if (max_fps > 0) capped_ms = 1000 / max_fps;
		
	else capped_ms = 0;
		
}

void Application::SetCustomCursor(EnGineCursors cursor_type)
{
	switch (cursor_type)
	{
	case Application::ENGINE_CURSOR_ADD:
		cursor = SDL_CreateColorCursor(cursor_add,0,0);
		break;
	case Application::ENGINE_CURSOR_ARROW:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		break;
	case Application::ENGINE_CURSOR_IBEAM:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		break;
	case Application::ENGINE_CURSOR_WAIT:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
		break;
	case Application::ENGINE_CURSOR_WAITARROW:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
		break;
	case Application::ENGINE_CURSOR_HAND:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		break;
	case Application::ENGINE_CURSOR_NO:
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
		break;
	default:
		break;
	}
	
	SDL_SetCursor(cursor);
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}