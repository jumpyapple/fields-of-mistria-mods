#include <optional>
#include <regex>

#include <YYToolkit/YYTK_Shared.hpp>

#include "Dumper.hpp"
#include "common.hpp"

#include "resource.h"

using namespace Aurie;
using namespace YYTK;
using namespace inja;
using json = nlohmann::json;


namespace Dumper {
    namespace fs = std::filesystem;

    std::optional<std::string> LoadTemplate(YYTK::YYTKInterface* g_ModuleInterface, std::string template_name) {
        // Load our DLL for resource.
        HMODULE module_handle = LoadLibraryExA("DumperLib.dll", NULL, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
        if (module_handle == NULL) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Failed to load 'DumperLib.dll'!", PLUGIN_NAME, VERSION);
            return {};
        }

        HRSRC resource_handle;
        if (template_name == "page.html") {
            resource_handle = FindResource(module_handle, MAKEINTRESOURCE(IDR_HTML1), RT_HTML);
        }
        else if (template_name == "z.js") {
            resource_handle = FindResource(module_handle, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
        }
        else if (template_name == "z.css") {
            resource_handle = FindResource(module_handle, MAKEINTRESOURCE(IDR_RCDATA2), RT_RCDATA);
        }
        else {
            resource_handle = NULL;
        }
        if (resource_handle == NULL) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Failed to load the embed resource for '%s'!", PLUGIN_NAME, VERSION, template_name.c_str());
            return {};
        }

        HGLOBAL memory_handle = LoadResource(module_handle, resource_handle);
        if (resource_handle == NULL) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Failed to get memory handle for the resource!", PLUGIN_NAME, VERSION);
            return {};
        }

