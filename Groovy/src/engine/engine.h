#pragma once

#include "core/core.h"
#include "engine/project.h"
#include "classes/class_db.h"

extern CORE_API bool gEngineShouldRun;
extern CORE_API class Window* gWindow;
extern CORE_API class FrameBuffer* gScreenFrameBuffer;
extern CORE_API GroovyProject gProj;
extern CORE_API ClassDB gClassDB;
extern CORE_API Vec4 gScreenClearColor;
extern CORE_API double gTime;
extern CORE_API double gDeltaTime;
extern CORE_API std::vector<GroovyClass*> ENGINE_CLASSES;
extern CORE_API std::vector<GroovyClass*> GAME_CLASSES;