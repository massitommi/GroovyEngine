#include "log.h"
#include "core.h"
#include <cstdarg>

static GroovyLoggerProc gLogger = nullptr;
static char gTempLogBuffer[512];

void GroovyLog(ELogSeverity severity, const char* msg, ...)
{
	if (!gLogger)
		return;

	va_list args;
	va_start(args, msg);
	vsnprintf(gTempLogBuffer, 512, msg, args);
	va_end(args);

	gLogger(severity, gTempLogBuffer);
}

void SetGroovyLogger(GroovyLoggerProc proc)
{
	gLogger = proc;
}