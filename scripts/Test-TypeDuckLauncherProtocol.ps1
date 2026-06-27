#Requires -Version 5.1
<#
.SYNOPSIS
  Static contract guard for the TypeDuck launcher/backend protocol recovery path.

.PARAMETER RepoRoot
  Repository root to inspect. Defaults to the parent directory of this script.

.PARAMETER Strict
  Enables stronger checks for legacy Moqi identity leakage in the launcher bridge.
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

function Assert-Ordered {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $BeforePattern,
        [string] $AfterPattern,
        [string] $Message
    )

    $before = [regex]::Match($Content, $BeforePattern)
    $after = [regex]::Match($Content, $AfterPattern)
    if (-not $before.Success -or -not $after.Success -or $before.Index -gt $after.Index) {
        Add-Failure $Failures $Message
    }
}

function Assert-Fixture {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $FixtureJson
    )

    $fixture = $FixtureJson | ConvertFrom-Json
    if ($fixture.phase -ne "04-typeduck-protocol-and-typing-mvp") {
        Add-Failure $Failures "launcher-recovery.json must name Phase 4."
    }
    if ($fixture.transport.preserved -ne $true -or $fixture.transport.client_to_launcher -notmatch "named pipe" -or $fixture.transport.launcher_to_backend -notmatch "stdin/stdout") {
        Add-Failure $Failures "launcher-recovery.json must preserve the D-01 named-pipe and backend stdin/stdout transport."
    }

    $requiredCases = @(
        "malformed-client-frame-cleanup",
        "oversized-backend-stdout-rejection",
        "missing-backend-init-failure-response",
        "backend-timeout-response-before-restart",
        "backend-restart-degraded-response"
    )
    $actualCases = @($fixture.runtime_cases | ForEach-Object { $_.id })
    foreach ($case in $requiredCases) {
        if ($actualCases -notcontains $case) {
            Add-Failure $Failures "launcher-recovery.json is missing runtime case: $case"
        }
    }

    foreach ($case in $fixture.runtime_cases) {
        if ([string]::IsNullOrWhiteSpace($case.expected_response.error_code) -or
            [string]::IsNullOrWhiteSpace($case.expected_response.health_status)) {
            Add-Failure $Failures "Runtime case '$($case.id)' must declare expected TypeDuck error code and health status."
        }
        if ([string]::IsNullOrWhiteSpace($case.plan_04_04_proof)) {
            Add-Failure $Failures "Runtime case '$($case.id)' must declare a Plan 04-04 proof obligation."
        }
    }
}

function Assert-ClientRecovery {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $PipeClientCpp,
        [string] $PipeClientH
    )

    $client = "$PipeClientCpp`n$PipeClientH"
    Assert-AllMatch $Failures $client @(
        "Proto::kMaxClientFramePayloadBytes",
        "readBuffer_\.hasViolation\(",
        "FrameError::PayloadTooLarge",
        "FrameError::MalformedHeader|FrameError::MalformedFrame",
        "TYPEDUCK_ERROR_MALFORMED_FRAME",
        "TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE",
        "TYPEDUCK_ERROR_MALFORMED_PAYLOAD",
        "TYPEDUCK_ERROR_ENGINE_INIT_FAILED",
        "TYPEDUCK_ERROR_BACKEND_TIMEOUT",
        "writeTypeDuckErrorResponse|sendTypeDuckErrorResponse",
        "writeBackendResponse",
        "pendingSeqNum_",
        "seqNum\s*!=\s*pendingSeqNum_",
        "stopRequestTimeoutTimer\(\);[\s\S]*writeTypeDuckErrorResponse"
    ) "PipeClient must bound client frames, return TypeDuck errors, and keep stale backend responses from clearing active requests."
}

