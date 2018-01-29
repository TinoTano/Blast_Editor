#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include "SDL2-2.0.7/include/SDL.h"

class Application;

typedef unsigned int uint;

class ModuleWindow : public Module
{
public:

	ModuleWindow(Application* app);

	// Destructor
	virtual ~ModuleWindow();

	bool Init();
	bool CleanUp();

	void SetTitle(const char* title);
	uint GetWidth() const;
	uint GetHeight() const;
	void SetWidth(uint width);
	void SetHeight(uint height);
	void SetSize(uint width, uint height);
	float GetBrightness() const;
	void SetBrightness(float brightness);
	void SetFullScreen(bool fullscreen);
	void SetFullScreenDesktop(bool full_desktop);

	void SetIcon(const char* path);

public:
	//The window we'll be rendering to
	SDL_Window* window;

	//The surface contained by the window
	SDL_Surface* screen_surface;

	SDL_Texture* engine_icon;

private:
	uint screen_width;
	uint screen_height;
};

#endif // __ModuleWindow_H__