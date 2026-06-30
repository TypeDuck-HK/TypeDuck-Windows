#Requires -Version 5.1
<#
.SYNOPSIS
  Generates THIRD_PARTY_NOTICES.txt from pinned license-text URLs.
#>
[CmdletBinding()]
param(
  [string] $OutputPath = (Join-Path (Split-Path -Parent $PSScriptRoot) "THIRD_PARTY_NOTICES.txt"),
  [int] $TimeoutSec = 30
)

$ErrorActionPreference = "Stop"

$recursiveChecks = @(
  "TypeDuck-Windows: checked .gitmodules and CMake dependency roots: jsoncpp, libIME2, libuv, spdlog, Protocol Buffers, protobuf bundled utf8_range, protobuf CMake Abseil dependency, and spdlog bundled fmt.",
  "TypeDuck-Windows-backend: checked go.mod/go module closure: google.golang.org/protobuf, gopkg.in/yaml.v3, golang.org/x/mobile, golang.org/x/mod, golang.org/x/sync, golang.org/x/tools, and transitive modules reported by go mod download all.",
  "librime: checked recursive submodules/deps: glog, googletest, leveldb, marisa-trie, OpenCC, yaml-cpp, and rime-dictionary-lookup-filter.",
  "OpenCC: checked deps directory recursively: darts-clone-0.32, google-benchmark, googletest-1.15.0, marisa-0.3.1, pybind11-2.13.1, rapidjson-1.1.0, tclap-1.2.5, plus RapidJSON's nested msinttypes notice.",
  "schema: checked bundled schema/data sources: luna-pinyin, loengfan, stroke, cangjie5, cangjie3, and OpenCC."
)

foreach ($check in $recursiveChecks) {
  Write-Verbose $check
}

function New-NoticeEntry {
  param(
    [string] $Name,
    [string] $License,
    [string[]] $Source,
    [string] $TextUrl,
    [string] $Extract = "Full"
  )

  [PSCustomObject]@{
    Name = $Name
    License = $License
    Source = $Source
    TextUrl = $TextUrl
    Extract = $Extract
  }
}

function Get-UrlText {
  param([string] $Url)

  $response = Invoke-WebRequest -Uri $Url -UseBasicParsing -TimeoutSec $TimeoutSec -Verbose:$false
  return [string] $response.Content
}

function Select-LicenseText {
  param(
    [string] $Text,
    [string] $Extract
  )

  if ($Extract -ne "LeadingSlashCommentBlock") {
    return $Text
  }

  $selected = [System.Collections.Generic.List[string]]::new()
  foreach ($line in ($Text -split '\r?\n')) {
    $trimmed = $line.TrimStart()
    if ($trimmed.StartsWith("//")) {
      $selected.Add(($trimmed -replace '^// ?', '')) | Out-Null
      continue
    }
    if ($selected.Count -gt 0 -and $trimmed.Length -eq 0) {
      $selected.Add("") | Out-Null
      continue
    }
    if ($selected.Count -gt 0) {
      break
    }
  }

  if ($selected.Count -eq 0) {
    throw "No leading slash-comment license block found."
  }
  return ($selected -join "`n")
}

function Normalize-NoticeText {
  param([string] $Text)

  $textWithoutFormFeeds = $Text -replace "`f", ""
  $lines = $textWithoutFormFeeds -split '\r?\n'
  $trimmedLines = foreach ($line in $lines) {
    $line -replace '[ \t]+$', ''
  }

  $start = 0
  while ($start -lt $trimmedLines.Count -and $trimmedLines[$start].Length -eq 0) {
    $start += 1
  }

  $end = $trimmedLines.Count - 1
  while ($end -ge $start -and $trimmedLines[$end].Length -eq 0) {
    $end -= 1
  }

  if ($end -lt $start) {
    return ""
  }

  return (($trimmedLines[$start..$end]) -join "`r`n")
}

function Get-Sha256Hex {
  param([string] $Text)

  $sha = [System.Security.Cryptography.SHA256]::Create()
  try {
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($Text)
    return (($sha.ComputeHash($bytes) | ForEach-Object { $_.ToString("x2") }) -join "")
  } finally {
    $sha.Dispose()
  }
}

