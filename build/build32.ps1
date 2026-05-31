$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvars32.bat"

if (-not (Test-Path $vcvarsPath)) {
    Write-Error "vcvars32.bat not found at: $vcvarsPath"
    exit 1
}

$output = & cmd /c "`"$vcvarsPath`" >nul 2>&1 && set" 2>&1
foreach ($line in $output) {
    if ($line -match '^([^=]+)=(.*)$') {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}

$srcDir = "d:\EvPlugins\src"
$outDir = "d:\EvPlugins\bin\x86"
$dllName = "ev_rclone.dll"

if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}

Write-Host "Compiling $dllName (32-bit)..." -ForegroundColor Cyan

$sources = @(
    "ev_rclone.c",
    "ev_http_client.c",
    "ev_json.c",
    "ev_rclone_rc.c",
    "ev_rclone_config.c",
    "ev_string_util.c"
)

$objFiles = @()
$compileFailed = $false

foreach ($src in $sources) {
    $srcPath = Join-Path $srcDir $src
    $objName = [System.IO.Path]::GetFileNameWithoutExtension($src) + ".obj"

    Write-Host "  Compiling $src..." -ForegroundColor Gray

    $result = & cl /nologo /c /O2 /W3 /DWIN32 /D_WINDOWS /D_USRDLL /DNDEBUG /D_CRT_SECURE_NO_WARNINGS /I$srcDir $srcPath /Fo$objName 2>&1

    if ($LASTEXITCODE -ne 0) {
        Write-Host "  FAILED: $src" -ForegroundColor Red
        Write-Host ($result -join "`n")
        $compileFailed = $true
        break
    }

    $objFiles += $objName
}

if ($compileFailed) {
    Write-Error "Compilation failed."
    Get-ChildItem "*.obj" -ErrorAction SilentlyContinue | Remove-Item -Force
    exit 1
}

Write-Host "Linking $dllName..." -ForegroundColor Cyan

$linkArgs = @("/nologo", "/DLL", "/OUT:$outDir\$dllName", "/DEF:$srcDir\ev_rclone.def") + $objFiles + @("ws2_32.lib", "user32.lib", "comdlg32.lib", "ole32.lib")
$linkResult = & link @linkArgs 2>&1

if ($LASTEXITCODE -ne 0) {
    Write-Error "Linking failed."
    Write-Host ($linkResult -join "`n")
    Get-ChildItem "*.obj" -ErrorAction SilentlyContinue | Remove-Item -Force
    exit 1
}

Get-ChildItem "*.obj" -ErrorAction SilentlyContinue | Remove-Item -Force

Write-Host ""
Write-Host "Build succeeded: $outDir\$dllName" -ForegroundColor Green
