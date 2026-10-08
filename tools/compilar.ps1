param(
  [string]$Fqbn='esp32:esp32:esp32',
  [string]$CliPath='',
  [string]$ConfigFile=''
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$bundledCli=Join-Path $PSScriptRoot 'vendor/arduino-cli/arduino-cli.exe'
if(!$CliPath) {
  if(Test-Path -LiteralPath $bundledCli) {
    $CliPath=$bundledCli
    if(!$ConfigFile){$ConfigFile=Join-Path $PSScriptRoot 'vendor/arduino-cli.yaml'}
  } else {$CliPath=(Get-Command arduino-cli -ErrorAction Stop).Source}
}
$buildRoot=Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null
foreach($role in @('tx','rx','noise')) {
  $compileArgs=@('compile','--fqbn',$Fqbn,'--libraries',(Join-Path $projectRoot 'libraries'),
    '--build-path',(Join-Path $buildRoot $role),(Join-Path $projectRoot "firmware/$role"))
  if($ConfigFile){$compileArgs+=@('--config-file',$ConfigFile)}
  Write-Output "Compilando $role para $Fqbn"
  & $CliPath @compileArgs 2>&1 | Tee-Object -FilePath (Join-Path $buildRoot "$role.log")
  if($LASTEXITCODE -ne 0){throw "Falha ao compilar $role"}
}
