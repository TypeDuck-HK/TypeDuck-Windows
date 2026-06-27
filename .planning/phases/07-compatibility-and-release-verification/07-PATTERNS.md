# Phase 7: Compatibility and Release Verification - Pattern Map

**Mapped:** 2026-06-27
**Files analyzed:** 16 likely new/modified files
**Analogs found:** 16 / 16
**Research mode:** skipped by request; used existing GSD docs and codebase only.

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `scripts/Invoke-TypeDuckReleaseVerification.ps1` | utility/script | batch, file-I/O | `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` + `scripts/Invoke-TypeDuckTypingMvpProof.ps1` | exact |
| `scripts/Test-TypeDuckReleaseVerification.ps1` | test/script | batch, transform | `scripts/Test-TypeDuckTypingMvpProof.ps1` + `scripts/Test-TypeDuckPhase05UiEvidence.ps1` | exact with anti-pattern caveat |
| `scripts/Test-TypeDuckReleaseArtifacts.ps1` | test/script | file-I/O, batch | `scripts/Test-TypeDuckInstallerSkeleton.ps1` + `scripts/Test-TypeDuckIconPackaging.ps1` | exact |
| `scripts/Test-TypeDuckPhase06Cleanup.ps1` | test/script | transform, batch | `scripts/Test-TypeDuckInstallerSkeleton.ps1` + `.planning/product/TYPEDUCK-BANNED-SURFACES.md` | role-match |
| `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` | utility/script | VM file-I/O, registry checks | same file | exact, needs Phase 7 no-screenshot edit |
| `Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp` | test | transform, request-response | `Tests/TypeDuckProtocol/ProtoFraming_test.cpp` | exact |
| `Tests/TypeDuckProtocol/ProtoFraming_test.cpp` | test | transform, request-response | same file | exact |
| `Tests/TypeDuckProtocol/CMakeLists.txt` | config/test wiring | batch | same file | exact |
| `Tools/TypeduckBackendProbe/main.cpp` | utility/probe | process I/O, framed protocol | same file | role-match |
| `Tools/TypeduckBackendProbe/CMakeLists.txt` | config/tool wiring | batch | same file | exact |
| `.planning/product/release-fixtures/phase-07/release-verification.json` | evidence/config | file-I/O, batch | `.planning/product/protocol-fixtures/phase-04/typing-mvp-evidence.json` | exact |
| `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md` | documentation/evidence | manual request-response | `.planning/product/protocol-fixtures/phase-04/windows-smoke-checklist.md` | role-match |
| `.planning/product/release-fixtures/phase-07/verification-notes.md` | documentation/evidence | file-I/O | `.planning/product/installer-fixtures/phase-03/verification-notes.md` | role-match |
| `installer/build-installer.ps1` | utility/script | file-I/O, batch | same file | exact |
| `scripts/_all_in_package.ps1` | utility/script | batch, file-I/O | same file | exact |
| `.github/workflows/release.yml`, `.github/workflows/nightly.yml` | CI config | batch, artifact upload | same files | role-match, current names are stale |

## Pattern Assignments

### `scripts/Invoke-TypeDuckReleaseVerification.ps1` (utility/script, batch + file-I/O)

**Analog:** `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` and `scripts/Invoke-TypeDuckTypingMvpProof.ps1`

**Script header and safety pattern** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 1-14):
```powershell
#Requires -Version 5.1
<#
.SYNOPSIS
  Collects VM-backed TypeDuck installer registration and uninstall evidence.

.DESCRIPTION
  This script is intentionally host-safe: it never runs the TypeDuck installer on
  the host machine.
  When guest automation is unavailable, use -ManualChecklistOnly to generate a
  checklist/evidence packet without touching the host or VM.
#>
```

**Copy this shape for Phase 7:** default to host-safe evidence generation, require explicit VM/installer inputs for any guest action, and support `-ManualChecklistOnly`.

**UTC, JSON, and hash helpers** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 46-87):
```powershell
function Format-UtcTimestamp {
    param([datetime] $Value = [datetime]::UtcNow)
    return $Value.ToUniversalTime().ToString(
        "yyyy-MM-dd'T'HH':'mm':'ss'Z'",
        [System.Globalization.CultureInfo]::InvariantCulture)
}

function Write-JsonFile {
    param([object] $Value, [string] $Path, [int] $Depth = 16)
    $json = $Value | ConvertTo-Json -Depth $Depth
    Set-Content -LiteralPath $Path -Value $json -Encoding UTF8
}

function Get-Sha256 {
    param([string] $Path)
    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    return (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
}
```

