#ifndef _TYPEDUCK_PREFERENCES_H_
#define _TYPEDUCK_PREFERENCES_H_

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Moqi::TypeDuck {

struct Preferences {
  std::vector<std::string> displayLanguages;
  std::string mainLanguage;
  int pageSize = 6;
  bool isHeiTypeface = false;
  std::string showRomanization;
  bool enableCompletion = true;
  bool enableCorrection = false;
  bool enableSentence = true;
  bool enableLearning = true;
  bool showReverseCode = true;
  bool isCangjie5 = true;
};

struct PreferenceDescriptor {
  std::string id;
  std::string label;
  std::string control;
  bool affectsRime = false;
};

struct CapabilityMetadata {
  std::string id;
  bool supported = true;
  std::string message;
};

struct ValidationResult {
  bool ok = true;
  Preferences preferences;
  std::string message;
};

struct SaveResult {
  bool ok = true;
  std::string message;
};

struct RimeSideEffects {
  std::string defaultCustomFile;
  std::string defaultCustomPath;
  int pageSize = 6;
  std::string commonCustomFile;
  std::string commonPatchKey;
  std::vector<std::string> commonPatches;
};

struct ApplyResult {
  bool ok = true;
  std::string message;
};

using RimeApplier = std::function<ApplyResult(const RimeSideEffects&)>;

Preferences defaultPreferences();
std::vector<PreferenceDescriptor> preferenceDescriptors();
std::vector<CapabilityMetadata> defaultCapabilities();
std::vector<std::string> interfaceOnlyPreferenceIdsForTest();

bool preferenceAffectsRime(const std::string& id);
ValidationResult validatePreferences(const Preferences& preferences);
RimeSideEffects rimeSideEffects(const Preferences& preferences);

std::filesystem::path defaultPreferencesPath();
ValidationResult loadPreferences(const std::filesystem::path& path);
SaveResult savePreferences(const std::filesystem::path& path,
                           const Preferences& preferences);
ApplyResult applyPreferences(const std::filesystem::path& path,
                             const Preferences& preferences,
                             RimeApplier applyRime);

} // namespace Moqi::TypeDuck

#endif // _TYPEDUCK_PREFERENCES_H_
