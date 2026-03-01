# Flash lcars-esp32 hello_lcars demo to board
# Usage: .\flash.ps1 [board]
#   board: waveshare147 (default) | tdisplays3

param(
    [ValidateSet("waveshare147", "tdisplays3")]
    [string]$Board = "waveshare147"
)

$ErrorActionPreference = "Stop"

Write-Host "Flashing hello_lcars demo to $Board..." -ForegroundColor Cyan
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e $Board -t upload -d $PSScriptRoot

if ($LASTEXITCODE -eq 0) {
    Write-Host "Flash complete!" -ForegroundColor Green
} else {
    Write-Host "Flash failed (exit code $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}
