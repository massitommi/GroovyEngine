#pragma once

class GameModule
{
public:
	static void Startup();
	static void Shutdown();

	const char* GetName();
	const char* GetStartupScene();
};