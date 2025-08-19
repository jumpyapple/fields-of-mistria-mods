# Maintenance

## Packaging for MOMI

1. Use `subst.exe` to create a virtual drive. `subst.exe R: C:\Users\<username>`.
2. Change the directory to this mod folder.
3. Run `./package.ps1`.
4. The zip file that is MOMI compatible can be found in the `./release/zip/` folder.

## Building a map from sprite name to NPC's ID

After exporting sprites from UMT, we can use the following PowerShell script
to get the sprite names.

```powershell
$filenames = Get-ChildItem -Filter "spr_ui_generic_icon_npc_small_*.png" | Select-Object Name
foreach ($filename in $filenames) {
    if (!$filename.Name.Contains("_0.png")) {
        Continue
    } 
    $sprite_name = $filename.Name -replace "_\d.png", ""
    $output = "{{ `"{0}`",  }}," -f $sprite_name
    Write-output $output
}
```