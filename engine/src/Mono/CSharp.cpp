#include "Editor/GUI/MainGUI.hpp"
#include "Engine/GameObject.hpp"
#include "Engine/Mono/Format/Functions.hpp"
#include "Engine/Mono/HelperFunctions.hpp"
#include "Engine/Mono/Text3D/Functions.hpp"
#include "Engine/Mono/Time/Functions.hpp"
#include "Engine/Mono/Transform/Functions.hpp"
#include "Engine/Mono/GameObject/Functions.hpp"
#include "Engine/Mono/SpriteRenderer/Functions.hpp"
#include "Engine/Mono/Rigidbody2D/Functions.hpp"
#include "Engine/Mono/Audio/Functions.hpp"
#include "Engine/Mono/Mathf/Functions.hpp"
#include "mono/metadata/assembly.h"
#include "mono/metadata/loader.h"
#include "mono/metadata/object-forward.h"
#include "mono/metadata/object.h"
#include <Engine/Mono/CSharp.hpp>
#include <iostream>
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <fstream>
#include <filesystem>
#include <glm/ext.hpp>
#include <Engine/Input/Input.hpp>
#include <Engine/Mono/HelperFunctions.hpp>

namespace fs = std::filesystem;

namespace Engine {
    CSharp *CSharp::instance;

    char *ReadBytes(const std::string &filepath, uint32_t *outSize) {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream) {
            // Failed to open the file
            return nullptr;
        }

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint32_t size = end - stream.tellg();

        if (size == 0) {
            // File is empty
            return nullptr;
        }

        char *buffer = new char[size];
        stream.read((char *)buffer, size);
        stream.close();

