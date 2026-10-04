param([string]$Fqbn='esp32:esp32:esp32')
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
foreach($role in @('tx','rx','noise')) {
  & arduino-cli compile --fqbn $Fqbn --libraries (Join-Path $projectRoot 'libraries') (Join-Path $projectRoot "firmware/$role")
  if($LASTEXITCODE -ne 0){throw "Falha ao compilar $role"}
}