**Guard evidence aggregation** (`scripts/Invoke-TypeDuckTypingMvpProof.ps1` lines 343-358):
```powershell
$guardCommands += New-GuardEvidence -Id "protocol-contract" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckProtocolContract.ps1", "-RepoRoot", ".", "-Strict") ...
$guardCommands += New-GuardEvidence -Id "launcher-protocol" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckLauncherProtocol.ps1", "-RepoRoot", ".", "-Strict") ...
$guardCommands += New-GuardEvidence -Id "lookup-payload" -FilePath "pwsh" -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckLookupPayload.ps1", ...) ...
$guardCommands += New-GuardEvidence -Id "run-protoframing-test" -FilePath "build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe" ...
```

**Planner notes:**
- Include guards for clean install, reinstall/upgrade, uninstall, reboot-required registration, typing smoke, host-app notes, bitness, protocol recovery, settings update/redeploy failure, banned surfaces, artifact names, and hashes.
- Evidence root should be `.planning/product/release-fixtures/phase-07`.
- Do not add screenshot capture parameters or screenshot file slots.

### `scripts/Test-TypeDuckReleaseVerification.ps1` (test/script, batch + transform)

**Analog:** `scripts/Test-TypeDuckTypingMvpProof.ps1`

**Evidence validation pattern** (`scripts/Test-TypeDuckTypingMvpProof.ps1` lines 53-79, 93-106):
```powershell
function Assert-CommandSucceeded {
  param([System.Collections.Generic.List[string]] $Failures, [object] $CommandEvidence, [string] $Id, [string] $Root)
  if (-not $CommandEvidence) {
    Add-Failure $Failures "Missing guard command evidence: $Id"
    return
  }
  if ($CommandEvidence.status -ne "passed") {
    Add-Failure $Failures "Guard command '$Id' must be passed."
  }
  if (-not [string]::IsNullOrWhiteSpace([string] $CommandEvidence.artifact_path)) {
    $artifactPath = Resolve-ProofPath -BasePath $Root -Path ([string] $CommandEvidence.artifact_path)
    if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
      Add-Failure $Failures "Guard command '$Id' artifact_path does not exist: $($CommandEvidence.artifact_path)"
    }
  }
}
```

**Required Phase 7 case list should extend this pattern** (`scripts/Test-TypeDuckTypingMvpProof.ps1` lines 212-226):
```powershell
foreach ($required in @(
  "common-cantonese-input",
  "raw-lookup-payload",
  "reverse-lookup-marker",
  "cangjie-key-forwarding",
  "malformed-frame",
  "oversized-frame",
  "backend-timeout",
  "backend-restart"
)) {
  if ($requiredGoldenIds -notcontains $required) {
    Add-Failure $failures "Golden case list is missing required case '$required'."
  }
}
```

**Phase 7 required case IDs to use:**
`clean-install`, `reinstall-upgrade`, `uninstall-cleanup`, `reboot-required-registration`, `typing-smoke`, `host-app-notes`, `bitness-win32-x64`, `normal-cantonese-frames`, `raw-lookup-payload`, `reverse-lookup-if-supported`, `malformed-frame`, `oversized-frame`, `invalid-protobuf`, `backend-timeout`, `backend-restart-crash`, `stale-mismatched-sequence`, `settings-update-redeploy-failure`, `bounded-degraded-state`, `artifact-name-and-sha256`.

**Anti-pattern from Phase 5 aggregate guard:** do not copy screenshot slot requirements from `scripts/Test-TypeDuckPhase05UiEvidence.ps1` lines 224-242:
```powershell
$requiredPreviewScreenshots = @(
  "candidateBaseline",
  "candidateMultilingual",
  "dictionaryDetail",
  "compoundHousam",
  "reverseLookupCangjie"
)
Assert-SlotsPresent $manifest.automated.previewScreenshots $requiredPreviewScreenshots "Automated preview screenshot"
```

For Phase 7, the equivalent guard should assert that no `screenshots`, `previewScreenshots`, `screenshotManifest`, or image-capture paths are required.

### `scripts/Test-TypeDuckReleaseArtifacts.ps1` (test/script, file-I/O + batch)

**Analog:** `scripts/Test-TypeDuckInstallerSkeleton.ps1`, `installer/build-installer.ps1`, `scripts/_all_in_package.ps1`

