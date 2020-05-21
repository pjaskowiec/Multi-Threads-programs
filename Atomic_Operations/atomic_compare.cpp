#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <winnt.h>

using namespace std;

int A = 0;
LONG volatile B = 0;

void atomic_thread( int N );
DWORD WINAPI thread_increase( LPVOID args );

int main( int argc, char* argv[] )
{
	if( argc < 2 ) return -1;
	atomic_thread( atoi( argv[ 1 ] ) );
	cout << "A(Normal operation)=" << A << endl << "B(Atomic operation)=" << B << endl;
	return 0;
}

void atomic_thread( int N )
{
	HANDLE* thread = new HANDLE[ N ];
	for( int i = 0; i < N; i++ )
	{

		thread[ i ] = CreateThread( NULL, 0, &thread_increase, 0, NULL, 0 );
		if( !thread[ i ] ) continue;

	}

	WaitForMultipleObjects( N, thread, TRUE, INFINITE );

	for( int j = 0; j < N; j++ )  CloseHandle( thread[ j ] );

	delete[] thread;
}

DWORD WINAPI thread_increase( LPVOID args )
{
	for( int j = 0; j < 1000000; j++ )
	{
		++A;
		InterlockedIncrement( &B );
		--A;
		InterlockedDecrement( &B );
	}
	return 0;
}