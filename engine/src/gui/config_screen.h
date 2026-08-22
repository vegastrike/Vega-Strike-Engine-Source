// config_screen.h — in-game configuration screen for vs-settings-ng.
//
// Port of the vs-05 modern UI frames into the engine, drawn inside the in-game
// ImGui overlay (DrawConfigOverlay / Alt+C). Reads and writes the engine's
// Configuration object directly — no parallel config model.
//
// Temporary module; will become the engine's settings UI.
#ifndef VS_SETTINGS_NG_CONFIG_SCREEN_H
#define VS_SETTINGS_NG_CONFIG_SCREEN_H

#include <SDL3/SDL.h>

namespace vs_settings_ng {

// Draw the in-game config screen. Call inside an active ImGui frame (between
// NewFrame and Render). Draws nothing unless the config screen is active.
void DrawConfigScreen();

// Handle an SDL event (binding capture, joystick hotplug). Call from the event
// loop. No-op unless the config screen is active.
void HandleConfigEvent(const SDL_Event *event);

} // namespace vs_settings_ng

#endif