**Static assertion helpers** (`scripts/Test-TypeDuckInstallerSkeleton.ps1` lines 28-74):
```powershell
function Read-RequiredFile {
    param([string] $Root, [string] $RelativePath)
    $path = Join-Path $Root $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required file not found: $RelativePath"
    }
    return [System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)
}

function Assert-Match { ... }
function Assert-NotMatch { ... }
```

**Installer artifact identity checks** (`scripts/Test-TypeDuckInstallerSkeleton.ps1` lines 127-145):
```powershell
Assert-AllMatch $Failures $Iss @(
    "DefaultDirName=\{autopf32\}\\TypeDuckIME",
    "OutputBaseFilename=typeduck-windows-ime-setup",
    "TypeDuckLauncher\.exe",
    "TypeDuckSetupHelper\.exe",
    "TypeDuckTextService\.dll"
) "Installer script must use TypeDuck AppId, CLSID, names, paths, startup, and scheduled-task identity."
Assert-NotMatch $Failures $Iss "MoqiLauncher\.exe|(?<!TypeDuck)SetupHelper\.exe|MoqiTextService\.dll|DefaultDirName=\{autopf32\}\\MoqiIM|OutputBaseFilename=moqi-im-windows-setup" `
    "Installer-controlled payload, install directory, and artifact names must not use Moqi deployed names."
```

**Package output pattern** (`scripts/_all_in_package.ps1` lines 153-166):
```powershell
Write-Host "== Step 3/3: Build TypeDuck installer package =="
Invoke-Step -FilePath "pwsh" -ArgumentList @(
    "-NoProfile",
    "-ExecutionPolicy", "Bypass",
    "-File", "`"$windowsInstallScript`"",
    "-RepoRoot", "`"$RepoRoot`"",
    "-MoqiImeSource", "`"$moqiImeRuntimeDir`""
) -WorkingDirectory $RepoRoot

$installerPath = Join-Path $RepoRoot "installer\dist\typeduck-windows-ime-setup.exe"
if (Test-Path -LiteralPath $installerPath) {
    Write-Host "OK: $installerPath"
} else {
    throw "Installer was not produced: $installerPath"
}
```

**Planner notes:**
- Add SHA-256 evidence for `installer/dist/typeduck-windows-ime-setup.exe`.
- Check final artifact names in `.github/workflows/release.yml` and `.github/workflows/nightly.yml`; current workflow lines 90-102 and 89-99 still reference `moqi-im-windows-*` and `moqi-im-windows-setup.exe`.

### `scripts/Test-TypeDuckPhase06Cleanup.ps1` (test/script, transform + batch)

**Analog:** `scripts/Test-TypeDuckInstallerSkeleton.ps1` and `.planning/product/TYPEDUCK-BANNED-SURFACES.md`

**Narrow legacy allowlist pattern** (`scripts/Test-TypeDuckInstallerSkeleton.ps1` lines 104-118):
```powershell
$legacyMatches = [regex]::Matches($Content, "Moqi|墨奇|MoqiIM|MoqiLauncher|MoqiTextService|moqi-im-windows|ChineseSimplified|chinesesimplified")
if ($legacyMatches.Count -eq 0) {
    return
}

if ($Content -notmatch "Legacy Moqi migration cleanup|Moqi scaffold compatibility|transition-only|legacy scaffold") {
    Add-Failure $Failures "$SurfaceName contains legacy Moqi/Simplified markers without an explicit narrow migration or scaffold-compatibility note."
}
```

**Strict no Simplified installer chrome** (`scripts/Test-TypeDuckInstallerSkeleton.ps1` lines 213-222):
```powershell
foreach ($entry in $Files.GetEnumerator()) {
    Assert-NotMatch $Failures $entry.Value "chinesesimplified|ChineseSimplified\.isl|输入法|检测到|安装程序|请在|卸载清理|墨奇" `
        "$($entry.Key) must not contain Simplified-only installer/setup chrome."
}
```

**Planner notes:**
- Phase 7 should verify Phase 6 outcomes, not implement broad cleanup.
- If this guard fails on product-visible surfaces, route the smallest repair back through the Phase 6 failure path, then rerun Phase 7 verification.

### `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` (utility/script, VM file-I/O + registry checks)

**Analog:** same file, but edit for Phase 7 no-screenshot boundary.

**Keep the non-screenshot evidence mechanics** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 340-360):
```powershell
function Get-FileEvidence {
    param([string] $Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        return [ordered]@{ path = $Path; exists = $false; sha256 = $null; length = $null; lastWriteTimeUtc = $null }
    }
    $item = Get-Item -LiteralPath $Path
    return [ordered]@{
        path = $Path
        exists = $true
        sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
        length = $item.Length
        lastWriteTimeUtc = Format-UtcTimestamp -Value $item.LastWriteTimeUtc
    }
}
```

**Keep install/uninstall execution pattern** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 470-528):
```powershell
$args = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG=`"$InstallLog`""
$process = Start-Process -FilePath $InstallerPath -ArgumentList $args -Wait -PassThru
...
$args = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG=`"$UninstallLog`""
$process = Start-Process -FilePath $exePath -ArgumentList $args -Wait -PassThru
```

**Keep install/uninstall checks** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 538-563):
```powershell
if (-not $Snapshot.files.win32Dll.exists) { $failures += "Win32 SysWOW64 TypeDuckTextService.dll missing after install." }
if (-not $Snapshot.files.x64Dll.exists) { $failures += "x64 System32 TypeDuckTextService.dll missing after install." }
if (-not ($Snapshot.registry.hklmTip.exists -or $Snapshot.registry.hkcuTip.exists)) { $failures += "TypeDuck CTF TIP key missing after install." }
...
if ($Snapshot.files.installDir.exists) { $failures += "TypeDuck install directory remains after uninstall." }
if ($Snapshot.scheduledTask.exists) { $failures += "TypeDuck re-registration scheduled task remains after uninstall." }
```

**Remove or rewrite these screenshot prompts** (`scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 127-144 and 610-614):
```powershell
- Capture screenshots only of installer/input settings surfaces; avoid personal typed content.
- Save screenshot as ``installer-ui.png`` if possible.
- Save screenshot as ``zh-hk-input-settings.png`` if possible.
...
- If a screenshot is needed for product review, capture Windows Settings ...
```

