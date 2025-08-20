#include <YYToolkit/YYTK_Shared.hpp>
#include "Dumper.hpp"
#include "common.hpp"

using namespace Aurie;
using namespace YYTK;
using namespace inja;
using json = nlohmann::json;


namespace Dumper {
	namespace fs = std::filesystem;

	void EnableDumper() {
		g_SHOULD_DUMP = true;
	}

	void DisableDumper() {
		g_SHOULD_DUMP = false;
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
			item["pointer"] = pointer;

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
							if (!visited_pointers.contains(array_item["pointer"]) && !dont_queue) {
								std::string filename = std::to_string(array_item["pointer"].get<uint64_t>());
								filename.append(".html");
								queue.push_back(std::make_tuple(array_values[i], filename));
								visited_pointers.insert(pointer);
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
						if (!visited_pointers.contains(item["pointer"]) && !dont_queue) {
							std::string filename = std::to_string(pointer);
							filename.append(".html");
							queue.push_back(std::make_tuple(value, filename));
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

					if (!visited_pointers.contains(item["pointer"]) && !dont_queue) {
						std::string filename = std::to_string(pointer);
						filename.append(".html");
						queue.push_back(std::make_tuple(value, filename));
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
		// Check for global disable.
		if (g_SHOULD_DUMP) {
			g_ModuleInterface->Print(YYTK::CM_LIGHTYELLOW, "[Dumper %s] - Ignoring a call to DumpRValue since g_SHOULD_DUMP is false", VERSION);
			return;
		}

		// Sanity checks
		if (fs::exists(target_directory) && !fs::is_directory(target_directory)) {
			g_ModuleInterface->Print(CM_LIGHTYELLOW, "[Dumper %s] - Stop dumping because the provided path is not a directory.", VERSION);
			return;
		}
		else if (!fs::exists(target_directory)) {
			// Create the target folder if it does not exist.
			fs::create_directories(target_directory);
		}

		// We want each folder (when using DumpHookVariables) to have their own copy
		// of the files.
		visited_pointers.clear();

		queue.clear();
		queue.push_back(std::make_tuple(Instance, index_filename));

		int count = 0;
		while (!queue.empty()) {
			std::tuple<RValue, std::string> queue_entry = queue.front();
			RValue instance = std::get<0>(queue_entry);
			std::string filename = std::get<1>(queue_entry);

			auto items = instance.ToRefMap();

			// Various stores by type of the item.
			std::vector<json> method_items, boolean_items, string_items, struct_items,
				int64_items, number_items, ref_items, array_items, undefined_items, other_items;

			if (count % 10000 == 0 && count != 0) {
				g_ModuleInterface->Print(CM_WHITE, "[Dumper %s] - Dumped %d values so far (queue size: %d, visited pointers: %d). Currently dumping to %s",
					VERSION, count, queue.size(), visited_pointers.size(), filename.c_str());
			}
			count++;

			// Run the shallow version of the decoding bit.
			// Also not queuing any recursive value.
			// TODO: Let the caller pass in the name, but this will break the public API.
			json item = ToJsonObject(g_ModuleInterface, "??", Instance, false, true);

			// Group all the items by their type.
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
			template_data["name"] = filename;
			template_data["value"] = item["value"];
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
				page_text = inja::render(INDEX_TEMPLATE_TEXT, template_data);
			}
			catch (const inja::RenderError& inja_ex) {
				g_ModuleInterface->Print(CM_LIGHTRED, "[Dumper %s] - Error during template rendering: %s", VERSION, inja_ex.what());
				return;
			}

			try {
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
		if (g_SHOULD_DUMP) {
			std::string self_folder = Prefix;
			self_folder.append("_self_dumps");
			DumpCInstance(g_ModuleInterface, Self, self_folder, std::format("index_{}.html", Count));

			std::string other_folder = Prefix;
			other_folder.append("_other_dumps");
			DumpCInstance(g_ModuleInterface, Other, other_folder, std::format("index_{}.html", Count));

			std::string result_folder = Prefix;
			result_folder.append("_result_dumps");
			DumpRValue(g_ModuleInterface, Result, result_folder, std::format("index_{}.html", Count));

			for (int i = 0; i < ArgumentCount; i++) {
				std::string arg_folder = Prefix;
				arg_folder.append("_arg");
				arg_folder.append(std::to_string(i));
				arg_folder.append("_dumps");

				DumpRValue(g_ModuleInterface, *Arguments[i], arg_folder, std::format("index_{}.html", Count));
			}
		}
		else {
			g_ModuleInterface->Print(YYTK::CM_LIGHTYELLOW, "[Dumper %s] - Ignoring a call to DumpHookVariables since g_SHOULD_DUMP is false", VERSION);
		}
	}
}