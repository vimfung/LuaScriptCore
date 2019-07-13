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
	../../../../../lua-core-5.1.5/src/lapi.c \
	../../../../../lua-core-5.1.5/src/lauxlib.c \
	../../../../../lua-core-5.1.5/src/lbaselib.c \
	../../../../../lua-core-5.1.5/src/lcode.c \
	../../../../../lua-core-5.1.5/src/ldblib.c \
	../../../../../lua-core-5.1.5/src/ldebug.c \
	../../../../../lua-core-5.1.5/src/ldo.c \
	../../../../../lua-core-5.1.5/src/ldump.c \
	../../../../../lua-core-5.1.5/src/lext.c \
	../../../../../lua-core-5.1.5/src/lfunc.c \
	../../../../../lua-core-5.1.5/src/lgc.c \
	../../../../../lua-core-5.1.5/src/linit.c \
	../../../../../lua-core-5.1.5/src/liolib.c \
	../../../../../lua-core-5.1.5/src/llex.c \
	../../../../../lua-core-5.1.5/src/lmathlib.c \
	../../../../../lua-core-5.1.5/src/lmem.c \
	../../../../../lua-core-5.1.5/src/loadlib.c \
	../../../../../lua-core-5.1.5/src/lobject.c \
	../../../../../lua-core-5.1.5/src/lopcodes.c \
	../../../../../lua-core-5.1.5/src/loslib.c \
	../../../../../lua-core-5.1.5/src/lparser.c \
	../../../../../lua-core-5.1.5/src/lstate.c \
	../../../../../lua-core-5.1.5/src/lstring.c \
	../../../../../lua-core-5.1.5/src/lstrlib.c \
	../../../../../lua-core-5.1.5/src/ltable.c \
	../../../../../lua-core-5.1.5/src/ltablib.c \
	../../../../../lua-core-5.1.5/src/ltm.c \
	../../../../../lua-core-5.1.5/src/lundump.c \
	../../../../../lua-core-5.1.5/src/lvm.c \
	../../../../../lua-core-5.1.5/src/lzio.c \
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
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-core-5.1.5/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-common

include $(BUILD_SHARED_LIBRARY)
