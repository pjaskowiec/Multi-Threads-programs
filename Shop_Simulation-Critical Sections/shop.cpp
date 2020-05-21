#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#include <Psapi.h>
#include <time.h>
#include <string>

#define WIN32_LEAN_AND_MEAN
#pragma comment( lib, "winmm.lib")   
#pragma warning (disable:4996)


using std::cout;
using std::endl;
using std::string;
using std::rand;

CRITICAL_SECTION CriticalSection;
int yeast_in_shop = 0;

typedef struct shop_data
{
	char* client_id;
	int buy_time;
	int yeast_number;
	int prio;
}shop_data;


void shop_management( int n );
int thread_function( shop_data* args );
void guard( shop_data* shop_guard, int size );



int main( int argc, char* argv[] )
{
	if( argc < 2 ) return -1;
	srand( (unsigned)time( 0 ) );

	yeast_in_shop = 50 + rand() % ( 150 - 50 + 1 );
	printf( "Sklep Biedronka : Otwieramy, mamy dzis %d paczek drozdzy.\n", yeast_in_shop );


	shop_management( atoi( argv[ 1 ] ) );

	return 0;
}

void shop_management( int n )
{ 
	InitializeCriticalSection( &CriticalSection );


	HANDLE* thread_handles = new HANDLE[ n ];
	shop_data* shop_manage = new shop_data[ n ];


	for( int i = 0; i < n; i++ )
	{
		char* fusion = new char[ 2 ];
		char tmp1, tmp2;
		tmp1 = 65 + rand() % ( 90 - 65 + 1 );
		tmp2 = 65 + rand() % ( 90 - 65 + 1 );
		sprintf( fusion, "%c%c", tmp1, tmp2 );
		shop_manage[ i ].client_id = fusion;
		shop_manage[ i ].buy_time = 0;
		shop_manage[ i ].yeast_number = 0;
		shop_manage[ i ].prio = 0;
	}


	DWORD init_time = timeGetTime();
	DWORD init_guar = timeGetTime();
	int guard_time;
	bool to_check=false;

	while( 1 )
	{
		if( timeGetTime() - init_time >= 5000 || to_check ) break;
		for( int i = 0; i < n; i++ )
		{

			if( !yeast_in_shop )
			{
				cout << endl << "Klient " << shop_manage[ i ].client_id << " dla mnie braklo!" << endl << "Sklep biedronka zamykamy!" << endl;
				to_check = true;
				break;
			}

			thread_handles[ i ] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)thread_function, &shop_manage[ i ], CREATE_SUSPENDED, 0 );
			if( !thread_handles[ i ] ) continue;



			shop_manage[ i ].buy_time = 10 + rand() % ( 100 - 10 + 1 );

			guard_time = timeGetTime() - init_guar;

			if( guard_time >= 300 )
			{
				guard( shop_manage, n );
				SetThreadPriority( thread_handles[ i ], shop_manage[ i ].prio );
				init_guar = timeGetTime();
			}

			ResumeThread( thread_handles[ i ] );

		}

	}
	WaitForMultipleObjects( n, thread_handles, FALSE, INFINITE );


	printf( "Statystyki zakupow :\nSklep Biedronka : zostalo %d paczek\n", yeast_in_shop );
	for( int j = 0; j < n; j++ )
	{
		printf( "Klient %s : %d paczek\n", shop_manage[ j ].client_id, shop_manage[ j ].yeast_number );
		CloseHandle( thread_handles[ j ] );
	}

	delete[] shop_manage;
	delete[] thread_handles;

	shop_manage = NULL;
	thread_handles = NULL;

	DeleteCriticalSection( &CriticalSection );

}

int thread_function( shop_data* args )
{

	if( yeast_in_shop > 0 )
	{

		EnterCriticalSection( &CriticalSection );
		args->yeast_number++;
		yeast_in_shop--;
		cout << "Klient " << args->client_id << " kupilem drozdze w " << args->buy_time << "ms." << endl;
		Sleep( args->buy_time );
		LeaveCriticalSection( &CriticalSection );
		return 0;
	}
	return 1;

}

void guard( shop_data* shop_guard, int size )
{

	int max = shop_guard[ 0 ].yeast_number;
	int min = shop_guard[ 1 ].yeast_number;
	if( max < min )
	{
		int tmp = max;
		max = min;
		min = tmp;
	}

	if( size > 2 )
	{
		for( int j = 2; j < size; j++ )
		{
				if( shop_guard[ j ].yeast_number > max ) max = shop_guard[ j ].yeast_number;
				else if( shop_guard[ j ].yeast_number < min ) min = shop_guard[ j ].yeast_number;
		}
	}

	if( max == min )
	{
		for( int y = 0; y < size; y++ )shop_guard[ y ].prio = 0;
		return;
	}

	for( int w = 0; w < size; w++ )
	{
		if( shop_guard[ w ].yeast_number == max )
		{
			shop_guard[ w ].prio = -2;
			printf( "Ochroniarz: Klient %s na koniec kolejki\n", shop_guard[ w ].client_id );
		}
		else if( shop_guard[ w ].yeast_number == min )
		{
			shop_guard[ w ].prio = 2;
			printf( "Ochroniarz : Kliencie %s zapraszam.\n", shop_guard[ w ].client_id );
		}
		else shop_guard[ w ].prio = 0;

	}


}

