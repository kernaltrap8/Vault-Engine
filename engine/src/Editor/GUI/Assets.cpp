#include <Editor/GUI/MainGUI.hpp>
#include <Engine/Scene.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/IncludeComponents.hpp>
#include <icons/icons.h>
#include <imgui/imgui.h>
#include <filesystem>
#include <iostream>
#include <Engine/SceneSerialization.hpp>

using namespace Engine;
using namespace Engine::Components;
namespace fs = std::filesystem;

void DirectoryIterator(const std::string &str, const char *filter_str) {
    auto iter = std::filesystem::directory_iterator(str);

    std::string filterStr = filter_str;
    std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), Editor::asciitolower);

    for (auto &dir : iter) {

        if (dir.is_directory()) {
            if (ImGui::TreeNode((std::string(ICON_FA_FOLDER) + " " + dir.path().filename().string()).c_str())) {
                DirectoryIterator(dir.path().string(), filter_str);
                ImGui::TreePop();
            }
        } else {
            std::string dir_filename = dir.path().filename().string();
            std::transform(dir_filename.begin(), dir_filename.end(), dir_filename.begin(), Editor::asciitolower);
            if (dir_filename.find(filterStr) == std::string::npos && filterStr != "")
                continue;
            std::string icon = ICON_FA_CUBE;
            std::string name = dir.path().filename();

            if (name.ends_with(".ttf")) {
                icon = ICON_FA_FONT;
            }
            if (name.ends_with(".png") || name.ends_with(".tiff") || name.ends_with(".jpg") || name.ends_with(".jpeg")) {
                icon = ICON_FA_IMAGE;
            }
            if (name.ends_with(".obj")) {
                icon = ICON_FA_CUBES;
            }
            if (name.ends_with(".mtl") || name.ends_with(".material")) {
                icon = ICON_FA_PAINT_ROLLER;
            }

            bool pressed = ImGui::Selectable((icon + " " + dir.path().filename().string()).c_str());
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                Editor::GUI::dragPayload = fs::absolute(dir.path()).string();
                ImGui::SetDragDropPayload("material", dir.path().string().c_str(), dir.path().string().length() + 1);
                ImGui::Text("%s %s", icon.c_str(), dir.path().filename().string().c_str());
                ImGui::EndDragDropSource();
            }

            if ((name.ends_with(".mtl") || name.ends_with(".material")) && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))) {
                Editor::GUI::MaterialInspector_material_path = dir.path().relative_path();
                Editor::GUI::isMaterialInspectorOpen = true;

                Serializer::DeserializeMaterial(dir.path().relative_path(), Editor::GUI::MaterialInspector_material);
            }
        }
    }
}

namespace Editor {
    void GUI::Assets() {
        constexpr int filter_str_size = 256;
        static char filter_str[filter_str_size];

        ImGui::Begin(ICON_FA_FOLDER " Assets");
        ImGui::PushItemWidth(ImGui::GetWindowSize().x - 20);
        ImGui::InputText("##AssetsFilterString", filter_str, filter_str_size);
        std::string icon = ICON_FA_CUBE;

        DirectoryIterator("../assets", filter_str);

        ImGui::End();
    }
} // namespace Editor