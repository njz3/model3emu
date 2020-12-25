#pragma once
#include <cstdint>
#include <string>
#include "SDL/Types.h"
#include "Model3/IEmulator.h"

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
class IScripting
{
public:
    virtual void Initialize(IEmulator* emulator) = 0;
    virtual void LoadScript(string filename) = 0;

    // Script hooks
    virtual void Init() = 0;
    virtual void Reset() = 0;
    virtual void Frame() = 0;
    virtual void PostDraw() = 0;
    virtual void EndFrame() = 0;
    virtual void End() = 0;
};

