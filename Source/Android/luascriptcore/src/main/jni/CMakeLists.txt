# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.0)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             LuaScriptCore

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.对应的C文件的目录位置
             cn_vimfung_luascriptcore_LuaNativeUtil.cpp
             LuaJavaConverter.cpp
             LuaJavaEnv.cpp
             LuaJavaExceptionHandler.cpp
             LuaJavaObjectDescriptor.cpp
             LuaJavaType.cpp
             LuaJavaExportTypeDescriptor.cpp
             LuaJavaExportMethodDescriptor.cpp
             LuaJavaExportPropertyDescriptor.cpp
             ../../../../../lua-core/src/lapi.c
             ../../../../../lua-core/src/lauxlib.c
             ../../../../../lua-core/src/lbaselib.c
             ../../../../../lua-core/src/lbitlib.c
             ../../../../../lua-core/src/lcode.c
             ../../../../../lua-core/src/lcorolib.c
             ../../../../../lua-core/src/lctype.c
             ../../../../../lua-core/src/ldblib.c
             ../../../../../lua-core/src/ldebug.c
             ../../../../../lua-core/src/ldo.c
             ../../../../../lua-core/src/ldump.c
             ../../../../../lua-core/src/lfunc.c
             ../../../../../lua-core/src/lgc.c
             ../../../../../lua-core/src/linit.c
             ../../../../../lua-core/src/liolib.c
             ../../../../../lua-core/src/llex.c
             ../../../../../lua-core/src/lmathlib.c
             ../../../../../lua-core/src/lmem.c
             ../../../../../lua-core/src/loadlib.c
             ../../../../../lua-core/src/lobject.c
             ../../../../../lua-core/src/lopcodes.c
             ../../../../../lua-core/src/loslib.c
             ../../../../../lua-core/src/lparser.c
             ../../../../../lua-core/src/lstate.c
             ../../../../../lua-core/src/lstring.c
             ../../../../../lua-core/src/lstrlib.c
             ../../../../../lua-core/src/ltable.c
             ../../../../../lua-core/src/ltablib.c
             ../../../../../lua-core/src/ltm.c
             ../../../../../lua-core/src/lundump.c
             ../../../../../lua-core/src/lunity.c
             ../../../../../lua-core/src/lutf8lib.c
             ../../../../../lua-core/src/lvm.c
             ../../../../../lua-core/src/lzio.c
             ../../../../../lua-common/LuaContext.cpp
             ../../../../../lua-common/LuaFunction.cpp
             ../../../../../lua-common/LuaNativeClass.cpp
             ../../../../../lua-common/LuaNativeClassFactory.cpp
             ../../../../../lua-common/LuaObject.cpp
             ../../../../../lua-common/LuaObjectDecoder.cpp
             ../../../../../lua-common/LuaObjectDescriptor.cpp
             ../../../../../lua-common/LuaObjectEncoder.cpp
             ../../../../../lua-common/LuaObjectManager.cpp
             ../../../../../lua-common/LuaPointer.cpp
             ../../../../../lua-common/LuaValue.cpp
             ../../../../../lua-common/LuaTmpValue.cpp
             ../../../../../lua-common/LuaTuple.cpp
             ../../../../../lua-common/StringUtils.cpp
             ../../../../../lua-common/LuaDataExchanger.cpp
             ../../../../../lua-common/LuaManagedObject.cpp
             ../../../../../lua-common/LuaSession.cpp
             ../../../../../lua-common/LuaEngineAdapter.cpp
             ../../../../../lua-common/LuaExportMethodDescriptor.cpp
             ../../../../../lua-common/LuaExportsTypeManager.cpp
             ../../../../../lua-common/LuaExportTypeDescriptor.cpp
             ../../../../../lua-common/LuaExportPropertyDescriptor.cpp
             ../../../../../lua-common/LuaError.cpp
             ../../../../../lua-common/LuaScriptController.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.指定依赖库
                       LuaScriptCore

                       # Links the target library to the log library
                       # included in the NDK.关联日志记录库文件，在ndk目录中
                       ${log-lib} )

include_directories(../../../../../lua-core/src ../../../../../lua-common)