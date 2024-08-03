#include <fstream>

#include "engine/tachyon_file_helpers.h"

std::string Tachyon_GetFileContents(const char* path) {
  std::string source;
  std::ifstream file(path);

  if (file.fail()) {
    // @todo print warning/show error dialog
    return "";
  }

  std::string line;

  while (std::getline(file, line)) {
    source.append(line + "\n");
  }

  file.close();

  return source;
}