Phase 7 replacement: "Human visual judgement: user directly observes DPI and host-app behavior in the VM; record notes only, no screenshot files required."

### `Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp` and `ProtoFraming_test.cpp` (test, transform + request-response)

**Analog:** `Tests/TypeDuckProtocol/ProtoFraming_test.cpp`

**Imports and helper pattern** (`Tests/TypeDuckProtocol/ProtoFraming_test.cpp` lines 1-19):
```cpp
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdint>
#include <cstring>
#include <string>

#include "proto/ProtoFraming.h"
#include "proto/moqi.pb.h"

std::string frameWithDeclaredSize(std::uint32_t declaredSize,
                                  const std::string &body = std::string()) {
  std::string frame(sizeof(std::uint32_t), '\0');
  std::memcpy(&frame[0], &declaredSize, sizeof(declaredSize));
  frame.append(body);
  return frame;
}
```

**Oversized/malformed frame tests** (`Tests/TypeDuckProtocol/ProtoFraming_test.cpp` lines 23-57):
```cpp
TEST(TypeDuckProtoFraming, RejectsOversizedFrameAndDropsBody) {
  Moqi::Proto::FrameBuffer buffer(8);
  const std::string frame = frameWithDeclaredSize(9, "abcdefghi");

  buffer.append(frame.data(), frame.size());

  std::string payload;
  EXPECT_FALSE(buffer.nextFrame(payload));
  EXPECT_EQ(buffer.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
  EXPECT_TRUE(buffer.hasViolation());
  EXPECT_EQ(buffer.bufferedBytes(), 0u);
}
```

**Invalid protobuf and raw lookup preservation tests** (`Tests/TypeDuckProtocol/ProtoFraming_test.cpp` lines 59-90):
```cpp
TEST(TypeDuckProtoFraming, MalformedPayloadDoesNotParseRequestOrResponse) {
  const std::string malformed("\x08\xff\xff\xff", 4);
  moqi::protocol::ClientRequest request;
  moqi::protocol::ServerResponse response;
  EXPECT_FALSE(Moqi::Proto::parsePayload(malformed, request));
  EXPECT_FALSE(Moqi::Proto::parsePayload(malformed, response));
}

TEST(TypeDuckProtocol, RawLookupSeparatorsSurviveCandidateComments) {
  moqi::protocol::CandidateEntry candidate;
  const std::string rawComment =
      "\vReverse lookup marker\f\rmatch_input_buffer,honzi,jyutping\rnei,\xE4\xBD\xA0,nei5";
  candidate.set_raw_lookup_comment(rawComment);
  candidate.set_comment(rawComment);
  ...
  EXPECT_EQ(parsed.raw_lookup_comment(), rawComment);
}
```

