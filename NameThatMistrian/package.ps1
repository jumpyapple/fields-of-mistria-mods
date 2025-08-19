param (
    [switch]$NoCreateZip
)

# From https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022#developer-powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1'

# Launch-VsDevShell.ps1 changes the working directory to ~/source/repos/
Set-Location -Path $PSScriptRoot

if ($NoCreateZip) {
    msbuild package.proj /p:CreateZip="0"
} else {
    msbuild package.proj /p:CreateZip="1"
}
