#include "Scripting/LuaEngine.h"
#include "OSD/Logger.h"
#include "assert.h"




#pragma region Method registered in Lua 
int LuaEngine::printMessage(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isstring(lua, 1));
    // debug output
    auto msg =  "Script: " + string(lua_tostring(lua, 1));
    printf(msg.c_str());
    DebugLog(msg.c_str());
    return 0;
}


// int PPC_Read8(int addr)
int LuaEngine::PPC_Read8(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int addr = lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read8(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}

// int PPC_Read16(int addr)
int LuaEngine::PPC_Read16(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int addr = lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read16(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}

// int PPC_Read32(int addr)
int LuaEngine::PPC_Read32(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int addr = lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read32(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}
// int PPC_Read64(int area, int offset)
// Lua does not handle 64 bits integer for now
int LuaEngine::PPC_Read64(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int addr = lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read64(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}

// int Gfx_SetWideScreen(int mode)
int LuaEngine::Gfx_SetWideScreen(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int mode = lua_tointeger(lua, 1);  /* get argument */

    return 0;  /* number of results */
}

// int Gfx_SetWideScreen(int mode)
int LuaEngine::Gfx_SetStretchBLow(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    assert(lua_isnumber(lua, 1));
    int mode = lua_tointeger(lua, 1);  /* get argument */

    return 0;  /* number of results */
}
#pragma endregion

#pragma region LuaEngine

void LuaEngine::Initialize(IEmulator* emulator)
{
    // Save back ref pointer to access emulator's memory
    _model3 = dynamic_cast<CModel3*>(emulator);
    // Create Lua
    _lua = luaL_newstate();
    // Load basic lua libs?
    luaL_openlibs(_lua);
    // Register methods
    RegisterMethodToLua("printMessage", printMessage);
    RegisterMethodToLua("PPC_Read8", PPC_Read8);
    RegisterMethodToLua("PPC_Read16", PPC_Read16);
    RegisterMethodToLua("PPC_Read32", PPC_Read32);
    RegisterMethodToLua("PPC_Read64", PPC_Read64);
    RegisterMethodToLua("Gfx_SetWideScreen", Gfx_SetWideScreen);
    RegisterMethodToLua("Gfx_SetStretchBLow", Gfx_SetStretchBLow);
    // Register globals
    const auto game = _model3->GetGame();
    RegisterStringToLua("Game", game.name.c_str());
    ScriptLoaded = false;
    for (int i=0; i<sizeof(HasHook)/sizeof(HasHook[0]); i++) {
        HasHook[i] = true;
    }
}

void LuaEngine::LoadScript(string filename)
{
    ScriptLoaded = false;
    for (int i=0; i<sizeof(HasHook)/sizeof(HasHook[0]); i++) {
        HasHook[i] = true;
    }
    // Load from "Scripts/" subdirectory
    auto completefilemane = "Scripts/" + filename;
    if (luaL_loadfile(_lua, completefilemane.c_str())) {
        string msg = "Something went wrong loading the chunk in " + filename + " (missing file or syntax error?)";
        DebugLog(msg.c_str());
        DebugLog(lua_tostring(_lua, -1));
        fprintf(stderr, msg.c_str());
        lua_pop(_lua, 1);
    } else {
        string msg = "Successfully loaded " + filename;
        DebugLog(msg.c_str());
        // Do a prime run to get all variables and functions declared
        if (lua_pcall(_lua, 0, 0, 0)) {
            string msg = "Error while priming script";
            DebugLog(msg.c_str());
            DebugLog(lua_tostring(_lua, -1));
            fprintf(stderr, msg.c_str());
        } else {
            ScriptLoaded = true;
        }
    }
}


void LuaEngine::Init()
{
    if (ScriptLoaded && HasHook[0]) {
        HasHook[0] = CallLuaMethod("Init");
    }
}
void LuaEngine::Reset()
{
    if (ScriptLoaded && HasHook[1]) {
        HasHook[1] = CallLuaMethod("Reset");
    }
}
void LuaEngine::Frame()
{
    if (ScriptLoaded && HasHook[2]) {
        HasHook[2] = CallLuaMethod("Frame");
    }
}
void LuaEngine::PostDraw()
{
    if (ScriptLoaded && HasHook[3]) {
        HasHook[3] = CallLuaMethod("PostDraw");
    }
}
void LuaEngine::EndFrame()
{
    if (ScriptLoaded && HasHook[4]) {
        HasHook[4] = CallLuaMethod("EndFrame");
    }
}
void LuaEngine::End()
{
    if (ScriptLoaded && HasHook[5]) {
        HasHook[5] = CallLuaMethod("End");
    }
    lua_close(_lua);
    _lua = nullptr;
}

bool LuaEngine::CallLuaMethod(string name)
{
    // Find method
    lua_getglobal(_lua, name.c_str());
    if (lua_isnil(_lua, -1)) {
        string msg = "Error in " + name + " (method missing?)";
        DebugLog(msg.c_str());
        fprintf(stderr, msg.c_str());
        return false;
    }
    // Run the function
    if (lua_pcall(_lua, 0, 0, 0)) {
        string msg = "Error in " + name;
        DebugLog(msg.c_str());
        DebugLog(lua_tostring(_lua, -1));
        fprintf(stderr, msg.c_str());
        return false;
    }
    return true;
}

void LuaEngine::RegisterMethodToLua(string globalname, lua_CFunction fct)
{
    // https://stackoverflow.com/questions/8094992/register-c-function-in-lua
    /* register function as closure */
    lua_pushlightuserdata(_lua, this);
    lua_pushcclosure(_lua, fct, 1);
    lua_setglobal(_lua, globalname.c_str());
}
void LuaEngine::RegisterStringToLua(string globalname, string value)
{
    lua_pushstring(_lua, value.c_str());
    lua_setglobal(_lua, globalname.c_str());
}
void LuaEngine::RegisterIntegerToLua(string globalname, long long value)
{
    lua_pushinteger(_lua, value);
    lua_setglobal(_lua, globalname.c_str());
}
void LuaEngine::RegisterDoubleToLua(string globalname, double value)
{
    lua_pushnumber(_lua, value);
    lua_setglobal(_lua, globalname.c_str());
}

#pragma endregion
