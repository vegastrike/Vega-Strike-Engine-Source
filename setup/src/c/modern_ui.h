// modern_ui.h — the Modern mode of the VS-05 settings app.
//
// Self-contained module: owns the Modern UI state (display frame, flight
// control), drives the clean vs05cfg::Model, and generates vs-modern.config.
// Classic mode in setup.cpp is untouched.
//
// Ownership model (see plan-vs05-modern-mode-implementation.md):
//   - presets.xml  = the modern UI's preset structure (engine-owned)
//   - asset config = the base data seed (colors/binds/game vars)
//   - display/input= canonical engine vars the app writes
#ifndef VS05_MODERN_UI_H
#define VS05_MODERN_UI_H

#include <string>
#include <SDL3/SDL.h>

namespace vs05ui {

// Mode flag (persisted in the vs-modern.config switch / app_state).
enum Mode { MODE_CLASSIC = 0, MODE_MODERN = 1 };

// --- lifecycle ---

// Set the active asset + its data dir; load presets + seed the model.
// Call once at startup and whenever the asset changes. Returns false if there
// is no asset (nothing to configure).
bool init(const std::string &active_asset, const std::string &data_dir);

// Draw the Modern screen content (call inside the main ImGui window, after the
// header). Renders the display frame + flight control; the classic group table
// is NOT drawn here.
void draw();

// Handle an SDL event (capture dialog + joystick). Call from SDL_AppEvent.
void handle_event(const SDL_Event *event);

// --- persistence ---

// Rebuild vs-modern.config from the model + current UI state. Returns false on
// failure (e.g. no model). Writes the mode switch header.
bool save();

// Reset the modern config: reload the model from the asset config and reset the
// preset selections to the shipped defaults, marking the config dirty.
void reset();

// True if any Modern change is staged but unsaved.
bool has_unsaved();

// --- mode ---
Mode mode();
void set_mode(Mode m);

// --- launch helper: the config file the engine reads in this mode ---
std::string engine_config_file();

} // namespace vs05ui

#endif
