#pragma once

#include "core/core.h"

enum EMessageBoxType
{
	MESSAGE_BOX_TYPE_INFO,
	MESSAGE_BOX_TYPE_WARNING,
	MESSAGE_BOX_TYPE_ERROR
};

enum EMessageBoxOptions
{
	MESSAGE_BOX_OPTIONS_OK,
	MESSAGE_BOX_OPTIONS_YESNO,
	MESSAGE_BOX_OPTIONS_YESNOCANCEL
};

enum EMessageBoxResponse
{
	MESSAGE_BOX_RESPONSE_OK,
	MESSAGE_BOX_RESPONSE_YES,
	MESSAGE_BOX_RESPONSE_NO,
	MESSAGE_BOX_RESPONSE_CANCEL
};

class SysMessageBox
{
public:
	static EMessageBoxResponse Show(const std::string& caption, const std::string& msg, EMessageBoxType type, EMessageBoxOptions options);
	inline static EMessageBoxResponse Show_Info(const std::string& caption, const std::string msg) { return Show(caption, msg, MESSAGE_BOX_TYPE_INFO, MESSAGE_BOX_OPTIONS_OK); }
	inline static EMessageBoxResponse Show_Warning(const std::string& caption, const std::string msg) { return Show(caption, msg, MESSAGE_BOX_TYPE_WARNING, MESSAGE_BOX_OPTIONS_OK); }
	inline static EMessageBoxResponse Show_Error(const std::string& caption, const std::string msg) { return Show(caption, msg, MESSAGE_BOX_TYPE_ERROR, MESSAGE_BOX_OPTIONS_OK); }
};