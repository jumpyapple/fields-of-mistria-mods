using AurieSharpInterop;
using System.Linq.Expressions;
using System.Xml.Linq;
using YYTKInterop;

namespace NameThatMistrianSharp
{
    public static class ThirdPartyClassExtensions
    {
        public static GameVariable ToGameVariable(this GameVariable[] instance)
        {
            GameVariable[] args = { instance.Length };
            var arr = Game.Engine.CallFunction("array_create", args);

            for (int i = 0; i < instance.Length; i++)
            {
                GameVariable[] as_args = { arr, i, instance[i] };
                Game.Engine.CallFunction("array_set", as_args);
            }
            return arr;
        }

        /*public static GameVariable ToGameVariable(this Dictionary<string, GameVariable> instance)
        {
            GameVariable[] args = { "a", 3 };
            var st = Game.Engine.CallScript("gml_Script_array_to_struct", args.ToGameVariable());

            foreach (KeyValuePair<string, GameVariable> entry in instance)
            {
                
            }
            return st;
        }*/
    }

    internal static class NameThatMistrianMod
    {
        static string OriginalMapName = "";
        static GameVariable? NameTextNode = null;

        public static AurieStatus InitializeMod(AurieManagedModule Module)
        {
            Framework.Print("Initializing mod ...");
            Game.Events.AddPostScriptNotification(Module, "gml_Script_spawn_menu@Anchor@Anchor", SpawnMenuCallback);
            Game.Events.AddPostScriptNotification(Module, "gml_Script_select_location@MapMenu@MapMenu", SelectLocationCallback);
            Game.Events.AddPreScriptNotification(Module, "gml_Script_anon@9836@MapMenu@MapMenu", NorthArrowTapCallback);
            Framework.Print("Successfully initialized the mod");
            return AurieStatus.Success;
        }

        public static void UnloadMod(AurieManagedModule Module)
        {
        }

        public static void InjectTapCallback(GameVariable MapMenu, GameVariable OriginalOnTapFunction)
        {
            var map = MapMenu["map"];
            foreach (var positional_node in map["children"].ToArrayView())
            {
                foreach (var sprite_node in positional_node["children"].ToArrayView())
                {
                    var sprite = sprite_node["sprite"];
                    if (sprite.Type == "ref")
                    {
                        GameVariable[] args = { sprite };
                        var result = Game.Engine.CallFunction("sprite_get_name", args);

                        if (result.Type == "string")
                        {
                            result.TryGetString(out string sprite_name);
                            string name = "";

                            if (sprite_name.IndexOf("icon_npc") != -1)
                            {
                                name = sprite_name.Substring(30);

                                if (name == "player")
                                {
                                    name = "me";
                                }
                            }
                            else if (sprite_name.IndexOf("icon_pet") != -1)
                            {
                                name = "pet";
                            }

                            GameVariable[] arg_array = { 30, name };

                            Dictionary<string, GameVariable> our_tap_event_callback = new();
                            our_tap_event_callback.Add("arg_array", arg_array.ToGameVariable());
                            our_tap_event_callback.Add("func", OriginalOnTapFunction);

                            var target_event_callbacks = sprite_node["event_callbacks"];
                            // NOTE: This feature is not in AurieSharp yet.
                            target_event_callbacks["tap"] = new GameVariable(our_tap_event_callback);

                            sprite_node["listens_for_taps"] = true;
                            sprite_node["listens_for_hovers"] = true;
                        }
                    }
                }
            }
        }

        public static void NorthArrowTapCallback(ScriptExecutionContext Context)
        {
            var func_args = Context.Arguments.ToArray();
            if (func_args.Length == 2)
            {
                if (func_args[0].ToInt64() == 30)
                {
                    var name_id = func_args[1].ToString();
                    string name;

                    if (name_id != "me" && name_id != "pet" && name_id != "unknown")
                    {
                        if (false)
                        {
                            //name = get_npc_name(name_id);
                        }
                        else
                        {
                            name = name_id;
                        }
                    }
                    else
                    {
                        name = name_id;
                    }

                    GameVariable new_map_name = OriginalMapName;
                    new_map_name += " - ";
                    new_map_name += name;

                    if (NameTextNode != null)
                    {
                        NameTextNode["display_text"] = new_map_name;
                    }

                    Context.OverrideResult(new GameVariable(true));
                }
            }
        }

        public static void Setup(GameVariable Menu)
        {
            Framework.Print("Setting up ...");
            try
            {
                var func = Menu["north_arrow"]["event_callbacks"]["tap"]["func"];
                var map_text = Menu["name"]["text"];
                var text_node = Menu["name"];

                NameTextNode = text_node;
                OriginalMapName = map_text.ToString();

                InjectTapCallback(Menu, func);
            }
            catch (Exception e)
            {
                Framework.Print("Failed during setup!");
            }
        }

        public static void SpawnMenuCallback(ScriptExecutionContext Context)
        {
            var result = Context.GetResult();
            if (result.Type == "struct MapMenu")
            {
                Setup(result);
            }
        }

        public static void SelectLocationCallback(ScriptExecutionContext Context)
        {
            Setup(new GameVariable(Context.Self));
        }
    }
}
