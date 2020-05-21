#include "pch.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <conio.h>
#include <tchar.h>


#pragma comment( lib, "winmm.lib")   
#pragma warning (disable:4996)
char map_name[ 30 ] = "mapping_object";

int main()
{
	LPVOID p_buffer;
	HANDLE hFileMapp = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, 256, map_name );
	if( !hFileMapp )
	{
		perror( "Can't create map file!" );
		return -1;
	}
	p_buffer = MapViewOfFile( hFileMapp, FILE_MAP_ALL_ACCESS, 0, 0, 256 );

	if( !p_buffer )
	{
		perror( "Could not map view of file.\n" );
		CloseHandle( hFileMapp );
		return -1;
	}

	PROCESS_INFORMATION ProcInfo;
	STARTUPINFO StartInfo;

	ZeroMemory( &ProcInfo, sizeof( ProcInfo ) );
	ZeroMemory( &StartInfo, sizeof( StartInfo ) );
	const char* tmp = "/c start C:/Users/jasko/Desktop/sys_cw7/dst/Debug/dst.exe";
	char* exec = new char[ 256 ];
	strcpy( exec, tmp );


	if( !CreateProcess( "C:\\Windows\\system32\\cmd.exe", exec, NULL, NULL, FALSE, 0, NULL, NULL, &StartInfo, &ProcInfo ) )
	{
		perror( "Can't open shared process!" );
		return 1;
	}

	char var[ MAX_PATH ];


	while( true )
	{
		printf( "Podaj tekst:\n" );
		scanf( "%s", var );
		printf( "Zmienna to:%s\n", var );
		CopyMemory( p_buffer, var, 256 );
	}
	UnmapViewOfFile( p_buffer );

	CloseHandle( hFileMapp );


	return 0;
}
