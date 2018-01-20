//
// Created by 冯鸿杰 on 17/3/27.
//

#include "StringUtils.h"
#include <stdio.h>
#include <algorithm>

#if _WINDOWS

#include <stdarg.h>

#endif


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

std::string StringUtils::format (const char *format, ...)
{
    va_list marker;
    va_start(marker, format);

    char buffer[1024] = {0};
    
#if _WINDOWS
	int size = vsnprintf_s(buffer, 1024, format, marker);
#else
    int size = vsprintf(buffer, format, marker);
#endif
    
    va_end(marker);

    //长度加1，用于放入结束符\0
    size++;
    
    char *cStr = new char[size];
    memset(cStr, 0, size);
    memcpy(cStr, buffer, size);
    
    std::string str(cStr);
    
    delete[] cStr;

    return str;
}

std::vector<std::string> StringUtils::split(std::string srcStr, std::string delimStr, bool repeatedCharIgnored)
{
    std::vector<std::string> resultStringVector;
    std::replace_if(srcStr.begin(),
                    srcStr.end(),
                    [&](const char& c)
                    {
                        if (delimStr.find(c) != std::string::npos)
                        {
                            return true;
                            
                        }
                        else
                        {
                            return false;
                            
                        }
                        
                    }/*pred*/,
                    delimStr.at(0));//将出现的所有分隔符都替换成为一个相同的字符（分隔符字符串的第一个）
    size_t pos=srcStr.find(delimStr.at(0));
    std::string addedString="";
    while (pos!=std::string::npos)
    {
        addedString=srcStr.substr(0,pos);
        if (!addedString.empty()||!repeatedCharIgnored) {
            resultStringVector.push_back(addedString);
        }
        srcStr.erase(srcStr.begin(), srcStr.begin()+pos+1);
        pos=srcStr.find(delimStr.at(0));
    }
    addedString=srcStr;
    if (!addedString.empty()||!repeatedCharIgnored) {
        resultStringVector.push_back(addedString);
    }
    return resultStringVector;
}