        LPVOID address = LockResource(memory_handle);
        if (address == NULL) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Failed to lock resource!", PLUGIN_NAME, VERSION);
            return {};
        }

        DWORD size = SizeofResource(module_handle, resource_handle);
        if (size == 0) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Failed to get the size of the resource!", PLUGIN_NAME, VERSION);
            return {};
        }

        std::string data(static_cast<const char*>(address), size);
        return data;
    }

    void LoadResources(YYTK::YYTKInterface* g_ModuleInterface) {
        if (PAGE_TEMPLATE.size() == 0) {
            PAGE_TEMPLATE = LoadTemplate(g_ModuleInterface, "page.html").value_or("");
        }

        if (JAVASCRIPT_TEMPLATE.size() == 0) {
            JAVASCRIPT_TEMPLATE = LoadTemplate(g_ModuleInterface, "z.js").value_or("");
        }

        if (CSS_TEMPLATE.size() == 0) {
            CSS_TEMPLATE = LoadTemplate(g_ModuleInterface, "z.css").value_or("");
        }
    }

    CScript* TryLookupScriptByFunctionPointer(
        IN YYTK::YYTKInterface* g_ModuleInterface,
        IN PFUNC_YYGMLScript ScriptFunction
    )
    {
        Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

        int script_index = 0;
        while (Aurie::AurieSuccess(last_status))
        {
            CScript* script = nullptr;

            last_status = g_ModuleInterface->GetScriptData(
                script_index,
                script
            );

            if (!Aurie::AurieSuccess(last_status))
                break;

            if (script->m_Functions)
            {
                if (script->m_Functions->m_ScriptFunction == ScriptFunction)
                    return script;
            }

            script_index++;
        }
        return nullptr;
    }

    json ToJsonObject(IN YYTK::YYTKInterface* g_ModuleInterface, std::string name, RValue value, bool should_recurse_array, bool dont_queue) {
        std::string kind_name = value.GetKindName();

        json item;
        item["name"] = name;
        item["type"] = kind_name;

        if (value.m_Kind != VALUE_UNDEFINED && value.m_Kind != VALUE_UNSET && value.m_Kind != VALUE_NULL) {
            uint64_t pointer = reinterpret_cast<uint64_t>(value.ToPointer());
            item["pointer"] = std::format("{:#010x}", pointer);
            item["filename"] = std::format("p{:#010x}.htm", pointer);
            item["pointer_uint64"] = pointer;

            if (value.m_Kind == VALUE_REAL || value.m_Kind == VALUE_INT64 || value.m_Kind == VALUE_INT32 || value.m_Kind == VALUE_BOOL) {
                item["value"] = value.ToDouble();
            }
            else if (value.m_Kind == VALUE_STRING) {
                std::string value_str = "'";
                value_str.append(value.ToCString());
                value_str.append("'");
                item["value"] = value_str;
            }
            else if (value.m_Kind == VALUE_ARRAY) {
                auto array_values = value.ToVector();

                std::vector<json> values;
                item["value"] = "array (size = " + std::to_string(array_values.size()) + ")";

                if (IGNORING_KEYS.find(name) != IGNORING_KEYS.end()) {
                    item["values"] = values;
                    return item;
                }

                if (should_recurse_array) {
                    for (int i = 0; i < array_values.size(); i++) {
                        std::string array_item_name = name;
                        array_item_name += "[";
                        array_item_name += std::to_string(i);
                        array_item_name += "]";
                        // We want the name and type, but not the detail of the element in the array,
                        // so we set should_recurse_array=false.
                        json array_item = ToJsonObject(g_ModuleInterface, array_item_name, array_values[i], false, true);
                        values.push_back(array_item);

                        // Queue the RValue for later processing.
                        if (array_item["type"] == "struct") {
                            if (!visited_pointers.contains(array_item["pointer_uint64"]) && !dont_queue) {
                                queue.push_back(std::make_tuple(array_values[i], std::format("p{:#010x}.htm", array_item["pointer_uint64"].get<uint64_t>())));
                                visited_pointers.insert(array_item["pointer_uint64"].get<uint64_t>());
                            }
                        }
                    }
                    item["values"] = values;
                }
            }
            else if (value.m_Kind == VALUE_OBJECT) {
                YYObjectBase* object = value.ToObject();
                if (object->m_ObjectKind == OBJECT_KIND_SCRIPTREF) {
                    CScriptRef* ref = value.ToPointer<CScriptRef*>();
                    CScript* script = TryLookupScriptByFunctionPointer(
                        g_ModuleInterface,
                        ref->m_CallYYC
                    );

                    if (script) {
                        std::string value_str = "script '";
                        value_str.append(script->GetName());
                        value_str.append("'");
                        item["value"] = value_str;
                    }
                    else {
                        item["value"] = "not a script?";
                        if (!visited_pointers.contains(pointer) && !dont_queue) {
                            queue.push_back(std::make_tuple(value, std::format("p{:#010x}.htm", pointer)));
                            visited_pointers.insert(pointer);
                        }
                    }
                }
                else {
                    RValue result = g_ModuleInterface->CallBuiltin("instanceof", { value });

                    if (result.m_Kind == VALUE_STRING) {
                        std::string result_str = result.ToCString();
                        item["value"] = result_str;
                    }
                    else {
                        // So far the instanceof function will fall back to "struct", so
                        // this does not show up yet.
                        item["value"] = "TODO2";
                    }

                    if (!visited_pointers.contains(pointer) && !dont_queue) {
                        queue.push_back(std::make_tuple(value, std::format("p{:#010x}.htm", pointer)));
                        visited_pointers.insert(pointer);
                    }
                }
            }
            else if (value.m_Kind == VALUE_REF) {
                // stuff like __sprite_vertex_buffer_format exist which does not seem to be a sprite.
                // There are other like enabled_sprite, hovered_sprite, but it should be appearing close
                // to the "sprite", so that should be eaisly deduce. Otherwise, add those name here.
                if (name == "sprite") {
                    RValue result = g_ModuleInterface->CallBuiltin("sprite_get_name", { value });

                    // TODO: Attempt to call sprite_get_name on `icon`.

                    if (result.m_Kind == VALUE_STRING) {
                        std::string result_str = result.ToCString();
                        item["value"] = result_str;
                    }
                    else {
                        item["value"] = ""; // intentionally left blank.
                    }
                }
                else {
                    item["value"] = ""; // intentionally left blank.
                }
            }
            else {
                // other value kinds.
                item["value"] = "TODO3";
            }
        }
        else {
            item["value"] = ""; // intentionally left blank.
        }
        return item;
    }

    void DumpRValue(
        IN YYTK::YYTKInterface* g_ModuleInterface,
        IN YYTK::RValue& Instance,
        fs::path target_directory,
        std::string index_filename
    )
    {
        // Sanity checks
        if (fs::exists(target_directory) && !fs::is_directory(target_directory)) {
            g_ModuleInterface->Print(CM_LIGHTYELLOW, "[Dumper %s] - Stop dumping because the provided path is not a directory.", VERSION);
            return;
        }
        else if (!fs::exists(target_directory)) {
            // Create the target folder if it does not exist.
            fs::create_directories(target_directory);
        }

        // Obtain the embeded resources.
        LoadResources(g_ModuleInterface);
        const std::regex whitespace_regex("\\s+");
        const std::regex newline_regex("\\r\\n");

        // Write the helper files. E.g. z.js. z.css.
        try {
            JAVASCRIPT_TEMPLATE = std::regex_replace(JAVASCRIPT_TEMPLATE, newline_regex, "");
            JAVASCRIPT_TEMPLATE = std::regex_replace(JAVASCRIPT_TEMPLATE, whitespace_regex, " ");
            std::ofstream helper_file;
            helper_file.open((target_directory / "z.js").c_str());
            helper_file << JAVASCRIPT_TEMPLATE << std::endl;
            helper_file.close();

            CSS_TEMPLATE = std::regex_replace(CSS_TEMPLATE, whitespace_regex, " ");
            CSS_TEMPLATE = std::regex_replace(CSS_TEMPLATE, newline_regex, "");
            helper_file.open((target_directory / "z.css").c_str());
            helper_file << CSS_TEMPLATE << std::endl;
            helper_file.close();
        }
        catch (const std::ios_base::failure& io_ex) {
            g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Error occured during a write to the helper files: %s", PLUGIN_NAME, VERSION, io_ex.what());
            return;
        }

        // We want each folder (when using DumpHookVariables) to have their own copy
        // of the files.
        // FIXME: This will overwrite the page if its content has been changed across a call.
        // E.g. dumping spawn menu multiple time will overwrite the page for `__anchor` and only the last
        // dump will persist.
        visited_pointers.clear();

        queue.clear();
        queue.push_back(std::make_tuple(Instance, index_filename));

        int count = 0;
        while (!queue.empty()) {
            std::tuple<RValue, std::string> queue_entry = queue.front();
            RValue instance = std::get<0>(queue_entry);
            std::string filename = std::get<1>(queue_entry);

            // Various stores by type of the item.
            std::vector<json> method_items, boolean_items, string_items, struct_items,
                int64_items, number_items, ref_items, array_items, undefined_items, other_items;

            // If we are going to take a long time, let's show the queue size after the first iteration.
            if (count % 10000 == 0 && count != 0) {
                g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumped %d values so far (queue size: %d, visited pointers: %d). Currently dumping to %s",
                    VERSION, count, queue.size(), visited_pointers.size(), filename.c_str());
            }
            count++;

            // Run the shallow version of the decoding bit.
            // Also not queuing any recursive value.
            // TODO: Let the caller pass in the name, but this will break the public API.
            json item = ToJsonObject(g_ModuleInterface, "??", Instance, false, true);
            item["filename"] = filename;

            // Group all the items by their type.
            auto items = instance.ToRefMap();
            for (auto& [key, value] : items)
            {
                std::string key_name = key.c_str();
                RValue value_copied = *value;

                // We want to list array content on the same page, but we do not want to
                // traverse deeper.
                json item = ToJsonObject(g_ModuleInterface, key_name, value_copied, true, false);

                // Get the item's type, so we can categorize it.
                if (item["type"] == "method") {
                    method_items.push_back(item);
                }
                else if (item["type"] == "bool") {
                    boolean_items.push_back(item);
                }
                else if (item["type"] == "string") {
                    string_items.push_back(item);
                }
                else if (item["type"] == "struct") {
                    struct_items.push_back(item);
                }
                else if (item["type"] == "int64") {
                    int64_items.push_back(item);
                }
                else if (item["type"] == "number") {
                    number_items.push_back(item);
                }
                else if (item["type"] == "ref") {
                    ref_items.push_back(item);
                }
                else if (item["type"] == "array") {
                    array_items.push_back(item);
                }
                else if (item["type"] == "undefined") {
                    undefined_items.push_back(item);
                }
                else {
                    // e.g. unknown, "" (just empty)
                    other_items.push_back(item);
                }
            }

            json template_data;
            // Details of the value itself.
            template_data["name"] = filename;
            template_data["type"] = item["type"];
            template_data["value"] = item["value"];

            if (item.contains("pointer")) {
                template_data["pointer"] = item["pointer"];
                template_data["filename"] = item["filename"];
                template_data["pointer_uint64"] = item["pointer_uint64"];
            }

            // Sections.
            template_data["methods"] = method_items;
            template_data["booleans"] = boolean_items;
            template_data["strings"] = string_items;
            template_data["structs"] = struct_items;
            template_data["int64s"] = int64_items;
            template_data["numbers"] = number_items;
            template_data["refs"] = ref_items;
            template_data["undefineds"] = undefined_items;
            template_data["arrays"] = array_items;
            template_data["other"] = other_items;

            // Render the template to a HTML file.
            // Initially, the file name was formatted as "{variable_name}_{type}_{pointer}.html".
            // However, we cannot reliably link to the file if the same pointer show up at a different
            // section of the dump since it will have a different name.
            fs::path index_path = target_directory / filename;

            std::string page_text;
            try {
                page_text = inja::render(PAGE_TEMPLATE, template_data);
            }
            catch (const inja::ParserError& inja_parse_ex) {
                g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Error during template parsing: %s", PLUGIN_NAME, VERSION, inja_parse_ex.what());
                return;
            }
            catch (const inja::RenderError& inja_render_ex) {
                g_ModuleInterface->PrintError(__FILE__, __LINE__, "[%s %s] Error during template rendering: %s", PLUGIN_NAME, VERSION, inja_render_ex.what());
                return;
            }

            try {
                page_text = std::regex_replace(page_text, newline_regex, "");
                page_text = std::regex_replace(page_text, whitespace_regex, " ");

                std::ofstream out_file;
                out_file.open(index_path.string());
                out_file << page_text << std::endl;
                out_file.close();
            }
            catch (const std::ios_base::failure& io_ex) {
                g_ModuleInterface->Print(CM_LIGHTRED, "[Dumper %s] - Error during writing to a file: %s", VERSION, io_ex.what());
                return;
            }

            queue.erase(queue.begin());
        }
    }

