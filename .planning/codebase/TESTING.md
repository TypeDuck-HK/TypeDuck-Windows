# Testing Patterns

**Analysis Date:** 2026-06-23

## Test Framework

**Runner:**
- GoogleTest/GoogleMock 1.10.0, vendored at `libIME2/lib/googletest-release-1.10.0`.
- CTest is enabled from `libIME2/CMakeLists.txt` with `enable_testing()`.
- First-party test registration is in `libIME2/test/CMakeLists.txt`.

**Assertion Library:**
- GoogleTest assertions: `EXPECT_EQ` in `libIME2/test/ComPtr_test.cpp` and `libIME2/test/ComObject_test.cpp`.
- GoogleMock macros: `MOCK_METHOD` and `EXPECT_CALL` in `libIME2/test/ComObject_test.cpp`.

**Run Commands:**
```bash
cmake -S . -B build-vs32 -G "Visual Studio 17 2022" -A Win32  # Configure Win32 build
cmake --build build-vs32 --config Debug --target ComPtr_test   # Build one test target
ctest --test-dir build-vs32/libIME2/test -C Debug --output-on-failure  # Run registered libIME2 tests
```

## Test File Organization

**Location:**
- First-party tests are separate under `libIME2/test`.
- TypeDuck product tests may live under `Tests/<area>` when they exercise product code outside `libIME2`, for example `Tests/TypeDuckCandidateData`.
- Test-only CMake wiring lives in `libIME2/test/CMakeLists.txt`.
- Product test CMake wiring lives beside the tests and is registered from the root `CMakeLists.txt`, for example `Tests/TypeDuckCandidateData/CMakeLists.txt`.
- Focused TypeDuck guard scripts live under `scripts/`, for example `scripts/Test-TypeDuckCandidateWindow.ps1` for native candidate/dictionary popup rendering, toolkit bans, movement reveal, DPI, fallback, and no-activate placement.
- Vendored tests exist under `jsoncpp/test`, `libuv/test`, and `libIME2/lib/googletest-release-1.10.0`, but these are third-party code and not the first-party project testing pattern.

**Naming:**
- Use `*_test.cpp` for C++ GoogleTest files: `libIME2/test/ComPtr_test.cpp`, `libIME2/test/ComObject_test.cpp`.
- Match each test executable name to its source file and CTest name: `ComPtr_test` and `ComObject_test` in `libIME2/test/CMakeLists.txt`.

**Structure:**
```text
libIME2/
├── src/                 # Code under test
└── test/
    ├── CMakeLists.txt   # add_executable, target_link_libraries, add_test
    ├── ComPtr_test.cpp
    └── ComObject_test.cpp

Tests/
└── TypeDuckCandidateData/
    ├── CMakeLists.txt
    └── TypeDuckCandidateInfo_test.cpp
```

## Test Structure

**Suite Organization:**
```cpp
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <unknwn.h>
#include "ComPtr.h"
#include "ComObject.h"

TEST(TestComPtr, DefaultsToNull)
{
    Ime::ComPtr<IUnknownMock> ptr;
    EXPECT_EQ(ptr, nullptr);
}
```

**Patterns:**
- Include `gtest/gtest.h` and `gmock/gmock.h` first in test files: `libIME2/test/ComPtr_test.cpp`, `libIME2/test/ComObject_test.cpp`.
- Include Windows COM headers required by the unit under test: `<unknwn.h>` and `<msctf.h>` in `libIME2/test/ComObject_test.cpp`.
- Test COM reference counts directly through helper-visible `refCount()` methods: `libIME2/test/ComPtr_test.cpp`, `libIME2/test/ComObject_test.cpp`.
- Assert HRESULT outcomes explicitly with `S_OK`, `E_POINTER`, and `E_NOINTERFACE`: `libIME2/test/ComObject_test.cpp`.
- Prefer narrow behavior tests over broad integration tests when code can be isolated from TSF runtime state.

## Mocking

**Framework:** GoogleMock.

**Patterns:**
```cpp
class IUnknownMock : public Ime::ComObject<Ime::ComInterface<IUnknown>> {
public:
    MOCK_METHOD(HRESULT, QueryInterface, (REFIID riid, void** ppvObject),
                (Calltype(STDMETHODCALLTYPE), override));
};

EXPECT_CALL((*obj), OnChange(GUID_COMPARTMENT_KEYBOARD_DISABLED)).Times(1);
obj->OnChange(GUID_COMPARTMENT_KEYBOARD_DISABLED);
```

**What to Mock:**
- Mock COM interfaces when behavior can be verified without registering a real TSF service, as in `IUnknownMock` in `libIME2/test/ComPtr_test.cpp`.
- Mock TSF sink interfaces to verify callbacks and destruction paths, as in `ITfCompartmentEventSink` testing in `libIME2/test/ComObject_test.cpp`.
- For TypeDuck logic, mock backend/RPC boundaries rather than invoking the full launcher or Windows input stack. Candidate serialization and protocol helpers should be isolated around `proto/ProtoFraming.h` and generated `proto/moqi.pb.h` equivalents.

