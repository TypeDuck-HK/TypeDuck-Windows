# Testing Patterns

**Analysis Date:** 2026-06-28

**Repositories:**
- `TypeDuck-Windows` means public repo `https://github.com/TypeDuck-HK/TypeDuck-Windows`.
- `TypeDuck-Windows-backend` means public repo `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend`.

## Test Framework

**Runner:**
- `TypeDuck-Windows`: GoogleTest/GoogleMock through vendored `libIME2/lib/googletest-release-1.10.0`, wired by CMake in `TypeDuck-Windows:libIME2/CMakeLists.txt` and focused test CMake files under `TypeDuck-Windows:Tests/`.
- `TypeDuck-Windows`: PowerShell guard scripts under `TypeDuck-Windows:scripts/` validate static contracts, packaging layout, runtime evidence, installer behavior, and release readiness.
- `TypeDuck-Windows-backend`: Go `testing` package across root package tests, `imecore/`, `mobilebridge/`, and `input_methods/*/`.
- `TypeDuck-Windows-backend`: PowerShell guard scripts under `TypeDuck-Windows-backend:scripts/` validate runtime packaging, candidate parity, and backend customization contracts.
- Config: `TypeDuck-Windows:CMakeLists.txt`, `TypeDuck-Windows:libIME2/CMakeLists.txt`, `TypeDuck-Windows:Tests/TypeDuckCandidateData/CMakeLists.txt`, `TypeDuck-Windows:Tests/TypeDuckProtocol/CMakeLists.txt`, `TypeDuck-Windows:Tests/TypeDuckSettings/CMakeLists.txt`, and `TypeDuck-Windows-backend:go.mod`.

**Assertion Library:**
- `TypeDuck-Windows`: `EXPECT_*` / `ASSERT_*` from GoogleTest in `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`, `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`, and `TypeDuck-Windows:Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp`.
- `TypeDuck-Windows`: PowerShell guard assertions use helper functions such as `Assert-Contains`, `Assert-File`, `Add-Failure`, and `Write-Error` in `TypeDuck-Windows:scripts/Test-TypeDuckCandidateData.ps1`, `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1`, and `TypeDuck-Windows:scripts/Test-TypeDuckReleaseArtifacts.ps1`.
- `TypeDuck-Windows-backend`: Go tests use `t.Fatal`, `t.Fatalf`, `t.Skip`, `t.Skipf`, `t.Helper`, `t.TempDir`, and `t.Setenv`.
- `TypeDuck-Windows-backend`: HTTP/download tests use `net/http/httptest` and generated zip fixtures in `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.

**Run Commands:**
```bash
# TypeDuck-Windows: configure/build Win32 and x64
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1

# TypeDuck-Windows: run all CTest-registered tests for a configured Win32 build
ctest --test-dir build-vs32 -C Debug --output-on-failure

# TypeDuck-Windows: build and run focused GoogleTest binaries
cmake --build build-vs32 --config Debug --target TypeDuckCandidateInfo_test
build-vs32/Tests/TypeDuckCandidateData/Debug/TypeDuckCandidateInfo_test.exe
cmake --build build-vs32 --config Debug --target ProtoFraming_test
build-vs32/Tests/TypeDuckProtocol/Debug/ProtoFraming_test.exe
cmake --build build-vs32 --config Debug --target ProtocolRecovery_test
build-vs32/Tests/TypeDuckProtocol/Debug/ProtocolRecovery_test.exe
cmake --build build-vs32 --config Debug --target TypeDuckPreferences_test
build-vs32/Tests/TypeDuckSettings/Debug/TypeDuckPreferences_test.exe

# TypeDuck-Windows: run representative PowerShell guards
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -Strict

# TypeDuck-Windows-backend: run Go tests eligible for the current OS/build tags
go test ./...

# TypeDuck-Windows-backend: run focused Rime/protocol tests
go test ./imecore ./input_methods/rime ./mobilebridge

