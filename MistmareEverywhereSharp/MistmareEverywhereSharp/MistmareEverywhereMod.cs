using AurieSharpInterop;
using YYTKInterop;

namespace MistmareEverywhereSharp
{
    internal static class MistmareEverywhereMod
    {
        public static AurieStatus InitializeMod(AurieManagedModule Module)
        {
            Game.Events.AddPostScriptNotification(Module, "gml_Script_can_mount@gml_Object_obj_ari_Create_0", CanMountCallback);
            return AurieStatus.Success;
        }

        public static void CanMountCallback(ScriptExecutionContext Context)
        {
            Context.OverrideResult(true);
        }

        public static void UnloadMod(AurieManagedModule Module)
        {
        }
    }
}
