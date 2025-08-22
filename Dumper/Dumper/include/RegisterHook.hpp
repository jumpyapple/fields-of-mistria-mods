#pragma once
#include "common.hpp"

void RegisterHook(
    OUT Aurie::AurieStatus& status,
    IN const char* script_name,
    IN std::string_view hook_identifier,
    IN PVOID destination_function
)
{
    YYTK::CScript* can_mount_ptr = nullptr;

    status = g_ModuleInterface->GetNamedRoutinePointer(script_name, reinterpret_cast<PVOID*>(&can_mount_ptr));
    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(YYTK::CM_LIGHTRED, "[Dumper %s] - Failed to get script (%s)!", VERSION, script_name);
        return;
    }

    status = Aurie::MmCreateHook(
        Aurie::g_ArSelfModule,
        hook_identifier,
        can_mount_ptr->m_Functions->m_ScriptFunction,
        destination_function,
        nullptr
    );

    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(YYTK::CM_LIGHTRED, "[Dumper %s] - Failed to create hook for '%s'!", VERSION, script_name);
        return;
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
void RegisterHooks(
    OUT Aurie::AurieStatus& status,
    std::vector<std::tuple<const char*, std::string_view, PVOID>> hooks)
{
    for (size_t i = 0; i < hooks.size(); i++) {
        RegisterHook(status, std::get<0>(hooks[i]), std::get<1>(hooks[i]), std::get<2>(hooks[i]));
        if (!AurieSuccess(status)) {
            break;
        }
    }
}
