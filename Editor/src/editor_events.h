#pragma once

#include "core/core.h"

// macro utils

#define EVENT_NAME(CallbackSignature) editor_event_##CallbackSignature
#define CREATE_EVENT(EventType, Params) typedef void (*EventType)(Params)
#define DECLARE_EVENT(CallbackSignature) extern std::vector<CallbackSignature> EVENT_NAME(CallbackSignature)
#define IMPLEMENT_EVENT(CallbackSignature) std::vector<CallbackSignature> EVENT_NAME(CallbackSignature)

#define DISPATCH_EVENT(CallbackSignature, Params)		\
	{for (auto proc : EVENT_NAME(CallbackSignature))	\
	{													\
		proc(Params);									\
	}}													\

#define SUBSCRIBE_TO_EVENT(CallbackSignature, Callback) EVENT_NAME(CallbackSignature).push_back(Callback)

// event definitions
CREATE_EVENT(DropFilesEvent, const std::vector<String>&);

// event declarations
DECLARE_EVENT(DropFilesEvent);