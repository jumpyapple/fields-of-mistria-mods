#include <YYToolkit/YYTK_Shared.hpp>
#include "Dumper.hpp"

using namespace Aurie;
using namespace YYTK;

static const char* const ON_DRAW_GUI_SCRIPT = "gml_Script_on_draw_gui@Display@Display";
static const char* const SPAWN_MENU_SCRIPT = "gml_Script_spawn_menu@Anchor@Anchor";

static const bool SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY = false;
static bool SHOULD_DUMP = false;

static uint64_t event_count_1 = 0;
static uint64_t event_count_2 = 0;

void RegisterHook(OUT AurieStatus& status, IN const char* script_name, IN std::string_view hook_identifier, IN PVOID destination_function) {
	CScript* can_mount_ptr = nullptr;

	status = g_ModuleInterface->GetNamedRoutinePointer(script_name, reinterpret_cast<PVOID*>(&can_mount_ptr));
	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Failed to get script (%s)!", script_name);
	}

	status = MmCreateHook(
		g_ArSelfModule,
		hook_identifier,
		can_mount_ptr->m_Functions->m_ScriptFunction,
		destination_function,
		nullptr
	);

	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Failed to create hook for '%s'!", script_name);
	}
}

void RegisterHooks(OUT AurieStatus& status, std::vector<std::tuple<const char*, std::string_view, PVOID>> hooks) {
	for (size_t i = 0; i < hooks.size(); i++) {
		RegisterHook(status, std::get<0>(hooks[i]), std::get<1>(hooks[i]), std::get<2>(hooks[i]));
		if (!AurieSuccess(status)) {
			break;
		}
	}
}

void EventObjectCallback(IN FWCodeEvent& CallContext)
{
	auto& [self, other, code, argc, args] = CallContext.Arguments();

	if ((GetAsyncKeyState('B') & 1)) {
		SHOULD_DUMP = !SHOULD_DUMP;
		g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - SHOULD_DUMP = %d  ...", VERSION, SHOULD_DUMP);

		if (SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY) {
			CInstance* global_instance = nullptr;
			AurieStatus status = g_ModuleInterface->GetGlobalInstance(&global_instance);
			if (!AurieSuccess(status)) {
				return;
			}

			g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - Dumping instance ...", VERSION);

			RValue instance = global_instance->ToRValue();
			DumpInstance(instance, "global_instance_dumps");

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

	if (SHOULD_DUMP) {
		DumpHookVariables("on_draw_gui", event_count_1, Self, Other, Result, ArgumentCount, Arguments);
		event_count_1++;
	}

	return Result;
}

RValue& SpawnMenuHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
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

	if (SHOULD_DUMP) {
		DumpHookVariables("spawn_menu", event_count_2, Self, Other, Result, ArgumentCount, Arguments);
		event_count_2++;
	}

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
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	g_ModuleInterface = YYTK::GetInterface();
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[Dumper %s] - Plugin starting ...", VERSION);

	RegisterHooks(last_status, {
		{ ON_DRAW_GUI_SCRIPT, ON_DRAW_GUI_SCRIPT, OnDrawGuiHook },
		{ SPAWN_MENU_SCRIPT, SPAWN_MENU_SCRIPT, SpawnMenuHook }
	});

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_OBJECT_CALL,
		EventObjectCallback,
		0
	);
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Exiting due to failure on start!");
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