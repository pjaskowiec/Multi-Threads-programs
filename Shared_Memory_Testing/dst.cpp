#include "pch.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <conio.h>
#include <tchar.h>

#pragma comment( lib, "winmm.lib")   
#pragma warning (disable:4996)
char map_name[30] = "mapping_object";

int main()
{
	LPVOID p_buffer;
	HANDLE hFileMapp= OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, map_name );               // name of mapping object

	if( !hFileMapp )
	{
		_tprintf( TEXT( "Could not open file mapping object (%d).\n" ),
			GetLastError() );
		return 1;
	}

	p_buffer = MapViewOfFile( hFileMapp, FILE_MAP_ALL_ACCESS, 0, 0, 256 );

	if( !p_buffer )
	{
		_tprintf( TEXT( "Could not map view of file (%d).\n" ),
			GetLastError() );

		CloseHandle( hFileMapp );

		return 1;
	}
	while( true )
	{
		if( (const char*)p_buffer == "cos" ) break;
		printf( "%s ", (char*)p_buffer );
		Sleep( 1000 );
	}

	UnmapViewOfFile( p_buffer );
	CloseHandle( hFileMapp );
	getch();

	   return 0;
}
