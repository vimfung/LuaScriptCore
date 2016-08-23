//
// Created by vimfung on 16/8/23.
//

#ifndef SAMPLE_LUAOBJECT_H
#define SAMPLE_LUAOBJECT_H

namespace cn
{
    namespace vimfung
    {
        namespace luascriptcore
        {
            class LuaObject
            {
            private:
                int _retainCount;

            public:
                LuaObject ();
                virtual ~LuaObject();

            public:
                void retain ();
                void release ();
            };
        }
    }
}


#endif //SAMPLE_LUAOBJECT_H