# TypeDuck-Windows-backend: run runtime package build
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -RepoRoot . -RimeDataSource <schema-source>
```

## Test File Organization

**Location:**
- `TypeDuck-Windows` unit tests are separated under `Tests/` by product boundary: `Tests/TypeDuckCandidateData/`, `Tests/TypeDuckProtocol/`, and `Tests/TypeDuckSettings/`.
- `TypeDuck-Windows` TSF utility tests for `libIME2` are in `libIME2/test/`.
- `TypeDuck-Windows` static/evidence guards live in `scripts/Test-TypeDuck*.ps1`; evidence-producing harnesses live in `scripts/Invoke-TypeDuck*.ps1`; runtime staging logic lives in `scripts/Stage-TypeDuckRuntime.ps1`.
- `TypeDuck-Windows-backend` Go tests are co-located with package code: `server_test.go`, `imecore/protocol_test.go`, `input_methods/rime/rime_test.go`, `input_methods/rime/rime_runtime_test.go`, `mobilebridge/bridge_test.go`.
- `TypeDuck-Windows-backend` guard scripts live in `scripts/Test-TypeDuck*.ps1`.

**Naming:**
- `TypeDuck-Windows` GoogleTest binaries use target names ending in `_test`: `TypeDuckCandidateInfo_test`, `ProtoFraming_test`, `ProtocolRecovery_test`, `TypeDuckPreferences_test`.
- `TypeDuck-Windows` GoogleTest suites use `TEST(SuiteName, CaseName)` with descriptive PascalCase cases: `TEST(TypeDuckCandidateInfoTest, MapsHouToMultipleReadingRows)` and `TEST(TypeDuckProtoFraming, RejectsOversizedFrameAndDropsBody)`.
- `TypeDuck-Windows` PowerShell guards use `Test-TypeDuck<Area>.ps1`; scripts that collect evidence or run multi-step probes use `Invoke-TypeDuck<Area>.ps1`.
- `TypeDuck-Windows-backend` Go tests use `TestNameScenario` functions: `TestBuildProtoResponseIncludesClearedCompositionState`, `TestInstallSchemeSetPackageRejectsZipPathTraversal`, `TestPackagedTypeDuckRimeProvidesDictionaryLookupComments`, and `TestTypeDuckRimeModulesIncludeDictionaryLookup`.

**Structure:**
```text
TypeDuck-Windows/
├── Tests/TypeDuckCandidateData/       # Candidate parser/model GoogleTest target
├── Tests/TypeDuckProtocol/            # Protobuf framing and protocol recovery GoogleTest targets
├── Tests/TypeDuckSettings/            # Preferences persistence and side-effect GoogleTest target
├── libIME2/test/                      # Reusable TSF/COM utility tests
└── scripts/Test-TypeDuck*.ps1         # Static, packaging, installer, and evidence guards

