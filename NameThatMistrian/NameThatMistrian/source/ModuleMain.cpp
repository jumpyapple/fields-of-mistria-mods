/**
* @brief Main logic for the plugin that show NPC name on the minimap.
* @author jumpyapple
* 
*/
#include <YYToolkit/YYTK_Shared.hpp>

using namespace Aurie;
using namespace YYTK;

static YYTK::YYTKInterface* g_ModuleInterface = nullptr;
static const char* const VERSION = "0.1.0";

// We need to hook into this so we can re-parse MapMenu for NPCs' icon.
static const char* const SPAWN_MENU_SCRIPT = "gml_Script_spawn_menu@Anchor@Anchor";
static const char* const SELECT_LOCATION_SCRIPT = "gml_Script_select_location@MapMenu@MapMenu";

// We are hijacking this function to act as our tap event callback.
static const char* const NORTH_ARROW_TAP_SCRIPT = "gml_Script_anon@9836@MapMenu@MapMenu";

//static const char* const SOUTH_ARROW_TAP_SCRIPT = "gml_Script_anon@10679@MapMenu@MapMenu";
//static const char* const EAST_ARROW_TAP_SCRIPT = "gml_Script_anon@12362@MapMenu@MapMenu";
//static const char* const WEST_ARROW_TAP_SCRIPT = "gml_Script_anon@11522@MapMenu@MapMenu";

static RValue MapMenu;
static RValue map_name;
static std::string original_map_name;
static std::string new_map_name;

static RValue original_on_tap_function;
static RValue our_tap_event_callback;
//static RValue our_think_event_callback;

// Somehow the event callback function need to return something.
// true seems to work fine, so we leave it as that.
static RValue empty_result = true;

/**
* Check if the menu is the MapMenu
* 
* The spawn_menu function we are hooking into can spawn other menu as well (e.g. JournalMenu,
* game load confirming dialog?).
*/
bool IsMapMenu(RValue menu) {
	uint16_t has_map_menu_elements = 0;
	auto members = menu.ToRefMap();

	// These elements show up in the MapMenu.
	// We could possibly just call instanceof and check
	// if the returned value is MapMenu.
	for (auto& [key, value] : members)
	{
		if (key == "map") {
			has_map_menu_elements |= 1;
		}
		else if (key == "east_arrow") {
			has_map_menu_elements |= 2;
		}
		else if (key == "south_arrow") {
			has_map_menu_elements |= 4;
		}
		else if (key == "selected_location_id") {
			has_map_menu_elements |= 8;
		}
	}

	return has_map_menu_elements == 15;
}

