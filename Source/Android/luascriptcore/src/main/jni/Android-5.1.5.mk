LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

APP_PLATFORM := android-13

LOCAL_MODULE := LuaScriptCore
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := \
	-llog \

LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/cn_vimfung_luascriptcore_LuaNativeUtil.cpp \
	$(LOCAL_PATH)/LuaJavaConverter.cpp \
	$(LOCAL_PATH)/LuaJavaEnv.cpp \
	$(LOCAL_PATH)/LuaJavaExceptionHandler.cpp \
	$(LOCAL_PATH)/LuaJavaModule.cpp \
	$(LOCAL_PATH)/LuaJavaObjectClass.cpp \
	$(LOCAL_PATH)/LuaJavaObjectDescriptor.cpp \
	$(LOCAL_PATH)/LuaJavaObjectInstanceDescriptor.cpp \
	$(LOCAL_PATH)/LuaJavaType.cpp \
	$(LOCAL_PATH)/LuaJavaClassImport.cpp \
	$(LOCAL_PATH)/LuaJavaExportClassProxy.cpp \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lapi.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lauxlib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lbaselib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lcode.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ldblib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ldebug.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ldo.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ldump.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lext.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lfunc.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lgc.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/linit.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/liolib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/llex.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lmathlib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lmem.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/loadlib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lobject.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lopcodes.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/loslib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lparser.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lstate.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lstring.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lstrlib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ltable.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ltablib.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/ltm.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lundump.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lvm.c \
	$(LOCAL_PATH)/../../../../../lua-core-5.1.5/src/lzio.c \
	$(LOCAL_PATH)/../../../../../lua-common/LuaContext.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaFunction.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaModule.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaNativeClass.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaNativeClassFactory.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObject.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectClass.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectDecoder.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectDescriptor.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectEncoder.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectInstanceDescriptor.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaObjectManager.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaPointer.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaValue.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaTuple.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaClassImport.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaExportClassProxy.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/StringUtils.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaDataExchanger.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaManagedObject.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaSession.cpp \
	$(LOCAL_PATH)/../../../../../lua-common/LuaEngineAdapter.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-core-5.1.5/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../lua-common

include $(BUILD_SHARED_LIBRARY)