**Phase 7 additions:**
- Add coverage for stale/mismatched sequence where possible at helper level.
- Add valid normal Cantonese frame serialization case.
- Add invalid protobuf payload case if not already sufficient.
- Keep tests isolated from live TSF registration.

### `Tests/TypeDuckProtocol/CMakeLists.txt` (config/test wiring, batch)

**Analog:** same file

**Pattern** (`Tests/TypeDuckProtocol/CMakeLists.txt` lines 1-18):
```cmake
add_executable(ProtoFraming_test ProtoFraming_test.cpp)

target_include_directories(ProtoFraming_test PRIVATE
  "${MOQI_GENERATED_PROTO_ROOT}"
  "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ProtoFraming_test
  gtest_main
  gmock_main
  ${MOQI_PROTOBUF_TARGET}
)

add_dependencies(ProtoFraming_test moqi_proto_cpp_gen)
target_sources(ProtoFraming_test PRIVATE "${MOQI_PROTO_CPP}")
add_test(NAME ProtoFraming_test COMMAND ProtoFraming_test)
```

Root registration already includes TypeDuck test subtrees (`CMakeLists.txt` lines 220-222):
```cmake
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckCandidateData")
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckSettings")
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckProtocol")
```

### `Tools/TypeduckBackendProbe/main.cpp` (utility/probe, process I/O + framed protocol)

**Analog:** same file

**Probe imports and evidence structures** (`Tools/TypeduckBackendProbe/main.cpp` lines 1-37):
```cpp
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "proto/ProtoFraming.h"
#include "proto/moqi.pb.h"

constexpr std::uint32_t kMaxFrameBytes = 4 * 1024 * 1024;
constexpr DWORD kDefaultTimeoutMs = 15000;

struct Options {
  std::filesystem::path serverExe;
  std::filesystem::path workingDir;
  std::filesystem::path runtimeManifest;
  std::string runtimeManifestSha256;
  ...
};
```

**JSON escaping and UTC pattern** (`Tools/TypeduckBackendProbe/main.cpp` lines 90-164):
```cpp
std::string jsonEscape(const std::string &value) { ... }
std::string jsonString(const std::string &value) {
  return "\"" + jsonEscape(value) + "\"";
}

std::string nowUtcIso() {
  SYSTEMTIME st{};
  GetSystemTime(&st);
  std::ostringstream out;
  out << std::setfill('0') << std::setw(4) << st.wYear << "-";
  ...
  return out.str();
}
```

**Planner notes:**
- Prefer extending this probe only if a repeatable protocol/recovery check cannot be expressed through existing PowerShell guard plus C++ unit tests.
- Do not create a broad host-app automation framework around it.

### `.planning/product/release-fixtures/phase-07/release-verification.json` (evidence/config, file-I/O)

**Analog:** `.planning/product/protocol-fixtures/phase-04/typing-mvp-evidence.json`

**Evidence schema pattern** (`typing-mvp-evidence.json` first fields):
```json
{
  "phase": "04-typeduck-protocol-and-typing-mvp",
  "plan": "04-04",
  "generated_at": "2026-06-23T21:44:16Z",
  "runner": "scripts\\Invoke-TypeDuckTypingMvpProof.ps1",
  "repo": {
    "root": "D:\\VSProjects\\moqi-im-windows",
    "commit": "..."
  },
  "guard_commands": [
    {
      "id": "protocol-contract",
      "status": "passed",
      "command": "pwsh -NoProfile -ExecutionPolicy Bypass -File ...",
      "timestamp": "2026-06-23T21:43:54Z",
      "artifact_path": "...",
      "observed_outcome": "command exited 0"
    }
  ]
}
```

**Phase 7 required top-level sections:**
`phase`, `generated_at`, `runner`, `repo`, `installer`, `guard_commands`, `automated_checks`, `vm_evidence`, `interactive_human_checks`, `protocol_recovery`, `host_apps`, `dpi`, `release_artifacts`, `limitations`, `rerun_after_repairs`.

**DPI fields should be notes, not image paths:**
```json
"dpi": {
  "mode": "human-judged-no-screenshots",
  "requiredScales": ["100%", "140% if available", "175%", "200%"],
  "notesPath": ".planning/product/release-fixtures/phase-07/interactive-vm-checklist.md"
}
```

### `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md` (documentation/evidence, manual)

**Analog:** `.planning/product/protocol-fixtures/phase-04/windows-smoke-checklist.md`

