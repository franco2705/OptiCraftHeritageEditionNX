#include "platform/Input.h"

#include "lwjgl/Display.h"
#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "switch/input/SwitchInput.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <switch.h>

namespace
{
PadState g_pad;
bool g_initialized = false;
u64 g_buttons = 0;
u64 g_pressed = 0;
PlatformGamepadSnapshot g_gamepad;
int g_cursorX = 640;
int g_cursorY = 360;

constexpr float kStickScale = 1.0f / 32768.0f;
constexpr float kPointerDeadzone = 0.18f;
constexpr float kPointerSpeed = 18.0f;

float axis(s32 value)
{
    return std::clamp(static_cast<float>(value) * kStickScale, -1.0f, 1.0f);
}

void initialize()
{
    if (g_initialized) return;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&g_pad);
    g_initialized = true;
}

std::uint32_t textActions(u64 buttons)
{
    std::uint32_t result = 0;
    if (buttons & HidNpadButton_Left)  result |= PLATFORM_TEXT_LEFT;
    if (buttons & HidNpadButton_Right) result |= PLATFORM_TEXT_RIGHT;
    if (buttons & HidNpadButton_Up)    result |= PLATFORM_TEXT_UP;
    if (buttons & HidNpadButton_Down)  result |= PLATFORM_TEXT_DOWN;
    if (buttons & HidNpadButton_A)     result |= PLATFORM_TEXT_TYPE;
    if (buttons & HidNpadButton_X)     result |= PLATFORM_TEXT_BACK;
    if (buttons & HidNpadButton_Minus) result |= PLATFORM_TEXT_SPACE;
    if (buttons & HidNpadButton_Y)     result |= PLATFORM_TEXT_SHIFT;
    if (buttons & HidNpadButton_Plus)  result |= PLATFORM_TEXT_ENTER;
    if (buttons & HidNpadButton_B)     result |= PLATFORM_TEXT_CLOSE;
    return result;
}

struct KeyBinding
{
    u64 button;
    int key;
};

constexpr KeyBinding kKeys[] = {
    {HidNpadButton_A, lwjgl::Keyboard::KEY_SPACE},
    {HidNpadButton_B, lwjgl::Keyboard::KEY_ESCAPE},
    {HidNpadButton_X, lwjgl::Keyboard::KEY_E},
    {HidNpadButton_Y, lwjgl::Keyboard::KEY_Q},
    {HidNpadButton_Plus, lwjgl::Keyboard::KEY_ESCAPE},
    {HidNpadButton_Minus, lwjgl::Keyboard::KEY_F3},
    {HidNpadButton_StickR, lwjgl::Keyboard::KEY_F5},
    {HidNpadButton_Up, lwjgl::Keyboard::KEY_UP},
    {HidNpadButton_Down, lwjgl::Keyboard::KEY_DOWN},
    {HidNpadButton_Left, lwjgl::Keyboard::KEY_LEFT},
    {HidNpadButton_Right, lwjgl::Keyboard::KEY_RIGHT},
};

void emitKeyChanges(u64 down, u64 up)
{
    if (platformTextInputExclusive()) return;
    for (const KeyBinding &binding : kKeys)
    {
        // A is the primary pointer click in menus and jump while the mouse is
        // grabbed for gameplay.
        if (binding.button == HidNpadButton_A && !lwjgl::Mouse::isGrabbed()) continue;
        if (down & binding.button) lwjgl::Keyboard::detail::pushKey(binding.key, true);
        if (up & binding.button) lwjgl::Keyboard::detail::pushKey(binding.key, false);
    }
}

void emitMouseButton(u64 down, u64 up, u64 mask, int button)
{
    if (down & mask) lwjgl::Mouse::detail::pushButton(button, true, g_cursorX, g_cursorY);
    if (up & mask) lwjgl::Mouse::detail::pushButton(button, false, g_cursorX, g_cursorY);
}
}

void switchInputPoll()
{
    initialize();
    padUpdate(&g_pad);

    g_buttons = padGetButtons(&g_pad);
    g_pressed = padGetButtonsDown(&g_pad);
    const u64 released = padGetButtonsUp(&g_pad);
    const HidAnalogStickState left = padGetStickPos(&g_pad, 0);
    const HidAnalogStickState right = padGetStickPos(&g_pad, 1);

    g_gamepad.connected = padIsConnected(&g_pad);
    g_gamepad.leftX = axis(left.x);
    g_gamepad.leftY = -axis(left.y);
    g_gamepad.rightX = axis(right.x);
    g_gamepad.rightY = -axis(right.y);

    emitKeyChanges(g_pressed, released);
    if (!lwjgl::Mouse::isGrabbed())
        emitMouseButton(g_pressed, released, HidNpadButton_A, 0);
    emitMouseButton(g_pressed, released, HidNpadButton_ZR, 0);
    emitMouseButton(g_pressed, released, HidNpadButton_ZL, 1);

    const float pointerX = std::abs(g_gamepad.rightX) >= kPointerDeadzone ? g_gamepad.rightX : 0.0f;
    const float pointerY = std::abs(g_gamepad.rightY) >= kPointerDeadzone ? g_gamepad.rightY : 0.0f;
    const int dx = static_cast<int>(std::lround(pointerX * kPointerSpeed));
    const int dy = static_cast<int>(std::lround(pointerY * kPointerSpeed));
    if (dx != 0 || dy != 0)
    {
        g_cursorX = std::clamp(g_cursorX + dx, 0, lwjgl::Display::getWidth() - 1);
        g_cursorY = std::clamp(g_cursorY + dy, 0, lwjgl::Display::getHeight() - 1);
        lwjgl::Mouse::detail::pushMotion(g_cursorX, g_cursorY, dx, dy);
    }
}

PlatformTextInputSnapshot platformTextInputSnapshot(int)
{
    initialize();
    PlatformTextInputSnapshot result;
    result.connected = padIsConnected(&g_pad);
    result.held = textActions(g_buttons);
    result.pressed = textActions(g_pressed);
    result.pointerValid = result.connected;
    result.pointerX = g_cursorX;
    result.pointerY = g_cursorY;
    result.pointerWidth = lwjgl::Display::getWidth();
    result.pointerHeight = lwjgl::Display::getHeight();
    return result;
}

PlatformGamepadSnapshot platformRawGamepadSnapshot(int)
{
    initialize();
    return g_gamepad;
}

PlatformGamepadSnapshot platformGamepadSnapshot(int port)
{
    return platformRawGamepadSnapshot(port);
}

int platformMenuPad() { return 0; }
bool platformMenuPointerActive() { return true; }
bool platformMenuCursorVisible() { return true; }

void platformSetMenuCursor(int x, int y)
{
    g_cursorX = std::clamp(x, 0, lwjgl::Display::getWidth() - 1);
    g_cursorY = std::clamp(y, 0, lwjgl::Display::getHeight() - 1);
    lwjgl::Mouse::setCursorPosition(g_cursorX, g_cursorY);
}

const PlatformKeyboardHints &platformKeyboardHints()
{
    static const PlatformKeyboardHints hints = {
        {"A:type  X:del  Y:shift  -:space  +:ok  B:close", nullptr, nullptr}, 1};
    return hints;
}

const char *platformInputDebugLine()
{
    return g_gamepad.connected ? "Joy-Con / Pro Controller connected" : "Controller disconnected";
}
