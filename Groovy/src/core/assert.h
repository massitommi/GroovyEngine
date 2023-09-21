#pragma once

#include "coreminimal.h"

#if _MSC_VER
	#define DEBUG_BREAK() __debugbreak()
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

CORE_API void DisplayAssertError(const char* condition, const char* file, int line, const char* proc, const char* msg);

#define	CORE_ASSERT(Condition, Msg)														\
{																						\
	if(!(Condition))																	\
	{																					\
		DisplayAssertError(#Condition, __FILE__, __LINE__, GET_CURRENT_PROC(), Msg);	\
		ASSERT_FAIL_BREAK();															\
	}																					\
}

/* Asserts guide:
* - checkslow : Evaluates and checks the condition ALL THE TIME IN EVERY BUILD CONFIGURATION
* - verify : In debug mode evaluates and checks the condition, in shipping it only evaluates the condition
* - check : In debug mode evaluates and checks the condition, in shipping the code is stripped out
*/

#if !BUILD_SHIPPING
	#define checkf(Condition, Msg)	CORE_ASSERT(Condition, Msg)
	#define verifyf(Condition, Msg)	CORE_ASSERT(Condition, Msg)
#else
	#define checkf(Condition, Msg)
	#define verifyf(Condition, Msg)	{ bool verifyRes = (Condition); }
#endif

#define checkslowf(Condition, Msg)	CORE_ASSERT(Condition, Msg)

#define check(Condition)			checkf(Condition, "")
#define verify(Condition)			verifyf(Condition, "")
#define checkslow(Condition)		checkslowf(Condition, "")