**Checklist pattern** (`windows-smoke-checklist.md` lines 1-18):
```markdown
# TypeDuck Phase 4 Windows Smoke Checklist

Use a disposable Windows VM or equivalent checkpointed machine. Do not install the IME on the developer host as the first verification target.

## Preflight

- [ ] Build or stage the current TypeDuck installer/profile payload from this repository.
- [ ] Confirm the VM is disposable or checkpointed before TSF registration.
- [ ] Install TypeDuck in the VM.
- [ ] Select TypeDuck under Chinese (Traditional, Hong Kong).
```

**Typing/recovery checklist pattern** (`windows-smoke-checklist.md` lines 19-44):
```markdown
## Typing Smoke
- [ ] Type ordinary Cantonese input such as `nei`.
- [ ] Composition text appears while typing.
- [ ] Candidate list appears.
- [ ] Candidate paging works.
- [ ] Numeric selection commits a candidate.

## Recovery Smoke
- [ ] Stop or restart the backend/launcher during typing.
- [ ] Host app remains responsive; no multi-second hang is observed.
- [ ] TypeDuck clears composition/candidates or enters degraded state.
- [ ] TypeDuck can recover after backend/launcher restart.
```

**Phase 7 additions:**
- Host apps: Notepad, browser, Office/Office-like app if available, terminal/console context, elevated app, awkward TSF host if available.
- DPI: user tries 100%, 140% if available, 175%, 200% and records pass/fail/notes.
- Explicit note: multi-monitor was already human verified in Phase 5; do not make it a new screenshot gate.

### `.planning/product/release-fixtures/phase-07/verification-notes.md` (documentation/evidence, file-I/O)

**Analog:** `.planning/product/installer-fixtures/phase-03/verification-notes.md`

**Evidence summary pattern** (`verification-notes.md` lines 3-21):
```markdown
**Mode:** automated Hyper-V / PowerShell Direct
**Status:** complete
**VM:** My Virtual Machine
**Started:** 2026-06-23T16:55:30Z
**Completed:** 2026-06-23T16:55:55Z
**Installer SHA-256:** 8c678b0f19491319d7b5026911b98d97edf0f26f9ec02fdad8a33be33acdcfc0

## Verification Results

- Installer exit code: 0
- Uninstaller exit code: 0
- CLSID: {7D92985A-BC53-47B5-A5CC-6E47F86B9D18}
- Profile GUID: {C6E8F5DF-6504-44F9-B7CF-17A195373A83}
- Display text: TypeDuck 粵語輸入法 / TypeDuck Cantonese IME
- Win32 DLL: C:\Windows\SysWOW64\TypeDuckTextService.dll
- x64 DLL: C:\Windows\System32\TypeDuckTextService.dll
```

**Rewrite screenshot section from Phase 3:** existing lines 31-35 are now an anti-pattern for Phase 7. Replace with:
```markdown
## Human Visual/DPI Notes

- No screenshot files were required or captured by this verification packet.
- DPI scales judged interactively: 100%, 140% if available, 175%, 200%.
- The user directly judged candidate/settings/About visual behavior in the VM.
```

### `.github/workflows/release.yml` and `.github/workflows/nightly.yml` (CI config, artifact upload)

**Analog:** same files, but current paths are stale.

**Current stale release workflow anti-pattern** (`.github/workflows/release.yml` lines 87-102):
```yaml
- name: Upload workflow artifact
  uses: actions/upload-artifact@v4
  with:
    name: moqi-im-windows-release-${{ github.event.release.tag_name || github.sha }}
    path: ${{ github.workspace }}/moqi-im-windows/installer/dist/moqi-im-windows-setup.exe

$assetPath = Join-Path $PWD "installer\dist\moqi-im-windows-setup.exe"
gh release upload $tag $assetPath --clobber
```

**Current stale nightly workflow anti-pattern** (`.github/workflows/nightly.yml` lines 86-99):
```yaml
name: moqi-im-windows-nightly-${{ github.sha }}
path: ${{ github.workspace }}/moqi-im-windows/installer/dist/moqi-im-windows-setup.exe
...
$assetPath = Join-Path $PWD "installer\dist\moqi-im-windows-setup.exe"
```

**Planner notes:**
- Update to `typeduck-windows-ime-release-*`, `typeduck-windows-ime-nightly-*`, and `installer/dist/typeduck-windows-ime-setup.exe`.
- Use `pwsh` in new scripts/commands; existing workflow uses `powershell.exe` at release lines 82 and nightly lines 81, which conflicts with project guidance for Unicode-safe scripts.
- Add artifact hash generation or validation in the release verification script if workflow edits are in scope.

