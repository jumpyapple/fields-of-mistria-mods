#include <fstream>
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

//static const char* const SCRIPT_SPAWN_MENU = "gml_Script_spawn_menu@Anchor@Anchor";
//static const char* const SCRIPT_ADD_TITLE = "gml_Script_add_title@PopupMenu@PopupMenu";
//static const char* const SCRIPT_REBUILD_ARI = "gml_Script_rebuild_ari@CustomizationMenu@CustomizationMenu";
//static const char* const SCRIPT_REQUEST_SHOW = "gml_Script_request_show@AnchorMenu@AnchorMenu";
//static const char* const SCRIPT_SETUP_LEFT_PAGE = "gml_Script_setup_left_page@CustomizationMenu@CustomizationMenu";

//static const char* const SCRIPT_GET_MENU = "gml_Script_get_menu@Anchor@Anchor";
//static const char* const SCRIPT_HOVER_NODE = "gml_Script_hover_node@Anchor@Anchor";
// Does not seems to work?
//static const char* const SCRIPT_TAP_PRESSED = "gml_Script_tap_pressed@Anchor@Anchor";
//static const char* const SCRIPT_POINT_IN_NODE = "gml_Script_point_in_node@Anchor@Anchor";

// Seems to be happending when the game want to tap the node.
//static const char* const SCRIPT_TAP_NODE = "gml_Script_tap_node@Anchor@Anchor";

// This does not get call when the node does not take tap.
//static const char* const SCRIPT_TAKE_TAP = "gml_Script_take_tap@Anchor@Anchor";
//static const char* const SCRIPT_REGISTER_NODE = "gml_Script_register_node@Anchor@Anchor";

// What in the Mistria is a Pilot.
//static const char* const SCRIPT_PILOT_ADD = "gml_Script_add@Pilot@Pilot";
//static const char* const SCRIPT_TRY_PILOT_HOVER = "gml_Script_try_pilot_hover@Anchor@Anchor";

//static const char* const SCRIPT_GET_RELATIVE_POSITION = "gml_Script_get_relative_position@Anchor@Anchor";

//static const char* const SCRIPT_GET_SCREEN_POSITION = "gml_Script_get_screen_position@Anchor@Anchor";

// Creating node?
//static const char* const SCRIPT_SPRITE_NODE = "gml_Script_SpriteNode";

// Change schedule location?
//static const char* const SCRIPT_SCHEDULE_CURRENT_DESTINATION = "gml_Script_schedule_current_destination@T2r@T2r";
//static const char* const SCRIPT_TRELLIS_POINT_POINT = "gml_Script_point@TrellisPoints@TrellisPoint";

//static const char* const SCRIPT_BUGGER_MENU = "gml_Script_BuggerMenu";
//static const char* const SCRIPT_GLBOAL_BUGGER_MENU = "gml_GlobalScript_BuggerMenu";
//static const char* const SCRIPT_DISPLAY_HELP_MESSAGE = "gml_Script_display_help_message@__BuggerCommand@Bugger";

//static const char* const SCRIPT_HEARTBEAT_PING = "gml_Script_heartbeat_ping@anon@1008@TcpListener@TcpListener";

