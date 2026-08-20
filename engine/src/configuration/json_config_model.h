/*
 * json_config_model.h
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

#ifndef VEGA_STRIKE_ENGINE_CONFIG_JSON_CONFIG_MODEL_H
#define VEGA_STRIKE_ENGINE_CONFIG_JSON_CONFIG_MODEL_H

// Standalone overlay config model for the vs-settings-ng config utility.
//
// Ported from the kivy settings app's ConfigBranch/ConfigLeaf dirty-tracking
// model (Assets-Production/python/settings_app/game_config.py). It merges an
// assets config (read-only defaults) with a user config (sparse overrides) and
// tracks which leaves the user has changed ("dirty"), so the app can write only
// the changed subset back to the user directory.
//
// This has no SDL/ImGui dependency, so it is unit-testable and reusable by both
// the standalone vs-settings-ng binary and the future in-game settings panel.

#include <boost/json.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace vs_settings {

// A leaf (scalar) value that may have been overridden by the user.
// A node is either a Branch (dict) or a Leaf (scalar).
class JsonConfigNode;

class JsonConfigNode {
public:
    virtual ~JsonConfigNode() = default;

    // Dirty iff this node or any descendant has a user override.
    virtual bool is_dirty() const = 0;
    // The merged value as a boost::json::value.
    virtual boost::json::value merged_value() const = 0;
    // The user-override portion (sparse), or null if clean.
    virtual boost::json::value user_value() const = 0;
};

class JsonConfigLeaf : public JsonConfigNode {
public:
    JsonConfigLeaf(const boost::json::value& original, const boost::json::value& current,
                   bool forced_dirty = false)
        : original_(original), current_(current), dirty_(forced_dirty) {
        if (!forced_dirty) dirty_ = (current_ != original_);
    }

    bool is_dirty() const override {
        return dirty_;
    }

    void set(const boost::json::value& v) {
        current_ = v;
        // If there is a real original (from assets) and the value returns to it, the leaf
        // is clean again. If this leaf had no asset counterpart (user-defined), keep it dirty.
        if (has_original_) dirty_ = (v != original_);
        else dirty_ = true;
    }

    void mark_user_defined() { has_original_ = false; dirty_ = true; }

    boost::json::value value() const { return current_; }
    boost::json::value original_value() const { return original_; }

    boost::json::value merged_value() const override {
        return current_;
    }

    boost::json::value user_value() const override {
        return is_dirty() ? current_ : boost::json::value(nullptr);
    }

private:
    boost::json::value original_;
    boost::json::value current_;
    bool dirty_ = false;
    bool has_original_ = true;   // true if there is an assets value to revert to
};

// A dictionary node. Children are JsonConfigLeaf or JsonConfigBranch.
class JsonConfigBranch : public JsonConfigNode {
public:
    // Build a branch from an assets dict and (optional) user dict. Children that
    // appear only in assets are clean leaves; children present in user too get
    // the user's value (marked dirty if it differs).
    JsonConfigBranch(const boost::json::object& assets,
                     const boost::json::object* user);

    // Build an array node. Children are indexed "0".."n-1".
    JsonConfigBranch(const boost::json::array& assets,
                     const boost::json::array* user);

    // Mark this branch and all descendants as user-defined (no asset counterpart).
    // Used for whole subtrees present only in the user config, which must always
    // be written back.
    void mark_user_defined();

    bool is_dirty() const override;
    boost::json::value merged_value() const override;
    boost::json::value user_value() const override;

    // Get the node at a dotted path like "graphics.resolution_x". Returns null if absent.
    const JsonConfigNode* get_node(const std::vector<std::string>& path) const;
    JsonConfigNode* get_node(const std::vector<std::string>& path);

    // Set a leaf value at a dotted path. Creates intermediate branches as needed
    // (so a user can set a key the assets config doesn't define).
    void set(const std::vector<std::string>& path, const boost::json::value& v);

    // Sparse overlay containing only the dirty leaves (the file to write).
    // Returns an object (normal) or array (for array nodes).
    boost::json::value changes_dictionary() const;

    std::vector<std::pair<std::string, std::shared_ptr<JsonConfigNode>>> children;
    bool user_defined_ = false;
    bool is_array_ = false;   // true if this node represents a JSON array (children indexed 0..n)
};

// Top-level config: an assets file (defaults) + a user file (overrides).
class JsonConfigModel {
public:
    // assets_json and user_json are JSON object documents. user_json may be null
    // (no user overrides yet).
    JsonConfigModel(const boost::json::value& assets_json,
                    const boost::json::value* user_json);

    const JsonConfigNode* get(const std::vector<std::string>& path) const;
    void set(const std::vector<std::string>& path, const boost::json::value& v);

    // The sparse overlay to write to the user config file.
    boost::json::value changes_dictionary() const;

private:
    std::shared_ptr<JsonConfigNode> root_;
};

} // namespace vs_settings

#endif // VEGA_STRIKE_ENGINE_CONFIG_JSON_CONFIG_MODEL_H
