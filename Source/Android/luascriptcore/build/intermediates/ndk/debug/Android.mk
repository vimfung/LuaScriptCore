LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := LuaScriptCore
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := \
	-llog \

LOCAL_SRC_FILES := \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/cn_vimfung_luascriptcore_LuaNativeUtil.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaConverter.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaEnv.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaModule.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaObjectClass.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaObjectDescriptor.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni/LuaJavaType.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lapi.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lauxlib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lbaselib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lbitlib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lcode.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lcorolib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lctype.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ldblib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ldebug.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ldo.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ldump.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lfunc.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lgc.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/linit.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/liolib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/llex.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lmathlib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lmem.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/loadlib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lobject.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lopcodes.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/loslib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lparser.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lstate.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lstring.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lstrlib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ltable.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ltablib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/ltm.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lua.hpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lundump.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lunity.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lutf8lib.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lvm.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/lzio.c \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src/Makefile \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaContext.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaFunction.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaModule.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaNativeClass.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaNativeClass.hpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaNativeClassFactory.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaNativeClassFactory.hpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObject.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectClass.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectDecoder.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectDecoder.hpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectDescriptor.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectEncoder.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectEncoder.hpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaObjectManager.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaPointer.cpp \
	/Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common/LuaValue.cpp \

LOCAL_C_INCLUDES += /Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/main/jni
LOCAL_C_INCLUDES += /Users/vimfung/Documents/project/LuaScriptCore/Source/lua-core/src
LOCAL_C_INCLUDES += /Users/vimfung/Documents/project/LuaScriptCore/Source/lua-common
LOCAL_C_INCLUDES += /Users/vimfung/Documents/project/LuaScriptCore/Source/Android/luascriptcore/src/debug/jni

include $(BUILD_SHARED_LIBRARY)
