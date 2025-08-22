#include <YYToolkit/YYTK_Shared.hpp>
#include "common.hpp"
#include "Dumper_shared.hpp"
#include "RegisterHook.hpp"

using namespace Aurie;
using namespace YYTK;

static bool g_SHOULD_DUMP = false;
static uint64_t event_count_1 = 0;
// This is set to false to not accidentally dump the global instance.
static bool SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY = false;

static const char* const SPAWN_MENU_SCRIPT = "gml_Script_spawn_menu@Anchor@Anchor";
static const char* const ADD_TITLE_SCRIPT = "gml_Script_add_title@PopupMenu@PopupMenu";

DEFINE_DUMPING_HOOK_FUNCTION(SpawnMenuHook, PLUGIN_NAME, "spawn_menu", SPAWN_MENU_SCRIPT, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(AddTitleHook, PLUGIN_NAME, "add_title", ADD_TITLE_SCRIPT, event_count_1);

void EventObjectCallback(IN FWCodeEvent& CallContext)
{
	auto& [self, other, code, argc, args] = CallContext.Arguments();

	// Check for F3 key.
	if ((GetAsyncKeyState(VK_F3) & 1)) {
		g_SHOULD_DUMP = !g_SHOULD_DUMP;
		g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - g_SHOULD_DUMP = %d  ...", PLUGIN_NAME, VERSION, g_SHOULD_DUMP);

		if (SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY) {
			CInstance* global_instance = nullptr;
			AurieStatus status = g_ModuleInterface->GetGlobalInstance(&global_instance);
			if (!AurieSuccess(status)) {
				return;
			}

			g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Dumping instance ...", PLUGIN_NAME, VERSION);

			RValue instance = global_instance->ToRValue();
			// Calling the function in the Dumper.dll. The dll has to be loaded by Aurie.
			Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, instance, "global_instance_dumps");

			g_ModuleInterface->Print(CM_GREEN, "[%s %s] - Done dumping!", PLUGIN_NAME, VERSION);
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
	//UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	g_ModuleInterface = YYTK::GetInterface();
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Plugin starting (path: %s) ...", PLUGIN_NAME, VERSION, ModulePath.c_str());

	RegisterHooks(last_status, {
		{ SPAWN_MENU_SCRIPT, SPAWN_MENU_SCRIPT, SpawnMenuHook },
		{ ADD_TITLE_SCRIPT, ADD_TITLE_SCRIPT, AddTitleHook },
		});
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[%s %s] - Exiting due to failure on start!", PLUGIN_NAME, VERSION);
		return last_status;
	}

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_OBJECT_CALL,
		EventObjectCallback,
		0
	);
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[%s %s] - Exiting due to failure on start!", PLUGIN_NAME, VERSION);
		return last_status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[%s %s] - Plugin started!", PLUGIN_NAME, VERSION);

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
