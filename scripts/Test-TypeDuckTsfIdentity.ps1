param(
    [string]$RepoRoot = ".",
    [switch]$Strict
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
    param([string]$Path)
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Read-RequiredFile {
    param(
        [string]$Root,
        [string]$RelativePath
    )
    $path = Join-Path $Root $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing required file: $RelativePath"
    }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8
}

function Add-Failure {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$Message
    )
    [void]$Failures.Add($Message)
}

function Assert-Match {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$Text,
        [string]$Pattern,
        [string]$Message
    )
    if ($Text -notmatch $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-NotMatch {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$Text,
        [string]$Pattern,
        [string]$Message
    )
    if ($Text -match $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Get-FirstIndex {
    param(
        [string]$Text,
        [string]$Needle
    )
    return $Text.IndexOf($Needle, [System.StringComparison]::Ordinal)
}

function Test-Order {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$Text,
        [string]$BeforeNeedle,
        [string]$AfterNeedle,
        [string]$Message
    )
    $beforeIndex = Get-FirstIndex $Text $BeforeNeedle
    $afterIndex = Get-FirstIndex $Text $AfterNeedle
    if ($beforeIndex -lt 0 -or $afterIndex -lt 0 -or $beforeIndex -gt $afterIndex) {
        Add-Failure $Failures $Message
    }
}

function Test-TypeDuckClsid {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$CombinedSource,
        [string]$ModuleSource
    )
    Assert-Match $Failures $CombinedSource "7D92985A|0x7d92985a|0x7D92985A" `
        "Missing TypeDuck text service CLSID {7D92985A-BC53-47B5-A5CC-6E47F86B9D18} in first-party source."
    Assert-Match $Failures $CombinedSource "BC53|0xbc53" `
        "Missing TypeDuck text service CLSID segment BC53."
    Assert-Match $Failures $CombinedSource "47B5|0x47b5|0x47B5" `
        "Missing TypeDuck text service CLSID segment 47B5."
    Assert-Match $Failures $CombinedSource "A5CC|0xa5|0xA5" `
        "Missing TypeDuck text service CLSID bytes A5CC."
    Assert-Match $Failures $CombinedSource "6E47F86B9D18|0x6e,\s*0x47,\s*0xf8,\s*0x6b,\s*0x9d,\s*0x18|0x6E,\s*0x47,\s*0xF8,\s*0x6B,\s*0x9D,\s*0x18" `
        "Missing TypeDuck text service CLSID tail 6E47F86B9D18."
    Assert-NotMatch $Failures $ModuleSource "8F204C91|0x8f204c91|0x8F204C91" `
        "Moqi scaffold CLSID {8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F} still appears in MoqiImeModule.cpp."
}

function Test-TypeDuckProfileMetadata {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$CombinedSource
    )
    Assert-Match $Failures $CombinedSource "C6E8F5DF|0xc6e8f5df|0xC6E8F5DF" `
        "Missing TypeDuck zh-HK profile GUID {C6E8F5DF-6504-44F9-B7CF-17A195373A83}."
    Assert-Match $Failures $CombinedSource "6504|0x6504" `
        "Missing TypeDuck profile GUID segment 6504."
    Assert-Match $Failures $CombinedSource "44F9|0x44f9|0x44F9" `
        "Missing TypeDuck profile GUID segment 44F9."
    Assert-Match $Failures $CombinedSource "B7CF|0xb7|0xB7" `
        "Missing TypeDuck profile GUID bytes B7CF."
    Assert-Match $Failures $CombinedSource "17A195373A83|0x17,\s*0xa1,\s*0x95,\s*0x37,\s*0x3a,\s*0x83|0x17,\s*0xA1,\s*0x95,\s*0x37,\s*0x3A,\s*0x83" `
        "Missing TypeDuck profile GUID tail 17A195373A83."
    Assert-Match $Failures $CombinedSource "zh-HK" `
        "Missing first-party zh-HK locale metadata."
    Assert-Match $Failures $CombinedSource "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME" `
        "Missing bilingual TypeDuck profile display text."
    Assert-Match $Failures $CombinedSource "TypeDuckTextService\.dll" `
        "Missing deployed TypeDuck TSF DLL name TypeDuckTextService.dll."
    Assert-Match $Failures $CombinedSource "TYPEDUCK_PROGRAM_DIR" `
        "Missing TYPEDUCK_PROGRAM_DIR runtime program-directory override."
    Assert-Match $Failures $CombinedSource "TypeDuckIME" `
        "Missing TypeDuckIME default install directory identity."
}

function Test-RegistrationAuthority {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$DllEntrySource
    )
    Assert-Match $Failures $DllEntrySource "TypeDuckProfile" `
        "DllEntry.cpp does not include the first-party TypeDuckProfile registration source."
    Assert-Match $Failures $DllEntrySource "makeLangProfile|MakeLangProfile|typeDuckLangProfile|TypeDuckLangProfile" `
        "DllEntry.cpp does not construct a first-party TypeDuck LangProfileInfo."
    Assert-Match $Failures $DllEntrySource "langProfiles\.push_back" `
        "DllEntry.cpp does not seed langProfiles with a first-party profile."
    Test-Order $Failures $DllEntrySource "TypeDuckProfile" "langProfileFromJson" `
        "TypeDuck profile authority must appear before optional backend langProfileFromJson scanning."
    Test-Order $Failures $DllEntrySource "langProfiles.push_back" "for (const auto backendDir" `
        "DllRegisterServer must seed the TypeDuck profile before backend ime.json scanning."
    Assert-Match $Failures $DllEntrySource "registerServer\(Moqi::TypeDuck::serviceName\(\)|registerServer\(TypeDuck::serviceName\(\)|registerServer\(L`"TypeDuckTextService`"|registerServer\(L`"TypeDuck" `
        "DllRegisterServer must register the COM server with TypeDuck service naming."
}

