#pragma once
#include <cstdint>
#include <string>
#include "SDL/Types.h"
#include "Model3/IEmulator.h"

using namespace std;

/*
 Support for scripting engine running at particular hooks in Model3 emulator.
 - Initialize(), Load() are called when AttachScripting() is called by the 
   emulator once the game is loaded.
 - The script command Init() is called rigth after Load(), also in
   AttachScripting()
 - Reset(), Frame(), PostDraw() and EndFrame() are called in Model3 emulator
   either in RunFrame() or Reset()
 - End() is called when the Model3 emulator is terminating.
 
 The SetGlobalXX methods are not used yet, but I put them just in case the
 emulator should add or refresh global variables in the scripting engine while
 it is running.
*/
class IScripting
{
public:
    virtual void Initialize(IEmulator* emulator) = 0;
    virtual void LoadScript(string filename) = 0;

    virtual void SetGlobalString(string varname, string value) = 0;
    virtual void SetGlobalDouble(string varname, double value) = 0;
    virtual void SetGlobalInteger(string varname, long long value) = 0;

    // Script hooks
    virtual void Init() = 0;
    virtual void Reset() = 0;
    virtual void Frame() = 0;
    virtual void PostDraw() = 0;
    virtual void EndFrame() = 0;
    virtual void End() = 0;
};

