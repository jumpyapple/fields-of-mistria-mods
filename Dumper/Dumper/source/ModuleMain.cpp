#include <YYToolkit/YYTK_Shared.hpp>
#include "Dumper.hpp"

using namespace Aurie;
using namespace YYTK;

static const char* const CAN_MOUNT_SCRIPT = "gml_Script_can_mount@gml_Object_obj_ari_Create_0";

void EventObjectCallback(IN FWCodeEvent& CallContext)
{
	auto& [self, other, code, argc, args] = CallContext.Arguments();

	if (GetAsyncKeyState('B') & 1) {
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