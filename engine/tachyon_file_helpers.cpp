#include <fstream>
#include <filesystem>

#include "engine/tachyon_file_helpers.h"

std::string Tachyon_GetFileContents(const char* path) {
  std::string source;
  std::ifstream file(path);

  if (file.fail()) {
    printf("\033[31m" "[Tachyon_GetFileContents] Failed to load file: %s\n" "\033[0m", path);

    return "";
  }

  std::string line;

  while (std::getline(file, line)) {
    source.append(line + "\n");
  }

  file.close();

  return source;
}

void Tachyon_WriteFileContents(const std::string& path, const std::string& contents) {
  auto last_slash_index = path.find_last_of("/");
  auto directories = path.substr(0, last_slash_index);
  std::ofstream file;

  std::filesystem::create_directories(directories);

  file.open(path, std::fstream::out);
  file << contents;
  file.close();
}