## Shared Patterns

### PowerShell Script Conventions

**Source:** `scripts/Test-TypeDuckInstallerSkeleton.ps1` lines 12-18 and `scripts/_all_in_package.ps1` lines 36-60

```powershell
param(
    [string] $RepoRoot = "",
    [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Invoke-Step {
    ...
    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $FilePath"
    }
}
```

Apply to all Phase 7 scripts. Prefer command strings in evidence that start with:
`pwsh -NoProfile -ExecutionPolicy Bypass -File ...`

### Protocol Bounds and Recovery

**Source:** `proto/ProtoFraming.h` lines 11-21, 42-77, 140-168

```cpp
inline constexpr std::size_t kMaxClientFramePayloadBytes = 1024 * 1024;
inline constexpr std::size_t kMaxBackendFramePayloadBytes = 1024 * 1024;

enum class FrameError {
  None,
  Incomplete,
  PayloadTooLarge,
  MalformedHeader,
};

bool nextFrame(std::string &payload) {
  if (payloadSize > maxPayloadBytes_) {
    fail(FrameError::PayloadTooLarge);
    return false;
  }
  ...
}

template <typename Message>
bool parsePayload(const std::string &payload, Message &message) {
  if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
    message.Clear();
    return false;
  }
  return true;
}
```

Apply to malformed frames, oversized frames, invalid protobuf, normal frames, dictionary payload preservation, and bounded degraded-state checks.

### Launcher Error Responses and Stale Sequence Handling

**Source:** `MoqLauncher/PipeClient.cpp` lines 180-323 and 433-445

```cpp
if (!Proto::parsePayload(payload, request)) {
    writeTypeDuckErrorResponse(
        0,
        moqi::protocol::TYPEDUCK_ERROR_MALFORMED_PAYLOAD,
        "Malformed TypeDuck client protobuf payload",
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true);
    continue;
}

if (pendingSeqNum_ != 0 && seqNum != pendingSeqNum_) {
    logger()->warn("Dropping stale backend response ...");
    return false;
}

void PipeClient::onRequestTimeout() {
    writeTypeDuckErrorResponse(
        pendingSeqNum_,
        moqi::protocol::TYPEDUCK_ERROR_BACKEND_TIMEOUT,
        "TypeDuck backend request timed out; restarting backend bridge",
        moqi::protocol::TYPEDUCK_HEALTH_RESTARTING,
        true);
    backend_->restartProcess();
}
```

Phase 7 should verify these paths with automated/static checks and, where practical, a live protocol probe.

### Backend Restart and Crash Behavior

**Source:** `MoqLauncher/BackendServer.cpp` lines 343-394

```cpp
stdoutFrameBuf_.append(buf, len);
if (stdoutFrameBuf_.hasViolation()) {
  pipeServer_->notifyClientsOfBackendError(...);
  stdoutFrameBuf_.clear();
  restartProcess();
  return;
}
...
void BackendServer::onReadError(int status) {
  pipeServer_->notifyClientsOfBackendError(
      this,
      moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
      "TypeDuck backend bridge read failed; restarting backend bridge",
      moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
      true,
      uv_strerror(status));
  restartProcess();
}
```

Apply to backend crash/restart verification and bounded degraded-state evidence.

### Installer, Bitness, and Reboot-Required Registration

**Source:** `SetupHelper/SetupHelper.cpp` lines 13-22, 692-783, 786-814

```cpp
constexpr int kExitSuccess = 0;
constexpr int kExitFailure = 1;
constexpr int kExitRestartRequired = 2;
constexpr wchar_t kProgramDirEnvVar[] = L"TYPEDUCK_PROGRAM_DIR";
constexpr wchar_t kReregisterTaskName[] = L"TypeDuckIME-ReRegisterTSF";
constexpr wchar_t kTextServiceDllName[] = L"TypeDuckTextService.dll";
...
const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / kTextServiceDllName;
const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / kTextServiceDllName;
const fs::path regsvr32 = fs::path(GetSyswow64DirectoryPath()) / L"regsvr32.exe";
const fs::path regsvr64 = fs::path(GetNativeSystemDirectoryPath()) / L"regsvr32.exe";
...
if (reboot_required) {
  if (!ScheduleReregisterTask(options, schedule_error)) { ... }
  return kExitRestartRequired;
}
```

Apply to clean install, reinstall/upgrade, uninstall, reboot-required registration, and bitness evidence.

