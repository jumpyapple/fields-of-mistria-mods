#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

void FrameCallback(FWFrame& FrameContext) {
	UNREFERENCED_PARAMETER(FrameContext);

	static uint32_t frame_counter = 0;

	if (frame_counter % 30 == 0)
		g_ModuleInterface->PrintWarning("[MistmareIsEverywhere] - 30 frames have passed! Framecount: %u", frame_counter);

	frame_counter++;
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

	g_ModuleInterface->Print(CM_LIGHTAQUA, "[MistmareIsEverywhere] - Plugin starting ...");

	last_status = g_ModuleInterface->CreateCallback(Module, EVENT_FRAME, FrameCallback, 0);

	if (!AurieSuccess(last_status)) {
		g_ModuleInterface->Print(CM_LIGHTRED, "[MistmareIsEverywhere] - Failed to register a callback!");
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[MistmareIsEverywhere] - Plugin started!");

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