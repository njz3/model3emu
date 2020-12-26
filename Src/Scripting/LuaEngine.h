#pragma once
#include "IScripting.h"


#include "BlockFile.h"
#include "Graphics/New3D/New3D.h"
#include "Graphics/Render2D.h"
#include "Graphics/Legacy3D/TextureRefs.h"
#include "Graphics/Legacy3D/Legacy3D.h"
#include "Graphics/Shader.h"
#ifdef SUPERMODEL_DEBUGGER
#include "Debugger/SupermodelDebugger.h"
#include "Debugger/CPU/PPCDebug.h"
#include "Debugger/CPU/Musashi68KDebug.h"
#include "Debugger/CPU/Z80Debug.h"
#endif // SUPERMODEL_DEBUGGER
#include "CPU/Bus.h"
#include "CPU/PowerPC/PPCDisasm.h"
#include "CPU/PowerPC/ppc.h"
#include "CPU/68K/68K.h"
#include "CPU/Z80/Z80.h"
#include "Inputs/Input.h"
#include "Inputs/Inputs.h"
#include "Inputs/InputSource.h"
#include "Inputs/InputSystem.h"
#include "Inputs/InputTypes.h"
#include "Inputs/MultiInputSource.h"
#include "Model3/IRQ.h"
#include "Model3/PCI.h"
#include "Model3/53C810.h"
#include "Model3/MPC10x.h"
#include "Model3/RTC72421.h"
#include "Model3/93C46.h"
#include "Model3/TileGen.h"
#include "Model3/Real3D.h"
#include "Sound/SCSP.h"
#include "Model3/SoundBoard.h"
#include "Model3/DSB.h"
#include "Model3/DriveBoard.h"
#ifdef NET_BOARD
#include "Network/NetBoard.h"
#endif
#include "Model3/Model3.h"


#ifndef __LUA_INC_H__
#define __LUA_INC_H__
extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#endif // __LUA_INC_H__

using namespace std;

/*
From Nebula Model2 Emulator:

LUA Scripts

The emulator will load a script named the same than the romset, ended in .lua in the SCRIPTS folder (ex: daytona.lua)

Each script can add some functions that will be called by the emulator:

Init()
    Called when the game has been loaded and is about to start

Reset()
    Called when the game has been reset (not on first reset, use Init for the first reset)

Frame()
    Called right after emulating a frame, and before starting video rendering

PostDraw()
    Called right after rendering the frame, and before blitting the frame to the screen (so you can add text
    and other images to the rendered image

End()
    Called when the emulator is about to terminate the emulation (use for cleanup or data persistence)

Inside the LUA scripts you can access the emulator by using some helper functions provided by the emulator:


ROM Access functions

int Romset_ReadByte(int area,int offset)
int Romset_ReadWord(int area,int offset)
int Romset_ReadDWord(int area,int offset)
    Read byte/word/dword from rom files at specified area and offset

Romset_PatchByte(int area,int offset,int data)
Romset_PatchWord(int area,int offset,int data)
Romset_PatchDWord(int area,int offset,int data)
    Write byte/word/dword to rom files at specified area and offset

*/

class LuaEngine : IScripting
{

public:
    // IScripting API
    void Initialize(IEmulator* emulator) override;
    void LoadScript(string filename) override;
    
    // Script hooks
    void Init() override;
    void Reset() override;
    void Frame() override;
    void PostDraw() override;
    void EndFrame() override;
    void End() override;


    // Special methods exported into Lua
    static int printMessage(lua_State* lua);

    static int PPC_Read8(lua_State* lua);
    static int PPC_Read16(lua_State* lua);
    static int PPC_Read32(lua_State* lua);
    static int PPC_Read64(lua_State* lua);

    static int PPC_Write8(lua_State* lua);
    static int PPC_Write16(lua_State* lua);
    static int PPC_Write32(lua_State* lua);
    static int PPC_Write64(lua_State* lua);

    static int Gfx_SetWideScreen(lua_State* lua);
    static int Gfx_SetStretchBLow(lua_State* lua);

    // Lua helpers
    bool CallLuaMethod(string name);
    void RegisterMethodToLua(string globalname, lua_CFunction fct);
    void RegisterStringToLua(string globalname, string value);
    void RegisterIntegerToLua(string globalname, long long value);
    void RegisterDoubleToLua(string globalname, double value);

protected:
    CModel3* _model3;
    lua_State* _lua;
    bool ScriptLoaded;
    bool HasHook[6];
};