function Assert-BackendRecovery {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $BackendServerCpp,
        [string] $BackendServerH,
        [string] $PipeServerCpp,
        [string] $PipeServerH
    )

    $backend = "$BackendServerCpp`n$BackendServerH`n$PipeServerCpp`n$PipeServerH"
    Assert-AllMatch $Failures $backend @(
        "Proto::kMaxBackendFramePayloadBytes",
        "stdoutFrameBuf_\.hasViolation\(",
        "FrameError::PayloadTooLarge",
        "FrameError::MalformedHeader|FrameError::MalformedFrame",
        "bool\s+BackendServer::startProcess\(",
        "stdinPipe_\s*==\s*nullptr",
        "TYPEDUCK_ERROR_ENGINE_INIT_FAILED",
        "TYPEDUCK_ERROR_BACKEND_RESTART",
        "TYPEDUCK_HEALTH_DEGRADED",
        "notifyClientsOfBackendError|notifyBackendError",
        "terminateProcess\(false\)"
    ) "BackendServer must bound backend stdout, report spawn/restart failures, and avoid restart-time client close loops."
}

function Assert-TypeDuckMapping {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Root,
        [string] $PipeServerCpp
    )

    Assert-AllMatch $Failures $PipeServerCpp @(
        "\{c6e8f5df-6504-44f9-b7cf-17a195373a83\}",
        "typeduck-runtime-bridge",
        "TypeDuckRuntime\\\\server\.exe",
        "TypeDuckRuntime",
        "normalizeGuidKey",
        "seedTypeDuckProfileBackendMapping",
        "backendMap_\\[kTypeDuckProfileGuid\\]|backendMap_\.insert"
    ) "PipeServer must define the fixed TypeDuck runtime bridge and seed the Phase 3 TypeDuck zh-HK profile GUID to it."
    Assert-Ordered $Failures $PipeServerCpp "seedTypeDuckProfileBackendMapping\(\)" "initInputMethods\(topDirPath\)" `
        "First-party TypeDuck mapping must be seeded before optional backend ime.json scanning."

    Assert-NotMatch $Failures $PipeServerCpp 'loadJsonFile\([^)]*backends\.json|backends\.json' `
        "PipeServer must not load or reference product backend discovery through backends.json."
    Assert-NotMatch $Failures $PipeServerCpp 'moqi-ime\\server\.exe|workingDir"\]\s*=\s*"moqi-ime"|backendFromName\("moqi-ime"\)' `
        "PipeServer fixed runtime bridge must not point at legacy moqi-ime backend paths."

    $sourceBackends = Join-Path $Root "backends.json"
    if (Test-Path -LiteralPath $sourceBackends -PathType Leaf) {
        Add-Failure $Failures "Source backends.json must be deleted; product runtime discovery must be fixed in PipeServer.cpp."
    }

    $stagedBackends = Join-Path $Root "installer\stage\win32\TypeDuckIME\backends.json"
    if (Test-Path -LiteralPath $stagedBackends -PathType Leaf) {
        Add-Failure $Failures "Staged TypeDuckIME payload must not contain top-level backends.json."
    }
}

function Assert-TransportPreserved {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $PipeClientCpp,
        [string] $BackendServerCpp,
        [string] $PipeServerCpp
    )

    Assert-AllMatch $Failures "$PipeClientCpp`n$BackendServerCpp`n$PipeServerCpp" @(
        "Proto::serializeMessage|Proto::serializeMessageBounded",
        "Proto::framePayload",
        "uv_pipe_init_windows_named_pipe",
        "stdinPipe_->write",
        "stdoutPipe_->startRead",
        "writeBackendResponse\(\s*response\.seq_num\(\)"
    ) "D-01 transport must remain named pipe plus backend stdin/stdout framed protobuf."
}

function Assert-IpcHardening {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $PipeServerCpp,
        [string] $PipeSecurityCpp,
        [string] $MoqiClientCpp
    )

    Assert-AllMatch $Failures "$PipeServerCpp`n$MoqiClientCpp" @(
        'TypeDuckIME',
        'pipe',
        'Launcher'
    ) "Client and server must construct the TypeDuck-owned launcher pipe namespace."

    Assert-NotMatch $Failures "$PipeServerCpp`n$MoqiClientCpp" '\\MoqiIM\\Launcher|\\Moqi\\Launcher' `
        "Launcher pipe namespace must not use legacy Moqi product paths."

    Assert-NotMatch $Failures $PipeSecurityCpp 'PROCESS_ALL_ACCESS|PROCESS_ALL_ACCESS_HEX|0x1fffff|SDDL_GENERIC_ALL\s+L";;;"\s*\+' `
        "Pipe allow ACEs must not grant process-style all-access rights."
    Assert-AllMatch $Failures $PipeSecurityCpp @(
        'FILE_GENERIC_READ',
        'FILE_GENERIC_WRITE',
        'SYNCHRONIZE',
        'SDDL_NETWORK'
    ) "Pipe DACL must use explicit pipe-compatible read/write/connect rights and keep the network deny ACE."

    Assert-AllMatch $Failures $MoqiClientCpp @(
        'GetNamedPipeServerProcessId',
        'OpenProcess\s*\(\s*PROCESS_QUERY_LIMITED_INFORMATION',
        'QueryFullProcessImageNameW',
        'TypeDuckLauncher',
        'programDir\(\)'
    ) "Client pipe sanity check must inspect server PID, image path, TypeDuck launcher shape, and configured program directory when available."
    Assert-NotMatch $Failures $MoqiClientCpp 'FIXME:\s*check the command line of the server[\s\S]*return\s+true;\s*\}\s*(?:// establish a connection|\r?\n\s*// establish a connection)' `
        "Client pipe sanity check must not return true unconditionally."
    Assert-NotMatch $Failures $MoqiClientCpp 'GetFileVersionInfo|VerQueryValue|WinVerifyTrust|CryptQueryObject|CertGetCertificateChain|SHA256|sha256|hash' `
        "Client pipe sanity check must not require exact version, signature, or hash checks."
}

