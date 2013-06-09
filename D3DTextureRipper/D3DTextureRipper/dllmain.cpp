// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <Windows.h>
#include <direct.h>
#include <d3d9.h>
#include <vector>
#include <d3dx9.h>
#include <Detours.h>
#include <thread>

#pragma comment(lib, "Detours.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

using namespace std;

vector<long> texturePointers;

typedef struct _INIT_STRUCT 
{
	long addresses[14];
} INIT_STRUCT, *PINIT_STRUCT;

HMODULE dll;

extern "C" typedef HRESULT (WINAPI *pSetTexture)(LPDIRECT3DDEVICE9 pDevice, DWORD stage, IDirect3DBaseTexture9 * texture); 

pSetTexture SetTexture;

bool WINAPI Contains(long textureP)
{
	for (vector<long>::iterator iter = texturePointers.begin(); iter != texturePointers.end(); iter++)
	{
		if(*iter == textureP)
			return true;
	}
	return false;
}

HRESULT WINAPI MySetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD stage, IDirect3DBaseTexture9 * texture)
{
	if(!Contains((long)texture))
	{
		std::stringstream sstm;
		string filename = "C:\\Users\\emist\\Desktop\\Textures\\";
		sstm << filename << (long)texture << ".bmp";
		texturePointers.push_back((long)texture);
		D3DXSaveTextureToFile(sstm.str().c_str(), D3DXIFF_BMP, texture, NULL);
	}
	return SetTexture(pDevice, stage, texture);
}

extern "C" __declspec(dllexport) void InstallHook(LPVOID message)
{
	PINIT_STRUCT messageStruct = reinterpret_cast<PINIT_STRUCT>(message);	  
	SetTexture = (pSetTexture)messageStruct->addresses[0];
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)SetTexture, MySetTexture);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		dll = hModule;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

