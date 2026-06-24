#include "TypeDuckAboutDialog.h"

#include <windows.h>

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int) {
  Moqi::TypeDuckSettings::ShowTypeDuckAboutDialog(instance, nullptr);
  return 0;
}