$root = Resolve-RepoRoot -RequestedRoot $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$files = @{
    "MoqLauncher/PipeClient.cpp" = Read-RequiredFile $root "MoqLauncher/PipeClient.cpp"
    "MoqLauncher/PipeClient.h" = Read-RequiredFile $root "MoqLauncher/PipeClient.h"
    "MoqLauncher/BackendServer.cpp" = Read-RequiredFile $root "MoqLauncher/BackendServer.cpp"
    "MoqLauncher/BackendServer.h" = Read-RequiredFile $root "MoqLauncher/BackendServer.h"
    "MoqLauncher/PipeSecurity.cpp" = Read-RequiredFile $root "MoqLauncher/PipeSecurity.cpp"
    "MoqLauncher/PipeServer.cpp" = Read-RequiredFile $root "MoqLauncher/PipeServer.cpp"
    "MoqLauncher/PipeServer.h" = Read-RequiredFile $root "MoqLauncher/PipeServer.h"
    "MoqiTextService/MoqiClient.cpp" = Read-RequiredFile $root "MoqiTextService/MoqiClient.cpp"
    ".planning/product/protocol-fixtures/phase-04/launcher-recovery.json" = Read-RequiredFile $root ".planning/product/protocol-fixtures/phase-04/launcher-recovery.json"
}

Assert-Fixture $failures $files[".planning/product/protocol-fixtures/phase-04/launcher-recovery.json"]
Assert-ClientRecovery $failures $files["MoqLauncher/PipeClient.cpp"] $files["MoqLauncher/PipeClient.h"]
Assert-BackendRecovery $failures $files["MoqLauncher/BackendServer.cpp"] $files["MoqLauncher/BackendServer.h"] $files["MoqLauncher/PipeServer.cpp"] $files["MoqLauncher/PipeServer.h"]
Assert-TypeDuckMapping $failures $root $files["MoqLauncher/PipeServer.cpp"]
Assert-TransportPreserved $failures $files["MoqLauncher/PipeClient.cpp"] $files["MoqLauncher/BackendServer.cpp"] $files["MoqLauncher/PipeServer.cpp"]
Assert-IpcHardening $failures $files["MoqLauncher/PipeServer.cpp"] $files["MoqLauncher/PipeSecurity.cpp"] $files["MoqiTextService/MoqiClient.cpp"]

if ($Strict) {
    Assert-NotMatch $failures $files["MoqLauncher/PipeServer.cpp"] 'backendFromName\("moqi-ime"\)' `
        "Strict launcher guard: runtime bridge lookup must not hard-code moqi-ime."
}

if ($failures.Count -gt 0) {
    Write-Error ("TypeDuck launcher protocol check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck launcher protocol check passed."
exit 0
