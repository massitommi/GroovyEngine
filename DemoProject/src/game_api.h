#pragma once
#include "core/api.h"
#include "classes/class.h"

#define ADD_CLASS(Class) &GROOVY_CLASS_NAME(Class)

#if !BUILD_MONOLITHIC
	#define GAME_CLASSES_BEGIN() extern "C" { DLL_EXPORT std::vector<GroovyClass*> GAME_CLASSES_LIST
	#define GAME_CLASSES_END() ; }
#else
	#define GAME_CLASSES_BEGIN() std::vector<GroovyClass*> GAME_CLASSES_LIST
	#define GAME_CLASSES_END() ;
#endif