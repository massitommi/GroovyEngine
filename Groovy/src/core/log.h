#pragma once

#include "coreminimal.h"

enum ELogSeverity : uint32
{
	LOG_SEVERITY_INFO = BITFLAG(0),
	LOG_SEVERITY_WARNING = BITFLAG(1),
	LOG_SEVERITY_ERROR = BITFLAG(3)
};

typedef void (*GroovyLoggerProc)(ELogSeverity, const char*);

CORE_API void GroovyLog(ELogSeverity severity, const char* msg, ...);
CORE_API void SetGroovyLogger(GroovyLoggerProc proc);

#if !BUILD_SHIPPING
	#define GROOVY_LOG(Severity, Message, ...)	{ GroovyLog(Severity, Message, __VA_ARGS__); }
#else
	#define GROOVY_LOG(Severity, Message, ...)
#endif

#define GROOVY_LOG_INFO(Message, ...)		GROOVY_LOG(LOG_SEVERITY_INFO, Message, __VA_ARGS__)
#define GROOVY_LOG_WARN(Message, ...)		GROOVY_LOG(LOG_SEVERITY_WARNING, Message, __VA_ARGS__)
#define GROOVY_LOG_ERR(Message, ...)		GROOVY_LOG(LOG_SEVERITY_ERROR, Message, __VA_ARGS__)