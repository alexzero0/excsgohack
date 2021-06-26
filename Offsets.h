#pragma once

//cs go
enum Offset : const DWORD
{
	StrucrPlayer = 0x04A923A4,
	DistanceStruct = 0x10,

	PlayerTeam = 0xF0,
	Health = 0xFC,

	PlayerCoordX = 0xA0,  //0xe8 || 0xec,
	ViewMatrix = 0x29B83DD4//62DC14//6548b0
};



/* css v34
enum Offset : const DWORD
{
StrucrPlayer = 0x4035C0,
DistanceStruct = 0x10,

PlayerTeam = 0x58,
Health = 0x19c,

PlayerCoordX = 0x1a0,
PlayerCoordY = 0x1a4,
PlayerCoordZ = 0x1a8,

ViewMatrix = 0x4CF20C
};
*/