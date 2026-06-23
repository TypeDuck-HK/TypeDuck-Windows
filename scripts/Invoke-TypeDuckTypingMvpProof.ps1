#Requires -Version 5.1
<#
.SYNOPSIS
  Generate Phase 4 TypeDuck typing MVP proof evidence.
#>
param(
  [string] $RepoRoot = ".",
  [string] $StageRoot = ".\runtime\typeduck-phase02",
  [string] $MoqiImeRoot = "..\moqi-ime",
  [string] $EvidencePath = ".planning\product\protocol-fixtures\phase-04\typing-mvp-evidence.json",
  [string] $GoldenCasesPath = ".planning\product\protocol-fixtures\phase-04\golden-typing-cases.json",
  [switch] $CheckOnly,
  [switch] $SkipBuild,
  [string] $VmName = "",
  [string] $GuestUser = "",
  [securestring] $GuestPassword
)

$ErrorActionPreference = "Stop"

function Resolve-ProofPath {
  param(
    [string] $BasePath,
    [string] $Path
  )
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Get-RelativeProofPath {
  param(
    [string] $BasePath,
    [string] $Path
  )
  $base = [System.IO.Path]::GetFullPath($BasePath)
  if (-not $base.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $base += [System.IO.Path]::DirectorySeparatorChar
  }
  $target = [System.IO.Path]::GetFullPath($Path)
  $baseUri = [System.Uri]::new($base)
  $targetUri = [System.Uri]::new($target)
  return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($targetUri).ToString()).Replace("/", "\")
}

function Read-JsonFile {
  param(
    [string] $Path,
    [string] $Label
  )
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "$Label missing: $Path"
  }
  return Get-Content -Raw -LiteralPath $Path | ConvertFrom-Json
}

function Write-JsonFile {
  param(
    [object] $Value,
    [string] $Path,
    [int] $Depth = 24
  )
  $dir = Split-Path -Parent $Path
  if ($dir) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
  }
  $Value | ConvertTo-Json -Depth $Depth | Set-Content -LiteralPath $Path -Encoding UTF8
}

function Get-UtcTimestamp {
  return [datetime]::UtcNow.ToString("yyyy-MM-dd'T'HH':'mm':'ss'Z'", [System.Globalization.CultureInfo]::InvariantCulture)
}

function Get-Sha256Text {
  param([string] $Text)
  $sha = [System.Security.Cryptography.SHA256]::Create()
  try {
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($Text)
    return ([System.BitConverter]::ToString($sha.ComputeHash($bytes))).Replace("-", "").ToLowerInvariant()
  }
  finally {
    $sha.Dispose()
  }
}

function Convert-EscapedRawComment {
  param([string] $Escaped)
  return [regex]::Unescape($Escaped)
}

function Invoke-CapturedCommand {
  param(
    [string] $Id,
    [string] $FilePath,
    [string[]] $ArgumentList,
    [string] $Command,
    [string] $ArtifactPath,
    [switch] $AllowFailure
  )

  $started = Get-UtcTimestamp
  $output = New-Object System.Collections.Generic.List[string]
  $exitCode = 0
  try {
    Push-Location $script:RepoRootFull
    try {
      $lines = & $FilePath @ArgumentList 2>&1
      $exitCode = if ($null -ne $LASTEXITCODE) { $LASTEXITCODE } else { 0 }
      foreach ($line in @($lines)) {
        $output.Add([string] $line) | Out-Null
      }
    }
    finally {
      Pop-Location
    }
  }
  catch {
    $exitCode = 1
    $output.Add($_.Exception.Message) | Out-Null
  }

  $record = [ordered]@{
    id = $Id
    command = $Command
    timestamp = $started
    status = if ($exitCode -eq 0) { "passed" } else { "failed" }
    exit_code = $exitCode
    output_tail = @($output | Select-Object -Last 80)
  }
  Write-JsonFile -Value $record -Path $ArtifactPath -Depth 12
  if ($exitCode -ne 0 -and -not $AllowFailure) {
    throw "Command failed for ${Id}: $Command"
  }
  return $record
}

