#include "Scripting/LuaEngine.h"
#include "OSD/Logger.h"
#include "assert.h"



#pragma region Method registered in Lua
// print(string text)
int LuaEngine::print(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (lua_isstring(lua, 1)) {
        // debug output
        auto msg =  lua_tostring(lua, 1);
        printf(msg);
    } else if (lua_isnumber(lua, 1)==0) {
        // debug output
        auto val =  lua_tonumber(lua, 1);
        printf("%f", val);
    } else if (lua_isinteger(lua, 1)) {
        // debug output
        auto val =  lua_tointeger(lua, 1);
        printf("%lld", val);
    }
    return 0;
}
// println(string text)
int LuaEngine::println(lua_State* lua)
{
    print(lua); printf("\n");
    return 0;
}


// int PPC_Read8(int addr)
int LuaEngine::PPC_Read8(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1)) {
        return luaL_error(lua, "argument to PPC_Read8() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read8(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}
// PPC_Write8(int addr, int data)
int LuaEngine::PPC_Write8(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1) || !lua_isinteger(lua, 2)) {
        return luaL_error(lua, "argument to PPC_Write8() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    UINT8 data = (UINT8)(lua_tointeger(lua, 2)&0xFF);  /* get argument */
    me->_model3->Write8(addr, data);
    return 1;  /* number of results */
}
// int PPC_Read16(int addr)
int LuaEngine::PPC_Read16(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1)) {
        return luaL_error(lua, "argument to PPC_Read16() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read16(addr);

    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}
// PPC_Write16(int addr, int data)
int LuaEngine::PPC_Write16(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1) || !lua_isinteger(lua, 2)) {
        return luaL_error(lua, "argument to PPC_Write16() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    UINT16 data = (UINT16)(lua_tointeger(lua, 2)&0xFFFF);  /* get argument */
    me->_model3->Write16(addr, data);
    return 1;  /* number of results */
}

// int PPC_Read32(int addr)
int LuaEngine::PPC_Read32(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1)) {
        return luaL_error(lua, "argument to PPC_Read32() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read32(addr);
    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}
// PPC_Write32(int addr, int data)
int LuaEngine::PPC_Write32(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1) || !lua_isinteger(lua, 2)) {
        return luaL_error(lua, "argument to PPC_Write32() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    UINT32 data = (UINT32)(lua_tointeger(lua, 2)&0xFFFFFFFF);  /* get argument */
    me->_model3->Write32(addr, data);
    return 1;  /* number of results */
}
// int PPC_Read64(int addr)
// Lua handles 64 bits integer using 0x notation
int LuaEngine::PPC_Read64(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1)) {
        return luaL_error(lua, "argument to PPC_Read64() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    auto value = me->_model3->Read64(addr);
    lua_pushinteger(lua, value);  /* push result */
    return 1;  /* number of results */
}
// PPC_Write64(int addr, int data)
int LuaEngine::PPC_Write64(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isinteger(lua, 1) || !lua_isinteger(lua, 2)) {
        return luaL_error(lua, "argument to PPC_Write64() is not an integer");
    }
    UINT32 addr = (UINT32)lua_tointeger(lua, 1);  /* get argument */
    UINT64 data = (UINT64)(lua_tointeger(lua, 2));  /* get argument */
    me->_model3->Write64(addr, data);
    return 1;  /* number of results */
}

extern SDL_Window* s_window;

// Gfx_SetWideScreen(int mode)
int LuaEngine::Gfx_SetWideScreen(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isnumber(lua, 1)) {
        return luaL_error(lua, "argument to Gfx_SetWideScreen() is not a number");
    }
    int mode = (int)lua_tonumber(lua, 1);  /* get argument */
    auto &config = me->_model3->GetConfig();
    bool oldval = config["WideScreen"].ValueAsDefault<bool>(false);
    bool newval;
    if (mode==0)
        newval = false;
    else
        newval = true;

    if (oldval!=newval)
    {
        config.Set("WideScreen", newval);

        unsigned  xOffset, yOffset;      // offset of renderer output within OpenGL viewport
        unsigned  xResi, yResi;            // renderer output resolution (can be smaller than GL viewport)
        unsigned  totalXRes, totalYRes;  // total resolution (the whole GL viewport)

        // display config
        totalXRes = xResi = config["XResolution"].ValueAs<unsigned>();
        totalYRes = yResi = config["YResolution"].ValueAs<unsigned>();
        bool stretch = config["Stretch"].ValueAs<bool>();

        // window
        int actualWidth;
        int actualHeight;
        SDL_GetWindowSize(s_window, &actualWidth, &actualHeight);
        totalXRes = actualWidth;
        totalYRes = actualHeight;

        // If required, fix the aspect ratio of the resolution that the user passed to match Model 3 ratio
        float xRes = float(xResi);
        float yRes = float(yResi);
        if (!stretch) { //if (keepAspectRatio) {
            float model3Ratio = 496.0f/384.0f;
            if (yRes < (xRes/model3Ratio))
                xRes = yRes*model3Ratio;
            if (xRes < (yRes*model3Ratio))
                yRes = xRes/model3Ratio;
        }

        // Center the visible area
        xOffset = (xResi - (unsigned)xRes)/2;
        yOffset = (yResi - (unsigned)yRes)/2;

        // If the desired resolution is smaller than what we got, re-center again
        if (xResi < actualWidth)
            xOffset += (actualWidth - xResi)/2;
        if (yResi < actualHeight)
            yOffset += (actualHeight - yResi)/2;

        UINT32 correction = (UINT32)(((yRes / 384.f) * 2) + 0.5f);

        if (newval) {
            glScissor(0, correction, totalXRes, totalYRes - (correction * 2));
        } else {
            glScissor(xOffset + correction, yOffset + correction, xRes - (correction * 2), yRes - (correction * 3));
        }
    }


    return 0;  /* number of results */
}

