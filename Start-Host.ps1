# Lance le client hote 3.5 (serveur listen) et injecte la DLL Reboot.
# Usage: powershell -ExecutionPolicy Bypass -File E:\reboot4launcher\Start-Host.ps1
# Le joueur se lance ensuite avec: instigator start s35  (+ Jouer dans le salon)

$BackendDir = 'C:\Program Files\Reboot Launcher\data\flutter_assets\assets\backend'
$Instigator  = 'C:\Users\kosmix\AppData\Local\Programs\instigator\instigator.exe'
$Injector   = 'C:\Users\kosmix\AppData\Local\Temp\opencode\injector.exe'
$ServerDll  = 'E:\reboot4launcher\Project-Reboot-3.0\x64\Release\Project Reboot 3.0.dll'
$LobbyWait  = 55   # secondes pour arriver au salon avant injection
$ListenWait = 150  # attente max de l'ecoute UDP 7777

# 1. Backend
try { $b = curl.exe -s --max-time 5 http://127.0.0.1:3551/fortnite/api/game/v2/enabled_features } catch { $b = $null }
if (-not $b) {
    Write-Output 'Backend arrete, demarrage...'
    Start-Process "$BackendDir\lawinserver.exe" -WorkingDirectory $BackendDir
    Start-Sleep 8
    $b = curl.exe -s --max-time 10 http://127.0.0.1:3551/fortnite/api/game/v2/enabled_features
}
if (-not $b) { Write-Output 'ERREUR: backend injoignable sur 3551'; exit 1 }
Write-Output 'Backend OK'

# 2. Lancement hote (PID = le nouveau process)
$before = Get-Process FortniteClient-Win64-Shipping -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id
Start-Process $Instigator -ArgumentList 'start s35'
$HostPid = $null
for ($i = 0; $i -lt 30 -and -not $HostPid; $i++) {
    Start-Sleep 2
    $HostPid = Get-Process FortniteClient-Win64-Shipping -ErrorAction SilentlyContinue |
        Where-Object { $before -notcontains $_.Id } |
        Select-Object -ExpandProperty Id -First 1
}
if (-not $HostPid) { Write-Output 'ERREUR: process hote introuvable'; exit 1 }
Write-Output "Hote PID=$HostPid, attente salon ($LobbyWait s)..."
Start-Sleep $LobbyWait

# 3. Injection
& $Injector $HostPid $ServerDll
if ($LASTEXITCODE -ne 0) { Write-Output "ERREUR: injection exit=$LASTEXITCODE"; exit 1 }
Write-Output 'Injecte, attente ecoute 7777...'

# 4. Attente ecoute UDP 7777
$ok = $false
for ($i = 0; $i -lt $ListenWait; $i += 5) {
    Start-Sleep 5
    if (netstat -anop udp | Select-String "7777.*$HostPid") { $ok = $true; break }
    if (-not (Get-Process -Id $HostPid -ErrorAction SilentlyContinue)) { Write-Output 'ERREUR: hote mort (voir FortniteGame.log)'; exit 1 }
}
if (-not $ok) { Write-Output 'ERREUR: 7777 non ecoute (voir FortniteGame.log)'; exit 1 }
Write-Output "OK: hote $HostPid ecoute sur 7777. Lancez le joueur: instigator start s35, puis Jouer."
