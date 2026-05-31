$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$envOutput = & cmd /c "`"$vsPath\VC\Auxiliary\Build\vcvars64.bat`" >nul 2>&1 && set" 2>&1
foreach ($line in $envOutput) {
    if ($line -match '^([^=]+)=(.*)$') {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}

$result = & dumpbin /exports d:\EvPlugins\bin\x64\ev_rclone.dll 2>&1
$result | ForEach-Object { Write-Host $_ }
