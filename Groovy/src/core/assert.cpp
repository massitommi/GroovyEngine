#include "assert.h"
#include "core.h"
#include "platform/messagebox.h"
#include "utils/string_utils.h"

constexpr const char* ASSERT_ERROR_MESSAGE =
"Assertion failed!\n\n"
"Condition: %s\n"
"File: %s\n"
"Line: %i\n"
"Proc: %s\n\n"
"%s";

constexpr const char* ASSERT_ERROR_NOMSG =
"Assertion failed!\n\n"
"Condition: %s\n"
"File: %s\n"
"Line: %i\n"
"Proc: %s\n\n";

void DisplayAssertError(const char* condition, const char* file, int line, const char* proc, const char* msg)
{
	char* errorMsg = (char*)malloc(512);
	memset(errorMsg, 0, 512);
	
	if(msg)
		sprintf_s(errorMsg, 512, ASSERT_ERROR_MESSAGE, condition, file, line, proc, msg);
	else
		sprintf_s(errorMsg, 512, ASSERT_ERROR_NOMSG, condition, file, line, proc);

	SysMessageBox::Show("Fatal error!", errorMsg, MESSAGE_BOX_TYPE_ERROR, MESSAGE_BOX_OPTIONS_OK);
}