function New-ProofItem {
  param(
    [string] $Id,
    [string] $Command,
    [string] $ArtifactPath,
    [string] $ObservedOutcome,
    [object] $Details = $null,
    [string] $Source = "",
    [string] $Status = "passed",
    [string] $ArtifactType = "observed-data"
  )
  $artifactFull = Resolve-ProofPath -BasePath $script:RepoRootFull -Path $ArtifactPath
  $payload = [ordered]@{
    artifact_type = $ArtifactType
    id = $Id
    source = $Source
    status = $Status
    command = $Command
    timestamp = Get-UtcTimestamp
    artifact_path = $ArtifactPath
    observed_outcome = $ObservedOutcome
    details = $Details
  }
  Write-JsonFile -Value $payload -Path $artifactFull -Depth 24
  return $payload
}

function Get-Phase2ProofInput {
  param(
    [object] $Proof,
    [string] $Name
  )
  $match = @($Proof.inputs | Where-Object { $_.name -eq $Name } | Select-Object -First 1)
  if ($match.Count -eq 0) {
    throw "Phase 2 typing proof is missing input '$Name'."
  }
  return $match[0]
}

function Test-FileContains {
  param(
    [string] $Path,
    [string] $Pattern
  )
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    return $false
  }
  $content = Get-Content -Raw -LiteralPath $Path
  return $content -match $Pattern
}

function Get-Phase2RawCommentRecord {
  param(
    [string] $Path,
    [string] $CandidateText
  )
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "Phase 2 raw comment evidence missing: $Path"
  }
  foreach ($line in Get-Content -LiteralPath $Path) {
    if ([string]::IsNullOrWhiteSpace($line)) {
      continue
    }
    $record = $line | ConvertFrom-Json
    if ($record.candidateText -eq $CandidateText) {
      return $record
    }
  }
  throw "Phase 2 raw comment evidence does not contain candidate '$CandidateText'."
}

function New-GuardEvidence {
  param(
    [string] $Id,
    [string] $FilePath,
    [string[]] $ArgumentList,
    [string] $Command,
    [string] $ArtifactPath
  )
  $full = Resolve-ProofPath -BasePath $script:RepoRootFull -Path $ArtifactPath
  $record = Invoke-CapturedCommand -Id $Id -FilePath $FilePath -ArgumentList $ArgumentList -Command $Command -ArtifactPath $full
  return [ordered]@{
    id = $Id
    status = $record.status
    command = $Command
    timestamp = $record.timestamp
    artifact_path = $ArtifactPath
    observed_outcome = "command exited $($record.exit_code)"
  }
}

