#include <iostream>
#include <windows.h>
#include <Windows.h>
#include <timeapi.h>
#include <Psapi.h>
#include <time.h>
#include <cstdlib>


#define WIN32_LEAN_AND_MEAN
#pragma comment( lib, "winmm.lib")   
#pragma warning (disable:4996)

typedef struct tagOffice
{
	int czas_obslugi;
	LONG volatile kolejka;
	int max_kolejka;
	LONG volatile obsluzony;
	int select_queue;
} window_office;

HANDLE hSemaphore;
HANDLE binSemaphore;
HANDLE bin2Semaphore;
HANDLE bin3Semaphore;
HANDLE bin4Semaphore;



window_office windows[ 3 ] = { {300,0,5,0},{150,0,10,0},{100,0,15,0} };


//Okienko 1 - czas obsługi osoby to 300s, a liczba miejsc w kolejce to 5,
//Okienko 2 - czas obsługi osoby to 150s, a liczba miejsc w kolejce to 10,
//Okienko 3 - czas obsługi osoby to 100s, a liczba miejsc w kolejce to 15.

void post_office( int P1 );
DWORD client_thread( LPVOID args );
DWORD worker_thread( window_office* args );
DWORD worker_thread2( window_office* args );
DWORD worker_thread3( window_office* args );


int main( int argc, char* argv[] )
{
	if( argc < 2 ) return -1;
	int N = atoi( argv[ 1 ] );
	post_office( N );

	return 0;
}




void post_office( int P1 )
{


	HANDLE* client = new HANDLE[ P1 ];

	hSemaphore = CreateSemaphore( NULL, 30, 30, NULL );
	if( !hSemaphore ) return;
	binSemaphore = CreateSemaphore( NULL, 1, 1, NULL );
	if( !binSemaphore ) return;
	bin2Semaphore = CreateSemaphore( NULL, 1, 1, NULL );
	if( !bin2Semaphore ) return;
	bin3Semaphore = CreateSemaphore( NULL, 1, 1, NULL );
	if( !bin3Semaphore ) return;
	bin4Semaphore = CreateSemaphore( NULL, 1, 1, NULL );
	if( !bin4Semaphore ) return;

	srand( time( NULL ) + GetCurrentProcessId() );

	DWORD bef_time = timeGetTime();
	int* rand_time = new int[ P1 ];

	for( int i = 0; i < P1; i++ )
	{
		rand_time[ i ] = 10 + rand() % ( 50 - 10 + 1 );
		client[ i ] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)client_thread, &rand_time[ i ], 0, NULL );
		if( !client[ i ] ) return;

	}
	for( int i = 0; i < P1; i++ ) WaitForSingleObject( client[ i ], INFINITE );
	for( int i = 0; i < P1; i++ ) CloseHandle( client[ i ] );
	CloseHandle( hSemaphore );
	CloseHandle( binSemaphore );
	CloseHandle( bin2Semaphore );
	CloseHandle( bin3Semaphore );

	delete[] rand_time;
	delete[] client;
	DWORD aft_time = timeGetTime() - bef_time;
	printf( "Czas obslugi:%d\n Obsluzonych w okienku 1:\n%d\n Obsluzonych w okienku 2:\n%d\n Obsluzonych w okienku 3:\n%d\n", aft_time, windows[ 0 ].obsluzony, windows[ 1 ].obsluzony, windows[ 2 ].obsluzony );
}


