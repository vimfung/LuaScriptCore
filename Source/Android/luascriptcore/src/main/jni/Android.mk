LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

APP_PLATFORM := android-14
LOCAL_MODULE := LuaScriptCore
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := \
	-llog \

LOCAL_SRC_FILES := \
	cn_vimfung_luascriptcore_LuaNativeUtil.cpp \
	LuaJavaConverter.cpp \
	LuaJavaEnv.cpp \
	LuaJavaExceptionHandler.cpp \
	LuaJavaObjectDescriptor.cpp \
	LuaJavaType.cpp \
	LuaJavaExportTypeDescriptor.cpp \
	LuaJavaExportMethodDescriptor.cpp \
	LuaJavaExportPropertyDescriptor.cpp \
	../../../../../lua-core/src/lapi.c \
	../../../../../lua-core/src/lauxlib.c \
	../../../../../lua-core/src/lbaselib.c \
	../../../../../lua-core/src/lbitlib.c \
	../../../../../lua-core/src/lcode.c \
	../../../../../lua-core/src/lcorolib.c \
	../../../../../lua-core/src/lctype.c \
	../../../../../lua-core/src/ldblib.c \
	../../../../../lua-core/src/ldebug.c \
	../../../../../lua-core/src/ldo.c \
	../../../../../lua-core/src/ldump.c \
	../../../../../lua-core/src/lfunc.c \
	../../../../../lua-core/src/lgc.c \
	../../../../../lua-core/src/linit.c \
	../../../../../lua-core/src/liolib.c \
	../../../../../lua-core/src/llex.c \
	../../../../../lua-core/src/lmathlib.c \
	../../../../../lua-core/src/lmem.c \
    ../../../../../lua-core/src/loadlib.c \
	../../../../../lua-core/src/lobject.c \
	../../../../../lua-core/src/lopcodes.c \
	../../../../../lua-core/src/loslib.c \
	../../../../../lua-core/src/lparser.c \
	../../../../../lua-core/src/lstate.c \
	../../../../../lua-core/src/lstring.c \
	../../../../../lua-core/src/lstrlib.c \
	../../../../../lua-core/src/ltable.c \
	../../../../../lua-core/src/ltablib.c \
	../../../../../lua-core/src/ltm.c \
	../../../../../lua-core/src/lundump.c \
	../../../../../lua-core/src/lunity.c \
	../../../../../lua-core/src/lutf8lib.c \
	../../../../../lua-core/src/lvm.c \
	../../../../../lua-core/src/lzio.c \
	../../../../../lua-common/LuaContext.cpp \
	../../../../../lua-common/LuaFunction.cpp \
	../../../../../lua-common/LuaNativeClass.cpp \
	../../../../../lua-common/LuaNativeClassFactory.cpp \
	../../../../../lua-common/LuaObject.cpp \
	../../../../../lua-common/LuaObjectDecoder.cpp \
	../../../../../lua-common/LuaObjectDescriptor.cpp \
	../../../../../lua-common/LuaObjectEncoder.cpp \
	../../../../../lua-common/LuaObjectManager.cpp \
	../../../../../lua-common/LuaPointer.cpp \
	../../../../../lua-common/LuaValue.cpp \
	../../../../../lua-common/LuaTmpValue.cpp \
	../../../../../lua-common/LuaTuple.cpp \
	../../../../../lua-common/StringUtils.cpp \
	../../../../../lua-common/LuaDataExchanger.cpp \
	../../../../../lua-common/LuaManagedObject.cpp \
	../../../../../lua-common/LuaSession.cpp \
	../../../../../lua-common/LuaEngineAdapter.cpp \
	../../../../../lua-common/LuaExportMethodDescriptor.cpp \
    ../../../../../lua-common/LuaExportsTypeManager.cpp \
    ../../../../../lua-common/LuaExportTypeDescriptor.cpp \
    ../../../../../lua-common/LuaExportPropertyDescriptor.cpp \
    ../../../../../lua-common/LuaOperationQueue.cpp \
    ../../../../../lua-common/LuaCoroutine.cpp \
    ../../../../../lua-common/LuaError.cpp \
    ../../../../../lua-common/LuaScriptController.cpp \

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-core/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-common

include $(BUILD_SHARED_LIBRARY)