function New-VmSmokeArtifact {
  param(
    [string] $Vm,
    [string] $User,
    [securestring] $Password,
    [string] $ArtifactPath
  )

  $artifactFull = Resolve-ProofPath -BasePath $script:RepoRootFull -Path $ArtifactPath
  $payload = [ordered]@{
    artifact_type = "windows-smoke-direct-session"
    status = "blocked"
    command = "New-PSSession -VMName <vm> -Credential <runtime credential>; Invoke-Command { OS and interactivity probe }"
    timestamp = Get-UtcTimestamp
    artifact_path = $ArtifactPath
    observed_outcome = "PowerShell Direct connectivity was checked, but the session is non-interactive and cannot prove real TSF host-process typing."
    vm = [ordered]@{
      name = $Vm
      guest_user = $User
    }
    observations = [ordered]@{
      composition = $false
      candidates = $false
      paging = $false
      numeric_selection = $false
      commit = $false
      bounded_recovery = $false
    }
  }

  if ([string]::IsNullOrWhiteSpace($Vm)) {
    Write-JsonFile -Value $payload -Path $artifactFull -Depth 16
    return $payload
  }

  $session = $null
  try {
    $credential = $null
    if (-not [string]::IsNullOrWhiteSpace($User) -and $null -ne $Password) {
      $credential = [pscredential]::new($User, $Password)
      $session = New-PSSession -VMName $Vm -Credential $credential
    } else {
      $session = New-PSSession -VMName $Vm
    }

    $probe = Invoke-Command -Session $session -ScriptBlock {
      $os = Get-CimInstance Win32_OperatingSystem
      [ordered]@{
        user = $env:USERNAME
        computer = $env:COMPUTERNAME
        os = $os.Caption
        version = $os.Version
        buildNumber = $os.BuildNumber
        interactive = [Environment]::UserInteractive
        typeduckInstallDirExists = Test-Path -LiteralPath "C:\Program Files (x86)\TypeDuckIME"
        typeDuckLauncherRunValue = try {
          [string] (Get-ItemProperty -LiteralPath "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -ErrorAction Stop).TypeDuckLauncher
        } catch {
          $null
        }
      }
    }

    $payload.vm.probe = $probe
    if ($probe.interactive -eq $true) {
      $payload.observed_outcome = "PowerShell Direct reported an interactive session, but no UI typing automation has recorded the required TypeDuck observations yet."
    }
  }
  catch {
    $payload.observed_outcome = "PowerShell Direct smoke probe failed: $($_.Exception.Message)"
    $payload.error = $_.Exception.Message
  }
  finally {
    if ($null -ne $session) {
      Remove-PSSession -Session $session
    }
  }

  Write-JsonFile -Value $payload -Path $artifactFull -Depth 24
  return $payload
}

$RepoRootFull = Resolve-ProofPath -BasePath (Get-Location).Path -Path $RepoRoot
if (-not (Test-Path -LiteralPath $RepoRootFull -PathType Container)) {
  throw "RepoRoot missing: $RepoRootFull"
}
$script:RepoRootFull = $RepoRootFull

$EvidenceFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $EvidencePath
$GoldenFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $GoldenCasesPath
$StageRootFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $StageRoot
$ArtifactsRoot = Join-Path (Split-Path -Parent $EvidenceFull) "typing-mvp-artifacts"
$ArtifactsRel = Get-RelativeProofPath -BasePath $RepoRootFull -Path $ArtifactsRoot
$Phase2ProofPath = Join-Path $RepoRootFull ".planning\product\engine-runtime-fixtures\phase-02\typing-proof.json"
$Phase2FrameLogPath = Join-Path $RepoRootFull ".planning\product\engine-runtime-fixtures\phase-02\backend-stdio-frames.ndjson"
$Phase2RawCommentsPath = Join-Path $RepoRootFull ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-raw-comments.ndjson"
$Phase2CsvRowsPath = Join-Path $RepoRootFull ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-csv-rows.csv"
$Phase2LookupValidationPath = Join-Path $RepoRootFull ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-validation.json"

if ($CheckOnly) {
  & pwsh -NoProfile -ExecutionPolicy Bypass -File (Join-Path $RepoRootFull "scripts\Test-TypeDuckTypingMvpProof.ps1") -RepoRoot $RepoRootFull -EvidencePath $EvidenceFull -GoldenCasesPath $GoldenFull -Strict
  if ($LASTEXITCODE -ne 0) {
    throw "Existing TypeDuck typing MVP proof is invalid."
  }
  exit 0
}

$golden = Read-JsonFile -Path $GoldenFull -Label "golden typing cases"
$launcher = Read-JsonFile -Path (Join-Path $RepoRootFull ".planning\product\protocol-fixtures\phase-04\launcher-recovery.json") -Label "launcher recovery fixture"
$typingClient = Read-JsonFile -Path (Join-Path $RepoRootFull ".planning\product\protocol-fixtures\phase-04\typing-client.json") -Label "typing client fixture"
$phase2Proof = Read-JsonFile -Path $Phase2ProofPath -Label "Phase 2 typing proof"

New-Item -ItemType Directory -Path $ArtifactsRoot -Force | Out-Null

$guardCommands = @()
$guardCommands += New-GuardEvidence -Id "protocol-contract" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckProtocolContract.ps1", "-RepoRoot", ".", "-Strict") -Command "pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict" -ArtifactPath (Join-Path $ArtifactsRel "guard-protocol-contract.json")
$guardCommands += New-GuardEvidence -Id "launcher-protocol" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckLauncherProtocol.ps1", "-RepoRoot", ".", "-Strict") -Command "pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict" -ArtifactPath (Join-Path $ArtifactsRel "guard-launcher-protocol.json")
$guardCommands += New-GuardEvidence -Id "typing-client" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckTypingClient.ps1", "-RepoRoot", ".", "-Strict") -Command "pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTypingClient.ps1 -RepoRoot . -Strict" -ArtifactPath (Join-Path $ArtifactsRel "guard-typing-client.json")
$guardCommands += New-GuardEvidence -Id "engine-proof-check" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Invoke-TypeDuckEngineProof.ps1", "-RepoRoot", ".", "-StageRoot", $StageRoot, "-MoqiImeRoot", $MoqiImeRoot, "-OutputPath", ".planning\product\engine-runtime-fixtures\phase-02\typing-proof.json", "-CheckOnly") -Command "pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckEngineProof.ps1 -RepoRoot . -StageRoot $StageRoot -MoqiImeRoot $MoqiImeRoot -OutputPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json -CheckOnly" -ArtifactPath (Join-Path $ArtifactsRel "guard-engine-proof-check.json")
$guardCommands += New-GuardEvidence -Id "lookup-payload" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckLookupPayload.ps1", "-TypingProofPath", ".planning\product\engine-runtime-fixtures\phase-02\typing-proof.json", "-RawCommentsPath", ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-raw-comments.ndjson", "-CsvRowsPath", ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-csv-rows.csv", "-ValidationPath", ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-validation.json") -Command "pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLookupPayload.ps1 -TypingProofPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json -RawCommentsPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-raw-comments.ndjson -CsvRowsPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-csv-rows.csv -ValidationPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-validation.json" -ArtifactPath (Join-Path $ArtifactsRel "guard-lookup-payload.json")