//DEFINE_DUMPING_HOOK_FUNCTION(SpawnMenuHook, PLUGIN_NAME, "spawn_menu", SCRIPT_SPAWN_MENU, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(AddTitleHook, PLUGIN_NAME, "add_title", SCRIPT_ADD_TITLE, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(RebuildAriHook, PLUGIN_NAME, "rebuild_ari", SCRIPT_REBUILD_ARI, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(RequestShowHook, PLUGIN_NAME, "request_show", SCRIPT_REQUEST_SHOW, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(SetupLeftPageHook, PLUGIN_NAME, "setup_left_page", SCRIPT_SETUP_LEFT_PAGE, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(GetMenuHook, PLUGIN_NAME, "get_menu", SCRIPT_GET_MENU, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(HoverNodeHook, PLUGIN_NAME, "hover_node", SCRIPT_HOVER_NODE, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(TapNodeHook, PLUGIN_NAME, "tap_node", SCRIPT_TAP_NODE, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(TapPressedHook, PLUGIN_NAME, "tap_pressed", SCRIPT_TAP_PRESSED, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(PointInNodeHook, PLUGIN_NAME, "point_in_node", SCRIPT_POINT_IN_NODE, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(TakeTapHook, PLUGIN_NAME, "take_tap", SCRIPT_TAKE_TAP, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(RegisterNodeHook, PLUGIN_NAME, "register_node", SCRIPT_REGISTER_NODE, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(PilotAddHook, PLUGIN_NAME, "pilot_add", SCRIPT_PILOT_ADD, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(TryPilotHoverHook, PLUGIN_NAME, "try_pilot_hover", SCRIPT_TRY_PILOT_HOVER, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(SpriteNodeHook, PLUGIN_NAME, "sprite_node", SCRIPT_SPRITE_NODE, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(GetRelativePositionHook, PLUGIN_NAME, "get_relative_position", SCRIPT_GET_RELATIVE_POSITION, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(GetScreenPositionHook, PLUGIN_NAME, "get_screen_position", SCRIPT_GET_SCREEN_POSITION, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(ScheduleGetCurrentDestinationHook, PLUGIN_NAME, "schedule_get_current_destination", SCRIPT_SCHEDULE_CURRENT_DESTINATION, event_count_1);
//DEFINE_DUMPING_HOOK_FUNCTION(TrellisPointPoint, PLUGIN_NAME, "trellis_point_point", SCRIPT_TRELLIS_POINT_POINT, event_count_1);

//DEFINE_DUMPING_HOOK_FUNCTION(HeartBeatPingHook, PLUGIN_NAME, "heart_beat_ping", SCRIPT_HEARTBEAT_PING, event_count_1);

#pragma region Functions that are used to create a text input
// 1. For the name input that is in the CustomizationMenu, the starting point is `@5699@set_up_fields`.
static const char* const SCRIPT_TAP_CALLBACK_OF_NAME_FIELD = "gml_Script_anon@5699@set_up_fields@CustomizationMenu@CustomizationMenu";

// 2. This function then call this text_input_popup with
// self = __anchor
// other = an instance but nullptr seems fine
// - localization id of for the title
// - the text to be pre-loaded in the text box
// - the max allow length, I think
// - the function that will be set as a callback.
// - nullptr
static const char* const SCRIPT_TEXT_INPUT_POPUP = "gml_Script_text_input_popup";
// 3. This will then be called to create an empty popup menu. It take menu's id which can be viewed in __menu__
static const char* const SCRIPT_ANCHOR_SPAWN_MENU = "gml_Script_spawn_menu@Anchor@Anchor";
// 4. text_input_popup calls this next to setup title with that localization id.
static const char* const SCRIPT_POPUPMENU_ADD_TITLE = "gml_Script_add_title@PopupMenu@PopupMenu";
// 5. It then creates the Confirm button. At this point if we observe the Result, the event callback seems to be set
// to create_button's own callback.
static const char* const SCRIPT_POPUPMENU_CREATE_BUTTON = "gml_Script_create_button@PopupMenu@PopupMenu";

// At somepoint this is called and the menu is shown.
static const char* const SCRIPT_POPUPMENU_SPAWN = "gml_Script_spawn@PopupMenu@PopupMenu";
// At somepoint this initialize got called.
static const char* const SCRIPT_ANCHOR_INITIALIZE = "gml_Script_initialize@AnchorMenu@AnchorMenu";

// What I am missing is when the tap callback got changed and by which function.


// N. This function get call when the Confirm button is tapped. The one and only argument is the text in the text box.
static const char* const SCRIPT_NAME_FIELD_SETUP_FIELDS = "gml_Script_anon@5797@anon@5699@set_up_fields@CustomizationMenu@CustomizationMenu";


