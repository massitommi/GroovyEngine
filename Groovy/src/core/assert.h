#pragma once

#include "coreminimal.h"

#if _MSC_VER
	#define DEBUG_BREAK() __debugbreak()
/*
* other compilers:

#else
	#define DEBUG_BREAK() compilerBreak()

*/
#else
	#define DEBUG_BREAK()
#endif

#define RELEASE_CRASH() abort()

#if BUILD_DEBUG
	#define ASSERT_FAIL_BREAK() DEBUG_BREAK()
#else
	#define ASSERT_FAIL_BREAK() RELEASE_CRASH()
#endif

#define GVY_EXPAND_MACRO(x) x
#define GVY_STRINGIFY_MACRO(x) #x

#if _MSC_VER
	#define GET_CURRENT_PROC() GVY_EXPAND_MACRO(__FUNCSIG__)
#else
	#define GET_CURRENT_PROC() GVY_EXPAND_MACRO(__FUNCTION__)
#endif

CORE_API void DisplayAssertError(const char* condition, const char* file, int line, const char* proc, const char* msg, ...);

#define ASSERT_SHOW_MSG(Condition, Msg, ...) DisplayAssertError(#Condition, __FILE__, __LINE__, GET_CURRENT_PROC(), Msg, __VA_ARGS__ )

#define	CORE_ASSERT(Condition, Msg, ...) { bool assertResult = (Condition); if(!assertResult) { ASSERT_SHOW_MSG(Condition, Msg, __VA_ARGS__); ASSERT_FAIL_BREAK(); } }

/* Asserts guide:
* - checkslow : Evaluates and checks the condition ALL THE TIME IN EVERY BUILD CONFIGURATION
* - verify : In debug mode evaluates and checks the condition, in shipping it only evaluates the condition
* - check : In debug mode evaluates and checks the condition, in shipping the code is stripped out
*/

#define checkslowf(Condition, Msg, ...) CORE_ASSERT(Condition, Msg, __VA_ARGS__)

#if !BUILD_SHIPPING
	#define verifyf(Condition, Msg, ...) CORE_ASSERT(Condition, Msg, __VA_ARGS__)
	#define checkf(Condition, Msg, ...) CORE_ASSERT(Condition, Msg, __VA_ARGS__)
#else
	#define verifyf(Condition, Msg, ...) { bool assertResult = (Condition);  }
	#define checkf(Condition, Msg, ...)  {}
#endif

#define verify(Condition) verifyf(Condition, "")
#define check(Condition) checkf(Condition, "")
#define checkslow(Condition) checkslowf(Condition, "")