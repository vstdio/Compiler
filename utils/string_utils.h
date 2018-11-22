#ifndef COMPILER_60MIN_STRING_UTILS_H
#define COMPILER_60MIN_STRING_UTILS_H

#include <string>

namespace string_utils
{
template <typename Collection>
std::string JoinStrings(
        const Collection& strings,
        const std::string& separator = ", ",
        const std::string& prefix = "",
        const std::string& suffix = "")
{
    std::string result(prefix);
    for (auto it = strings.cbegin(); it != strings.cend(); ++it)
    {
        result.append(*it);
        if (std::next(it) != strings.cend())
        {
            result.append(separator);
        }
    }
    return result + suffix;
}

bool TrimString(std::string& str);
}

#endif //COMPILER_60MIN_STRING_UTILS_H