#pragma once

#define WEASEL_CODE_NAME "TypeDuck"
#define WEASEL_REG_KEY L"Software\\Rime\\TypeDuck"
#define RIME_REG_KEY L"Software\\Rime"

#define STRINGIZE(x) #x
#define VERSION_STR(x) STRINGIZE(x)
#define WEASEL_VERSION VERSION_STR(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)
