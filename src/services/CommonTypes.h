#pragma once
#pragma once
#include <string>

struct OperationResult {
    bool success = false;
    std::string message;
    std::string value;
    std::string outputPath;
};