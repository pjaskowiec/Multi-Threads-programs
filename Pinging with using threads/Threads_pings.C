#include "pch.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <string>


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning (disable:4996)


#define MAX_PATH 512
#define MAX_COMMAND 128

using namespace std;

void ProcessPing( char* IPV4 ); //zad1 
void AllProcessPing( char* IPV4, char* Mask ); //zad2
void ThreadICMP( char* IPV4, char* Mask ); //zad3

DWORD WINAPI ThreadFunction( LPVOID lpArg );


int main( int argc, char* argv[] )
{
	//ProcessPing( argv[ 1 ] );
	//AllProcessPing( argv[ 1 ], argv[ 2 ] );
	//ThreadICMP( argv[ 1 ], argv[ 2 ] );
	return 0;
}




void ProcessPing( char* IPV4 ) //kluczowe jest wlaczenie wielobajtowego zestawu znakow!!!!!
{
	PROCESS_INFORMATION ProcInfo;
	STARTUPINFO StartInfo;

	ZeroMemory( &ProcInfo, sizeof( ProcInfo ) );
	ZeroMemory( &StartInfo, sizeof( StartInfo ) );
	TCHAR NAME[ MAX_PATH ] = TEXT( "C:\\Windows\\system32\\cmd.exe" );
	TCHAR to_do[ MAX_COMMAND ] = TEXT( "/c ping -n 1 " ); //nie mozna dać start!!!! sprawdzi poprawnosc operacji dla commandera,nie dla ping
	strcat( to_do, IPV4 );
	if( !( CreateProcess( NAME, to_do, NULL, NULL, TRUE, 0, NULL, NULL, &StartInfo, &ProcInfo ) ) )
	{
		perror( "Can't create the process!" );
		return;
	}
	WaitForSingleObject( ProcInfo.hProcess, INFINITE );
	DWORD exit_code;
	if( !( GetExitCodeProcess( ProcInfo.hProcess, &exit_code ) ) )
	{
		perror( "Couldn't get information about exit code!" );
		return;
	}
	if( !exit_code ) printf( "Adres %s jest aktywny\n", IPV4 );
	else  printf( "Adres %s jest nieaktywny\n", IPV4 );
	CloseHandle( ProcInfo.hProcess );
	CloseHandle( ProcInfo.hThread );
}

void AllProcessPing( char* IPV4, char* Mask )
{
	string* correct_pings = new string[ 10000 ];

	int iterator = 0;
	//1.konwertujemy do intów
	int first_octal, sec_octal, th_octal, fo_octal;
	int first_mask, sec_mask, th_mask, fo_mask;
	sscanf( IPV4, "%d.%d.%d.%d", &first_octal, &sec_octal, &th_octal, &fo_octal ); //cztery oktety 
	sscanf( Mask, "%d.%d.%d.%d", &first_mask, &sec_mask, &th_mask, &fo_mask ); //cztery oktety maski
	//2.koniunkcje bitowe
	int intersec1 = first_octal & first_mask; //255
	int intersec2 = sec_octal & sec_mask; //255
	int	intersec3 = th_octal & th_mask; //jesli podamy klasyczna maske,to dopiero tutaj nastapi zmiana(2^8-1=255-->samej jedynki) 255.255.0.0
	int intersec4 = fo_octal & fo_mask;
	//3.szukamy host_range(dodajemy negacje maski do koniunkcji)
	int RANGE1 = (int)(unsigned char)~first_mask + intersec1;
	int RANGE2 = (int)(unsigned char)~sec_mask + intersec2;
	int RANGE3 = (int)(unsigned char)~th_mask + intersec3;
	int RANGE4 = (int)(unsigned char)~fo_mask + intersec4;
	for( int tmp1 = intersec1; tmp1 <= RANGE1; tmp1++ )
		for( int tmp2 = intersec2; tmp2 <= RANGE2; tmp2++ )
			for( int tmp3 = intersec3; tmp3 <= RANGE3; tmp3++ )
				for( int tmp4 = intersec4; tmp4 <= RANGE4; tmp4++ )
				{

					TCHAR to_do[ MAX_COMMAND ] = TEXT( "" );
					sprintf( to_do, "/c ping.exe -n 1 %d.%d.%d.%d\n", tmp1, tmp2, tmp3, tmp4 + 1 ); //ip adress
					PROCESS_INFORMATION ProcInfo;
					STARTUPINFO StartInfo;

					ZeroMemory( &ProcInfo, sizeof( ProcInfo ) );
					ZeroMemory( &StartInfo, sizeof( StartInfo ) );
					TCHAR NAME[ MAX_PATH ] = TEXT( "C:\\Windows\\system32\\cmd.exe" );
					if( !( CreateProcess( NAME, to_do, NULL, NULL, TRUE, 0, NULL, NULL, &StartInfo, &ProcInfo ) ) )
					{
						perror( "Can't create the process!" );
						continue;
					}
					WaitForSingleObject( ProcInfo.hProcess, INFINITE );
					DWORD exit_code;
					if( !( GetExitCodeProcess( ProcInfo.hProcess, &exit_code ) ) )
					{
						perror( "Couldn't get information about exit code!" );
						continue;
					}
					if( !exit_code )
					{
						char* string_holder = new char[ 50 ];
						sprintf( string_holder, "Adres %d.%d.%d.%d jest aktywny\n", tmp1, tmp2, tmp3, tmp4 ); //ip adress
						correct_pings[ iterator ] = string_holder;
						delete[] string_holder;
						++iterator;
					}
					CloseHandle( ProcInfo.hProcess );
					CloseHandle( ProcInfo.hThread );
				}
	for( int i = 0; i <= iterator; i++ )  cout << correct_pings[ i ];
	delete[] correct_pings;
}

