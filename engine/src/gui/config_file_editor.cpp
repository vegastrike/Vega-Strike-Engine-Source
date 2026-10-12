/*
 * config_file_editor.cpp
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

#include "config_file_editor.h"

#include "configuration/json_config_model.h"
#include "vegadisk/vsfilesystem.h"
#include "vega_cast_utils.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <imgui.h>

namespace {

/// Read a JSON file as an object, or null when it is missing or unparseable.
boost::json::value ReadJsonFile(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        return boost::json::value(nullptr);
    }
    std::stringstream text;
    text << in.rdbuf();
    try {
        boost::json::value parsed = boost::json::parse(text.str());
        if (parsed.is_object()) {
            return parsed;
        }
    } catch (const std::exception &) {
        // Unreadable or not an object: treated as absent.
    }
    return boost::json::value(nullptr);
}

/// The four numbers of a colour as floats, or false if this node is not one.
bool ColorComponents(const vs_settings::JsonConfigNode *node, float out[4]) {
    const auto *branch = dynamic_cast<const vs_settings::JsonConfigBranch *>(node);
    if (branch == nullptr || !branch->is_array_ || branch->children.size() != 4) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        const auto *leaf = dynamic_cast<const vs_settings::JsonConfigLeaf *>(branch->children[i].second.get());
        if (leaf == nullptr || !leaf->value().is_double()) {
            return false;
        }
        out[i] = static_cast<float>(leaf->value().as_double());
    }
    return true;
}

} // anonymous namespace

namespace vs_settings_ng {

ConfigFileEditor::ConfigFileEditor(std::string file_name, std::string title)
        : file_name_(std::move(file_name)), title_(std::move(title)) {
}

ConfigFileEditor::~ConfigFileEditor() = default;

std::array<char, 96> &ConfigFileEditor::Buffer(const std::string &path) {
    auto it = buffers_.find(path);
    if (it != buffers_.end()) {
        return it->second;
    }
    std::array<char, 96> fresh{};
    fresh[0] = '\0';
    return buffers_.emplace(path, fresh).first->second;
}

void ConfigFileEditor::Load() {
    // The screen's own settings are the player's file; this is the shipped file plus their overlay.
    boost::json::value assets = ReadJsonFile(VSFileSystem::datadir + "/" + file_name_);
    if (!assets.is_object()) {
        model_.reset();
        here_.clear();
        buffers_.clear();
        return;
    }
    boost::json::value user = ReadJsonFile(VSFileSystem::homedir + "/" + file_name_);
    model_ = std::make_unique<vs_settings::JsonConfigModel>(assets, user.is_object() ? &user : nullptr);
    here_.clear();
    buffers_.clear();
    filter_.fill('\0');
    changed_only_ = false;
}

bool ConfigFileEditor::HasChanges() const {
    if (model_ == nullptr) {
        return false;
    }
    const vs_settings::JsonConfigNode *root = model_->get(std::vector<std::string>());
    return root != nullptr && root->is_dirty();
}

boost::json::value ConfigFileEditor::Changes() const {
    return model_ == nullptr ? boost::json::value(nullptr) : model_->changes_dictionary();
}

void ConfigFileEditor::DrawBreadcrumb() {
    if (ImGui::SmallButton(title_.c_str())) {
        here_.clear();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", file_name_.c_str());
    }
    for (size_t i = 0; i < here_.size(); ++i) {
        ImGui::SameLine();
        ImGui::TextUnformatted(">");
        ImGui::SameLine();
        if (i + 1 == here_.size()) {
            ImGui::TextUnformatted(here_[i].c_str());
        } else {
            const std::string label = here_[i] + "###crumb" + std::to_string(i);
            if (ImGui::SmallButton(label.c_str())) {
                here_.resize(i + 1);
            }
        }
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(values are read at the next start)");
}

bool ConfigFileEditor::DrawLeaf(const std::string &name, vs_settings::JsonConfigNode *node) {
    auto *leaf = dynamic_cast<vs_settings::JsonConfigLeaf *>(node);
    if (leaf == nullptr) {
        ImGui::TextDisabled("(not a value)");
        return false;
    }

    std::string path;
    for (const std::string &part : here_) {
        path += part;
        path += ".";
    }
    path += name;

    bool changed = false;
    const boost::json::value &value = leaf->value();
    std::array<char, 96> &buf = Buffer(path);
    ImGui::PushID(path.c_str());

    if (value.is_bool()) {
        bool v = value.as_bool();
        if (ImGui::Checkbox("##v", &v)) {
            leaf->set(v);
            changed = true;
        }
    } else if (value.is_int64()) {
        if (buf[0] == '\0') {
            snprintf(buf.data(), buf.size(), "%lld", static_cast<long long>(value.as_int64()));
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##v", buf.data(), buf.size(), ImGuiInputTextFlags_CharsDecimal)) {
            leaf->set(static_cast<std::int64_t>(strtoll(buf.data(), nullptr, 10)));
            changed = true;
        }
    } else if (value.is_double()) {
        if (buf[0] == '\0') {
            snprintf(buf.data(), buf.size(), "%g", value.as_double());
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##v", buf.data(), buf.size(), ImGuiInputTextFlags_CharsDecimal)) {
            leaf->set(locale_aware_stod(std::string(buf.data())));
            changed = true;
        }
    } else if (value.is_string()) {
        if (buf[0] == '\0') {
            snprintf(buf.data(), buf.size(), "%s", value.as_string().c_str());
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##v", buf.data(), buf.size())) {
            leaf->set(boost::json::value(std::string(buf.data())));
            changed = true;
        }
    } else {
        ImGui::TextDisabled("%s", boost::json::serialize(value).c_str());
    }

    if (leaf->is_dirty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "*");
    }
    ImGui::PopID();
    return changed;
}

bool ConfigFileEditor::DrawColor(const std::string &name, vs_settings::JsonConfigNode *node) {
    auto *branch = dynamic_cast<vs_settings::JsonConfigBranch *>(node);
    float rgba[4];
    if (branch == nullptr || !ColorComponents(node, rgba)) {
        return false;
    }
    ImGui::PushID(name.c_str());
    ImGui::SetNextItemWidth(-1);
    bool changed = false;
    if (ImGui::ColorEdit4("##c", rgba)) {
        for (size_t i = 0; i < branch->children.size(); ++i) {
            auto *leaf = dynamic_cast<vs_settings::JsonConfigLeaf *>(branch->children[i].second.get());
            if (leaf != nullptr) {
                leaf->set(static_cast<double>(rgba[i]));
            }
        }
        changed = true;
    }
    ImGui::PopID();
    return changed;
}

bool ConfigFileEditor::DrawLevel() {
    vs_settings::JsonConfigNode *node = model_->get(here_);
    auto *branch = dynamic_cast<vs_settings::JsonConfigBranch *>(node);
    if (branch == nullptr) {
        ImGui::TextUnformatted("Nothing here.");
        return false;
    }

    bool changed = false;
    if (ImGui::BeginTable("##rows", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.40f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.44f);
        ImGui::TableSetupColumn("Default", ImGuiTableColumnFlags_WidthStretch, 0.16f);

        for (auto &child : branch->children) {
            const std::string &name = child.first;
            vs_settings::JsonConfigNode *child_node = child.second.get();

            if (filter_[0] != '\0' && name.find(filter_.data()) == std::string::npos) {
                continue;
            }
            if (changed_only_ && !child_node->is_dirty()) {
                continue;
            }

            std::string path;
            for (const std::string &part : here_) {
                path += part;
                path += ".";
            }
            path += name;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (auto *child_branch = dynamic_cast<vs_settings::JsonConfigBranch *>(child_node)) {
                float rgba[4];
                if (child_branch->is_array_ && ColorComponents(child_node, rgba)) {
                    ImGui::TextUnformatted(name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    changed |= DrawColor(name, child_node);
                } else if (child_branch->is_array_) {
                    ImGui::TextUnformatted(name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextDisabled("(%zu values)", child_branch->children.size());
                } else {
                    if (ImGui::Selectable(name.c_str(), false, 0, ImVec2(0, 0))) {
                        here_.push_back(name);
                    }
                }
            } else {
                ImGui::TextUnformatted(name.c_str());
                ImGui::TableSetColumnIndex(1);
                changed |= DrawLeaf(name, child_node);
            }
            ImGui::TableSetColumnIndex(2);
            changed |= DrawState(path, child_node);
        }
        ImGui::EndTable();
    }
    return changed;
}

bool ConfigFileEditor::DrawState(const std::string &path, vs_settings::JsonConfigNode *node) {
    if (!node->is_dirty()) {
        ImGui::TextDisabled("default");
        return false;
    }

    auto *leaf = dynamic_cast<vs_settings::JsonConfigLeaf *>(node);
    if (leaf == nullptr) {
        // A branch: something under it has been changed, which is what opening it will show.
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "changed");
        return false;
    }
    if (!leaf->has_original()) {
        // Added by the player: there is no shipped value to go back to.
        ImGui::TextDisabled("added");
        return false;
    }
    if (ImGui::SmallButton("Reset")) {
        leaf->set(leaf->original_value());
        buffers_.erase(path);   // the field shows the model's value again next frame
        return true;
    }
    return false;
}

bool ConfigFileEditor::Draw() {
    if (model_ == nullptr) {
        ImGui::TextUnformatted("This file could not be loaded.");
        return false;
    }

    DrawBreadcrumb();
    ImGui::Separator();

    ImGui::SetNextItemWidth(180);
    ImGui::InputTextWithHint("##filter", "filter", filter_.data(), filter_.size());
    ImGui::SameLine();
    ImGui::Checkbox("Changed only", &changed_only_);
    ImGui::Separator();

    bool changed = false;
    ImGui::BeginChild("##level", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar);
    changed = DrawLevel();
    ImGui::EndChild();

    if (HasChanges()) {
        ImGui::TextDisabled("These are written when you press Save on the settings screen.");
    }
    return changed;
}

} // namespace vs_settings_ng
