//
// Created by 冯鸿杰 on 17/3/27.
//

#ifndef ANDROID_STRINGUTILS_H
#define ANDROID_STRINGUTILS_H

#include <string>
#include <deque>

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            /**
             * 字符串工具类
             */
            class StringUtils
            {
            public:

                /**
                 * 替换字符串
                 *
                 * @param text 文本内容
                 * @param str 被替换的字符串
                 * @param replacement 替换字符串
                 *
                 * @return 替换后的文本内容
                 */
                static std::string replace (std::string const& text, std::string const& str, std::string const& replacement);
                
                

                /**
                 * 格式化字符串
                 *
                 * @param format 字符串格式
                 *
                 * @return 字符串
                 */
                static std::string format (const char *format, ...);
                
                
                /**
                 分割字符串

                 @param text 分割文本字符串
                 @param delimStr 分隔符
                 @param repeatedCharIgnored 是否忽视待分割字符串中的重复分隔符，也就是说如果忽视，那么多个连续分隔符将被视为一个分隔符
                 @return 分割后字符串数组
                 */
                static std::deque<std::string> split(std::string text, std::string const& delimStr, bool repeatedCharIgnored);

                /**
                 * 判断是否为UTF8字符串
                 * @param text 待检测字符串
                 * @return true 为UTF8字符串，否则不是
                 */
                static bool isUTF8String(std::string const& text);
            };
        }
    }
}




#endif //ANDROID_STRINGUTILS_H