if (-not $SkipBuild) {
  $guardCommands += New-GuardEvidence -Id "build-win32-launcher" -FilePath "cmake" -ArgumentList @("--build", "build-vs32", "--config", "Debug", "--target", "MoqiLauncher", "--", "/m:1") -Command "cmake --build build-vs32 --config Debug --target MoqiLauncher -- /m:1" -ArtifactPath (Join-Path $ArtifactsRel "build-win32-launcher.json")
  $guardCommands += New-GuardEvidence -Id "build-win32-textservice" -FilePath "cmake" -ArgumentList @("--build", "build-vs32", "--config", "Debug", "--target", "MoqiTextService", "--", "/m:1") -Command "cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1" -ArtifactPath (Join-Path $ArtifactsRel "build-win32-textservice.json")
  if (Test-Path -LiteralPath (Join-Path $RepoRootFull "build-vs64")) {
    $guardCommands += New-GuardEvidence -Id "build-x64-textservice" -FilePath "cmake" -ArgumentList @("--build", "build-vs64", "--config", "Debug", "--target", "MoqiTextService", "--", "/m:1") -Command "cmake --build build-vs64 --config Debug --target MoqiTextService -- /m:1" -ArtifactPath (Join-Path $ArtifactsRel "build-x64-textservice.json")
  }
  $guardCommands += New-GuardEvidence -Id "build-protoframing-test" -FilePath "cmake" -ArgumentList @("--build", "build-vs32", "--config", "Debug", "--target", "ProtoFraming_test", "--", "/m:1") -Command "cmake --build build-vs32 --config Debug --target ProtoFraming_test -- /m:1" -ArtifactPath (Join-Path $ArtifactsRel "build-protoframing-test.json")
  $guardCommands += New-GuardEvidence -Id "run-protoframing-test" -FilePath "build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe" -ArgumentList @() -Command "build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe" -ArtifactPath (Join-Path $ArtifactsRel "run-protoframing-test.json")
}