TypeDuck-Windows-backend/
├── *_test.go                          # Root server/protocol tests
├── imecore/*_test.go                  # Request/response mapping tests
├── input_methods/rime/*_test.go       # Rime behavior, settings, runtime, and package tests
├── input_methods/rime/cloudclipboard/*_test.go
├── mobilebridge/*_test.go             # Mobile bridge behavior tests
└── scripts/Test-TypeDuck*.ps1         # Backend package and parity guards
```

## Test Structure

**Suite Organization:**
```cpp
// TypeDuck-Windows: Tests/TypeDuckProtocol/ProtoFraming_test.cpp
namespace {
std::string frameWithDeclaredSize(std::uint32_t declaredSize,
                                  const std::string& body = std::string()) {
  std::string frame(sizeof(std::uint32_t), '\0');
  std::memcpy(&frame[0], &declaredSize, sizeof(declaredSize));
  frame.append(body);
  return frame;
}
} // namespace

TEST(TypeDuckProtoFraming, RejectsOversizedFrameAndDropsBody) {
  Moqi::Proto::FrameBuffer buffer(8);
  const std::string frame = frameWithDeclaredSize(9, "abcdefghi");
  buffer.append(frame.data(), frame.size());
  std::string payload;
  EXPECT_FALSE(buffer.nextFrame(payload));
  EXPECT_EQ(buffer.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
}
```

```go
// TypeDuck-Windows-backend: input_methods/rime/scheme_set_download_test.go
func makeSchemeSetZip(t *testing.T, entries map[string]string) []byte {
	t.Helper()
	var buf bytes.Buffer
	zw := zip.NewWriter(&buf)
	for name, body := range entries {
		w, err := zw.Create(name)
		if err != nil {
			t.Fatalf("create zip entry: %v", err)
		}
		if _, err := w.Write([]byte(body)); err != nil {
			t.Fatalf("write zip entry: %v", err)
		}
	}
	if err := zw.Close(); err != nil {
		t.Fatalf("close zip: %v", err)
	}
	return buf.Bytes()
}
```

**Patterns:**
- Keep Windows C++ test fixtures local to the test file inside anonymous namespaces: row factories in `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp` and frame builders in `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`.
- Prefer exact protocol and candidate assertions over snapshot-size-only assertions: `raw_lookup_comment`, `candidate_entries`, `TypeDuckCandidatePage`, frame-size bounds, and multilingual candidate fields are tested in `TypeDuck-Windows:Tests/TypeDuckProtocol/` and `TypeDuck-Windows:Tests/TypeDuckCandidateData/`.
- In Go tests, use helper fake backends for Rime state and behavior: `testBackend`, `newTestBackend()`, `newIsolatedTestIME(t)`, and `resetNativeRuntimeStateForTest()` in `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`.
- Use `t.TempDir()` and `t.Setenv()` for APPDATA/LOCALAPPDATA isolation: `TypeDuck-Windows-backend:server_test.go`, `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`, and `TypeDuck-Windows-backend:input_methods/rime/appearance_themes_test.go`.
- Use `t.Skip` for environment-dependent native Rime checks rather than hard-failing missing runtime data: `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`, `TypeDuck-Windows-backend:input_methods/rime/test/real_init_test.go`, and `TypeDuck-Windows-backend:mobilebridge/bridge_test.go`.

## Mocking

**Framework:** GoogleMock is linked in `TypeDuck-Windows` test targets; Go uses hand-written fakes and standard-library test servers.

**Patterns:**
```cpp
// TypeDuck-Windows: libIME2/test/ComPtr_test.cpp
class MockIUnknown : public IUnknown {
public:
  MOCK_METHOD(HRESULT, QueryInterface, (REFIID riid, void** ppvObject),
              (Calltype(STDMETHODCALLTYPE), override));
};
```

```go
// TypeDuck-Windows-backend: input_methods/rime/rime_test.go
type testBackend struct {
	session         bool
	composition     string
	candidates      []candidateItem
	pageSize        int
	redeployCalls   int
	setOptionCalls  int
	currentSchemaID string
}
```

```go
// TypeDuck-Windows-backend: input_methods/rime/scheme_set_download_test.go
func withSchemeSetDownloadServer(t *testing.T, schemeName string, zipData []byte) *httptest.Server {
	t.Helper()
	mux := http.NewServeMux()
	var server *httptest.Server
	mux.HandleFunc("/manifest.json", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, `{"packages":[{"name":%q,"url":%q}]}`, schemeName, server.URL+"/package.zip")
	})
	server = httptest.NewServer(mux)
	return server
}
```

**What to Mock:**
- Mock COM interfaces and TSF utility ownership in `TypeDuck-Windows:libIME2/test/` with GoogleMock when the test is about reference counting or COM contract behavior.
- Fake Rime backends when testing request handling, candidate paging, settings propagation, redeploy behavior, and UI payloads in `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`.
- Use `httptest.Server` for download, manifest, and network-normalization behavior in `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.
- Stub async response senders through callbacks/channels, as in `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.

**What NOT to Mock:**
- Do not mock protobuf serialization/framing when the test is validating protocol boundaries. Use real generated types in `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`, `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp`, `TypeDuck-Windows-backend:server_test.go`, and `TypeDuck-Windows-backend:imecore/protocol_test.go`.
- Do not mock filesystem behavior for user-data and package-safety tests. Use `t.TempDir()` / `Test-Path` / real temporary files in `TypeDuck-Windows-backend:input_methods/rime/appearance_themes_test.go`, `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`, and `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1`.
- Do not replace Rime native tests with fake tests when validating dictionary lookup comments, runtime init duration, or packaged `rime.dll` behavior. Use the environment-gated native tests in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.

## Fixtures and Factories

**Test Data:**
```cpp
// TypeDuck-Windows: Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp
std::wstring HouGoodRow() {
  return L"1,好,hou2,,,,,,,adj,yue,,,,,good; well,अच्छा,اچھا,राम्रो,baik";
}
```

```go
// TypeDuck-Windows-backend: input_methods/rime/rime_test.go
func newTestBackend() *testBackend {
	return &testBackend{
		redeployOK: true,
		syncOK:     true,
		selectKeys: "1234567890",
		currentSchemaID: "rime_frost",
		pageSize:        6,
	}
}
```

**Location:**
- Candidate dictionary row fixtures are inline in `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp` so parser expectations stay close to tests.
- Protocol frame fixtures are generated inline in `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`.
- Preferences fixtures use temp JSON files in `TypeDuck-Windows:Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp`.
- Rime fake state and helper factories live in `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`.
- Download fixtures are generated zip archives in `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.
- Runtime/package evidence guards read structured JSON and staged files from `.planning/product/` and generated package trees through repo-relative paths in `TypeDuck-Windows:scripts/` and `TypeDuck-Windows-backend:scripts/`.

## Coverage

**Requirements:** No enforced coverage threshold is detected in either repo.

**View Coverage:**
```bash
# TypeDuck-Windows-backend: package coverage profile
go test ./... -coverprofile coverage.out
go tool cover -html coverage.out

# TypeDuck-Windows: GoogleTest coverage tooling is not configured
ctest --test-dir build-vs32 -C Debug --output-on-failure
```

**Current Coverage Shape:**
- `TypeDuck-Windows` has focused GoogleTest coverage for candidate parsing/modeling, protocol framing/recovery, and settings persistence: `Tests/TypeDuckCandidateData/`, `Tests/TypeDuckProtocol/`, `Tests/TypeDuckSettings/`.
- `TypeDuck-Windows` uses PowerShell guards for static contract coverage around TSF identity, launcher protocol, installer skeleton, runtime package pruning, release artifacts, UI evidence, VM installer verification harness, and release verification under `scripts/`.
- `TypeDuck-Windows-backend` has broad Go unit coverage around root protocol server handling, `imecore` protobuf mapping, Rime input behavior, Rime settings, scheme-set downloads, appearance themes, package surfaces, cloudclipboard guard helpers, and mobile bridge sessions.
- `TypeDuck-Windows-backend` native Rime runtime checks are present but environment-gated by Windows build tags and environment variables.

## Test Types

**Unit Tests:**
- `TypeDuck-Windows`: Candidate parsing and display preferences in `Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`.
- `TypeDuck-Windows`: Protocol frame bounds, malformed payload rejection, raw lookup separator preservation, and recovery behavior in `Tests/TypeDuckProtocol/`.
- `TypeDuck-Windows`: Preferences defaults, validation, Rime side-effect mapping, and failed-apply preservation in `Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp`.
- `TypeDuck-Windows`: COM pointer and object behavior in `libIME2/test/ComPtr_test.cpp` and `libIME2/test/ComObject_test.cpp`.
- `TypeDuck-Windows-backend`: Request/response protobuf mapping in `imecore/protocol_test.go`.
- `TypeDuck-Windows-backend`: Backend frame I/O, log path behavior, and fixed profile registration in `server_test.go`.
- `TypeDuck-Windows-backend`: Rime key handling, UI payloads, preferences, sync state, custom phrases, super abbreviations, and candidate page metadata in `input_methods/rime/rime_test.go`.

**Integration Tests:**
- `TypeDuck-Windows-backend`: Root server request/response flow in `server_integration_test.go` uses real server handling and generated protobuf frames.
- `TypeDuck-Windows-backend`: Runtime Rime tests in `input_methods/rime/rime_runtime_test.go`, `input_methods/rime/test/real_init_test.go`, `input_methods/rime/t9_separator_runtime_test.go`, `input_methods/rime/wubi_debug_test.go`, and `input_methods/rime/z_probe_sentence_candidates_test.go` exercise real native Rime data where available.
- `TypeDuck-Windows`: Probe and proof scripts such as `scripts/Invoke-TypeDuckProtocolRecoveryProbe.ps1`, `scripts/Invoke-TypeDuckTypingMvpProof.ps1`, `scripts/Invoke-TypeDuckReleaseVerification.ps1`, and `Tools/TypeduckBackendProbe/main.cpp` validate cross-process protocol and installer/release evidence.

**E2E Tests:**
- No automated GUI E2E runner is detected.
- VM/manual installer verification is represented by `TypeDuck-Windows:scripts/Invoke-TypeDuckVmInstallerVerification.ps1` and guarded by `TypeDuck-Windows:scripts/Test-TypeDuckVmInstallerVerification.ps1`.
- Release-level verification is aggregated by `TypeDuck-Windows:scripts/Invoke-TypeDuckReleaseVerification.ps1` and `TypeDuck-Windows:scripts/Test-TypeDuckReleaseVerification.ps1`.

## Backend Rime and Data Tests

**Native Rime Runtime:**
- Windows-only tests use `//go:build windows` in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`, `TypeDuck-Windows-backend:input_methods/rime/test/real_init_test.go`, `TypeDuck-Windows-backend:input_methods/rime/t9_separator_runtime_test.go`, `TypeDuck-Windows-backend:input_methods/rime/wubi_debug_test.go`, and `TypeDuck-Windows-backend:input_methods/rime/z_probe_sentence_candidates_test.go`.
- Tests that need installed/user Rime data call `t.Skip` when prerequisites are unavailable; do not turn these into unconditional failures.
- `MOQI_RIME_PACKAGE_DIR` enables `TestPackagedTypeDuckRimeProvidesDictionaryLookupComments` in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.
- `MOQI_RIME_INIT_MAX_MS` optionally enforces Rime init duration in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go` and `TypeDuck-Windows-backend:input_methods/rime/test/real_init_test.go`.
- `MOQI_REAL_APPDATA` can override APPDATA for native Rime test user directories in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.

**Dictionary Lookup and Rime Data:**
- Keep dictionary lookup module coverage in `TypeDuck-Windows-backend:input_methods/rime/rime_test.go` through `TestTypeDuckRimeModulesIncludeDictionaryLookup`.
- Keep packaged dictionary lookup behavior covered by `TestPackagedTypeDuckRimeProvidesDictionaryLookupComments` in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.
- Keep candidate parser coverage aligned with raw lookup comments in `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`.
- Keep parity/package guards using `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`, which checks packaged `rime.dll`, packaged schema/build files, `dictionary_lookup_filter`, and frontend/backend protocol bridge files through repo-relative arguments.

## Common Patterns

**Async Testing:**
```go
// TypeDuck-Windows-backend: input_methods/rime/scheme_set_download_test.go
asyncResponses := make(chan *imecore.Response, 4)
ime.SetAsyncResponseSender(func(resp *imecore.Response) {
	asyncResponses <- resp
})

timeout := time.After(2 * time.Second)
for success == nil {
	select {
	case asyncResp := <-asyncResponses:
		if asyncResp.TrayNotification != nil && strings.Contains(asyncResp.TrayNotification.Message, "download and install succeeded") {
			success = asyncResp
		}
	case <-timeout:
		t.Fatal("timed out waiting for async download completion")
	}
}
```

**Error Testing:**
```cpp
// TypeDuck-Windows: Tests/TypeDuckProtocol/ProtoFraming_test.cpp
const std::string malformed("\x08\xff\xff\xff", 4);
moqi::protocol::ClientRequest request;
EXPECT_FALSE(Moqi::Proto::parsePayload(malformed, request));
EXPECT_EQ(request.ByteSizeLong(), 0u);
```

```go
// TypeDuck-Windows-backend: input_methods/rime/scheme_set_download_test.go
_, err := installSchemeSetPackage(context.Background(), schemeSetDownloadPackage{
	Name: "BadSet",
	URL:  server.URL + "/package.zip",
})
if err == nil {
	t.Fatal("expected path traversal archive to fail")
}
if !strings.Contains(err.Error(), "invalid path") {
	t.Fatalf("expected illegal path error, got %v", err)
}
```

**Protocol Testing:**
- Use real protobuf generated types across both repos for schema-sensitive behavior: `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`, `TypeDuck-Windows-backend:imecore/protocol_test.go`, and `TypeDuck-Windows-backend:server_test.go`.
- Verify raw lookup separators and candidate detail payloads at both transport and model layers: `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp` and `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`.
- Keep protocol guard scripts in `TypeDuck-Windows:scripts/Test-TypeDuckProtocolContract.ps1`, `TypeDuck-Windows:scripts/Test-TypeDuckLauncherProtocol.ps1`, and `TypeDuck-Windows:scripts/Test-TypeDuckTypingClient.ps1` aligned with `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:imecore/protocol.go`.

**PowerShell Guard Testing:**
- Guards should resolve repo-relative paths from `-RepoRoot`, avoid reading secret files, and fail with actionable errors. Follow `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1` and `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`.
- Strict modes should require built artifacts only when the caller asks for strict validation, as in `TypeDuck-Windows:scripts/Test-TypeDuckCandidateData.ps1`.
- Aggregate proof scripts should write evidence manifests and then validate them with local helper functions, as in `TypeDuck-Windows:scripts/Invoke-TypeDuckReleaseVerification.ps1`.

**CI Test Behavior:**
- Current CI workflows build/package artifacts but do not run `ctest`, `go test`, lint, or coverage as dedicated steps.
- For changes that affect behavior, run the focused local commands listed above and include the exact commands in verification notes.
- For release packaging changes, verify both repos: run `TypeDuck-Windows-backend:scripts/build.ps1` to produce `scripts/build/TypeDuckRuntime`, then run `TypeDuck-Windows:scripts/_all_in_package.ps1` with the backend runtime and Rime data source.

---

*Testing analysis: 2026-06-28*
