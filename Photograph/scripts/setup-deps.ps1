$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$depsRoot = Join-Path $projectRoot '.deps'
$downloadRoot = Join-Path $depsRoot 'downloads'
New-Item -ItemType Directory -Path $downloadRoot -Force | Out-Null
$packages = @(
    @{ Name = 'glfw-3.4.bin.WIN64.zip'; Folder = 'glfw-3.4.bin.WIN64'; Url = 'https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip'; Hash = '54EFA829400F2A0537F742B2B3BDD74E437BB4F2F048E4B7D3C5557D11A611E6' },
    @{ Name = 'glfw-3.4.bin.WIN32.zip'; Folder = 'glfw-3.4.bin.WIN32'; Url = 'https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN32.zip'; Hash = '59AF286ED9E1422D512E2B26CCB0C3100C600277279D305E3DF6D64A74728F8A' },
    @{ Name = 'glm-1.0.1.zip'; Folder = 'glm-1.0.1'; Url = 'https://codeload.github.com/g-truc/glm/zip/refs/tags/1.0.1'; Hash = '09C5716296787E1F7FCB87B1CBDBF26814EC1288ED6259CCD30D5D9795809FA5' }
)
foreach ($package in $packages) {
    $archivePath = Join-Path $downloadRoot $package.Name
    if (!(Test-Path -LiteralPath $archivePath)) {
        Write-Host "Downloading $($package.Name)"
        Invoke-WebRequest -Uri $package.Url -OutFile $archivePath -UseBasicParsing
    }
    if ((Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash -ne $package.Hash) {
        throw "SHA256 mismatch: $archivePath. Remove this archive and retry."
    }
    Expand-Archive -LiteralPath $archivePath -DestinationPath $depsRoot -Force
    if (!(Test-Path -LiteralPath (Join-Path $depsRoot $package.Folder))) {
        throw "Extraction failed: $($package.Name)"
    }
}
Write-Host 'GLFW 3.4 and GLM 1.0.1 ready. Open Photograph.slnx and build x64 Debug.'
