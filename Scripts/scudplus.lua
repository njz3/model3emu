--Some sample code follows to show how to draw strings and setup options/cheats
--

function Init()
    printMessage("Loading " .. Game);
	printMessage("Init");
	
	local val8bits = PPC_Read8(0x5010A4)
	local val16bits = PPC_Read16(0x5010A4)
	local val32bits = PPC_Read32(0x5010A4)
end

function Reset()
	printMessage("Reset");
end

function Frame()
	printMessage("Frame");
	local gameState = PPC_ReadByte(0x5010A4)

	if   gameState==0x16	-- Ingame
	  or gameState==0x03	-- Attract ini
	  or gameState==0x04	-- Attract Higscore ini
	  or gameState==0x05	-- Attract Highscore
	  or gameState==0x06	-- Attract VR Ini
	  or gameState==0x07	-- Attract VR
	then
	 	Gfx_SetStretchBLow(1)	-- Stretch the bg tilemap (sky & clouds) when widescreen
		Gfx_SetWideScreen(1)
	else					-- No widescreen on the rest of the screens
	 	Gfx_SetStretchBLow(0)
		Gfx_SetWideScreen(0)
	end
end

function PostDraw()
	printMessage("PostDraw");
end

function EndFrame()
	printMessage("EndFrame");
end

function End()
	printMessage("End");
end