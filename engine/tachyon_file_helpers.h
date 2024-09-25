#pragma

#include <string>

std::string Tachyon_GetFileContents(const char* path);
void Tachyon_WriteFileContents(const std::string& path, const std::string& contents);