void ThreadICMP( char* IPV4, char* Mask )
{
	int first_octal, sec_octal, th_octal, fo_octal;
	int first_mask, sec_mask, th_mask, fo_mask;
	sscanf( IPV4, "%d.%d.%d.%d", &first_octal, &sec_octal, &th_octal, &fo_octal );
	sscanf( Mask, "%d.%d.%d.%d", &first_mask, &sec_mask, &th_mask, &fo_mask );

	int intersec1 = first_octal & first_mask;
	int intersec2 = sec_octal & sec_mask;
	int	intersec3 = th_octal & th_mask;
	int intersec4 = fo_octal & fo_mask;
	int RANGE1 = (int)(unsigned char)~first_mask + intersec1;
	int RANGE2 = (int)(unsigned char)~sec_mask + intersec2;
	int RANGE3 = (int)(unsigned char)~th_mask + intersec3;
	int RANGE4 = (int)(unsigned char)~fo_mask + intersec4;
	for( int tmp1 = intersec1; tmp1 <= RANGE1; tmp1++ )
		for( int tmp2 = intersec2; tmp2 <= RANGE2; tmp2++ )
			for( int tmp3 = intersec3; tmp3 <= RANGE3; tmp3++ )
				for( int tmp4 = intersec4; tmp4 <= RANGE4; tmp4++ )
				{
					TCHAR to_do[ MAX_COMMAND ] = TEXT( "" );
					sprintf( to_do, "%d.%d.%d.%d\n", tmp1, tmp2, tmp3, tmp4 + 1 ); //ip adress
					DWORD ThreadID;
					HANDLE my_thread = CreateThread( NULL, 0, &ThreadFunction, to_do, 0, &ThreadID );
					Sleep( 1 );
					if( !my_thread ) continue;
					CloseHandle( my_thread );
				}
}

DWORD WINAPI ThreadFunction( LPVOID lpArg ) //LPVOID=VOID*,wskaznik dowolnego typu
{
	//z msdn
	char adress_ip[ 40 ];
	strcpy( adress_ip, (char*)lpArg );
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	char SendData[ 32 ] = "Data Buffer";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;
	ipaddr = inet_addr( adress_ip );


	hIcmpFile = IcmpCreateFile();
	if( hIcmpFile == INVALID_HANDLE_VALUE )
	{
		printf( "\tUnable to open handle.\n" );
		printf( "IcmpCreatefile returned error: %ld\n", GetLastError() );
		return 1;
	}

	ReplySize = sizeof( ICMP_ECHO_REPLY ) + sizeof( SendData );
	ReplyBuffer = (VOID*)malloc( ReplySize );
	if( ReplyBuffer == NULL ) {
		printf( "\tUnable to allocate memory\n" );
		return 1;
	}


	dwRetVal = IcmpSendEcho( hIcmpFile, ipaddr, SendData, sizeof( SendData ),
		NULL, ReplyBuffer, ReplySize, 1000 );
	if( dwRetVal != 0 ) {
		PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
		struct in_addr ReplyAddr;
		ReplyAddr.S_un.S_addr = pEchoReply->Address;
		printf( "\tSent icmp message to %s\n", adress_ip );
		if( dwRetVal > 1 ) {
			printf( "\tReceived %ld icmp message responses\n", dwRetVal );
			printf( "\tInformation from the first response:\n" );
		}
		else {
			printf( "\tReceived %ld icmp message response\n", dwRetVal );
			printf( "\tInformation from this response:\n" );
		}
		printf( "\t  Received from %s\n", inet_ntoa( ReplyAddr ) );
		printf( "\t  Status = %ld\n",
			pEchoReply->Status );
		printf( "\t  Roundtrip time = %ld milliseconds\n",
			pEchoReply->RoundTripTime );
	}
	else return 1;

	return 0;
}