$entries = @(
  New-NoticeEntry "TypeDuck-Windows" "MIT" @("https://github.com/TypeDuck-HK/TypeDuck-Windows") "https://raw.githubusercontent.com/TypeDuck-HK/TypeDuck-Windows/main/LICENSE"
  New-NoticeEntry "TypeDuck-Windows-backend" "MIT" @("https://github.com/TypeDuck-HK/TypeDuck-Windows-backend") "https://raw.githubusercontent.com/TypeDuck-HK/TypeDuck-Windows-backend/main/LICENSE"
  New-NoticeEntry "libIME2" "LGPL-2.1" @("https://github.com/gaboolic/libIME2") "https://raw.githubusercontent.com/gaboolic/libIME2/master/LICENSE.txt"
  New-NoticeEntry "jsoncpp" "Public Domain OR MIT" @("https://github.com/open-source-parsers/jsoncpp") "https://raw.githubusercontent.com/open-source-parsers/jsoncpp/master/LICENSE"
  New-NoticeEntry "libuv" "MIT with bundled external notices" @("https://github.com/EasyIME/libuv", "https://github.com/libuv/libuv") "https://raw.githubusercontent.com/EasyIME/libuv/pime/LICENSE"
  New-NoticeEntry "spdlog" "MIT" @("https://github.com/gabime/spdlog") "https://raw.githubusercontent.com/gabime/spdlog/v1.2.1/LICENSE"
  New-NoticeEntry "fmt bundled with spdlog" "BSD-2-Clause" @("https://github.com/gabime/spdlog/tree/v1.2.1/include/spdlog/fmt/bundled", "https://github.com/fmtlib/fmt") "https://raw.githubusercontent.com/gabime/spdlog/v1.2.1/include/spdlog/fmt/bundled/LICENSE.rst"
  New-NoticeEntry "Protocol Buffers C++ runtime" "BSD-3-Clause" @("https://github.com/protocolbuffers/protobuf") "https://raw.githubusercontent.com/protocolbuffers/protobuf/v33.5/LICENSE"
  New-NoticeEntry "utf8_range bundled with Protocol Buffers" "MIT" @("https://github.com/protocolbuffers/utf8_range", "https://github.com/protocolbuffers/protobuf/tree/v33.5/third_party/utf8_range") "https://raw.githubusercontent.com/protocolbuffers/protobuf/v33.5/third_party/utf8_range/LICENSE"
  New-NoticeEntry "Abseil C++" "Apache-2.0" @("https://github.com/abseil/abseil-cpp") "https://raw.githubusercontent.com/abseil/abseil-cpp/master/LICENSE"
  New-NoticeEntry "TypeDuck-HK librime fork" "BSD-3-Clause" @("https://github.com/TypeDuck-HK/librime") "https://raw.githubusercontent.com/TypeDuck-HK/librime/master/LICENSE"
  New-NoticeEntry "glog" "BSD-3-Clause" @("https://github.com/google/glog") "https://raw.githubusercontent.com/google/glog/master/LICENSE.md"
  New-NoticeEntry "googletest / OpenCC deps/googletest-1.15.0" "BSD-3-Clause" @("https://github.com/google/googletest", "https://github.com/BYVoid/OpenCC/tree/master/deps/googletest-1.15.0") "https://raw.githubusercontent.com/google/googletest/main/LICENSE"
  New-NoticeEntry "leveldb" "BSD-3-Clause" @("https://github.com/google/leveldb") "https://raw.githubusercontent.com/google/leveldb/main/LICENSE"
  New-NoticeEntry "marisa-trie / OpenCC deps/marisa-0.3.1" "BSD-2-Clause OR LGPL-2.1-or-later" @("https://github.com/s-yata/marisa-trie", "https://github.com/BYVoid/OpenCC/tree/master/deps/marisa-0.3.1") "https://raw.githubusercontent.com/s-yata/marisa-trie/master/COPYING.md"
  New-NoticeEntry "OpenCC" "Apache-2.0" @("https://github.com/BYVoid/OpenCC") "https://raw.githubusercontent.com/BYVoid/OpenCC/master/LICENSE"
  New-NoticeEntry "yaml-cpp" "MIT" @("https://github.com/jbeder/yaml-cpp") "https://raw.githubusercontent.com/jbeder/yaml-cpp/master/LICENSE"
  New-NoticeEntry "rime-dictionary-lookup-filter" "LGPL-2.1" @("https://github.com/TypeDuck-HK/rime-dictionary-lookup-filter") "https://raw.githubusercontent.com/TypeDuck-HK/rime-dictionary-lookup-filter/master/LICENSE"
  New-NoticeEntry "darts-clone / OpenCC deps/darts-clone-0.32" "BSD-2-Clause" @("https://github.com/s-yata/darts-clone", "https://github.com/BYVoid/OpenCC/tree/master/deps/darts-clone-0.32") "https://raw.githubusercontent.com/s-yata/darts-clone/master/COPYING.md"
  New-NoticeEntry "OpenCC deps/google-benchmark" "Apache-2.0" @("https://github.com/google/benchmark", "https://github.com/BYVoid/OpenCC/tree/master/deps/google-benchmark") "https://raw.githubusercontent.com/google/benchmark/main/LICENSE"
  New-NoticeEntry "OpenCC deps/pybind11-2.13.1" "BSD-3-Clause" @("https://github.com/pybind/pybind11", "https://github.com/BYVoid/OpenCC/tree/master/deps/pybind11-2.13.1") "https://raw.githubusercontent.com/pybind/pybind11/master/LICENSE"
  New-NoticeEntry "OpenCC deps/rapidjson-1.1.0" "MIT" @("https://github.com/Tencent/rapidjson", "https://github.com/BYVoid/OpenCC/tree/master/deps/rapidjson-1.1.0") "https://raw.githubusercontent.com/Tencent/rapidjson/v1.1.0/license.txt"
  New-NoticeEntry "OpenCC deps/rapidjson-1.1.0 msinttypes" "BSD-3-Clause" @("https://github.com/BYVoid/OpenCC/tree/master/deps/rapidjson-1.1.0/rapidjson/msinttypes") "https://raw.githubusercontent.com/Tencent/rapidjson/v1.1.0/include/rapidjson/msinttypes/stdint.h" "LeadingSlashCommentBlock"
  New-NoticeEntry "OpenCC deps/tclap-1.2.5" "MIT" @("https://github.com/BYVoid/OpenCC/tree/master/deps/tclap-1.2.5", "https://github.com/mirror/tclap") "https://raw.githubusercontent.com/mirror/tclap/master/COPYING"
  New-NoticeEntry "TypeDuck-HK schema" "CC-BY-4.0" @("https://github.com/TypeDuck-HK/schema") "https://raw.githubusercontent.com/TypeDuck-HK/schema/master/LICENSE"
  New-NoticeEntry "luna-pinyin schema data" "LGPL-3.0" @("https://github.com/rime/rime-luna-pinyin") "https://raw.githubusercontent.com/rime/rime-luna-pinyin/master/LICENSE"
  New-NoticeEntry "loengfan schema data" "CC-BY-4.0" @("https://github.com/CanCLID/rime-loengfan") "https://raw.githubusercontent.com/CanCLID/rime-loengfan/main/LICENSE"
  New-NoticeEntry "stroke schema data" "LGPL-3.0" @("https://github.com/rime/rime-stroke") "https://raw.githubusercontent.com/rime/rime-stroke/master/LICENSE"
  New-NoticeEntry "cangjie5 schema data" "MIT" @("https://github.com/Jackchows/Cangjie5") "https://raw.githubusercontent.com/Jackchows/Cangjie5/master/LICENSE"
  New-NoticeEntry "cangjie3 schema data" "MIT" @("https://github.com/Arthurmcarthur/Cangjie3-Plus") "https://raw.githubusercontent.com/Arthurmcarthur/Cangjie3-Plus/master/LICENSE"
  New-NoticeEntry "Go module github.com/golang/protobuf v1.5.0" "BSD-3-Clause" @("https://pkg.go.dev/github.com/golang/protobuf") "https://raw.githubusercontent.com/golang/protobuf/v1.5.0/LICENSE"
  New-NoticeEntry "Go module github.com/google/go-cmp v0.7.0" "BSD-3-Clause" @("https://pkg.go.dev/github.com/google/go-cmp") "https://raw.githubusercontent.com/google/go-cmp/v0.7.0/LICENSE"
  New-NoticeEntry "Go module github.com/yuin/goldmark v1.4.13" "MIT" @("https://pkg.go.dev/github.com/yuin/goldmark") "https://raw.githubusercontent.com/yuin/goldmark/v1.4.13/LICENSE"
  New-NoticeEntry "Go module golang.org/x/exp/shiny v0.0.0-20230817173708-d852ddb80c63" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/exp/shiny") "https://raw.githubusercontent.com/golang/exp/d852ddb80c63/LICENSE"
  New-NoticeEntry "Go module golang.org/x/image v0.14.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/image") "https://raw.githubusercontent.com/golang/image/v0.14.0/LICENSE"
  New-NoticeEntry "Go module golang.org/x/mobile v0.0.0-20231127183840-76ac6878050a" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/mobile") "https://raw.githubusercontent.com/golang/mobile/76ac6878050a/LICENSE"
  New-NoticeEntry "Go module golang.org/x/mod v0.14.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/mod") "https://raw.githubusercontent.com/golang/mod/v0.14.0/LICENSE"
  New-NoticeEntry "Go module golang.org/x/net v0.19.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/net") "https://raw.githubusercontent.com/golang/net/v0.19.0/LICENSE"
  New-NoticeEntry "Go module golang.org/x/sync v0.5.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/sync") "https://raw.githubusercontent.com/golang/sync/v0.5.0/LICENSE"
  New-NoticeEntry "Go module golang.org/x/sys v0.15.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/sys") "https://raw.githubusercontent.com/golang/sys/v0.15.0/LICENSE"
  New-NoticeEntry "Go module golang.org/x/tools v0.16.0" "BSD-3-Clause" @("https://pkg.go.dev/golang.org/x/tools") "https://raw.githubusercontent.com/golang/tools/v0.16.0/LICENSE"
  New-NoticeEntry "Go module google.golang.org/protobuf v1.36.11" "BSD-3-Clause" @("https://pkg.go.dev/google.golang.org/protobuf") "https://raw.githubusercontent.com/protocolbuffers/protobuf-go/v1.36.11/LICENSE"
  New-NoticeEntry "Go module gopkg.in/check.v1 v0.0.0-20161208181325-20d25e280405" "BSD-2-Clause" @("https://pkg.go.dev/gopkg.in/check.v1") "https://raw.githubusercontent.com/go-check/check/20d25e280405/LICENSE"
  New-NoticeEntry "Go module gopkg.in/yaml.v3 v3.0.1" "Apache-2.0" @("https://pkg.go.dev/gopkg.in/yaml.v3") "https://raw.githubusercontent.com/go-yaml/yaml/v3.0.1/LICENSE"
)

