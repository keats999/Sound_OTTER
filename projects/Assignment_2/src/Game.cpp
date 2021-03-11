//------------------------------------------------------------------------
// Game.cpp
//------------------------------------------------------------------------

#include "AudioEngine.h"

float gameTime;

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------

void Init()
{
	AudioEngine& engine = AudioEngine::Instance();
	engine.LoadBank("Master");
	engine.LoadBank("Master.strings");

	AudioEvent& CarCrashSound = engine.CreateEvent("sound", "event:/Car Crash");
	CarCrashSound.Play();
}

//------------------------------------------------------------------------
// Update your game here. 
//------------------------------------------------------------------------
void Update(float deltaTime)
{

	// Increment game time
	gameTime += deltaTime;
	
	// After 32 seconds, stop running
	if (gameTime >= 32.0f)
	{
		exit(0);
	}
}


//------------------------------------------------------------------------
// Add your display calls here
//------------------------------------------------------------------------
void Render()
{
	// Who needs graphics when you have audio?
}


//------------------------------------------------------------------------
// Add your shutdown code here.
//------------------------------------------------------------------------
void Shutdown()
{

}