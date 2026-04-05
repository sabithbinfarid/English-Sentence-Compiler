$ErrorActionPreference = "Stop"

$rootScript = Join-Path $PSScriptRoot "..\build_flexbison.ps1"

if (-not (Test-Path $rootScript)) {
    throw "Could not find root build script at $rootScript"
}

& $rootScript
