#if PLATFORM_WIN32

#include "win32_globals.h"
#include "platform/input.h"
#include "core/core.h"

static byte sPlatformKeyCodes[EKeyCode::KeyCodesEnd];
static byte sKeyboardState[256];
static byte sPrevKeyboardState[256];
static int32 sMouseDelta[2];

void Input::Init()
{
	sPlatformKeyCodes[EKeyCode::MouseLeft] = VK_LBUTTON;
	sPlatformKeyCodes[EKeyCode::MouseRight] = VK_RBUTTON;
	
	sPlatformKeyCodes[EKeyCode::Backspace] = VK_BACK;
	sPlatformKeyCodes[EKeyCode::Tab] = VK_TAB;
	sPlatformKeyCodes[EKeyCode::Return] = VK_RETURN;
	sPlatformKeyCodes[EKeyCode::Shift] = VK_SHIFT;
	sPlatformKeyCodes[EKeyCode::Ctrl] = VK_CONTROL;
	sPlatformKeyCodes[EKeyCode::Alt] = VK_MENU;
	sPlatformKeyCodes[EKeyCode::CapsLock] = VK_CAPITAL;
	sPlatformKeyCodes[EKeyCode::Esc] = VK_ESCAPE;
	sPlatformKeyCodes[EKeyCode::Space] = VK_SPACE;

	sPlatformKeyCodes[EKeyCode::ArrowLeft] = VK_LEFT;
	sPlatformKeyCodes[EKeyCode::ArrowUp] = VK_UP;
	sPlatformKeyCodes[EKeyCode::ArrowRight] = VK_RIGHT;
	sPlatformKeyCodes[EKeyCode::ArrowDown] = VK_DOWN;

	sPlatformKeyCodes[EKeyCode::LeftShift] = VK_LSHIFT;
	sPlatformKeyCodes[EKeyCode::RightShift] = VK_RSHIFT;
	sPlatformKeyCodes[EKeyCode::LeftCtrl] = VK_LCONTROL;
	sPlatformKeyCodes[EKeyCode::RightCtrl] = VK_RCONTROL;
	sPlatformKeyCodes[EKeyCode::LeftAlt] = VK_LMENU;
	sPlatformKeyCodes[EKeyCode::RightAlt] = VK_RMENU;

	// numbers
	for (uint32 i = 0; i < EKeyCode::Alpha9 - EKeyCode::Alpha0 + 1; i++)
		sPlatformKeyCodes[EKeyCode::Alpha0 + i] = 0x30 + i;

	// letters
	for (uint32 i = 0; i < EKeyCode::Z - EKeyCode::A + 1; i++)
		sPlatformKeyCodes[EKeyCode::A + i] = 0x41 + i;

	// function keys
	for (uint32 i = 0; i < EKeyCode::F24 - EKeyCode::F1 + 1; i++)
		sPlatformKeyCodes[EKeyCode::F1 + i] = 0x70 + i;


	memset(sKeyboardState, 0, 256);
	memset(sPrevKeyboardState, 0, 256);
	sMouseDelta[0] = sMouseDelta[1] = 0;
}

void Input::OnMouseMove(int32 x, int32 y)
{
	sMouseDelta[0] += x;
	sMouseDelta[1] += y;
}

void Input::GetRawMouseDelta(int32* xy)
{
	xy[0] = sMouseDelta[0];
	xy[1] = sMouseDelta[1];
}

void Input::Clear()
{
	// clear mouse data
	sMouseDelta[0] = sMouseDelta[1] = 0;

	// set new data
	memcpy(sPrevKeyboardState, sKeyboardState, 256);
}

void Input::OnKeyDown(byte key)
{
	sKeyboardState[key] = 1;
}

void Input::OnKeyUp(byte key)
{
	sKeyboardState[key] = 0;
}

void Input::Shutdown()
{
}

bool Input::IsKeyDown(EKeyCode key)
{
	return sKeyboardState[sPlatformKeyCodes[key]];
}

bool Input::IsKeyPressed(EKeyCode key)
{
	bool currentlyPressed = sKeyboardState[sPlatformKeyCodes[key]];
	bool wasntPressedLastFrame = !sPrevKeyboardState[sPlatformKeyCodes[key]];
	return currentlyPressed && wasntPressedLastFrame;
}

bool Input::IsKeyReleased(EKeyCode key)
{
	bool wasPressedLastFrame = sPrevKeyboardState[sPlatformKeyCodes[key]];
	bool currentlyNotPressed = !sKeyboardState[sPlatformKeyCodes[key]];
	return wasPressedLastFrame && currentlyNotPressed;
}

#endif