//
//  LSCEngineAdapter.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCEngineAdapter.h"
#import "lauxlib.h"
#import "lua.h"
#import "luaconf.h"
#import "lualib.h"
#import "lapi.h"

#if LUA_VERSION_NUM == 501

#import "lext.h"

#endif

@implementation LSCEngineAdapter

+ (lua_State *)newState
{
    return luaL_newstate();
}

+ (void)close:(lua_State *)state
{
    lua_close(state);
}

+ (int)absIndex:(int)index state:(lua_State *)state
{
    return lua_absindex(state, index);
}

+ (int)gc:(lua_State *)state what:(LSCGCType)what data:(int)data
{
    return lua_gc(state, what, data);
}

+ (void)openLibs:(lua_State *)state
{
    luaL_openlibs(state);
}

+ (void)setGlobal:(lua_State *)state name:(const char *)name
{
    lua_setglobal(state, name);
}

+ (void)getGlobal:(lua_State *)state name:(const char *)name
{
    lua_getglobal(state, name);
}

+ (int)getTop:(lua_State *)state
{
    return lua_gettop(state);
}

+ (int)loadString:(lua_State *)state string:(const char *)string
{
    return luaL_loadstring(state, string);
}

+ (int)loadFile:(lua_State *)state path:(const char *)path
{
    return luaL_loadfile(state, path);
}

+ (int)pCall:(lua_State *)state nargs:(int)nargs nresults:(int)nresults errfunc:(int)errfunc
{
    return lua_pcall(state, nargs, nresults, errfunc);
}

+ (void)pop:(lua_State *)state count:(int)count
{
    lua_pop(state, count);
}

+ (BOOL)isNil:(lua_State *)state index:(int)index
{
    return lua_isnil(state, index);
}

+ (BOOL)isTable:(lua_State *)state index:(int)index
{
    return lua_istable(state, index);
}

+ (BOOL)isFunction:(lua_State *)state index:(int)index
{
    return lua_isfunction(state, index);
}

+ (BOOL)isUserdata:(lua_State *)state index:(int)index
{
    return lua_isuserdata(state, index);
}

+ (void)pushNil:(void *)state
{
    lua_pushnil(state);
}

+ (void)pushInteger:(lua_Integer)integer state:(lua_State *)state
{
    lua_pushinteger(state, integer);
}

+ (void)pushNumber:(lua_Number)number state:(lua_State *)state
{
    lua_pushnumber(state, number);
}

+ (void)pushBoolean:(int)boolean state:(lua_State *)state
{
    lua_pushboolean(state, boolean);
}

+ (void)pushLightUserdata:(void *)userdata state:(lua_State *)state
{
    lua_pushlightuserdata(state, userdata);
}

+ (void)pushString:(const char *)string state:(lua_State *)state
{
    lua_pushstring(state, string);
}

+ (void)pushString:(const char *)string len:(size_t)len state:(lua_State *)state
{
    lua_pushlstring(state, string, len);
}

+ (void)pushCFunction:(lua_CFunction)cfunction state:(lua_State *)state
{
    lua_pushcfunction(state, cfunction);
}

+ (void)pushCClosure:(lua_CFunction)cclosure n:(int)n state:(lua_State *)state
{
    lua_pushcclosure(state, cclosure, n);
}

+ (void)pushValue:(int)index state:(lua_State *)state
{
    lua_pushvalue(state, index);
}

+ (int)upvalueIndex:(int)index
{
    return lua_upvalueindex(index);
}

+ (lua_Integer)toInteger:(lua_State *)state index:(int)index
{
    return lua_tointeger(state, index);
}

+ (lua_Number)toNumber:(lua_State *)state index:(int)index
{
    return lua_tonumber(state, index);
}

+ (int)toBoolean:(lua_State *)state index:(int)index
{
    return lua_toboolean(state, index);
}

+ (const void *)toPointer:(lua_State *)state index:(int)index
{
    return lua_topointer(state, index);
}

+ (void *)toUserdata:(lua_State *)state index:(int)index
{
    return lua_touserdata(state, index);
}

+ (const char *)toString:(lua_State *)state index:(int)index
{
    return lua_tostring(state, index);
}

+ (const char *)toString:(lua_State *)state index:(int)index len:(size_t *)len
{
    return lua_tolstring(state, index, len);
}

+ (void)newTable:(lua_State *)state
{
    lua_newtable(state);
}

+ (void *)newUserdata:(lua_State *)state size:(size_t)size
{
    return lua_newuserdata(state, size);
}

+ (int)newMetatable:(lua_State *)state name:(const char *)name
{
    return luaL_newmetatable(state, name);
}

+ (void)getField:(lua_State *)state index:(int)index name:(const char *)name
{
    lua_getfield(state, index, name);
}

+ (void)setField:(lua_State *)state index:(int)index name:(const char *)name
{
    lua_setfield(state, index, name);
}

+ (void)getMetatable:(lua_State *)state name:(const char *)name
{
    luaL_getmetatable(state, name);
}

+ (int)getMetatable:(lua_State *)state index:(int)index
{
    return lua_getmetatable(state, index);
}

+ (int)setMetatable:(lua_State *)state index:(int)index
{
    return lua_setmetatable(state, index);
}

+ (void)rawSetI:(lua_State *)state index:(int)index n:(int)n
{
    lua_rawseti(state, index, n);
}

+ (void)rawSet:(lua_State *)state index:(int)index
{
    lua_rawset(state, index);
}

+ (void)rawGet:(lua_State *)state index:(int)index
{
    lua_rawget(state, index);
}

+ (int)type:(lua_State *)state index:(int)index
{
    return lua_type(state, index);
}

+ (int)next:(lua_State *)state index:(int)index
{
    return lua_next(state, index);
}

+ (void)insert:(lua_State *)state index:(int)index
{
    lua_insert(state, index);
}

+ (void)remove:(lua_State *)state index:(int)index
{
    lua_remove(state, index);
}

+ (const char *)checkString:(lua_State *)state index:(int)index
{
    return luaL_checkstring(state, index);
}

+ (int)error:(lua_State *)state message:(const char *)message
{
    return luaL_error(state, message);
}

+ (lua_State *)newThread:(lua_State *)state
{
    return lua_newthread(state);
}

+ (int)resumeThread:(lua_State *)state
    fromThreadState:(lua_State *)fromThreadState
           argCount:(int)argCount
{
    return lua_resume(state, fromThreadState, argCount);
}

+ (int)yielyThread:(lua_State *)state
       resultCount:(int)resultCount
{
    return lua_yield(state, resultCount);
}

+ (int)rawRunProtected:(lua_State *)state
                  func:(Pfunc)func
              userdata:(void *)userdata
{
    return luaD_rawrunprotected(state, func, userdata);
}

+ (void)setHook:(lua_State *)state
           hook:(lua_Hook)hook
           mask:(int)mask
          count:(int)count
{
    lua_sethook(state, hook, mask, count);
}

@end