        *outSize = size;
        return buffer;
    }

    MonoAssembly *LoadCSharpAssembly(const std::string &assemblyPath) {
        uint32_t file_size = 0;
        char *file_data = ReadBytes(assemblyPath, &file_size);

        MonoImageOpenStatus status;
        MonoImage *image = mono_image_open_from_data_full(file_data, file_size, 1, &status, 0);

        if (status != MONO_IMAGE_OK) {
            const char *errorMessage = mono_image_strerror(status);
            return nullptr;
        }

        MonoAssembly *assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
        mono_image_close(image);

        delete[] file_data;

        return assembly;
    }

    void LoadSubClasses(MonoAssembly *assembly) {
        MonoImage *image = mono_assembly_get_image(assembly);
        const MonoTableInfo *typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t type_count = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass *vault_script_class = mono_class_from_name(image, "Vault", "Entity");

        for (int32_t i = 0; i < type_count; i++) {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char *nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char *name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            MonoClass *entity_class = mono_class_from_name(image, nameSpace, name);
            bool isSubclass = mono_class_is_subclass_of(entity_class, vault_script_class, false);

            if (entity_class == vault_script_class)
                continue;

            if (isSubclass) {
                CSharp::instance->entity_classes[std::string(std::string(nameSpace) + "." + name)] = std::pair(nameSpace, name);
            }

            printf("%s.%s\n", nameSpace, name);
        }
    }

    MonoClass *GetClassInAssembly(MonoAssembly *assembly, const char *namespaceName, const char *className) {
        MonoImage *image = mono_assembly_get_image(assembly);
        MonoClass *klass = mono_class_from_name(image, namespaceName, className);

        return klass;
    }

    void CSharp::InitRuntime() {
        if (root_domain != nullptr) return;
        root_domain = mono_jit_init((char *)runtime_name.c_str());
    }

    void CSharp::InitMono() {
        mono_set_assemblies_path("../mono/lib");
        InitRuntime();

        app_domain = mono_domain_create_appdomain((char *)appdomain_name.c_str(), nullptr);
        mono_domain_set(app_domain, true);

        core_assembly = LoadCSharpAssembly("../csharp-lib/bin/Debug/net6.0/csharp-lib.dll");

        LoadSubClasses(core_assembly);

        RegisterVaultFunctions();

        core_assembly_image = GetImage(core_assembly);
    }

    void CSharp::ReloadAssembly() {
        // if (!fs::exists("assets/VAULT_OUT/cs-assembly.dll"))
        //     return;

        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(app_domain);

        InitMono();
    }

    void CSharp_EditorConsole_Log(MonoString *str) {
        const std::string content = CSharpHelper::MonoStrToString(str);
        Editor::GUI::LogInfo(content);
    }

    void CSharp_EditorConsole_LogError(MonoString *str) {
        const std::string content = CSharpHelper::MonoStrToString(str);
        Editor::GUI::LogError(content);
    }

    void CSharp_EditorConsole_LogWarning(MonoString *str) {
        const std::string content = CSharpHelper::MonoStrToString(str);
        Editor::GUI::LogWarning(content);
    }

    void CSharp::RegisterVaultFunctions() {
        using namespace CSharpInternalFunctions;

        // GameObject
        VAULT_REGISTER_FUNCTION(GameObject_GetName);
        VAULT_REGISTER_FUNCTION(GameObject_GetTag);
        VAULT_REGISTER_FUNCTION(GameObject_GetIDByName);
        VAULT_REGISTER_FUNCTION(GameObject_GetIDByTag);
        VAULT_REGISTER_FUNCTION(Scene_LoadScene);
        VAULT_REGISTER_FUNCTION(GameObject_InstantiatePrefab);
        VAULT_REGISTER_FUNCTION(GameObject_InstantiatePrefabWithProps);

        // Transform
        VAULT_REGISTER_FUNCTION(Transform_GetPosition);
        VAULT_REGISTER_FUNCTION(Transform_GetRotation);
        VAULT_REGISTER_FUNCTION(Transform_GetScale);
        VAULT_REGISTER_FUNCTION(Transform_SetField);

        // Format
        VAULT_REGISTER_FUNCTION(float_ToString);
        VAULT_REGISTER_FUNCTION(double_ToString);
        VAULT_REGISTER_FUNCTION(int_ToString);

        // Time
        VAULT_REGISTER_FUNCTION(Time_GetDeltaTime);

        // Audio
        VAULT_REGISTER_FUNCTION(Audio2D_PlayMusic);
        VAULT_REGISTER_FUNCTION(Audio2D_StopMusic);
        VAULT_REGISTER_FUNCTION(Audio2D_PlaySound);
        VAULT_REGISTER_FUNCTION(Audio2D_StopSound);

        // Input
        VAULT_REGISTER_FUNCTION_NAME("Vault.Input::IsKeyPressed", Input::IsKeyPressed);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Input::IsKeyReleased", Input::IsKeyReleased);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Input::IsKeyDown", Input::IsKeyDown);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Input::IsKeyUp", Input::IsKeyUp);

        // Log
        VAULT_REGISTER_FUNCTION_NAME("Vault.Debug::Log", CSharp_EditorConsole_Log);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Debug::Error", CSharp_EditorConsole_LogError);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Debug::Warning", CSharp_EditorConsole_LogWarning);

        // Text
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_GetText", Text3D_GetText);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_GetScale", Text3D_GetScale);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_GetYOffset", Text3D_GetYOffset);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_GetColor", Text3D_GetColor);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_SetText", Text3D_SetText);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_SetColor", Text3D_SetColor);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_SetScale", Text3D_SetScale);
        VAULT_REGISTER_FUNCTION_NAME("Vault.Text3D::Text3D_SetYOffset", Text3D_SetYOffset);

        // SpriteRenderer
        VAULT_REGISTER_FUNCTION_NAME("Vault.SpriteRenderer::SpriteRenderer_GetTexture", SpriteRenderer_GetTexture);
        VAULT_REGISTER_FUNCTION_NAME("Vault.SpriteRenderer::SpriteRenderer_SetTexture", SpriteRenderer_SetTexture);
        VAULT_REGISTER_FUNCTION_NAME("Vault.SpriteRenderer::SpriteRenderer_GetColor", SpriteRenderer_GetColor);
        VAULT_REGISTER_FUNCTION_NAME("Vault.SpriteRenderer::SpriteRenderer_SetColor", SpriteRenderer_SetColor);

        // Rigidbody2D
        VAULT_REGISTER_FUNCTION_NAME("Vault.Rigidbody2D::Rigidbody2D_GetKey", Rigidbody2D_GetKey);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_SetVelocity);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_SetAngularVelocity);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_SetPosition);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_Force);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_Torque);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Rigidbody2D::", Rigidbody2D_SetType);

        // Mathf
        using namespace Mathf;
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Deg2Rad);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Rad2Deg);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Abs);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Acos);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Asin);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Atan);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Atan2);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Ceil);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Clamp);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Cos);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Sin);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Sqrt);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Tan);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Round);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Pow);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Log);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Log10);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Max);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Min);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Exp);
        VAULT_REGISTER_FUNCTION_PREFIX("Vault.Mathf::", Lerp);
    }

    void CSharp::RegisterFunction(const std::string &cs_path, void *func) {
        mono_add_internal_call(cs_path.c_str(), func);
    }

    CSharp::CSharp(const std::string &lib_path, const std::string &runtime_name, const std::string &appdomain_name) : runtime_name(runtime_name), appdomain_name(appdomain_name) {
        instance = this;
        InitMono();
    }

    CSharp::~CSharp() {
    }

    MonoImage *CSharp::GetImage(MonoAssembly *core_assembly) {
        return mono_assembly_get_image(core_assembly);
    }

    CSharpClass::CSharpClass(MonoImage *image, const std::string &name_space, const std::string &name) {
        klass = mono_class_from_name(image, name_space.c_str(), name.c_str());
        instance = mono_object_new(CSharp::instance->app_domain, klass);
        gc_handle = mono_gchandle_new(instance, false);
    }

    MonoMethod *CSharpClass::GetMethod(const std::string &name, int param_count) {
        return mono_class_get_method_from_name(klass, name.c_str(), param_count);
    }

    void *CSharpClass::GetThunkFromMethod(MonoMethod *method) {
        return mono_method_get_unmanaged_thunk(method);
    }

    void *CSharpClass::GetMethodThunk(const std::string &name, int param_count) {
        return mono_method_get_unmanaged_thunk(mono_class_get_method_from_name(klass, name.c_str(), param_count));
    }

    MonoObject *CSharpClass::GetHandleTarget() {
        return mono_gchandle_get_target(gc_handle);
    }

    ScriptClass::ScriptClass(MonoImage *image, const std::string &name_space, const std::string &name) : CSharpClass(image, name_space, name) {
        update_method = GetMethod("OnUpdate", 0);
        update_thunk = (OnUpdateType)GetThunkFromMethod(update_method);

        start_method = GetMethod("OnStart", 1);
        start_thunk = (OnStartType)GetThunkFromMethod(start_method);
    }

    void ScriptClass::OnStart(const std::string &gameObject_ID) {
        MonoObject *exception = nullptr;
        void *p = mono_string_new(CSharp::instance->app_domain, gameObject_ID.c_str());
        mono_runtime_invoke(start_method, GetHandleTarget(), &p, &exception);

        if (exception) {
            MonoObject *exc = NULL;
            MonoString *str = mono_object_to_string(exception, &exc);
            if (exc) {
                mono_print_unhandled_exception(exc);
            } else {
                Editor::GUI::LogError(mono_string_to_utf8(str)); // Log log(mono_string_to_utf8(str), LOG_ERROR);
            }
        }
    }

    void ScriptClass::OnUpdate() {
        MonoObject *exception = nullptr;
        update_thunk(GetHandleTarget(), &exception);

        if (exception) {
            MonoObject *exc = NULL;
            MonoString *str = mono_object_to_string(exception, &exc);
            if (exc) {
                mono_print_unhandled_exception(exc);
            } else {
                Editor::GUI::LogError(mono_string_to_utf8(str)); // Log log(mono_string_to_utf8(str), LOG_ERROR);
            }
        }
    }
} // namespace Engine