//DEFINE_DUMPING_HOOK_FUNCTION(set_up_fields_hook, PLUGIN_NAME, "set_up_fields_hook", SCRIPT_TAP_CALLBACK_OF_NAME_FIELD, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(text_input_popup_hook, PLUGIN_NAME, "text_input_popup_hook", SCRIPT_TEXT_INPUT_POPUP, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(anchor_spawn_menu_hook, PLUGIN_NAME, "anchor_spawn_menu_hook", SCRIPT_ANCHOR_SPAWN_MENU, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(popupmenu_add_title_hook, PLUGIN_NAME, "popupmenu_add_title_hook", SCRIPT_POPUPMENU_ADD_TITLE, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(popupmenu_create_button_hook, PLUGIN_NAME, "popupmenu_create_button_hook", SCRIPT_POPUPMENU_CREATE_BUTTON, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(popupmenu_spawn_hook, PLUGIN_NAME, "popupmenu_spawn_hook", SCRIPT_POPUPMENU_SPAWN, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(anchor_initialize_hook, PLUGIN_NAME, "anchor_initialize_hook", SCRIPT_ANCHOR_INITIALIZE, event_count_1);
DEFINE_DUMPING_HOOK_FUNCTION(name_field_setup_fields_hook, PLUGIN_NAME, "name_field_setup_fields_hook", SCRIPT_NAME_FIELD_SETUP_FIELDS, event_count_1);


static const char* const SCRIPT_HIJACKING_TARGET = "gml_Script_anon@4863@InfoHudMenu@InfoHudMenu";
#pragma endregion

static uint64_t event_counter = 0;
static RValue customization_menu;

bool IsCustomizationMenu(RValue menu) {
    uint16_t has_elements = 0;
    auto members = menu.ToRefMap();

    for (auto& [key, value] : members)
    {
        if (key == "active_par") {
            has_elements |= 1;
        }
        else if (key == "ari") {
            has_elements |= 2;
        }
        else if (key == "birthday_field") {
            has_elements |= 4;
        }
        else if (key == "name_field") {
            has_elements |= 8;
        }
        else if (key == "customization_label") {
            has_elements |= 16;
        }
        else if (key == "pronoun_field") {
            has_elements |= 32;
        }
    }

    return has_elements == 63;
}

bool IsNameInputPopupMenu(RValue menu) {
    uint16_t has_elements = 0;
    auto members = menu.ToRefMap();

    for (auto& [key, value] : members)
    {
        if (key == "name_input") {
            has_elements |= 1;
        }
        else if (key == "buttons") {
            has_elements |= 2;
        }
        else if (key == "title") {
            has_elements |= 4;
        }
        else if (key == "header") {
            has_elements |= 8;
        }
    }

    return has_elements == 15;
}

bool IsInfoHudMenu(RValue menu) {
    uint16_t has_elements = 0;
    auto members = menu.ToRefMap();

    for (auto& [key, value] : members)
    {
        if (key == "journal_pin") {
            has_elements |= 1;
        }
        else if (key == "map_pin") {
            has_elements |= 2;
        }
        else if (key == "weather_icon") {
            has_elements |= 4;
        }
        else if (key == "currency_icon") {
            has_elements |= 8;
        }
    }
    return has_elements == 15;
}


void CreateSimplePopupMenu() {
    CInstance* global_instance = nullptr;
    AurieStatus status = g_ModuleInterface->GetGlobalInstance(&global_instance);
    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(CM_LIGHTAQUA, "[NameThatGift %s] - Failed to get global instance!", VERSION);
        return;
    }

    RValue anchor = global_instance->GetMember("__anchor");

    RValue popup_menu;
    g_ModuleInterface->CallGameScriptEx(popup_menu, SCRIPT_ANCHOR_SPAWN_MENU, anchor.ToInstance(), nullptr, {
        31
        });

    RValue ret1;
    g_ModuleInterface->CallGameScriptEx(ret1, SCRIPT_POPUPMENU_ADD_TITLE, popup_menu.ToInstance(), nullptr, { "items/furniture/lovely_cottage_set/cottage_flooring_ash/name" });

    RValue ret2;
    g_ModuleInterface->CallGameScriptEx(ret1, SCRIPT_POPUPMENU_CREATE_BUTTON, popup_menu.ToInstance(), nullptr, { "misc_local/confirm" });

    RValue ret3;
    g_ModuleInterface->CallGameScriptEx(ret1, SCRIPT_POPUPMENU_SPAWN, popup_menu.ToInstance(), nullptr, { });

    return;
}