/**
* Add `tap` event callback to the NPC's icon sprite.
*/
void InjectTapCallbacks(RValue MapMenu) {
	RValue map = MapMenu.GetMember("map");
	std::vector<RValue> positional_nodes = map.GetMember("children").ToVector();

	// Find NPC's icon.
	for (int i = 0; i < positional_nodes.size(); i++) {
		std::vector<RValue> sprite_nodes = positional_nodes[i].GetMember("children").ToVector();
		for (int j = 0; j < sprite_nodes.size(); j++) {
			RValue sprite = sprite_nodes[j].GetMember("sprite");
			if (sprite.m_Kind == VALUE_REF) {
				RValue result = g_ModuleInterface->CallBuiltin("sprite_get_name", { sprite });

				if (result.m_Kind == VALUE_STRING) {
					std::string sprite_name = result.ToCString();
					std::string name;


					// Process a sprite name to NPC name.
					if (sprite_name.find("icon_npc") != std::string::npos) {
						// Strip the spr_bha_bha_npc_icon_small_.
						name = sprite_name.substr(30);

						if (name == "player") {
							name = "me";
						}
					}
					else if (sprite_name.find("icon_pet") != std::string::npos) {
						name = "pet";
					}

					// Prepare and inject our tap callback.
					// 30 is just arbitary number that we can check in
					// the hooked function to filter out the call.
					//
					// If you use the dumper, you will see that the original
					// callback registration has no argument, but we are adding two
					// argument just to make sure we can filter it.
					RValue our_tap_event_callback = RValue({
						{ "arg_array", std::vector<RValue>({30, name.c_str()})},
						{ "func", original_on_tap_function },
					});

					// Inject our event callback to any NPC icon found.
					// TODO: Maybe Archie's big brain already implements a way for GameMaker to 
					// call C++ function? Still have to test that out.
					RValue target_event_callbacks = sprite_nodes[j].GetMember("event_callbacks");
					g_ModuleInterface->CallBuiltin(
						"struct_set",
						{ target_event_callbacks, "tap", our_tap_event_callback }
					);

					/*
					g_ModuleInterface->CallBuiltin(
						"struct_set",
						{ target_event_callbacks, "think", our_think_event_callback }
					);
					*/

					// Turn on tap and hover listening.
					// 
					// Somehow we need both, but then when I add a hover listener (via `think`
					// event callback as shown in the comment above), the hover event is fired all
					// the time regardless of whether the mouse is on the icon or not.
					// Maybe checking the in_hover during `think` callback is not it.
					g_ModuleInterface->CallBuiltin(
						"struct_set",
						{ sprite_nodes[j], "listens_for_taps", true }
					);
					g_ModuleInterface->CallBuiltin(
						"struct_set",
						{ sprite_nodes[j], "listens_for_hovers", true }
					);

					// Initially thought that we need this to elevate the icon up so
					// that we can click it, but it seems to work fine without it.
					/*g_ModuleInterface->CallBuiltin(
						"struct_set",
						{ sprite_nodes[j], "z", 102.0 }
					);*/
				}
			}
		}
	}
}

void Setup(RValue MapMenu) {
	// Retrieve event_callback from the north_arrow.
	RValue north_arrow = MapMenu.GetMember("north_arrow");
	RValue event_callbacks = north_arrow.GetMember("event_callbacks");
	RValue tap_event_callback = event_callbacks.GetMember("tap");
	original_on_tap_function = tap_event_callback.GetMember("func");

	map_name = MapMenu.GetMember("name");
	original_map_name = map_name.GetMember("text").ToCString();

	InjectTapCallbacks(MapMenu);
}

/**
* A hook for the spawn_menu script.
* 
* We are using this to intercept the intial call that create the mapmenu.
* It seems to be called everytime the player click back to the map tab of the journal menu
* as well.
* 
* The Self is the __anchor (in the global instance) and the Result is what ever is being spawned.
*/
RValue& SpawnMenuHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	//g_ModuleInterface->Print(CM_LIGHTGREEN, "[NameThatMistrian %s] - SpawnMenuHook called!", VERSION);

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
		g_ArSelfModule,
		SPAWN_MENU_SCRIPT
	));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (Result.m_Kind != VALUE_UNDEFINED && Result.m_Kind != VALUE_UNSET && Result.m_Kind != VALUE_NULL) {
		if (IsMapMenu(Result)) {
			// Save map menu. Althought, if it is active, we should
			// be able to retrieve it from __anchor in the global instance.
			MapMenu = Result;
			Setup(MapMenu);
		}
	}

	return Result;
}

