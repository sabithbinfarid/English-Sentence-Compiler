$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    throw "gcc is not installed or not in PATH."
}

$bisonCmd = $null
$flexCmd = $null

if (Get-Command win_bison -ErrorAction SilentlyContinue) {
    $bisonCmd = "win_bison"
} elseif (Get-Command bison -ErrorAction SilentlyContinue) {
    $bisonCmd = "bison"
} else {
    throw "Neither win_bison nor bison is installed or available in PATH."
}

if (Get-Command win_flex -ErrorAction SilentlyContinue) {
    $flexCmd = "win_flex"
} elseif (Get-Command flex -ErrorAction SilentlyContinue) {
    $flexCmd = "flex"
} else {
    throw "Neither win_flex nor flex is installed or available in PATH."
}

# GNUWin32 bison can fail on Windows when its data path includes spaces/parentheses.
# Only apply this fallback when using classic bison, not win_bison.
if (($IsWindows -or $env:OS -eq "Windows_NT") -and $bisonCmd -eq "bison") {
    $gnuRootShort = "C:\PROGRA~2\GnuWin32"
    if (Test-Path "$gnuRootShort\share\bison") {
        $env:BISON_PKGDATADIR = "$gnuRootShort\share\bison"
    }
    if (Test-Path "$gnuRootShort\bin\m4.exe") {
        $env:M4 = "$gnuRootShort\bin\m4.exe"
    }
}

Write-Host "Using parser generator: $bisonCmd"
Write-Host "Using lexer generator:  $flexCmd"

& $bisonCmd -d -o src/parser.tab.c src/parser.y
& $flexCmd src/lexer.l

gcc -w -Isrc -o blockc src/parser.tab.c lex.yy.c -lm

Write-Host "Build successful: blockc"