**What NOT to Mock:**
- Do not mock `Ime::ComPtr` or `Ime::ComObject` when they are the unit under test; use real objects as in `libIME2/test/ComPtr_test.cpp`.
- Do not rely on live TSF registration, installed keyboard profiles, `regsvr32`, or shell language settings in unit tests. Those belong in manual/installer verification around `SetupHelper/SetupHelper.cpp` and `MoqiTextService/DllEntry.cpp`.
- Do not test generated protobuf internals in `proto/moqi.pb.h` or `proto/moqi.pb.cc`; test hand-written protocol framing in `proto/ProtoFraming.h` and product message mapping in code that consumes generated types.

## Fixtures and Factories

**Test Data:**
```cpp
auto obj = new Ime::ComObject<
    Ime::ComInterface<Interface1>,
    Ime::ComInterface<Interface2>
>();

Ime::ComPtr<IUnknownMock> ptr{&obj};
EXPECT_EQ(obj.refCount(), 2);
```

**Location:**
- Test data and mock interface definitions are inline in each test file: `libIME2/test/ComPtr_test.cpp`, `libIME2/test/ComObject_test.cpp`.
- No shared fixture directory or factory library is present.
- For future TypeDuck tests, keep small COM mocks inline until duplication appears; add shared fixtures under `libIME2/test` or a new product test directory only when multiple test files need the same setup.

## Coverage

**Requirements:** None enforced. No coverage target, threshold, or reporting configuration is detected in `CMakeLists.txt`, `libIME2/CMakeLists.txt`, `.github/workflows/nightly.yml`, or `.github/workflows/release.yml`.

**View Coverage:**
```bash
# Not detected: no project coverage command exists.
```

## Test Types

**Unit Tests:**
- Present for `libIME2/src/ComPtr.h` and `libIME2/src/ComObject.h`.
- Tests are registered as independent executables in `libIME2/test/CMakeLists.txt` and linked with `gtest_main` and `gmock_main`.

**Integration Tests:**
- Not detected for first-party code. There are no tests for TSF activation, candidate windows, named-pipe RPC, launcher/backend lifecycle, installer registration, or generated protobuf request/response mapping in `MoqiTextService`, `MoqLauncher`, `SetupHelper`, `Preview`, or `proto`.

**E2E Tests:**
- Not used. CI workflows `.github/workflows/nightly.yml` and `.github/workflows/release.yml` build installer artifacts through `scripts/_all_in_package.ps1` but do not run CTest, install the IME, or exercise Windows input behavior.

## Common Patterns

**Async Testing:**
```cpp
// No first-party async test pattern exists.
// Current async code is production-only, for example Client::pollAsyncResponses()
// in MoqiTextService/MoqiClient.cpp and uv_async callbacks in MoqLauncher/PipeServer.cpp.
```

**Error Testing:**
```cpp
IUnknown* ptr = nullptr;
EXPECT_EQ(obj->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&ptr)), S_OK);

EXPECT_EQ(obj->QueryInterface(IID_IUnknown, nullptr), E_POINTER);

ITfTextInputProcessor* ptr2;
EXPECT_EQ(obj->QueryInterface(IID_ITfTextInputProcessor,
                              reinterpret_cast<void**>(&ptr2)), E_NOINTERFACE);
EXPECT_EQ(ptr2, nullptr);
```

## Build and CI Relationship

- Build automation is in `scripts/build.ps1`, `scripts/install.ps1`, and `scripts/_all_in_package.ps1`.
- CI release/nightly workflows use Windows runners and package installers in `.github/workflows/release.yml` and `.github/workflows/nightly.yml`.
- CI does not invoke `ctest`, `ComPtr_test`, or `ComObject_test`.
- Add or update test targets in `libIME2/test/CMakeLists.txt` for reusable TSF primitives. Add a new product-level test CMake subtree when TypeDuck-owned backend/protocol/candidate behavior appears outside `libIME2/src`.
- For native candidate-window UI parity, run `scripts/Test-TypeDuckCandidateWindow.ps1 -Strict` plus `MoqiCandidatePreview` and `MoqiTextService` builds; VM host-app evidence remains a separate acceptance layer.

## TypeDuck-Specific Testing Guidance

- Treat Moqi tests as scaffold coverage only. Keep the useful COM ownership tests in `libIME2/test`, but add TypeDuck-specific tests for Cantonese candidate ordering, Traditional Hong Kong Chinese strings, dictionary lookup filter behavior, TypeDuck librime fork integration boundaries, and installation metadata that targets Chinese (Traditional, Hong Kong).
- Test non-UI product logic before TSF integration. Protocol framing belongs near `proto/ProtoFraming.h`; candidate and settings mapping currently lives in `MoqiTextService/MoqiClient.cpp` and should be split/tested when TypeDuck behavior replaces it.
- Do not add tests that assert Moqi branding, Simplified Chinese menu strings, or unused fcitx-facing clutter from `MoqLauncher/PipeServer.cpp` and `MoqiTextService/MoqiClient.cpp` unless explicitly testing migration cleanup.

---

*Testing analysis: 2026-06-23*
