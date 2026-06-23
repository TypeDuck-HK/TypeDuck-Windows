#Requires -Version 5.1
<#
.SYNOPSIS
  Static contract guard for the TypeDuck TSF typing client path.

.PARAMETER RepoRoot
  Repository root to inspect. Defaults to the parent directory of this script.

.PARAMETER Strict
  Enables stronger checks that reverse lookup and Cangjie remain backend-owned.
#>
param(
    [string] $RepoRoot = "",
    [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    param([string] $RequestedRoot)

    if ([string]::IsNullOrWhiteSpace($RequestedRoot)) {
        return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
    }
    return [System.IO.Path]::GetFullPath($RequestedRoot)
}

function Read-RequiredFile {
    param(
        [string] $Root,
        [string] $RelativePath
    )

    $path = Join-Path $Root $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required file not found: $RelativePath"
    }
    return [System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)
}

function Add-Failure {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Message
    )

    $Failures.Add($Message) | Out-Null
}

function Assert-Match {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $Pattern,
        [string] $Message
    )

    if ($Content -notmatch $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-NotMatch {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $Pattern,
        [string] $Message
    )

    if ($Content -match $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-AllMatch {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string[]] $Patterns,
        [string] $Message
    )

    foreach ($pattern in $Patterns) {
        if ($Content -notmatch $pattern) {
            Add-Failure $Failures "$Message Missing pattern: $pattern"
        }
    }
}

function Assert-Fixture {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $FixtureJson
    )

    $fixture = $FixtureJson | ConvertFrom-Json
    if ($fixture.phase -ne "04-typeduck-protocol-and-typing-mvp") {
        Add-Failure $Failures "typing-client.json must name Phase 4."
    }
    $requiredCases = @(
        "ordinary-key-response-latency",
        "missing-launcher-fast-fail-degraded-state",
        "malformed-response-reset",
        "backend-restart-reset-reconnect",
        "raw-comment-byte-preservation",
        "reverse-lookup-key-forwarding",
        "cangjie-key-forwarding"
    )
    $actualCases = @($fixture.runtime_cases | ForEach-Object { $_.id })
    foreach ($case in $requiredCases) {
        if ($actualCases -notcontains $case) {
            Add-Failure $Failures "typing-client.json is missing runtime case: $case"
        }
    }
    if ($FixtureJson -notmatch "\\u000b" -or $FixtureJson -notmatch "\\u000c" -or $FixtureJson -notmatch "\\r") {
        Add-Failure $Failures "typing-client.json must include raw lookup separator proof data for vertical-tab, form-feed, and carriage-return."
    }
}

function Assert-ResponseMapping {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $ClientCpp,
        [string] $ClientH,
        [string] $TextServiceCpp,
        [string] $TextServiceH,
        [string] $Proto
    )

    $client = "$ClientCpp`n$ClientH"
    $service = "$TextServiceCpp`n$TextServiceH"
    Assert-AllMatch $Failures $client @(
        "candidate\.raw_lookup_comment\(\)",
        "rawLookupComment|raw_lookup_comment",
        "typeduck_candidate_page",
        "candidatePageSize|setCandidatePageSize",
        "candidateTotalCount|setCandidateTotalCount",
        "typeduck_engine_health",
        "typeduck_error",
        "TYPEDUCK_HEALTH_DEGRADED|TYPEDUCK_HEALTH_RESTARTING|TYPEDUCK_HEALTH_FAILED",
        "resetTypeDuckDegradedState|handleTypeDuckFailure|markRpcDegraded"
    ) "MoqiClient must map TypeDuck candidate raw comments, page metadata, and health/error responses."

    Assert-AllMatch $Failures $service @(
        "setCandidatePageSize",
        "setCandidateTotalCount",
        "candidatePageSize_",
        "candidateTotalCount_",
        "hideCandidates\(\)",
        "candidatePreedit_\.clear\(\)"
    ) "MoqiTextService must store TypeDuck page/count state and reset visible candidate state on degraded paths."

    Assert-AllMatch $Failures $Proto @(
        "raw_lookup_comment",
        "TypeDuckCandidatePage",
        "TypeDuckEngineHealth",
        "TypeDuckError"
    ) "Protocol must expose the TypeDuck fields consumed by the TSF client."
}

