#include "CodeValidator.h"

bool isValidSixDigitCode(const std::string& code)
{
    if (code.size() != 6)
        return false;

    for (char ch : code)
        if (ch < '0' || ch > '9')
            return false;

    return true;
}