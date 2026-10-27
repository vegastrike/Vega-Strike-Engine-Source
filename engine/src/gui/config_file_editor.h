/*
 * config_file_editor.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEGA_STRIKE_ENGINE_GUI_CONFIG_FILE_EDITOR_H
#define VEGA_STRIKE_ENGINE_GUI_CONFIG_FILE_EDITOR_H

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/json.hpp>

namespace vs_settings {
class JsonConfigModel;
class JsonConfigNode;
}

namespace vs_settings_ng {

/**
 * One JSON config file the settings screen can edit: the copy shipped in the data directory, with the
 * player's own overrides in the home directory on top of it.
 *
 * The screen shows one level of the file at a time, with a breadcrumb to walk back up. Edits are
 * staged here and written by the screen's Save, sparsely - only what differs from the shipped file -
 * so nothing takes effect until the next start.
 */
class ConfigFileEditor {
public:
    /// file_name is the file's name within the data and home config directories, e.g. "engine.json".
    ConfigFileEditor(std::string file_name, std::string title);
    ~ConfigFileEditor();

    /// Read the shipped copy and the player's overlay, discarding any staged edits. Called when the
    /// dialog is opened, so closing it without saving discards.
    void Load();

    /// True if any leaf differs from the shipped file.
    bool HasChanges() const;

    /// The sparse overlay this file would be written with: only the changed leaves.
    boost::json::value Changes() const;

    /// Draw the dialog's contents between BeginPopupModal and EndPopup. Returns true if the player
    /// edited something in this call, so the caller can mark itself dirty.
    bool Draw();

    const std::string &Title() const { return title_; }
    const std::string &FileName() const { return file_name_; }

private:
    void DrawBreadcrumb();
    bool DrawLevel();
    bool DrawLeaf(const std::string &name, vs_settings::JsonConfigNode *node);
    bool DrawColor(const std::string &name, vs_settings::JsonConfigNode *node);
    /// The Default column: whether this leaf is the shipped value, and the way back if it is not.
    bool DrawState(const std::string &path, vs_settings::JsonConfigNode *node);

    /// The buffer for a row, filled from the model the first time the row is drawn and left alone
    /// afterwards so typing is not fought by the model each frame.
    std::array<char, 96> &Buffer(const std::string &path);

    std::string file_name_;
    std::string title_;
    std::unique_ptr<vs_settings::JsonConfigModel> model_;
    /// The object being shown, as child names from the file's root.
    std::vector<std::string> here_;
    /// Row text, keyed by dotted path, kept between frames while typing.
    std::map<std::string, std::array<char, 96>> buffers_;
    std::array<char, 96> filter_{};
    bool changed_only_ = false;
};

} // namespace vs_settings_ng

#endif // VEGA_STRIKE_ENGINE_GUI_CONFIG_FILE_EDITOR_H