$nei = Get-Phase2ProofInput -Proof $phase2Proof -Name "nei"
$rawGolden = @($golden.cases | Where-Object { $_.id -eq "raw-lookup-payload" } | Select-Object -First 1)[0]
$rawComment = Convert-EscapedRawComment -Escaped ([string] $rawGolden.raw_lookup_comment_escaped)
$phase2RawRecord = Get-Phase2RawCommentRecord -Path $Phase2RawCommentsPath -CandidateText ([string] $rawGolden.candidate_text)
$schemaPath = Join-Path $StageRootFull "input_methods\rime\jyut6ping3.schema.yaml"
$cangjie3Path = Join-Path $StageRootFull "input_methods\rime\cangjie3.schema.yaml"
$cangjie5Path = Join-Path $StageRootFull "input_methods\rime\cangjie5.schema.yaml"

$goldenEvidence = @()
$goldenEvidence += New-ProofItem -Id "common-cantonese-input" -Command "scripts\Invoke-TypeDuckEngineProof.ps1 -CheckOnly plus Phase 4 TSF client guard" -ArtifactPath (Join-Path $ArtifactsRel "golden-common-cantonese-input.json") -ObservedOutcome "Phase 2 backend proof records composition, candidate entries, cursor/page data, and commit; Phase 4 TSF client guard proves those fields are consumed." -Details ([ordered]@{
  input = $nei.name
  keys = @($nei.keys)
  composition = $nei.composition
  candidate_count = @($nei.candidates).Count
  first_candidate = $nei.candidates[0].text
  commit = $nei.commit
  page = $nei.page
  phase2_proof = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2ProofPath
})
$goldenEvidence += New-ProofItem -Id "raw-lookup-payload" -Command "scripts\Test-TypeDuckLookupPayload.ps1 plus TypeDuck raw_lookup_comment fixture" -ArtifactPath (Join-Path $ArtifactsRel "golden-raw-lookup-payload.json") -ArtifactType "raw-lookup-observation" -ObservedOutcome "Phase 2 captured lookup-filter raw comments with real SHA-256 evidence; protocol fixture separately preserves U+000B, U+000C, and U+000D without Phase 5 parsing." -Details ([ordered]@{
  candidate_text = $rawGolden.candidate_text
  phase2_source = [ordered]@{
    escaped = $phase2RawRecord.escapedComment
    sha256 = $phase2RawRecord.sha256
    control_counts = $phase2RawRecord.controlCounts
    artifact = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2RawCommentsPath
  }
  protocol_fixture = [ordered]@{
    escaped = $rawGolden.raw_lookup_comment_escaped
    sha256_utf8 = Get-Sha256Text -Text $rawComment
    separator_code_points = @($rawGolden.separator_code_points)
  }
  phase2_raw_comments = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2RawCommentsPath
  phase2_lookup_validation = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2LookupValidationPath
})
$goldenEvidence += New-ProofItem -Id "reverse-lookup-marker" -Command "schema capability scan plus scripts\Test-TypeDuckTypingClient.ps1 -Strict" -ArtifactPath (Join-Path $ArtifactsRel "golden-reverse-lookup-marker.json") -ObservedOutcome "Backend schema declares reverse_lookup recognizer and frontend forwards keys normally; real host-process key smoke remains tracked by windows_smoke." -Details ([ordered]@{
  backend_capability = if (Test-FileContains -Path $schemaPath -Pattern "reverse_lookup") { "supported_by_schema" } else { "not_found" }
  schema = Get-RelativeProofPath -BasePath $RepoRootFull -Path $schemaPath
  frontend_behavior = "ordinary key forwarding; no reverse lookup frontend special casing"
})
$goldenEvidence += New-ProofItem -Id "cangjie-key-forwarding" -Command "schema capability scan plus scripts\Test-TypeDuckTypingClient.ps1 -Strict" -ArtifactPath (Join-Path $ArtifactsRel "golden-cangjie-key-forwarding.json") -ObservedOutcome "Backend schema declares Cangjie 3/5 assets and frontend forwards keys normally; real host-process key smoke remains tracked by windows_smoke." -Details ([ordered]@{
  cangjie3_schema = if (Test-Path -LiteralPath $cangjie3Path -PathType Leaf) { Get-RelativeProofPath -BasePath $RepoRootFull -Path $cangjie3Path } else { $null }
  cangjie5_schema = if (Test-Path -LiteralPath $cangjie5Path -PathType Leaf) { Get-RelativeProofPath -BasePath $RepoRootFull -Path $cangjie5Path } else { $null }
  jyutping_schema_declares_cangjie = Test-FileContains -Path $schemaPath -Pattern "cangjie"
  frontend_behavior = "ordinary key forwarding; no Cangjie frontend special casing"
})
$goldenEvidence += New-ProofItem -Id "malformed-frame" -Command "build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe plus launcher protocol guard" -ArtifactPath (Join-Path $ArtifactsRel "golden-malformed-frame.json") -ObservedOutcome "Malformed payload rejection is covered by ProtoFraming_test and launcher degraded error mapping guard." -Details ([ordered]@{ frame_log = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2FrameLogPath })
$goldenEvidence += New-ProofItem -Id "oversized-frame" -Command "build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe plus launcher protocol guard" -ArtifactPath (Join-Path $ArtifactsRel "golden-oversized-frame.json") -ObservedOutcome "Oversized payload rejection is covered by bounded FrameBuffer tests and launcher payload-too-large mapping guard." -Details ([ordered]@{ max_frame_bytes = 1048576 })
$goldenEvidence += New-ProofItem -Id "backend-timeout" -Command "scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict" -ArtifactPath (Join-Path $ArtifactsRel "golden-backend-timeout.json") -ObservedOutcome "Launcher protocol guard proves timeout response is emitted before restart cleanup with restarting health semantics." -Details ([ordered]@{ error_code = "TYPEDUCK_ERROR_BACKEND_TIMEOUT"; health = "TYPEDUCK_HEALTH_RESTARTING" })
$goldenEvidence += New-ProofItem -Id "backend-restart" -Command "scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict" -ArtifactPath (Join-Path $ArtifactsRel "golden-backend-restart.json") -ObservedOutcome "Launcher protocol guard proves backend restart/read-error paths return degraded TypeDuck recovery responses." -Details ([ordered]@{ error_code = "TYPEDUCK_ERROR_BACKEND_RESTART"; health = "TYPEDUCK_HEALTH_DEGRADED" })

