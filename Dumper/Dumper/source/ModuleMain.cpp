#include <YYToolkit/YYTK_Shared.hpp>
#include "common.hpp"
#include "Dumper.hpp"
#include "RegisterHook.hpp"

using namespace Aurie;
using namespace YYTK;

static const char* const ON_DRAW_GUI_SCRIPT = "gml_Script_on_draw_gui@Display@Display";

static const bool SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY = false;

static uint64_t event_count_1 = 0;
static uint64_t event_count_2 = 0;

#pragma region MapMenu stuff
static const char* const SPAWN_MENU_SCRIPT = "gml_Script_spawn_menu@Anchor@Anchor";

static RValue MapMenu;
static RValue map_name;

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

RValue& SpawnMenuHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Dumper %s] - SpawnMenuHook called!", VERSION);

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

	/*if (g_SHOULD_DUMP) {
		Dumper::DumpHookVariables(g_ModuleInterface, "spawn_menu", event_count_1, Self, Other, Result, ArgumentCount, Arguments);
		event_count_1++;
	}*/

	return Result;
}
#pragma endregion

void EventObjectCallback(IN FWCodeEvent& CallContext)
{
	auto& [self, other, code, argc, args] = CallContext.Arguments();

	// Check for F3 key.
	if ((GetAsyncKeyState(VK_F3) & 1)) {
		g_SHOULD_DUMP = !g_SHOULD_DUMP;
		g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - g_SHOULD_DUMP = %d  ...", VERSION, g_SHOULD_DUMP);

		if (SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY) {
			CInstance* global_instance = nullptr;
			AurieStatus status = g_ModuleInterface->GetGlobalInstance(&global_instance);
			if (!AurieSuccess(status)) {
				return;
			}

			g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - Dumping instance ...", VERSION);

			RValue instance = global_instance->ToRValue();
			Dumper::DumpRValueWithDefaultIndexFilename(g_ModuleInterface, instance, "global_instance_dumps");

			g_ModuleInterface->Print(CM_GREEN, "[Dumper %s] - Done dumping!", VERSION);
		}
	}
}

RValue& OnDrawGuiHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
		g_ArSelfModule,
		ON_DRAW_GUI_SCRIPT
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
	//UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	g_ModuleInterface = YYTK::GetInterface();
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - Plugin starting (path: %s) ...", VERSION, ModulePath.c_str());

	RegisterHooks(last_status, {
		{ SPAWN_MENU_SCRIPT, SPAWN_MENU_SCRIPT, SpawnMenuHook },
	});

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_OBJECT_CALL,
		EventObjectCallback,
		0
	);
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[Dumper %s] - Exiting due to failure on start!", VERSION);
		return last_status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Dumper %s] - Plugin started!", VERSION);

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