YYTK::RValue& OurTextCallbackHook(YYTK::CInstance* Self, YYTK::CInstance* Other, YYTK::RValue& Result, int ArgumentCount, YYTK::RValue** Arguments) {
    g_ModuleInterface->Print(CM_LIGHTGREEN, "called! (argc: %d)", ArgumentCount);
    if (ArgumentCount == 1 && Arguments[0]->m_Kind == VALUE_STRING) {
        std::string text = Arguments[0]->ToString();
        g_ModuleInterface->Print(CM_LIGHTGREEN, "Got '%s'", text.c_str());
        return Result;
    }

    const YYTK::PFUNC_YYGMLScript original = reinterpret_cast<YYTK::PFUNC_YYGMLScript>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, SCRIPT_HIJACKING_TARGET));
    original(Self, Other, Result, ArgumentCount, Arguments);
    
   
    return Result;
}

CScript* TryLookupScriptWithName(IN YYTK::YYTKInterface* g_ModuleInterface, std::string script_name)
{
    Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

    int script_index = 0;
    while (Aurie::AurieSuccess(last_status))
    {
        CScript* script = nullptr;

        last_status = g_ModuleInterface->GetScriptData(
            script_index,
            script
        );

        if (!Aurie::AurieSuccess(last_status))
            break;

        if (script->m_Functions)
        {
            if (script->GetName() == script_name)
                return script;
        }

        script_index++;
    }
    return nullptr;
}

void CreateTextInputPopup() {
    CInstance* global_instance = nullptr;
    AurieStatus status = g_ModuleInterface->GetGlobalInstance(&global_instance);
    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Failed to get global instance!", PLUGIN_NAME, VERSION);
        return;
    }

    // Obtain the function to hijack.
    RValue anchor = global_instance->GetMember("__anchor");
    std::vector<RValue> open_menus = anchor["open_menus"]["__buffer"].ToVector();
    RValue info_hud_menu;
    for (int i = 0; i < open_menus.size(); i++) {
        if (IsInfoHudMenu(open_menus[i])) {
            info_hud_menu = open_menus[i];
        }
    }
    if (info_hud_menu.m_Kind != VALUE_OBJECT) {
        g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Failed to get InfoHudMenu!", PLUGIN_NAME, VERSION);
        return;
    }
    RValue func = info_hud_menu["journal_pin"]["event_callbacks"]["tap"]["func"];

    RValue script_name = "Cutscenes/Heart Events/Hayden/hayden_two_hearts/hayden_two_hearts/13/prompts/1";
    RValue preload_text = "Yup yup";
    RValue len_maybe = 143.0;

    RValue undefined_val;

    RValue ret1;
    g_ModuleInterface->CallGameScriptEx(ret1, SCRIPT_TEXT_INPUT_POPUP, anchor.ToInstance(), nullptr, {
        script_name,
        preload_text,
        len_maybe,
        func,
        undefined_val
        });

    //Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, ret1, "return_value_of_text_input_popup_dumps");
}

RValue& SpawnMenuHook(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
    const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(
        g_ArSelfModule,
        SCRIPT_POPUPMENU_SPAWN
    ));
    original(
        Self,
        Other,
        Result,
        ArgumentCount,
        Arguments
    );

    if (Result.m_Kind != VALUE_UNDEFINED && Result.m_Kind != VALUE_UNSET && Result.m_Kind != VALUE_NULL) {
        if (IsCustomizationMenu(Result)) {
            customization_menu = Result;
        }
    }

    return Result;
}

