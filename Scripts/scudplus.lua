--Some sample code follows to show how to print to console and setup options/cheats
-- Global Variables:
--  Game : string, name of the current gameNetwork.init()
--  FrameId : frame id, or # of the frame
--  Ticks : system time in ms


function Init()
	println("Init -- This is an example script that only prints driveboard and lamps registers")
    println("Loading " .. Game)
	
	local val8bits = PPC_Read8(0x5010A4)
	local val16bits = PPC_Read16(0x5010A4)
	local val32bits = PPC_Read32(0x5010A4)
end

function Reset()
	println("Reset")
end

function Frame()
	-- print("Frame=");
	-- print(FrameId);
	-- print(", ticks=");
	-- print(Ticks);
	-- print("\n");
	
	local driveboard = PPC_Read8(0xfe040010)
	local lamps = PPC_Read8(0xfe040014)
	--print("Drv=0x" .. string.format("%X", driveboard) .. " Lamps=0x")
    -- println(string.format("%X", lamps))
    
	-- Do not known yet where to find the gamemode ...
	-- local gameState = PPC_Read8(0x5010A4)
    local gameState = 0x16
    -- println(gameState)
    
	if   gameState==0x16	-- Ingame
	  or gameState==0x03	-- Attract ini
	  or gameState==0x04	-- Attract Higscore ini
	  or gameState==0x05	-- Attract Highscore
	  or gameState==0x06	-- Attract VR Ini
	  or gameState==0x07	-- Attract VR
	then
	 	-- Gfx_SetStretchBLow(1)	-- Stretch the bg tilemap (sky & clouds) when widescreen
		-- Gfx_SetWideScreen(1)
	else					-- No widescreen on the rest of the screens
	 	-- Gfx_SetStretchBLow(0)
		-- Gfx_SetWideScreen(0)
	end
end

function PostDraw()
	-- println("PostDraw")
end

function EndFrame()
	-- println("EndFrame")
end

function End()
	println("End")
end