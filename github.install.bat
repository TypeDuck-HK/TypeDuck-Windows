setlocal

git submodule init

set download_archive=rime-TypeDuck-%rime_version%-Windows.7z
set download_archive_deps=rime-deps-TypeDuck-%rime_version%-Windows.7z

curl -LO https://github.com/TypeDuck-HK/librime/releases/download/%rime_version%/%download_archive%
curl -LO https://github.com/TypeDuck-HK/librime/releases/download/%rime_version%/%download_archive_deps%

7z x %download_archive% * -olibrime\ | find "ing archive"
7z x %download_archive_deps% * -olibrime\ | find "ing archive"

copy /Y librime\dist\include\rime_*.h include\
copy /Y librime\dist\lib\rime.lib lib\
copy /Y librime\dist\lib\rime.dll output\
