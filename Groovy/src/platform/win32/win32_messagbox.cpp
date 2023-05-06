#if PLATFORM_WIN32

#include "core/core.h"
#include "platform/messageBox.h"
#include "platform/window.h"
#include "win32_globals.h"

static int32 GetFlags_type(EMessageBoxType type)
{
	switch (type)
	{
		case MESSAGE_BOX_TYPE_INFO:		return MB_ICONINFORMATION;
		case MESSAGE_BOX_TYPE_WARNING:	return MB_ICONWARNING;
		case MESSAGE_BOX_TYPE_ERROR:	return MB_ICONERROR;
	}
	return 0;
}

static int32 GetFlags_options(EMessageBoxOptions options)
{
	switch (options)
	{
		case MESSAGE_BOX_OPTIONS_OK:			return MB_OK;
		case MESSAGE_BOX_OPTIONS_YESNO:			return MB_YESNO;
		case MESSAGE_BOX_OPTIONS_YESNOCANCEL:	return MB_YESNOCANCEL;
	}
	return 0;
}

static EMessageBoxResponse GetResponse(int32 code)
{
	switch (code)
	{
		case IDOK:		return MESSAGE_BOX_RESPONSE_OK;
		case IDYES:		return MESSAGE_BOX_RESPONSE_YES;
		case IDNO:		return MESSAGE_BOX_RESPONSE_NO;
	}
	return MESSAGE_BOX_RESPONSE_CANCEL;
}

EMessageBoxResponse SysMessageBox::Show(const std::string& caption, const std::string& msg, EMessageBoxType type, EMessageBoxOptions options)
{
	HWND wnd = Window::Get() ? (HWND)Window::Get()->GetNativeHandle() : nullptr;
	int32 flags = GetFlags_type(type) | GetFlags_options(options);
	int32 response = MessageBoxA(wnd, msg.c_str(), caption.c_str(), flags);
	return GetResponse(response);
}

#endif