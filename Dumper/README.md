# Dumper

> Dump the details of the given instance into HTML files.

Provide `DumpInstance` function that creates HTML files with variables grouped into cetegory. It will also attempt to
find the name of the contructor of the struct with `instanceof`. For any field that is named `sprite`, it will
also attempt to get the sprite name via `sprite_get_name`

It may take a while to dump the global instance and there will also be more than 300k files generated. If you want to save
disk space, you can compress all the HTML files and run a web server that support compressed static file serving. One such
web server is [Static Web Server](https://static-web-server.net/).

## Installation

1. Get MOMI either from [its Nexusmod's page](https://www.nexusmods.com/fieldsofmistria/mods/78) or [its GitHub release page](https://github.com/Garethp/Mods-of-Mistria-Installer/releases)
2. Follow the instructions of MOMI to set it up.
3. Download the zip file and drop it into the `mods` folder.
4. Run MOMI and install the mod.

## Credits

- [Archie_UwU](https://github.com/Archie-osu) for [YYToolkit](https://github.com/AurieFramework/YYToolkit), [Aurie framework](https://github.com/AurieFramework/Aurie) and the intial version of the dumper code.
- [DeUloO](https://github.com/DeUloO) for the dumper code that is updated to the later version of the framework.
- [AnnaNomoly](https://github.com/AnnaNomoly) for [a Mod starter template](https://github.com/AnnaNomoly/YYToolkit)
- [christianspecht's answer](https://stackoverflow.com/a/14792225/) on StackOverflow for a packaging script for MOMI
