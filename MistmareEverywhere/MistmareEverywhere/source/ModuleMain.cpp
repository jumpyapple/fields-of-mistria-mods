#include <YYToolkit/YYTK_Shared.hpp>

using namespace Aurie;
using namespace YYTK;

static YYTK::YYTKInterface* g_ModuleInterface = nullptr;

static const char* const PLUGIN_NAME = "MistmareEverywhere";
static const char* const VERSION = "0.2.0";
static const char* const SCRIPT_CAN_MOUNT = "gml_Script_can_mount@gml_Object_obj_ari_Create_0";

RValue& CanMountHook(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
		g_ArSelfModule,
        SCRIPT_CAN_MOUNT
	));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	Result.m_Real = 1.0;
	return Result;
}

void CreateCanMountHook(AurieStatus& status)
{
    CScript *can_mount_ptr = nullptr;

	status = g_ModuleInterface->GetNamedRoutinePointer(SCRIPT_CAN_MOUNT, reinterpret_cast<PVOID*>(&can_mount_ptr));
	if (!AurieSuccess(status)) {
        DbgPrintEx(LOG_SEVERITY_ERROR, "[%s %s] Failed to get script (%s) with error: %s", PLUGIN_NAME, VERSION, SCRIPT_CAN_MOUNT, AurieStatusToString(status));
        return;
	}

	status = MmCreateHook(
		g_ArSelfModule,
        SCRIPT_CAN_MOUNT,
		can_mount_ptr->m_Functions->m_ScriptFunction,
		CanMountHook,
		nullptr
	);

	if (!AurieSuccess(status)) {
        DbgPrintEx(LOG_SEVERITY_ERROR, "[%s %s] Failed to create hook for '%s' with error: %s", PLUGIN_NAME, VERSION, SCRIPT_CAN_MOUNT, AurieStatusToString(status));
        return;
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

    DbgPrintEx(LOG_SEVERITY_DEBUG, "[%s %s] Plugin starting ...", PLUGIN_NAME, VERSION);

	CreateCanMountHook(last_status);
	if (!AurieSuccess(last_status)) {
        DbgPrintEx(LOG_SEVERITY_ERROR, "[%s %s] Exiting due to failure on start!", PLUGIN_NAME, VERSION);
		return last_status;
	}

    DbgPrintEx(LOG_SEVERITY_DEBUG, "[%s %s] Plugin started!", PLUGIN_NAME, VERSION);
	return AURIE_SUCCESS;
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
