# TypeDuck for Windows Development

## Prerequisites

  - **Visual Studio** for *Desktop development in C++*
    with components *ATL*, *MFC* and *Windows XP support*.
  - **[cmake](http://www.cmake.org/)**
  - **[NSIS](http://nsis.sourceforge.net/Download)** for creating installer.

## Checkout source code

```batch
git clone --recursive https://github.com/TypeDuck-HK/TypeDuck-Windows.git
cd TypeDuck-Windows
```

## Install Boost

``` batch
install-boost.bat
```

### Setup build environment

Copy `env.bat.template` to `env.bat` and edit the file according to your setup.
Specifically, make sure `BOOST_ROOT` is set to the root directory of Boost
source tree; modify `BJAM_TOOLSET`, `CMAKE_GENERATOR` and `PLATFORM_TOOLSET` if
using a different version of Visual Studio; also set `DEVTOOLS_PATH` for build
tools installed to custom location.

When prepared, do the following in a *Developer Command Prompt* window.

## Build Boost

This is already handled by `install-boost.bat`.

``` batch
build.bat boost
```

## Build librime

RIME is the input method engine that powers TypeDuck.

``` batch
build.bat librime
```

See [this page](https://github.com/TypeDuck-HK/librime/blob/master/README-windows.md) for a more detailed instructions on building librime.

### Alternative: using prebuilt RIME binaries

You may get a copy of prebuilt binaries of librime from the [release page of librime](https://github.com/TypeDuck-HK/librime/releases).
Once downloaded, you can simply copy `.dll`s / `.lib`s into `weasel\output` / `weasel\lib` directories respectively.

## Build the application

```batch
build.bat weasel hant
```

Or, create a debug build:

``` batch
build.bat weasel hant debug
```

Rebuilding the application:

``` batch
build.bat weasel hant rebuild
```

Build with installer:

```batch
build.bat weasel hant installer
```

You may combine the arguments as many as you like, e.g.:

```batch
build.bat weasel hant debug rebuild installer
```

Installer will be generated in the `output\archives` directory.