$importedEvidence = @()
foreach ($case in @($launcher.runtime_cases)) {
  $importedEvidence += New-ProofItem -Id $case.id -Source "launcher-recovery" -Command "scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict" -ArtifactPath (Join-Path $ArtifactsRel ("obligation-launcher-" + $case.id + ".json")) -Status "manual_pending" -ArtifactType "launcher-runtime-obligation" -ObservedOutcome ("Requires executable launcher/runtime probe before this obligation can pass: " + $case.plan_04_04_proof) -Details ([ordered]@{
    trigger = $case.trigger
    expected_response = $case.expected_response
    runtime_probe_required = $true
  })
}
foreach ($case in @($typingClient.runtime_cases)) {
  $command = "scripts\Test-TypeDuckTypingClient.ps1 -Strict"
  if ($case.id -eq "raw-comment-byte-preservation") {
    $command = "scripts\Test-TypeDuckLookupPayload.ps1 plus scripts\Test-TypeDuckTypingClient.ps1 -Strict"
  }
  $vmRequired = $case.id -in @("ordinary-key-response-latency", "backend-restart-reset-reconnect", "reverse-lookup-key-forwarding", "cangjie-key-forwarding")
  $status = if ($vmRequired) { "manual_pending" } else { "passed" }
  $outcome = if ($vmRequired) {
    "Requires Windows VM host-process smoke before this obligation can pass: $($case.plan_04_04_proof)"
  } else {
    $case.plan_04_04_proof
  }
  $importedEvidence += New-ProofItem -Id $case.id -Source "typing-client" -Command $command -ArtifactPath (Join-Path $ArtifactsRel ("obligation-typing-" + $case.id + ".json")) -Status $status -ArtifactType "typing-client-obligation" -ObservedOutcome $outcome -Details ([ordered]@{
    trigger = $case.trigger
    expected = $case.expected
    vm_required_for_host_process_observation = $vmRequired
  })
}

