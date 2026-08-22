/*
 * json_config_model.cpp
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

#include "json_config_model.h"

#include <boost/json.hpp>

namespace vs_settings {

// Build a child node from a value (object -> branch, array -> array-branch,
// scalar -> leaf). `user` is the matching user value or null.
static std::shared_ptr<JsonConfigNode> make_node(const boost::json::value& assets,
                                                 const boost::json::value* user) {
    if (assets.is_object()) {
        const boost::json::object* u = (user != nullptr && user->is_object()) ? &user->as_object() : nullptr;
        return std::make_shared<JsonConfigBranch>(assets.as_object(), u);
    }
    if (assets.is_array()) {
        const boost::json::array* u = (user != nullptr && user->is_array()) ? &user->as_array() : nullptr;
        return std::make_shared<JsonConfigBranch>(assets.as_array(), u);
    }
    // Scalar leaf.
    if (user != nullptr && !user->is_object() && !user->is_array()) {
        return std::make_shared<JsonConfigLeaf>(assets, *user);
    }
    return std::make_shared<JsonConfigLeaf>(assets, assets);
}

JsonConfigBranch::JsonConfigBranch(const boost::json::object& assets,
                                   const boost::json::object* user) {
    for (const auto& kv : assets) {
        const std::string& key = kv.key();
        const boost::json::value& value = kv.value();
        const auto* user_node = (user != nullptr) ? user->if_contains(key) : nullptr;
        children.emplace_back(key, make_node(value, user_node));
    }
    // Any user keys not present in assets are user-defined leaves/branches (they
    // round-trip and stay dirty so they are written back).
    if (user != nullptr) {
        for (const auto& kv : *user) {
            const std::string& key = kv.key();
            const boost::json::value& value = kv.value();
            if (!assets.contains(key)) {
                auto node = make_node(value, nullptr);
                if (auto leaf = std::dynamic_pointer_cast<JsonConfigLeaf>(node)) {
                    leaf->mark_user_defined();
                } else if (auto branch = std::dynamic_pointer_cast<JsonConfigBranch>(node)) {
                    branch->mark_user_defined();
                }
                children.emplace_back(key, node);
            }
        }
    }
}

JsonConfigBranch::JsonConfigBranch(const boost::json::array& assets,
                                   const boost::json::array* user) {
    is_array_ = true;
    for (size_t i = 0; i < assets.size(); ++i) {
        std::string key = std::to_string(i);
        const boost::json::value* u = (user != nullptr && i < user->size()) ? &(*user)[i] : nullptr;
        children.emplace_back(key, make_node(assets[i], u));
    }
}

void JsonConfigBranch::mark_user_defined() {
    user_defined_ = true;
}

bool JsonConfigBranch::is_dirty() const {
    if (user_defined_) {
        return true;
    }
    for (const auto& kv : children) {
        const std::string& key = kv.first;
        const auto& child = kv.second;
        if (child->is_dirty()) {
            return true;
        }
    }
    return false;
}

boost::json::value JsonConfigBranch::merged_value() const {
    if (is_array_) {
        boost::json::array arr;
        // Children are indexed "0".."n-1" in order.
        for (size_t i = 0; i < children.size(); ++i) {
            arr.push_back(children[i].second->merged_value());
        }
        return arr;
    }
    boost::json::object obj;
    for (const auto& kv : children) {
        obj[kv.first] = kv.second->merged_value();
    }
    return obj;
}

boost::json::value JsonConfigBranch::user_value() const {
    return changes_dictionary();
}

boost::json::value JsonConfigBranch::changes_dictionary() const {
    // If this whole branch is user-defined (no asset counterpart), the changes
    // value is just its full merged value (object or array).
    if (user_defined_) {
        return merged_value();
    }
    if (is_array_) {
        // Emit only the dirty array elements, preserving index order.
        boost::json::array arr;
        for (size_t i = 0; i < children.size(); ++i) {
            const auto& child = children[i].second;
            if (child->is_dirty()) {
                if (auto branch = dynamic_cast<const JsonConfigBranch*>(child.get())) {
                    arr.push_back(branch->changes_dictionary());
                } else if (auto leaf = dynamic_cast<const JsonConfigLeaf*>(child.get())) {
                    arr.push_back(leaf->value());
                }
            }
        }
        return arr;
    }
    boost::json::object changes;
    for (const auto& kv : children) {
        const std::string& key = kv.first;
        const auto& child = kv.second;
        if (child->is_dirty()) {
            if (auto branch = dynamic_cast<const JsonConfigBranch*>(child.get())) {
                changes[key] = branch->changes_dictionary();
            } else {
                auto leaf = dynamic_cast<const JsonConfigLeaf*>(child.get());
                changes[key] = leaf ? leaf->value() : boost::json::value();
            }
        }
    }
    return changes;
}

const JsonConfigNode* JsonConfigBranch::get_node(const std::vector<std::string>& path) const {
    if (path.empty()) {
        return this;
    }
    for (const auto& kv : children) {
        const std::string& key = kv.first;
        const auto& child = kv.second;
        if (key == path[0]) {
            if (path.size() == 1) {
                return child.get();
            }
            if (auto branch = dynamic_cast<const JsonConfigBranch*>(child.get())) {
                return branch->get_node(std::vector<std::string>(path.begin() + 1, path.end()));
            }
            return nullptr;
        }
    }
    return nullptr;
}

JsonConfigNode* JsonConfigBranch::get_node(const std::vector<std::string>& path) {
    return const_cast<JsonConfigNode*>(
        static_cast<const JsonConfigBranch*>(this)->get_node(path));
}

void JsonConfigBranch::set(const std::vector<std::string>& path, const boost::json::value& v) {
    if (path.empty()) {
        return;
    }
    // Find or create the child for path[0].
    JsonConfigNode* child = nullptr;
    for (auto& kv : children) {
        const std::string& key = kv.first;
        JsonConfigNode* node = kv.second.get();
        if (key == path[0]) {
            child = node;
            break;
        }
    }
    if (path.size() == 1) {
        if (child == nullptr) {
            auto leaf = std::make_shared<JsonConfigLeaf>(v, v, true);
            leaf->mark_user_defined();
            children.emplace_back(path[0], leaf);
        } else {
            if (auto leaf = dynamic_cast<JsonConfigLeaf*>(child)) {
                leaf->set(v);
            }
            // If path[0] exists but is a branch and we're setting a scalar, replace it.
            else if (auto branch = dynamic_cast<JsonConfigBranch*>(child)) {
                // Keep the branch only if v is an object; otherwise replace with leaf.
                (void)branch;
                if (!v.is_object()) {
                    for (auto it = children.begin(); it != children.end(); ++it) {
                        if (it->first == path[0]) {
                            children.erase(it);
                            break;
                        }
                    }
                    auto leaf = std::make_shared<JsonConfigLeaf>(v, v, true);
                    leaf->mark_user_defined();
                    children.emplace_back(path[0], leaf);
                }
            }
        }
        return;
    }
    // path.size() > 1: descend into/ create an intermediate branch.
    if (child == nullptr) {
        boost::json::object empty_obj;
        auto new_branch = std::make_shared<JsonConfigBranch>(empty_obj, nullptr);
        children.emplace_back(path[0], new_branch);
        new_branch->set(std::vector<std::string>(path.begin() + 1, path.end()), v);
    } else if (auto branch = dynamic_cast<JsonConfigBranch*>(child)) {
        branch->set(std::vector<std::string>(path.begin() + 1, path.end()), v);
    } else {
        // path[0] is a leaf but we need a branch: replace it.
        for (auto it = children.begin(); it != children.end(); ++it) {
            if (it->first == path[0]) {
                children.erase(it);
                break;
            }
        }
        boost::json::object empty_obj;
        auto new_branch = std::make_shared<JsonConfigBranch>(empty_obj, nullptr);
        children.emplace_back(path[0], new_branch);
        new_branch->set(std::vector<std::string>(path.begin() + 1, path.end()), v);
    }
}

JsonConfigModel::JsonConfigModel(const boost::json::value& assets_json,
                                 const boost::json::value* user_json) {
    const boost::json::object* user_obj =
        (user_json != nullptr && user_json->is_object()) ? &user_json->as_object() : nullptr;
    root_ = std::make_shared<JsonConfigBranch>(assets_json.as_object(), user_obj);
}

const JsonConfigNode* JsonConfigModel::get(const std::vector<std::string>& path) const {
    if (auto branch = dynamic_cast<const JsonConfigBranch*>(root_.get())) {
        return branch->get_node(path);
    }
    return nullptr;
}

void JsonConfigModel::set(const std::vector<std::string>& path, const boost::json::value& v) {
    if (auto branch = dynamic_cast<JsonConfigBranch*>(root_.get())) {
        branch->set(path, v);
    }
}

boost::json::value JsonConfigModel::changes_dictionary() const {
    if (auto branch = dynamic_cast<const JsonConfigBranch*>(root_.get())) {
        return branch->changes_dictionary();
    }
    return boost::json::object();
}

} // namespace vs_settings
