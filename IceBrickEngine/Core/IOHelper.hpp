#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace IceBrick
{
    std::string ReadShaderFile(const std::filesystem::path& filepath);
};
