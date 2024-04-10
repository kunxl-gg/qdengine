/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"

#include "qdengine/core/system/input/input_wndproc.h"
#include "qdengine/core/system/input/keyboard_input.h"
#include "qdengine/core/system/input/input_recorder.h"
#include "qdengine/core/system/input/mouse_input.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace input {

#if 0
bool keyboard_wndproc(const MSG &msg, keyboardDispatcher *dsp) {
	switch (msg.message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		dsp->handle_event((int)msg.wParam, true);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		dsp->handle_event((int)msg.wParam, false);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	}

	return false;
}
#endif

#if 0
bool mouse_wndproc(const MSG &msg, mouseDispatcher *dsp) {
	int x, y;
	switch (msg.message) {
	case WM_MOUSEMOVE:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_MOUSE_MOVE, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_LBUTTONDOWN:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_LEFT_DOWN, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_RBUTTONDOWN:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_RIGHT_DOWN, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_LBUTTONUP:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_LEFT_UP, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_RBUTTONUP:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_RIGHT_UP, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	}

	return false;
}
#endif

}; /* namespace input */
