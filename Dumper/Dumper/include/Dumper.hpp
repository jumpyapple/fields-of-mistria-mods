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
#include <windows.h>

#include <YYToolkit/YYTK_Shared.hpp>
#include <fstream>
#include <unordered_set>
#include <string_view>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <inja.hpp>

#include "Dumper_Shared.hpp"

// These static variables are probably different copies for this own mod
// and when other mod is calling.
static std::unordered_set<uint64_t> visited_pointers;
static std::vector<std::tuple<YYTK::RValue, std::string>> queue;

static std::string PAGE_TEMPLATE = "";
static std::string JAVASCRIPT_TEMPLATE = "";
static std::string CSS_TEMPLATE = "";

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

namespace Dumper {

	namespace fs = std::filesystem;

	YYTK::CScript* TryLookupScriptByFunctionPointer(IN YYTK::YYTKInterface* g_ModuleInterface, IN YYTK::PFUNC_YYGMLScript ScriptFunction);

	nlohmann::json ToJsonObject(
		IN YYTK::YYTKInterface* g_ModuleInterface,
		std::string name,
		YYTK::RValue value,
		bool should_recurse_array,
		bool dont_queue
	);

    std::optional<std::string> LoadTemplate(YYTK::YYTKInterface* g_ModuleInterface, std::string template_name);

    void LoadResources(YYTK::YYTKInterface* g_ModuleInterface);
}
