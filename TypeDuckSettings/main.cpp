#include "TypeDuckSettingsWindow.h"

#include <windows.h>

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int showCommand) {
  return Moqi::TypeDuckSettings::RunSettingsWindow(instance, showCommand);
}