DWORD client_thread( LPVOID args )
{

	WaitForSingleObject( hSemaphore, INFINITE ); //czeka na wykonanie
	//maskymalnie trzydziesci-opuszcza,podnosi
	int helper = *( (int*)args );
	/*WaitForSingleObject( bin4Semaphore, INFINITE );
	printf( "%d ", helper );
	Sleep( 50 );
	ReleaseSemaphore( bin4Semaphore, 1, NULL );*/
	//i tak musiałby czekać na sleepa z dołu,który jest dluższy,nic nie da 

	if( ( windows[ 2 ].kolejka*windows[ 2 ].czas_obslugi <= windows[ 1 ].kolejka*windows[ 1 ].czas_obslugi ) && ( windows[ 2 ].kolejka*windows[ 2 ].czas_obslugi <= windows[ 0 ].kolejka*windows[ 0 ].czas_obslugi ) )
	{
		if( windows[ 2 ].kolejka < windows[ 2 ].max_kolejka )
		{
			windows[ 2 ].select_queue = helper;
			HANDLE work = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)worker_thread, &windows[ 2 ], 0, NULL );
			WaitForSingleObject( work, INFINITE );
			ReleaseSemaphore( hSemaphore, 1, NULL );
			return TRUE;
		}
	}
	//albo kolejka 2 jest pelna,albo w 1 czeka sie krocej,albo w 0 czeka sie krocej
	if( ( windows[ 1 ].kolejka*windows[ 1 ].czas_obslugi <= windows[ 0 ].kolejka*windows[ 0 ].czas_obslugi ) ) //jesli dwojka jest pelna to nawet nie zwracamy na nia uwagi,jesli w jedynce sie czekalo krocej,to 
	{
		if( windows[ 1 ].kolejka < windows[ 1 ].max_kolejka )
		{
			windows[ 1 ].select_queue = helper;
			HANDLE work = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)worker_thread2, &windows[ 1 ], 0, NULL );
			WaitForSingleObject( work, INFINITE );
			ReleaseSemaphore( hSemaphore, 1, NULL );
			return TRUE;
		}
	}
	else
	{
		if( windows[ 0 ].kolejka < windows[ 0 ].max_kolejka )
		{
			windows[ 0 ].select_queue = helper;
			HANDLE work = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)worker_thread3, &windows[ 0 ], 0, NULL );
			WaitForSingleObject( work, INFINITE );
			ReleaseSemaphore( hSemaphore, 1, NULL );
			return TRUE;
		}
		else if( windows[ 2 ].kolejka < windows[ 2 ].max_kolejka )
		{
			windows[ 2 ].select_queue = helper;
			HANDLE work = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)worker_thread, &windows[ 2 ], 0, NULL );
			WaitForSingleObject( work, INFINITE );
			ReleaseSemaphore( hSemaphore, 1, NULL );
			return TRUE;
		}

		if( windows[ 1 ].kolejka < windows[ 1 ].max_kolejka )
		{
			windows[ 1 ].select_queue = helper;
			HANDLE work = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)worker_thread2, &windows[ 1 ], 0, NULL );
			WaitForSingleObject( work, INFINITE );
			ReleaseSemaphore( hSemaphore, 1, NULL );
			return TRUE;
		}

	}	//albo dwie kolejki byly pelne,albo w windows[0] bylo najkrocej 
	// wchodzi do kolejki!
	//ReleaseSemaphore( hSemaphore, 1, NULL );


	return FALSE;


}

DWORD worker_thread( window_office* args )
{
	InterlockedIncrement( &( args->kolejka ) );
	WaitForSingleObject( binSemaphore, INFINITE );
	Sleep( args->select_queue );
	InterlockedIncrement( &( args->obsluzony ) );
	InterlockedDecrement( &( args->kolejka ) );
	Sleep( args->czas_obslugi );//tutaj w teorii mozna dolozyc czas czekania i sumarycznie by sie zgadzalo,ale teoretycznie nie
	ReleaseSemaphore( binSemaphore, 1, NULL );
	return TRUE;
}

DWORD worker_thread2( window_office* args )
{
	InterlockedIncrement( &( args->kolejka ) );
	WaitForSingleObject( bin2Semaphore, INFINITE );
	Sleep( args->select_queue );
	InterlockedIncrement( &( args->obsluzony ) );
	InterlockedDecrement( &( args->kolejka ) );
	Sleep( args->czas_obslugi );
	ReleaseSemaphore( bin2Semaphore, 1, NULL );
	return TRUE;
}

DWORD worker_thread3( window_office* args )
{
	InterlockedIncrement( &( args->kolejka ) );
	WaitForSingleObject( bin3Semaphore, INFINITE );
	Sleep( args->select_queue );
	InterlockedIncrement( &( args->obsluzony ) );
	InterlockedDecrement( &( args->kolejka ) );
	Sleep( args->czas_obslugi );
	ReleaseSemaphore( bin3Semaphore, 1, NULL );
	return TRUE;
}


//Napisz program postoffice.exe, który symuluje działanie poczty.Przed budynkiem poczty czeka na jej otwarcie P1 osób( wątek klienta ).
//Jednocześnie na pocztę wejść może 30 osób, synchronizacja przy pomocy semafora.Poczta posiada 3 okienka do obsługi osób.
//Pracownik poczty symulowany jest poprzez wątek pracownika.
//Każde okienko charakteryzuje się inną zręcznością pracownika, co skutkuje innym czasem obsługi osoby i inną ilością wolnego miejsca przed okienkiem, 
//co się wiąże z inną maksymalną liczbą osób oczekujących :
//
//Okienko 1 - czas obsługi osoby to 300s, a liczba miejsc w kolejce to 5,
//Okienko 2 - czas obsługi osoby to 150s, a liczba miejsc w kolejce to 10,
//Okienko 3 - czas obsługi osoby to 100s, a liczba miejsc w kolejce to 15.
//Jeśli klientowi uda się już wcisnąć do środka to zaczyna obserwować szybkość obsługi kolejki w każdym okienku.Po czym staje tam gdzie wydaje mu się, że będzie najszybciej lub gdzie jest jeszcze wolne miejsce.Czas obserwacji to 10 - 50s.
//
//Czas w programie płynie 1000 razy szybciej niż w rzeczywistości.
//
//Parametr P1 wczytywany jest z wiersza poleceń.
//
//Po zakończeniu obsługi wszystkich klientów program wypisuje statystyki :
//
//Czas obsługi wszystkich klientów.
//Liczba osób obsłużona w danym okienku.