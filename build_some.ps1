#!/usr/bin/env pwsh
param(
    [string]$InputFile
)

# Get the directory of this script
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Compute relative path of input file to $ScriptDir/src
$FullInput = (Resolve-Path $InputFile).Path
$BasePath  = (Resolve-Path (Join-Path $ScriptDir "src")).Path
$RelPath   = $FullInput.Substring($BasePath.Length).TrimStart('\','/')

# Split into chapter and demo
$PathParts = $RelPath -split '[\\/]'   # works with / or \
$Chapter   = $PathParts[0]
$Demo      = $PathParts[1]

# Go to script dir and prepare build
Set-Location $ScriptDir
if (-not (Test-Path "build")) {
    New-Item "build" -ItemType Directory | Out-Null
}
Set-Location build

# Run cmake with environment variables
$env:INPUT_CHAPTER = $Chapter
$env:INPUT_DEMO    = $Demo
cmake ..
cmake --build .

cd ..\bin\${Chapter}\Debug
$ ".\${Chapter}__${Demo}"