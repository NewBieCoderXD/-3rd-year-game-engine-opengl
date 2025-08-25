#!/usr/bin/env pwsh

# Get the directory of the current script
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Change to the script directory
Set-Location $ScriptDir

# Create build directory if it doesn't exist
$BuildDir = Join-Path $ScriptDir "build"
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Enter build directory
Set-Location $BuildDir

# Run CMake
cmake ..
cmake --build .