/**
* A hook for the tap event of the north arrow of the map menu.
* 
* We are hijacking this and repurpose it as a target for our own callback definition.
* The original call does not have any argument, so we are differentiating between
* the orignal call and our call by the number of arguments. We double check it with an arbitary
* value in the first argument.
* 
* The second argument for our call is, however, the name of the NPC. We then use that
* to display to the player.
*/
RValue& NorthArrowTapHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (ArgumentCount == 2) {
		// This is the tap event callback for us.

		//g_ModuleInterface->Print(CM_LIGHTGREEN, "[NameThatMistrian %s] - Icon tapped!", VERSION);

		// Exrtact the arguments and also check that we are the intended receiver.
		if ((Arguments[0]->m_Kind == VALUE_INT32 || Arguments[0]->m_Kind == VALUE_INT64 || Arguments[0]->m_Kind == VALUE_REAL) &&
			(Arguments[1]->m_Kind == VALUE_STRING)) {
			// Confirming that we are the target of the callback.
			if (Arguments[0]->ToDouble() == 30) {
				// Extract the registered NPC name and reformat it for display.
				new_map_name = original_map_name;
				new_map_name.append(" - ");
				new_map_name.append(Arguments[1]->ToCString());

				// The `text` member does not work, so we are using `display_text`.
				g_ModuleInterface->CallBuiltin(
					"struct_set",
					{ map_name, "display_text", new_map_name.c_str() }
				);
			}
		}

		// Return since we do not want the orignal behavior to run.
		return empty_result;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
		g_ArSelfModule,
		NORTH_ARROW_TAP_SCRIPT
	));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

/**
* A hook for the selection_location script of the map menu.
* 
* We need this hook because when the location change all our injected event callback get removed.
* Therefore here is a good place to re-inject them all again.
*/
RValue& SelectLocationHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	//g_ModuleInterface->Print(CM_LIGHTGREEN, "[NameThatMistrian %s] - SelectLocationHook called!", VERSION);

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
		g_ArSelfModule,
		SELECT_LOCATION_SCRIPT
	));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	// Everytime the map/location in the minimap changes, we lose the event callback,
	// so we are re-registering them.
	MapMenu = Self;
	Setup(MapMenu);

	return Result;
}

#pragma region Plugin_Lifecycle_and_Hook_Creations
void RegisterHook(OUT AurieStatus& status, IN const char* script_name, IN std::string_view hook_identifier, IN PVOID destination_function) {
	CScript* can_mount_ptr = nullptr;

	status = g_ModuleInterface->GetNamedRoutinePointer(script_name, reinterpret_cast<PVOID*>(&can_mount_ptr));
	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[NameThatMistrian %s] - Failed to get script (%s)!", VERSION, script_name);
	}

	status = MmCreateHook(
		g_ArSelfModule,
		hook_identifier,
		can_mount_ptr->m_Functions->m_ScriptFunction,
		destination_function,
		nullptr
	);

	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[NameThatMistrian %s] - Failed to create hook for '%s'!", VERSION, script_name);
	}
}

/**
* Register multiple hooks all in one go.
* 
* If any fail, we return early with the error, so the caller can terminate the plugin.
* 
* Each entry in the hook is
* - 1st element of the tuple = the script name
* - 2nd = the hook identifier (usually the same as the script name)
* - 3rd = the hook to be called instead of the original function.
*/
void RegisterHooks(OUT AurieStatus& status, std::vector<std::tuple<const char*, std::string_view, PVOID>> hooks) {
	for (size_t i = 0; i < hooks.size(); i++) {
		RegisterHook(status, std::get<0>(hooks[i]), std::get<1>(hooks[i]), std::get<2>(hooks[i]));
		if (!AurieSuccess(status)) {
			break;
		}
	}
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	g_ModuleInterface = YYTK::GetInterface();
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[NameThatMistrian %s] - Plugin starting ...", VERSION);

	RegisterHooks(last_status, {
		{ SPAWN_MENU_SCRIPT, SPAWN_MENU_SCRIPT, SpawnMenuHook },
		{ NORTH_ARROW_TAP_SCRIPT, NORTH_ARROW_TAP_SCRIPT, NorthArrowTapHook },
		{ SELECT_LOCATION_SCRIPT, SELECT_LOCATION_SCRIPT, SelectLocationHook },
	});
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[NameThatMistrian %s] - Exiting due to failure on start!", VERSION);
		return last_status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[NameThatMistrian %s] - Plugin started!", VERSION);

	return last_status;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}
#pragma endregion