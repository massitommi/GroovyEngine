#include "assert.h"
#include "platform/messagebox.h"

#include <cstdarg>
#include <string.h>
#include <stdio.h>

const char* ASSERTION_FAILED_TEMPLATE_MSG =
"Assertion failed: %s \n"
"[File: %s] [Line: %i] [Proc: %s] \n"
"Message: %s"
;

const char* ASSERTION_FAILED_TEMPLATE_NOMSG =
"Assertion failed: %s \n"
"[File: %s] [Line: %i] [Proc: %s]"
;

void DisplayAssertError(const char* condition, const char* file, int line, const char* proc, const char* msg, ...)
{
	char* userMsg = (char*)malloc(256);
	memset(userMsg, 0, 256);

	va_list args;
	va_start(args, msg);
	vsnprintf(userMsg, 256, msg, args);
	va_end(args);

	char* finalMsgBuffer = (char*)malloc(1024);
	memset(finalMsgBuffer, 0, 1024);

	const char* msgTemplate = strlen(msg) ? ASSERTION_FAILED_TEMPLATE_MSG : ASSERTION_FAILED_TEMPLATE_NOMSG;

	snprintf(finalMsgBuffer, 1024, msgTemplate, condition, file, line, proc, userMsg);

	SysMessageBox::Show("Fatal error!", finalMsgBuffer, MESSAGE_BOX_TYPE_ERROR, MESSAGE_BOX_OPTIONS_OK);
}
