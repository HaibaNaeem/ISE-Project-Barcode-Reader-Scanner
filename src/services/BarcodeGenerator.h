#pragma once
#include <string>
#include "CommonTypes.h"

OperationResult generateCode128(
    const std::string& code,
    const std::string& outputPngPath
);