### Inno Setup Install/Uninstall Flow

**Source:** `installer/MoqiTsf.iss` lines 19-42, 201-247, 280-299

```pascal
DefaultDirName={autopf32}\TypeDuckIME
ArchitecturesAllowed=x64
OutputBaseFilename=typeduck-windows-ime-setup
SetupIconFile=..\TypeDuckSettings\assets\TypeDuck.ico
...
if CurStep = ssPostInstall then
begin
  if not RunSetupHelper(BuildInstallSetupHelperParameters('/i'), ResultCode) then
    HandleSetupHelperResult('TypeDuck setup-helper install / TypeDuck 安裝工具安裝', ResultCode);
  ...
  else if ResultCode = SetupHelperExitRestartRequired then
    HelperInstallNeedsRestart := True;
end;
...
if CurUninstallStep = usPostUninstall then
begin
  RegPurgeTypeDuckResiduals;
  RegPurgeLegacyMoqiResiduals;
end;
```

Apply to installer flow evidence and uninstall cleanup checks.

## Anti-Patterns

### Screenshot Automation or Screenshot Manifests

Do not create or require:
- VM screenshot capture scripts.
- Screenshot manifests.
- Automated screenshot comparison.
- Required screenshot file paths in release evidence.
- Preview screenshot acceptance gates.

Existing code to avoid copying:
- `scripts/Test-TypeDuckPhase05UiEvidence.ps1` lines 224-242 requires automated preview screenshots.
- `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` lines 127-144 and 610-614 still suggest optional screenshot files from Phase 3. Phase 7 should rewrite these to human observation notes only.

### Reintroducing Multi-Monitor Screenshot Gates

Phase 5 already human verified multi-monitor behavior. Phase 7 may record a note that it was previously verified, but should not add new multi-monitor screenshot automation or make it a release-blocking screenshot gate.

### Broad v1-Out-of-Scope Integration Infrastructure

Do not add a full UI automation farm, screenshot diff harness, new VM orchestration framework, cloud service checks, sync infrastructure, or side-by-side channel matrix. Use focused scripts, C++ unit tests, existing probes, and manual VM notes.

### Phase 7 Product Feature Work

Non-test product code changes belong in Phase 6 unless Phase 7 detects a release-blocking failure. If a verification case fails, route the smallest repair through a focused failure-repair path and rerun only the affected checks plus aggregate release verification.

### Strict Executable/Version Identity Rejection

Phase 6 explicitly prefers compatibility-tolerant checks. Do not add signature/version rules that reject legitimate past or future TypeDuck versions during upgrade/downgrade unless a concrete release blocker demands it.

## No Analog Found

| File/Idea | Role | Data Flow | Reason |
|-----------|------|-----------|--------|
| Automated DPI screenshot comparison harness | test/infrastructure | screenshot/image processing | Explicitly out of scope by Phase 7 decisions D-01 through D-05. Use human VM judgement at 100%, 140% if available, 175%, 200%. |
| Full host-app UI automation suite | test/infrastructure | GUI automation | No current first-party pattern, and broader integration infrastructure is v1-out-of-scope. Use interactive host-app checklist plus deterministic logs/checks. |

## Planner Notes

- Phase 7 should create an aggregate release evidence packet under `.planning/product/release-fixtures/phase-07`.
- Include repeatable non-screenshot verification for clean install, reinstall/upgrade, uninstall, reboot-required registration, typing smoke, host-app coverage, bitness, protocol recovery, final artifact naming, and SHA-256 hashes.
- Protocol/recovery coverage must include normal Cantonese input frames, dictionary lookup payload preservation/parsing expectations, reverse lookup where supported, malformed/oversized frames, invalid protobuf, timeouts, backend restart/crash, stale/mismatched sequence, settings update/redeploy failure, and bounded degraded states.
- Host-app and DPI judgements stay interactive. Required DPI list: 100%, 140% if available, 175%, 200%.
- Release workflow paths currently still reference Moqi artifact names. Planner should include a release artifact name/hash check and likely workflow/script corrections unless Phase 6 has already fixed them before execution.

## Metadata

**Analog search scope:** `scripts/`, `Tests/`, `Tools/`, `proto/`, `MoqLauncher/`, `MoqiTextService/`, `SetupHelper/`, `installer/`, `.github/workflows/`, `.planning/product/`
**Files scanned:** `rg --files` over PowerShell, C++, headers, proto, Inno, workflows, CMake, and planning docs
**Pattern extraction date:** 2026-06-27