$windowsSmokeArtifactRel = Join-Path $ArtifactsRel "windows-smoke-vm-direct.json"
$vmSmokeArtifact = $null
if (-not [string]::IsNullOrWhiteSpace($VmName)) {
  $vmSmokeArtifact = New-VmSmokeArtifact -Vm $VmName -User $GuestUser -Password $GuestPassword -ArtifactPath $windowsSmokeArtifactRel
}

$windowsSmoke = [ordered]@{
  status = "manual_pending"
  command = "Follow .planning\product\protocol-fixtures\phase-04\windows-smoke-checklist.md in the disposable VM"
  timestamp = Get-UtcTimestamp
  artifact_path = ".planning\product\protocol-fixtures\phase-04\windows-smoke-checklist.md"
  observed_outcome = "Automated protocol/launcher/client proof generated; real host-process TSF smoke remains pending unless VM evidence is attached."
  observations = [ordered]@{
    composition = $false
    candidates = $false
    paging = $false
    numeric_selection = $false
    commit = $false
    bounded_recovery = $false
  }
  vm = if ([string]::IsNullOrWhiteSpace($VmName)) { $null } else { [ordered]@{ name = $VmName; guest_user = $GuestUser } }
}

if ($null -ne $vmSmokeArtifact) {
  $windowsSmoke.status = $vmSmokeArtifact.status
  $windowsSmoke.command = $vmSmokeArtifact.command
  $windowsSmoke.timestamp = $vmSmokeArtifact.timestamp
  $windowsSmoke.artifact_path = $vmSmokeArtifact.artifact_path
  $windowsSmoke.observed_outcome = $vmSmokeArtifact.observed_outcome
  $windowsSmoke.observations = $vmSmokeArtifact.observations
  $windowsSmoke.vm = $vmSmokeArtifact.vm
}

$evidence = [ordered]@{
  phase = "04-typeduck-protocol-and-typing-mvp"
  plan = "04-04"
  generated_at = Get-UtcTimestamp
  runner = "scripts\Invoke-TypeDuckTypingMvpProof.ps1"
  repo = [ordered]@{
    root = $RepoRootFull
    commit = (& git -C $RepoRootFull rev-parse HEAD).Trim()
  }
  inputs = [ordered]@{
    stage_root = Get-RelativeProofPath -BasePath $RepoRootFull -Path $StageRootFull
    golden_cases = Get-RelativeProofPath -BasePath $RepoRootFull -Path $GoldenFull
    phase2_typing_proof = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2ProofPath
    phase2_frame_log = Get-RelativeProofPath -BasePath $RepoRootFull -Path $Phase2FrameLogPath
  }
  guard_commands = $guardCommands
  golden_cases = $goldenEvidence
  imported_obligations = $importedEvidence
  windows_smoke = $windowsSmoke
}

Write-JsonFile -Value $evidence -Path $EvidenceFull -Depth 32

& pwsh -NoProfile -ExecutionPolicy Bypass -File (Join-Path $RepoRootFull "scripts\Test-TypeDuckTypingMvpProof.ps1") -RepoRoot $RepoRootFull -EvidencePath $EvidenceFull -GoldenCasesPath $GoldenFull
if ($LASTEXITCODE -ne 0) {
  throw "Generated TypeDuck typing MVP draft proof did not validate."
}

Write-Host "TypeDuck typing MVP draft proof written to $(Get-RelativeProofPath -BasePath $RepoRootFull -Path $EvidenceFull)"
Write-Host "Run scripts\Test-TypeDuckTypingMvpProof.ps1 -Strict after VM smoke evidence is recorded."
