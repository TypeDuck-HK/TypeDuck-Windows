param(
  [string]$RepoRoot = ".",
  [switch]$Strict
)

$ErrorActionPreference = "Stop"

function Fail-Contract {
  param([string]$Message)
  Write-Error "TypeDuck protocol contract failure: $Message"
}

function Require-Text {
  param(
    [string]$Content,
    [string]$Pattern,
    [string]$Decision,
    [string]$Description
  )
  if ($Content -notmatch $Pattern) {
    Fail-Contract "$Description is required by $Decision."
  }
}

$root = (Resolve-Path $RepoRoot).Path
$protoPath = Join-Path $root "proto/moqi.proto"
$framingPath = Join-Path $root "proto/ProtoFraming.h"
$fixturePath = Join-Path $root ".planning/product/protocol-fixtures/phase-04/protocol-contract.json"

if (-not (Test-Path $protoPath)) { Fail-Contract "Missing proto/moqi.proto." }
if (-not (Test-Path $framingPath)) { Fail-Contract "Missing proto/ProtoFraming.h." }
if (-not (Test-Path $fixturePath)) { Fail-Contract "Missing Phase 4 protocol fixture matrix." }

$proto = Get-Content -Raw -LiteralPath $protoPath
$framing = Get-Content -Raw -LiteralPath $framingPath
$fixtureRaw = Get-Content -Raw -LiteralPath $fixturePath
$fixture = $fixtureRaw | ConvertFrom-Json

Require-Text $proto "TYPEDUCK_PROTOCOL_VERSION" "D-01/D-02/D-03" "Explicit TypeDuck protocol version"
Require-Text $proto "TYPE_DUCK_PROTOCOL_VERSION" "D-01/D-02/D-03" "Compatibility bridge field for TypeDuck protocol version"
Require-Text $proto "message\s+TypeDuckCandidatePage" "D-03/D-05/D-06/D-08" "Candidate page metadata"
Require-Text $proto "raw_lookup_comment" "D-05/D-06/D-07/D-08" "Raw lookup-filter comment preservation"
Require-Text $proto "message\s+TypeDuckSettingsSnapshot" "D-14/D-16" "Settings snapshot semantics"
Require-Text $proto "message\s+TypeDuckSettingsUpdate" "D-14/D-16" "Settings update semantics"
Require-Text $proto "message\s+TypeDuckEngineCapability" "D-14/D-16" "Engine capability semantics"
Require-Text $proto "message\s+TypeDuckDeployRequest" "D-14/D-16" "Deploy request semantics"
Require-Text $proto "message\s+TypeDuckReconfigureRequest" "D-14/D-16" "Reconfigure request semantics"
Require-Text $proto "message\s+TypeDuckEngineHealth" "D-14/D-16" "Engine health semantics"
Require-Text $proto "message\s+TypeDuckError" "D-13/D-14/D-16" "Bounded error semantics"
Require-Text $framing "kMax(Client|Backend)FramePayloadBytes" "D-13" "Named frame payload caps"
Require-Text $framing "FrameError" "D-13" "Frame parser error state"
Require-Text $framing "PayloadTooLarge" "D-13" "Oversized frame rejection"
Require-Text $framing "bufferedBytes" "D-13" "Bounded-buffer introspection"

if ($proto -match "message\s+TypeDuckDictionary(?:Entry|Row|View|Ui)|structured_dictionary|dictionary_ui") {
  Fail-Contract "Phase 4 must preserve raw lookup comments only; native dictionary UI parsing is deferred by D-05/D-06/D-08."
}

$expectedRequirements = @("PROTO-01", "PROTO-02", "PROTO-03", "PROTO-04", "PROTO-05", "PROTO-06")
foreach ($requirement in $expectedRequirements) {
  if ($fixture.requirements_covered -notcontains $requirement) {
    Fail-Contract "Fixture matrix must cover $requirement per D-07/D-16."
  }
}

$expectedCases = @(
  "common-cantonese-input",
  "raw-lookup-payload",
  "reverse-lookup-marker",
  "malformed-frame",
  "backend-timeout",
  "backend-restart"
)
foreach ($caseId in $expectedCases) {
  if (-not ($fixture.cases | Where-Object { $_.id -eq $caseId })) {
    Fail-Contract "Missing fixture case '$caseId' required by PROTO-06 and D-07/D-16."
  }
}

$rawCase = $fixture.cases | Where-Object { $_.id -eq "raw-lookup-payload" } | Select-Object -First 1
if (-not $rawCase) {
  Fail-Contract "Missing raw lookup fixture case."
}
$rawComment = [string]$rawCase.candidate.raw_lookup_comment
if (-not $rawComment.Contains([char]0x000B)) {
  Fail-Contract "Raw lookup fixture must preserve vertical-tab U+000B per D-07."
}
if (-not $rawComment.Contains([char]0x000C)) {
  Fail-Contract "Raw lookup fixture must preserve form-feed U+000C per D-07."
}
if (-not $rawComment.Contains([char]0x000D)) {
  Fail-Contract "Raw lookup fixture must preserve carriage-return U+000D per D-07."
}
if ($rawCase.candidate.PSObject.Properties.Name -match "dictionary|parsed|rows|native") {
  Fail-Contract "Raw lookup fixture must not expose Phase 5 native dictionary UI fields per D-05/D-06/D-08."
}

if ($Strict) {
  if ($fixture.decisions_cited.Count -lt 10) {
    Fail-Contract "Strict mode requires D-01, D-02, D-03, D-05, D-06, D-07, D-08, D-13, D-14, and D-16 fixture citations."
  }
  foreach ($decision in @("D-01", "D-02", "D-03", "D-05", "D-06", "D-07", "D-08", "D-13", "D-14", "D-16")) {
    if ($fixture.decisions_cited -notcontains $decision) {
      Fail-Contract "Strict mode fixture metadata must cite $decision."
    }
  }
}

Write-Host "TypeDuck protocol contract guard passed."
