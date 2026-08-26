/*
 * json_config_model_tests.cpp
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

#include <gtest/gtest.h>
#include "configuration/json_config_model.h"

#include <boost/json.hpp>

namespace vs_settings {

using boost::json::parse;
using boost::json::value;
using std::vector;
using std::string;

// A small model with the shape of config.json: nested sections + scalars.
static JsonConfigModel make_model(const value& assets, const value& user = value(nullptr)) {
    return JsonConfigModel(assets, user.is_null() ? nullptr : &user);
}

TEST(JsonConfigModel, MergeUserOverride) {
    auto assets = parse(R"({ "graphics": { "resolution_x": 1024, "resolution_y": 768, "fullscreen": false } })");
    auto user = parse(R"({ "graphics": { "resolution_x": 1920 } })");
    JsonConfigModel m = make_model(assets, user);

    // Overridden leaf reflects the user value.
    const JsonConfigNode* rx = m.get({"graphics", "resolution_x"});
    ASSERT_NE(rx, nullptr);
    EXPECT_TRUE(rx->is_dirty());
    EXPECT_EQ(rx->merged_value(), value(1920));

    // Non-overridden leaf stays at asset value and is clean.
    const JsonConfigNode* fy = m.get({"graphics", "resolution_y"});
    ASSERT_NE(fy, nullptr);
    EXPECT_FALSE(fy->is_dirty());
    EXPECT_EQ(fy->merged_value(), value(768));
}

TEST(JsonConfigModel, ChangesDictionaryIsSparse) {
    auto assets = parse(R"({ "graphics": { "resolution_x": 1024, "fullscreen": false }, "audio": { "volume": 1.0 } })");
    auto user = parse(R"({ "graphics": { "resolution_x": 1920 } })");
    JsonConfigModel m = make_model(assets, user);

    boost::json::object changes = m.changes_dictionary().as_object();
    ASSERT_EQ(changes.size(), 1u);  // only the graphics section is dirty
    ASSERT_TRUE(changes.contains("graphics"));
    auto& g = changes["graphics"].as_object();
    ASSERT_EQ(g.size(), 1u);  // only resolution_x, not fullscreen
    EXPECT_EQ(g["resolution_x"], value(1920));
}

TEST(JsonConfigModel, SetMarksDirty) {
    JsonConfigModel m = make_model(parse(R"({ "graphics": { "fullscreen": false } })"));
    m.set({"graphics", "fullscreen"}, value(true));
    EXPECT_TRUE(m.get({"graphics", "fullscreen"})->is_dirty());
    EXPECT_EQ(m.get({"graphics", "fullscreen"})->merged_value(), value(true));

    auto changes = m.changes_dictionary().as_object();
    EXPECT_EQ(changes["graphics"].as_object()["fullscreen"], value(true));
}

TEST(JsonConfigModel, SetBackToOriginalIsClean) {
    JsonConfigModel m = make_model(parse(R"({ "graphics": { "fullscreen": false } })"));
    m.set({"graphics", "fullscreen"}, value(true));
    m.set({"graphics", "fullscreen"}, value(false));
    EXPECT_FALSE(m.get({"graphics", "fullscreen"})->is_dirty());
    EXPECT_TRUE(m.changes_dictionary().as_object().empty());
}

TEST(JsonConfigModel, SetCreatesNewNestedBranch) {
    JsonConfigModel m = make_model(parse(R"({ "graphics": {} })"));
    m.set({"graphics", "resolution_x"}, value(2560));
    EXPECT_EQ(m.get({"graphics", "resolution_x"})->merged_value(), value(2560));
    auto changes = m.changes_dictionary().as_object();
    EXPECT_EQ(changes["graphics"].as_object()["resolution_x"], value(2560));
}

TEST(JsonConfigModel, NoUserConfigIsAllClean) {
    JsonConfigModel m = make_model(parse(R"({ "a": { "b": 1, "c": 2 } })"));
    EXPECT_FALSE(m.get({"a", "b"})->is_dirty());
    EXPECT_TRUE(m.changes_dictionary().as_object().empty());
}

TEST(JsonConfigModel, MissingPathReturnsNull) {
    JsonConfigModel m = make_model(parse(R"({ "graphics": {} })"));
    EXPECT_EQ(m.get({"does", "not", "exist"}), nullptr);
}

TEST(JsonConfigModel, UserOnlyKeysRoundTrip) {
    // A key present only in the user config (not assets) is preserved.
    auto assets = parse(R"({ "graphics": {} })");
    auto user = parse(R"({ "input": { "device": "joystick" } })");
    JsonConfigModel m = make_model(assets, user);
    const JsonConfigNode* dev = m.get({"input", "device"});
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->merged_value(), value("joystick"));
    auto changes = m.changes_dictionary().as_object();
    EXPECT_EQ(changes["input"].as_object()["device"], value("joystick"));
}

TEST(JsonConfigModel, BindingsActionOverlay) {
    // A rebind is a nested action change that must land in the bindings overlay.
    // bindings.json shape: { "actions": { "ABKey": { "keyboard": [ { key, modifier } ] } } }
    auto assets = parse(R"({ "actions": {
        "FireKey": { "keyboard": [ { "key": "space", "modifier": "none" } ] }
    } })");
    auto user = parse(R"({ "actions": {
        "FireKey": { "keyboard": [ { "key": "x", "modifier": "none" } ] }
    } })");
    JsonConfigModel m = make_model(assets, user);

    // The dirty action is the only thing in the changes overlay.
    auto changes = m.changes_dictionary().as_object();
    ASSERT_TRUE(changes.contains("actions"));
    auto& act = changes["actions"].as_object();
    ASSERT_EQ(act.size(), 1u);
    ASSERT_TRUE(act.contains("FireKey"));
    EXPECT_EQ(act["FireKey"].as_object()["keyboard"].as_array()[0].as_object()["key"], value("x"));
}

TEST(JsonConfigModel, BindingsKeyboardArrayOverlayIsArray) {
    // The engine's parseActions requires keyboard/mouse/joystick to be arrays.
    // A rebind through a numeric path must emit a JSON array, not an object
    // keyed by "0".
    auto assets = parse(R"({ "actions": {
        "FireKey": { "keyboard": [ { "key": "space", "modifier": "none" } ] }
    } })");
    JsonConfigModel m(assets, nullptr);
    m.set({"actions", "FireKey", "keyboard", "0", "key"}, value("x"));

    auto changes = m.changes_dictionary().as_object();
    auto& act = changes["actions"].as_object();
    auto& keyboard = act["FireKey"].as_object()["keyboard"];
    EXPECT_TRUE(keyboard.is_array()) << "keyboard overlay must be a JSON array";
    ASSERT_EQ(keyboard.as_array().size(), 1u);
    EXPECT_EQ(keyboard.as_array()[0].as_object()["key"], value("x"));
}

} // namespace vs_settings
