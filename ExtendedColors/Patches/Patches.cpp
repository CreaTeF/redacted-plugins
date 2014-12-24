#include "..\STDInclude.h"

#define MIN_NAME_LENGTH 3

void Patches::Apply()
{
	// Patch RB_LookupColor()
	DWORD* color_table = (DWORD*)Addresses::ColorTableByte;
	color_table[1] = RGB(255, 49, 49);
	color_table[2] = RGB(134, 192, 0);
	color_table[3] = RGB(255, 173, 34);
	color_table[4] = RGB(0, 135, 193);
	color_table[5] = RGB(32, 197, 255);
	color_table[6] = RGB(151, 80, 221);

	// Patch CL_LookupColor()
	for (int i = 0; i < 7 * 4; i++) ((float*)Addresses::ColorTableFloat)[i] = ((BYTE*)color_table)[i] / 255.0f;

	// Reduce min name length
	*(BYTE*)Addresses::NameLength1 = MIN_NAME_LENGTH;
	*(BYTE*)Addresses::NameLength2 = MIN_NAME_LENGTH;

	// Allow colored names
	QNOP(Addresses::ColoredName, 5);

	// Allow colored names ingame
	QCALL(Addresses::ClientUserinfoChanged, Patches::ClientUserinfoChanged_Hook, QPATCH_JUMP);

	// Below is multiplayer only
	if (Global::Game::Type == GAME_TYPE_ZM) return;

	// Apply colors to killfeed
	QCALL(Addresses::GetClientName1, Patches::CL_GetClientName_Hook, QPATCH_CALL);
	QCALL(Addresses::GetClientName2, Patches::CL_GetClientName_Hook, QPATCH_CALL);

	// Colors in spectator
	QCALL(Addresses::GetClientName3, Patches::CL_GetClientName_Hook, QPATCH_CALL);
}

void __declspec(naked) Patches::ClientUserinfoChanged_Hook()
{
	__asm
	{
		mov eax, [esp + 4h]
		dec eax

		push edi
		push esi
		push eax // length
		push ecx // name
		push edx // buffer

		call strncpy

		pop edx
		pop ecx
		pop eax
		pop esi
		pop edi

		retn
	}
}
		
char* Patches::CL_GetClientName_Hook(int a1, int a2, char* buffer, int a4, int a5)
{
	QNOP(Addresses::ICleanStrHook, 5);

	__asm
	{
		push a5
		push a4
		push buffer
		push a2
		push a1
		call Addresses::GetClientName
		add esp, 14h
	}

	QCALL(Addresses::ICleanStrHook, Addresses::ICleanStr, QPATCH_CALL);
	strcat(buffer, "^7");
}