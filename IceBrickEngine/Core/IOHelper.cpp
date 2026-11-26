#include "IOHelper.hpp"

namespace IceBrick
{
    std::string ReadShaderFile(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
            
        if (!file.is_open()) {
            spdlog::error("Failed to open shader file: {}", filepath.string());
            return "";
        }

        // Grab the size, allocate the string, and rewind to the start
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::string buffer(fileSize, '\0');
        file.seekg(0);
        
        // Read the entire file in one blocking system call
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

};