// Gfx_SetWideScreen(int mode)
int LuaEngine::Gfx_SetStretchBLow(lua_State* lua)
{
    LuaEngine* me = (LuaEngine*)lua_touserdata(lua, lua_upvalueindex(1));
    if (!lua_isnumber(lua, 1)) {
        return luaL_error(lua, "argument to Gfx_SetStretchBLow() is not an number");
    }
    int mode = (int)lua_tonumber(lua, 1);  /* get argument */
    auto& config = me->_model3->GetConfig();
    if (mode==0)
        config.Set("WideBackground", false);
    else
        config.Set("WideBackground", true);
    
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
    RegisterMethodToLua("print", print);
    RegisterMethodToLua("println", println);
    RegisterMethodToLua("PPC_Read8", PPC_Read8);
    RegisterMethodToLua("PPC_Read16", PPC_Read16);
    RegisterMethodToLua("PPC_Read32", PPC_Read32);
    RegisterMethodToLua("PPC_Read64", PPC_Read64);
    RegisterMethodToLua("PPC_Write8", PPC_Read8);
    RegisterMethodToLua("PPC_Write16", PPC_Read16);
    RegisterMethodToLua("PPC_Write32", PPC_Read32);
    RegisterMethodToLua("PPC_Write64", PPC_Read64);
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
        auto err = string(lua_tostring(_lua, -1));
        string msg = "Something went wrong loading the chunk in " + filename + " (missing file or syntax error?): " + err + "\n";
        DebugLog(msg.c_str());
        lua_pop(_lua, 1);
    } else {
        string msg = "Successfully loaded " + filename;
        DebugLog(msg.c_str());
        // Do a prime run to get all variables and functions declared
        if (lua_pcall(_lua, 0, 0, 0)) {
            auto err = string(lua_tostring(_lua, -1));
            string msg = "Error while priming script " + filename + ": " + err + "\n";
            DebugLog(msg.c_str());
            fprintf(stderr, msg.c_str());
        } else {
            ScriptLoaded = true;
        }
    }
}
void LuaEngine::SetGlobalString(string varname, string value)
{
    RegisterStringToLua(varname, value.c_str());
}
void LuaEngine::SetGlobalDouble(string varname, double value)
{
    RegisterDoubleToLua(varname, value);
}
void LuaEngine::SetGlobalInteger(string varname, long long value)
{
    RegisterIntegerToLua(varname, value);
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
    // Update common globals
    this->SetGlobalInteger("Ticks", CThread::GetTicks());
    this->SetGlobalInteger("FrameId", _model3->GetTimings().frameId);
    // Find method
    lua_getglobal(_lua, name.c_str());
    if (lua_isnil(_lua, -1)) {
        string msg = "Error in " + name + " (method missing?)\n";
        DebugLog(msg.c_str());
        fprintf(stderr, msg.c_str());
        return false;
    }
    // Run the function
    if (lua_pcall(_lua, 0, 0, 0)) {
        luaL_traceback(_lua, _lua, lua_tostring(_lua, -1), 1);
        auto err = string(lua_tostring(_lua, -1));
        string msg = "Lua error in " + name + "(): " + err + "\n";
        DebugLog(msg.c_str());
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
