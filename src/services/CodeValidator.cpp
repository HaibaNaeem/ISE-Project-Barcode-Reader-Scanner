#include "CodeValidator.h"

bool isValidSixDigitCode(const std::string& code)
{
    // exactly 6 characters
    if (code.size() != 6)
        return false;

    // pehla digit non-zero (1-9)
    if (code[0] < '1' || code[0] > '9')
        return false;

    // sab characters digits hon
    for (char ch : code)
        if (ch < '0' || ch > '9')
            return false;

    return true;
}