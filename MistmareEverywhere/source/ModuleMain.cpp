#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

static const char* const VERSION = "0.1.0";
static const char* const CAN_MOUNT_SCRIPT = "gml_Script_can_mount@gml_Object_obj_ari_Create_0";


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
		CAN_MOUNT_SCRIPT
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
	CScript* can_mount_ptr = nullptr;

	status = g_ModuleInterface->GetNamedRoutinePointer(CAN_MOUNT_SCRIPT, reinterpret_cast<PVOID*>(&can_mount_ptr));
	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Failed to get script (%s)!", CAN_MOUNT_SCRIPT);
	}

	status = MmCreateHook(
		g_ArSelfModule,
		CAN_MOUNT_SCRIPT,
		can_mount_ptr->m_Functions->m_ScriptFunction,
		CanMountHook,
		nullptr
	);

	if (!AurieSuccess(status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Failed to create hook for '%s'!", CAN_MOUNT_SCRIPT);
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
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	g_ModuleInterface = YYTK::GetInterface();
	if (!g_ModuleInterface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[MistmareEverywhere] - Plugin starting ...");

	CreateCanMountHook(last_status);
	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareEverywhere] - Exiting due to failure on start!");
		return last_status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[MistmareEverywhere] - Plugin started!");
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