$groups = [ordered]@{}
foreach ($entry in $entries) {
  $rawText = Get-UrlText $entry.TextUrl
  $licenseText = Normalize-NoticeText (Select-LicenseText $rawText $entry.Extract)
  $key = Get-Sha256Hex $licenseText
  if (-not $groups.Contains($key)) {
    $groups[$key] = [PSCustomObject]@{
      License = $entry.License
      Text = $licenseText
      Components = [System.Collections.Generic.List[object]]::new()
    }
  }
  $groups[$key].Components.Add([PSCustomObject]@{
      Name = $entry.Name
      Source = $entry.Source
    }) | Out-Null
}

$separator = "=" * 80
$sections = [System.Collections.Generic.List[string]]::new()
foreach ($group in $groups.Values) {
  $lines = [System.Collections.Generic.List[string]]::new()
  $lines.Add($separator) | Out-Null
  $lines.Add($separator) | Out-Null
  $lines.Add($separator) | Out-Null
  $lines.Add("License: $($group.License)") | Out-Null
  $lines.Add("") | Out-Null

  for ($i = 0; $i -lt $group.Components.Count; $i++) {
    $component = $group.Components[$i]
    $lines.Add($component.Name) | Out-Null
    foreach ($sourceUrl in $component.Source) {
      $lines.Add("Source: $sourceUrl") | Out-Null
    }
    $lines.Add("") | Out-Null
  }

  $lines.Add($group.Text) | Out-Null
  $sections.Add(($lines -join "`r`n")) | Out-Null
}

$noticeLines = [System.Collections.Generic.List[string]]::new()
$noticeLines.Add("THIRD PARTY NOTICES") | Out-Null
$noticeLines.Add("") | Out-Null
foreach ($section in $sections) {
  $noticeLines.Add($section) | Out-Null
  $noticeLines.Add("") | Out-Null
}

$notice = ($noticeLines -join "`r`n")
[System.IO.File]::WriteAllText($OutputPath, $notice, [System.Text.UTF8Encoding]::new($false))
Write-Host "Wrote $OutputPath from $($entries.Count) components and $($groups.Count) unique notice texts."