function Assert-KeyPathBounds {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $ClientCpp
    )

    Assert-AllMatch $Failures $ClientCpp @(
        "Proto::serializeMessageBounded",
        "Proto::kMaxClientFramePayloadBytes",
        "FrameBuffer\s+responseBuffer\{\s*Proto::kMaxClientFramePayloadBytes\s*\}",
        "TYPEDUCK_KEYPATH_CONNECT_TIMEOUT_MS|kTypeDuckKeyPathConnectTimeoutMs",
        "degradedUntilTick_|markRpcDegraded",
        "pendingAsyncResponses_\.clear\(\)",
        "resetTextServiceState\(\)"
    ) "TSF key-path RPC must use bounded frames, bounded connection wait, and degraded reset state."

    Assert-NotMatch $Failures $ClientCpp "attempt\\s*<\\s*10" `
        "TSF key path must not keep the old ten-attempt connection loop."
    Assert-NotMatch $Failures $ClientCpp "connectPipe\\(serverPipeName\\.c_str\\(\\),\\s*3000\\)" `
        "TSF key path must not wait through old three-second per-attempt launcher connection waits."
}

function Assert-KeyForwarding {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $ClientCpp
    )

    Assert-AllMatch $Failures $ClientCpp @(
        "METHOD_FILTER_KEY_DOWN",
        "METHOD_ON_KEY_DOWN",
        "addKeyEventToRpcRequest\(req, keyEvent\)",
        "set_key_code",
        "set_char_code",
        "add_key_states"
    ) "Reverse lookup and Cangjie keys must flow through ordinary key event forwarding."

    if ($Strict) {
        Assert-NotMatch $Failures $ClientCpp "(?i)reverse\\s*lookup|reverseLookup|cangjie|倉頡" `
            "Strict guard: frontend must not special-case reverse lookup or Cangjie product behavior."
    }
}

$root = Resolve-RepoRoot -RequestedRoot $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$files = @{
    "MoqiTextService/MoqiClient.cpp" = Read-RequiredFile $root "MoqiTextService/MoqiClient.cpp"
    "MoqiTextService/MoqiClient.h" = Read-RequiredFile $root "MoqiTextService/MoqiClient.h"
    "MoqiTextService/MoqiTextService.cpp" = Read-RequiredFile $root "MoqiTextService/MoqiTextService.cpp"
    "MoqiTextService/MoqiTextService.h" = Read-RequiredFile $root "MoqiTextService/MoqiTextService.h"
    "proto/moqi.proto" = Read-RequiredFile $root "proto/moqi.proto"
    ".planning/product/protocol-fixtures/phase-04/typing-client.json" = Read-RequiredFile $root ".planning/product/protocol-fixtures/phase-04/typing-client.json"
}

Assert-Fixture $failures $files[".planning/product/protocol-fixtures/phase-04/typing-client.json"]
Assert-ResponseMapping $failures `
    $files["MoqiTextService/MoqiClient.cpp"] `
    $files["MoqiTextService/MoqiClient.h"] `
    $files["MoqiTextService/MoqiTextService.cpp"] `
    $files["MoqiTextService/MoqiTextService.h"] `
    $files["proto/moqi.proto"]
Assert-KeyPathBounds $failures $files["MoqiTextService/MoqiClient.cpp"]
Assert-KeyForwarding $failures $files["MoqiTextService/MoqiClient.cpp"]

if ($failures.Count -gt 0) {
    Write-Error ("TypeDuck typing client check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck typing client check passed."
exit 0
