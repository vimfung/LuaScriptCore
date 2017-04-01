//
// Created by 冯鸿杰 on 17/3/27.
//

#ifndef ANDROID_STRINGUTILS_H
#define ANDROID_STRINGUTILS_H

#include <string>

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
                static std::string replace (std::string text, std::string str, std::string replacement);
            };
        }
    }
}




#endif //ANDROID_STRINGUTILS_H
