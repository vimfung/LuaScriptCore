//
// Created by 冯鸿杰 on 17/3/27.
//

#include "StringUtils.h"

using namespace cn::vimfung::luascriptcore;

std::string StringUtils::replace (std::string text, std::string str, std::string replacement)
{
    std::string ret;

    std::string::size_type pos_begin = 0;
    std::string::size_type pos       = text.find(str);
    while (pos != std::string::npos)
    {
        ret.append(text.data() + pos_begin, pos - pos_begin);
        ret += replacement;
        pos_begin = pos + 1;
        pos = text.find(str, pos_begin);
    }

    if (pos_begin < text.length())
    {
        ret.append(text.begin() + pos_begin, text.end());
    }

    return ret;
}