function Test-ResourceMetadata {
    param(
        [System.Collections.Generic.List[string]]$Failures,
        [string]$ResourceSource,
        [string]$CMakeSource
    )
    Assert-Match $Failures $ResourceSource "LANG_CHINESE,\s*SUBLANG_CHINESE_HONGKONG|0xC04|0xc04" `
        "Resource script must use Traditional Chinese Hong Kong metadata."
    Assert-Match $Failures $ResourceSource "LANG_ENGLISH,\s*SUBLANG_ENGLISH_US|0x409|0x0409" `
        "Resource script must include English resource metadata."
    Assert-Match $Failures $ResourceSource "VALUE `"CompanyName`", `"TypeDuck`"" `
        "Resource CompanyName must be TypeDuck."
    Assert-Match $Failures $ResourceSource "VALUE `"FileDescription`", `"TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`"" `
        "Resource FileDescription must be bilingual TypeDuck text."
    Assert-Match $Failures $ResourceSource "VALUE `"ProductName`", `"TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`"" `
        "Resource ProductName must be bilingual TypeDuck text."
    Assert-Match $Failures $ResourceSource "VALUE `"OriginalFilename`", `"TypeDuckTextService\.dll`"" `
        "Resource OriginalFilename must be TypeDuckTextService.dll."
    Assert-Match $Failures $ResourceSource "VALUE `"InternalName`", `"TypeDuckTextService\.dll`"" `
        "Resource InternalName must be TypeDuckTextService.dll."
    Assert-NotMatch $Failures $ResourceSource "Chinese \(Simplified|SUBLANG_CHINESE_SIMPLIFIED|080404b0|墨奇输入法" `
        "Resource metadata still contains Simplified-only or Moqi scaffold metadata."
    Assert-Match $Failures $CMakeSource "OUTPUT_NAME\s+`"?TypeDuckTextService|PROPERTIES[\s\S]*OUTPUT_NAME[\s\S]*TypeDuckTextService" `
        "MoqiTextService CMake target must output TypeDuckTextService.dll."
}

$root = Resolve-RepoPath $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$moduleSource = Read-RequiredFile $root "MoqiTextService/MoqiImeModule.cpp"
$moduleHeader = Read-RequiredFile $root "MoqiTextService/MoqiImeModule.h"
$dllEntrySource = Read-RequiredFile $root "MoqiTextService/DllEntry.cpp"
$resourceSource = Read-RequiredFile $root "MoqiTextService/MoqiTextService.rc.in"
$cmakeSource = Read-RequiredFile $root "MoqiTextService/CMakeLists.txt"

$profileHeaderPath = Join-Path $root "MoqiTextService/TypeDuckProfile.h"
$profileSourcePath = Join-Path $root "MoqiTextService/TypeDuckProfile.cpp"
$profileHeader = if (Test-Path -LiteralPath $profileHeaderPath -PathType Leaf) {
    Get-Content -LiteralPath $profileHeaderPath -Raw -Encoding UTF8
} else {
    Add-Failure $failures "Missing MoqiTextService/TypeDuckProfile.h."
    ""
}
$profileSource = if (Test-Path -LiteralPath $profileSourcePath -PathType Leaf) {
    Get-Content -LiteralPath $profileSourcePath -Raw -Encoding UTF8
} else {
    Add-Failure $failures "Missing MoqiTextService/TypeDuckProfile.cpp."
    ""
}

$combinedSource = @(
    $moduleSource
    $moduleHeader
    $dllEntrySource
    $resourceSource
    $cmakeSource
    $profileHeader
    $profileSource
) -join "`n"

Test-TypeDuckClsid $failures $combinedSource $moduleSource
Test-TypeDuckProfileMetadata $failures $combinedSource
Test-RegistrationAuthority $failures $dllEntrySource
Test-ResourceMetadata $failures $resourceSource $cmakeSource

if ($Strict) {
    Assert-Match $failures $cmakeSource "TypeDuckProfile\.cpp" `
        "Strict mode: CMake must compile TypeDuckProfile.cpp."
    Assert-Match $failures $cmakeSource "TypeDuckProfile\.h" `
        "Strict mode: CMake must list TypeDuckProfile.h."
    Assert-Match $failures $moduleSource "programDirEnvVar\(\)|TYPEDUCK_PROGRAM_DIR" `
        "Strict mode: MoqiImeModule.cpp must prefer the centralized TYPEDUCK_PROGRAM_DIR source."
    Assert-Match $failures $moduleSource "legacyProgramDirEnvVar\(\)|MOQI_PROGRAM_DIR" `
        "Strict mode: MoqiImeModule.cpp should retain MOQI_PROGRAM_DIR only as a compatibility fallback during transition."
}

if ($failures.Count -gt 0) {
    Write-Host "TypeDuck TSF identity check failed:" -ForegroundColor Red
    for ($i = 0; $i -lt $failures.Count; $i++) {
        Write-Host ("[{0}] {1}" -f ($i + 1), $failures[$i])
    }
    exit 1
}

Write-Host "TypeDuck TSF identity check passed." -ForegroundColor Green
exit 0