void SpawningBuggerMenu() {
    AurieStatus status;
    /*
    RValue ref;
    //status = g_ModuleInterface->GetBuiltin("@ref", self, NULL_INDEX, ref);
    status = g_ModuleInterface->CallBuiltinEx(ref, "@ref", nullptr, nullptr, {});

    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(CM_LIGHTRED, "[%s %s] Failed to get @ref", PLUGIN_NAME, VERSION);
    }
    */

    CInstance* global_instance = nullptr;

    status = g_ModuleInterface->GetGlobalInstance(&global_instance);
    if (!AurieSuccess(status)) {
        g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Failed to get global instance!", PLUGIN_NAME, VERSION);
        return;
    }
    RValue anchor = global_instance->GetMember("__anchor");

    RValue bugger_menu;
    g_ModuleInterface->CallGameScriptEx(bugger_menu, SCRIPT_POPUPMENU_SPAWN, anchor.ToInstance(), nullptr, { 4.0 });
    Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, bugger_menu, "spawn_menu_bugger");

    //RValue bugger = global_instance->GetMember("__bugger");

    RValue result;
    g_ModuleInterface->CallGameScriptEx(result, "gml_Script_bugger_initialize", anchor.ToInstance(), nullptr, { bugger_menu });
    Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, result, "bugger_init_result");

    /*
    RValue result = g_ModuleInterface->CallGameScript("gml_Script_Bugger", {});
    Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, result, "global_bugger_menu_result");

    // Crash
    //result = g_ModuleInterface->CallGameScript("gml_Script_bugger_initialize", {});
    //Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, result, "bugger_init_result");


    std::string cmd_str = "water_all";
    RValue cmd = cmd_str.c_str();
    result = g_ModuleInterface->CallGameScript("gml_Script_execute_command@Bugger@Bugger", {cmd});
    Dumper::CallDumpRValueWithDefaultIndexFilename(g_ModuleInterface, result, "bugger_display_help_result");
    */
}

void EventObjectCallback(IN FWCodeEvent& CallContext)
{
    CInstance* global_instance = nullptr;
    auto& [self, other, code, argc, args] = CallContext.Arguments();

    // Check for F3 key.
    if ((GetAsyncKeyState(VK_F3) & 1)) {
        g_SHOULD_DUMP = !g_SHOULD_DUMP;
        g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - g_SHOULD_DUMP = %d  ...", PLUGIN_NAME, VERSION, g_SHOULD_DUMP);

        if (SHOULD_DUMP_GLOBAL_INSTANCE_INTERACTIVELY) {
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
    else if ((GetAsyncKeyState(VK_F4) & 1)) {
        //CreateSimplePopupMenu();
        CreateTextInputPopup();
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

    AurieStatus last_status = AURIE_SUCCESS;

    g_ModuleInterface = YYTK::GetInterface();
    if (!g_ModuleInterface)
        return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    g_ModuleInterface->Print(CM_LIGHTAQUA, "[%s %s] - Plugin starting ...", PLUGIN_NAME, VERSION);

    /* Dump selected function's address, so we can loaded into a debugger.
    std::vector<std::string> func_names = { SCRIPT_BUGGER_MENU };

    std::ofstream out_file;
    out_file.open("_dbg_script_commands.txt");
    for (int i = 0; i < func_names.size(); i++) {
        YYTK::CScript* func_ptr = nullptr;

        last_status = g_ModuleInterface->GetNamedRoutinePointer(func_names[i].c_str(), reinterpret_cast<PVOID*>(&func_ptr));
        if (AurieSuccess(last_status)) {
            out_file << std::format("commentset {:#010x},\"{}\"", reinterpret_cast<intptr_t>(func_ptr->m_Functions->m_ScriptFunction), func_names[i]);
        }
    }
    out_file.close();
    */


    RegisterHooks(last_status, {
    { SCRIPT_HIJACKING_TARGET, SCRIPT_HIJACKING_TARGET, OurTextCallbackHook },

    { SCRIPT_TEXT_INPUT_POPUP, SCRIPT_TEXT_INPUT_POPUP, text_input_popup_hook },
    { SCRIPT_ANCHOR_SPAWN_MENU, SCRIPT_ANCHOR_SPAWN_MENU, anchor_spawn_menu_hook },
    { SCRIPT_POPUPMENU_ADD_TITLE, SCRIPT_POPUPMENU_ADD_TITLE, popupmenu_add_title_hook },
    { SCRIPT_POPUPMENU_CREATE_BUTTON, SCRIPT_POPUPMENU_CREATE_BUTTON, popupmenu_create_button_hook },
    { SCRIPT_POPUPMENU_SPAWN, SCRIPT_POPUPMENU_SPAWN, popupmenu_spawn_hook },
    { SCRIPT_ANCHOR_INITIALIZE, SCRIPT_ANCHOR_INITIALIZE, anchor_initialize_hook },
    { SCRIPT_NAME_FIELD_SETUP_FIELDS, SCRIPT_NAME_FIELD_SETUP_FIELDS, name_field_setup_fields_hook },

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
