#include "engine.h"

CORE_API bool gEngineShouldRun = true;
CORE_API Window* gWindow = nullptr;
CORE_API FrameBuffer* gScreenFrameBuffer = nullptr;
CORE_API GroovyProject gProj;
CORE_API ClassDB gClassDB;
CORE_API Vec4 gScreenClearColor = { 0.9f, 0.7f, 0.7f, 1.0f };
CORE_API double gTime = 0.0f;
CORE_API double gDeltaTime = 0.0f;

CORE_API std::vector<GroovyClass*> GAME_CLASSES;