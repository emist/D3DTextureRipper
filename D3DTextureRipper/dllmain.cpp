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
#include <fstream>
#include <Shlwapi.h>

#pragma comment(lib, "Detours.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Shlwapi.lib")

using namespace std;

vector<long> texturePointers;
ofstream error_file("Error.txt");
std::stringstream sstm;

typedef struct _INIT_STRUCT 
{
	long addresses[14];
} INIT_STRUCT, *PINIT_STRUCT;

HMODULE dll;

extern "C" typedef HRESULT (WINAPI *pSetTexture)(LPDIRECT3DDEVICE9 pDevice, DWORD stage, IDirect3DBaseTexture9 * texture); 

pSetTexture SetTexture;

bool DirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   

	return false;    
}



bool WINAPI Contains(long textureP)
{
	for (vector<long>::iterator iter = texturePointers.begin(); iter != texturePointers.end(); iter++)
	{
		if(*iter == textureP)
			return true;
	}
	return false;
}

string TexturePath() 
{
	char buffer[MAX_PATH];
	ZeroMemory(buffer, MAX_PATH);
	GetModuleFileName( NULL, buffer, MAX_PATH );
	PathRemoveFileSpec(buffer);
	string filename(buffer);
	filename.append("\\Textures");
	return filename;
}

HRESULT WINAPI MySetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD stage, IDirect3DBaseTexture9 * texture)
{
	if(!Contains((long)texture))
	{
		string filename = TexturePath();
		texturePointers.push_back((long)texture);
		if (CreateDirectory(filename.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
		{
			sstm << filename << "\\" << (long)texture << ".bmp";
			error_file << "Filename is " << sstm.str() << endl;
			D3DXSaveTextureToFile(sstm.str().c_str(), D3DXIFF_BMP, texture, NULL);
			sstm.str("");
		}
		else
		{
			error_file << "Can't create " << filename << endl;
		}
		
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