#pragma region Public_APIs
    void DumpRValueWithDefaultIndexFilename(
        IN YYTK::YYTKInterface* g_ModuleInterface,
        IN YYTK::RValue& Value,
        fs::path target_directory
    ) {
        DumpRValue(g_ModuleInterface, Value, target_directory, "index.html");
    }

    void DumpCInstance(
        IN YYTKInterface* g_ModuleInterface,
        IN CInstance* Instance,
        fs::path target_directory,
        std::string index_filename
    ) {
        RValue value = Instance->ToRValue();
        DumpRValue(g_ModuleInterface, value, target_directory, index_filename);
    }

    void DumpCInstanceWithDefaultIndexFilename(
        IN YYTKInterface* g_ModuleInterface,
        IN CInstance* Instance,
        fs::path target_directory
    )
    {
        RValue value = Instance->ToRValue();
        DumpRValueWithDefaultIndexFilename(g_ModuleInterface, value, target_directory);
    }

    void DumpHookVariables(
        IN YYTK::YYTKInterface* g_ModuleInterface,
        IN std::string Prefix,
        IN uint64_t Count,
        IN CInstance* Self,
        IN CInstance* Other,
        IN RValue Result,
        IN int ArgumentCount,
        IN RValue** Arguments
    )
    {
        g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumping Self of '%s'...", VERSION, Prefix.c_str());
        std::string self_folder = Prefix;
        self_folder.append("_self_dumps");
        DumpCInstance(g_ModuleInterface, Self, self_folder, std::format("index_{}.html", Count));

        g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumping Other of '%s'...", VERSION, Prefix.c_str());
        std::string other_folder = Prefix;
        other_folder.append("_other_dumps");
        DumpCInstance(g_ModuleInterface, Other, other_folder, std::format("index_{}.html", Count));

        g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumping Result of '%s'...", VERSION, Prefix.c_str());
        std::string result_folder = Prefix;
        result_folder.append("_result_dumps");
        DumpRValue(g_ModuleInterface, Result, result_folder, std::format("index_{}.html", Count));

        for (int i = 0; i < ArgumentCount; i++) {
            std::string arg_folder = Prefix;
            arg_folder.append("_arg");
            arg_folder.append(std::to_string(i));
            arg_folder.append("_dumps");

            g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumping Arguments[%d] of '%s'...", VERSION, i, Prefix.c_str());
            DumpRValue(g_ModuleInterface, *Arguments[i], arg_folder, std::format("index_{}.html", Count));
        }
    }
}
#pragma endregion Public_APIs
