/**
* @brief A dumper utility for YYToolkit
* @author jumpyapple
* 
* Based on Archie_uwu and Felix implementation.
* 
* Required libraries
* - nlohmann/json
* - pantor/inja
* 
* Need NOMINMAX to be define, otherwise the std::min, std::max will be
* redefined by windows.h and inja will fail to compile.
* 
* Also need access to YYTK's internal.
*/
#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include <fstream>
#include <unordered_set>
#include <string_view>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <inja.hpp>

#include "common.hpp"

using namespace Aurie;
using namespace YYTK;
using namespace inja;
using json = nlohmann::json;

namespace fs = std::filesystem;

static const std::unordered_set<std::string_view> IGNORING_KEYS = {
	"node_terrain_tile",
	"node_terrain_kind",
	"node_is_room_editor_collision",
	"node_object_id",
	"node_collideable",
	"node_terrain_is_watered",
	"node_rug_parent",
	"node_pathfinding_cost",
	"node_flags",
	"node_force",
	"node_terrain_variant",
	"node_terrain_is_watered",
	"node_rug_id",
	"node_is_room_editor_collision",
	"node_top_left_y",
	"node_top_left_x",
	"node_terrain_ground_kind",
	"node_pathfinding_sitting",
	"node_parent",
	"maximum_item_counts",
	"node_footstep_kind",
	"node_can_jump_over"
};

static const std::string INDEX_TEMPLATE_TEXT = R"(<!DOCTYPE html>
<html lang="en">
<head>
	<title>{{ name }}</title>
	<link
		rel="stylesheet"
		href="https://cdn.jsdelivr.net/npm/bulma@1.0.4/css/bulma.min.css"
	>
	<style>
	</style>
</head>
<body>
	<h1 class="title">{% if exists("type") %}[{{ type }}] {% endif %}{{ name }}</h1>
	<hr/>
	<section id="boolean-section">
		<h2 class="subtitle">Booleans</h2>
		<div class="content">
			<ul>
## for item in booleans
				<li>{{ item.name }} = {{ item.value }}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="int64-section">
		<h2 class="subtitle">Int64</h2>
		<div class="content">
			<ul>
## for item in int64s
				<li>{{ item.name }} = {{ item.value }}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="number-section">
		<h2 class="subtitle">Numbers</h2>
		<div class="content">
			<ul>
## for item in numbers
				<li>{{ item.name }} = {{ item.value }}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="string-section">
		<h2 class="subtitle">Strings</h2>
		<div class="content">
			<ul>
## for item in strings
				<li>{{ item.name }} = {{ item.value }}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="ref-section">
		<h2 class="subtitle">Refs</h2>
		<div class="content">
			<ul>
## for item in refs
				<li>{{ item.name }} = {{ item.value }} {% if existsIn(item, "pointer") %} {{ item.pointer }}{% endif %}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="undefined-section">
		<h2 class="subtitle">Undefined values</h2>
		<div class="content">
			<ul>
## for item in undefineds
				<li>{{ item.name }}</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="struct-section">
		<h2 class="subtitle">Structs</h2>
		<div class="content">
			<ul>
## for item in structs
				<li>
					{% if existsIn(item, "pointer") %}<a href="{{ item.pointer }}.html">{% endif %}
					{{ item.name }} = {{ item.value}} {% if existsIn(item, "pointer") %} {{ item.pointer }}{% endif %}
					{% if existsIn(item, "pointer") %}</a>{% endif %}
				</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="array-section">
		<h2 class="subtitle">Arrays</h2>
		<div class="content">
			<ul>
## for item in arrays
				<li>
					<details>
						<summary>{{ item.name }} = {{ item.value }}</summary>
						<ul>
## for array_item in item.values
							<li>
## if array_item.type == "struct" or array_item.type == "array"
								{% if existsIn(item, "pointer") %}<a href="{{ array_item.pointer }}.html">{% endif %}
									[{{ array_item.type }}] {{ array_item.name }} = {{ array_item.value }}
								{% if existsIn(item, "pointer") %}</a>{% endif %}
## else
								[{{ array_item.type }}] {{ array_item.name }} = {{ array_item.value }}
## endif
							</li>
## endfor
						</ul>
					</details>
				</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="other-section">
		<h2 class="subtitle">Other</h2>
		<div class="content">
			<ul>
## for item in other
				<li>
					{% if existsIn(item, "pointer") %}<a href="{{ item.pointer }}.html">{% endif %}
					[{{ item.type }}] {{ item.name }}
					{% if existsIn(item, "pointer") %}</a>{% endif %}
				</li>
## endfor
			</ul>
		</div>
	</section>
	<section id="method-section">
		<h2 class="subtitle">Methods</h2>
		<div class="content">
			<ul>
## for item in methods
				<li>{{ item.name }} = {{ item.value }}</li>
## endfor
			</ul>
		</div>
	</section>
</body>
</html>
)";

static std::unordered_set<uint64_t> visited_pointers;
static std::vector<std::tuple<RValue, std::string>> queue;

CScript* TryLookupScriptByFunctionPointer(
	IN PFUNC_YYGMLScript ScriptFunction
)
{
	AurieStatus last_status = AURIE_SUCCESS;

	int script_index = 0;
	while (AurieSuccess(last_status))
	{
		CScript* script = nullptr;

		last_status = g_ModuleInterface->GetScriptData(
			script_index,
			script
		);

		if (!AurieSuccess(last_status))
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

json ToJsonObject(std::string name, RValue value, bool should_recurse_array) {
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
					json array_item = ToJsonObject(array_item_name, array_values[i], false);
					values.push_back(array_item);

					// Queue the RValue for later processing.
					if (array_item["type"] == "struct") {
						if (!visited_pointers.contains(array_item["pointer"])) {
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
					if (!visited_pointers.contains(item["pointer"])) {
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

				if (!visited_pointers.contains(item["pointer"])) {
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

/**
* @brief Dump the details of the Instance into target_directory
* 
* The details of the given instenace will be put in index.html file while
* the details for other related instances are put in a HTML file with pointer (in uint64)
* as its name.
* 
* @param Instance The instance to start dumping from.
* @param target_directory The directory to store all the HTML files. 
*/
void DumpInstance(
	IN RValue& Instance, fs::path target_directory
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

	queue.clear();
	queue.push_back(std::make_tuple(Instance, "index.html"));

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

		// Group all the items by their type.
		for (auto& [key, value] : items)
		{
			std::string key_name = key.c_str();
			RValue value_copied = *value;

			// We want to list array content on the same page, but we do not want to
			// traverse deeper.
			json item = ToJsonObject(key_name, value_copied, true);

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
			page_text = render(INDEX_TEMPLATE_TEXT, template_data);
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