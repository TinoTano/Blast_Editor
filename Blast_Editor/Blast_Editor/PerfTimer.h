#pragma once

#include "Globals.h"
#include "SDL2-2.0.7/include/SDL.h"

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();
	double ReadMs() const;
	Uint64 ReadTicks() const;

private:
	Uint64	started_at;
	static Uint64 frequency;
};

