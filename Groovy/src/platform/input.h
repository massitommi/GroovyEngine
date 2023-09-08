#pragma once

#include "core/coreminimal.h"

enum EKeyCode
{
	MouseLeft,
	MouseRight,

	Backspace,
	Tab,
	Return,
	Shift,
	Ctrl,
	Alt,
	CapsLock,
	Esc,
	Space,

	ArrowLeft,
	ArrowUp,
	ArrowRight,
	ArrowDown,

	LeftShift, RightShift,
	LeftCtrl, RightCtrl,
	LeftAlt, RightAlt,

	Alpha0, Alpha1, Alpha2, Alpha3,
	Alpha4, Alpha5, Alpha6, Alpha7,
	Alpha8, Alpha9,

	A, B, C, D, E, F, G, H, I, J,
	K, L, M, N, O, P, Q, R, S, T,
	U, V, W, X, Y, Z,

	F1, F2, F3, F4, F5, F6, F7, F8,
	F9, F10, F11, F12, F13, F14, F15,
	F16, F17, F18, F19, F20, F21, F22,
	F23, F24,

	KeyCodesEnd
};

struct MouseDelta
{
	float x, y;
};

class CORE_API Input
{
public:
	static void Init();
	static void Clear();
	static void Shutdown();

	// for internal use by the engine
	static void OnKeyDown(byte key);
	// for internal use by the engine
	static void OnKeyUp(byte key);
	// for internal use by the engine
	static void OnMouseMove(int32 x, int32 y);

#if WITH_EDITOR
	static void Editor_BlockInput(bool block);
	static bool Editor_IsInputBlocked();
#endif

	// Application API

	static bool IsKeyDown(EKeyCode key);
	static bool IsKeyPressed(EKeyCode key);
	static bool IsKeyReleased(EKeyCode key);
	static void GetRawMouseDelta(int32* xy);
	static MouseDelta GetMouseDelta();
};