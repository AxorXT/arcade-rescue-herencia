#include "arcade/assets.hpp"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace arcade {
namespace {

namespace fs = std::filesystem;

bool isRegularFile(const fs::path& path) {
  std::error_code error;
  return fs::is_regular_file(path, error);
}

// Lee una variable de entorno. MSVC marca std::getenv como inseguro (C4996),
// por eso en Windows se usa _dupenv_s.
std::string environmentVariable(const char* name) {
#if defined(_MSC_VER)
  char* value = nullptr;
  std::size_t length = 0;
  if (_dupenv_s(&value, &length, name) != 0 || value == nullptr) {
    return {};
  }
  std::string result(value);
  std::free(value);
  return result;
#else
  const char* value = std::getenv(name);
  return value != nullptr ? std::string(value) : std::string{};
#endif
}

}  // namespace

std::string executableDirectory() {
#if defined(_WIN32)
  std::vector<char> buffer(MAX_PATH);
  DWORD length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
  if (length == 0 || length >= buffer.size()) {
    return {};
  }
  return fs::path(std::string(buffer.data(), length)).parent_path().string();
#elif defined(__APPLE__)
  std::uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::vector<char> buffer(size + 1, '\0');
  if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
    return {};
  }
  std::error_code error;
  const fs::path canonical = fs::weakly_canonical(fs::path(buffer.data()), error);
  return error ? std::string{} : canonical.parent_path().string();
#else
  std::vector<char> buffer(1024);
  const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
  if (length <= 0) {
    return {};
  }
  return fs::path(std::string(buffer.data(), static_cast<std::size_t>(length))).parent_path().string();
#endif
}

std::string findAsset(const std::string& relativePath, const std::string& explicitDirectory) {
  if (relativePath.empty()) {
    return {};
  }

  // Orden de busqueda documentado en assets.hpp.
  std::vector<fs::path> directories;
  if (!explicitDirectory.empty()) {
    directories.emplace_back(explicitDirectory);
  }
  const std::string fromEnvironment = environmentVariable("ARCADE_RESCUE_ASSETS");
  if (!fromEnvironment.empty()) {
    directories.emplace_back(fromEnvironment);
  }
  directories.emplace_back("assets");
  directories.emplace_back(fs::path("..") / "assets");
  directories.emplace_back(fs::path("..") / ".." / "assets");
  const std::string exeDirectory = executableDirectory();
  if (!exeDirectory.empty()) {
    directories.push_back(fs::path(exeDirectory) / "assets");
    directories.push_back(fs::path(exeDirectory) / ".." / "share" / "arcade-rescue" / "assets");
  }

  for (const fs::path& directory : directories) {
    const fs::path candidate = directory / relativePath;
    if (isRegularFile(candidate)) {
      return candidate.string();
    }
  }
  return {};
}

}  // namespace arcade
