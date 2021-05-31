#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7 //select minimal legacy support, needed for inet_pton, inet_ntop
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	int OSInit( void )
	{
		WSADATA wsaData;
		int WSAerror = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if(WSAerror !=0)
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAGetLastError());
			exit(-1);
		}
	}
	int OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ":WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
#endif

int initialization();
void execution( int internet_socket,int *count);
void cleanup( int internet_socket);


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	
//////////////////
//Initialization//
//////////////////

	//OSInit();
	
	int internet_socket = initialization();

/////////////
//Execution//
/////////////
	int count = 0;
	while(1)
	{
		execution( internet_socket, &count);
	}


////////////
//Clean up//
////////////
	
	cleanup( internet_socket);
	
	//OSCleanup();
	
	printf("compiler works!");
	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int initialization()
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result = NULL;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24040", &internet_address_setup, &internet_address_result );
	
	if(getaddrinfo_return != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
		exit(1);
	}
	
	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while(internet_address_result_iterator != NULL)
	{
		internet_socket = socket( internet_address_result->ai_family, internet_address_result->ai_socktype, internet_address_result_iterator->ai_protocol );
		if(internet_socket == -1)
		{
			perror( "socket" );
		}
		else
		{
			int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
			if(bind_return == -1)
			{
				close( internet_socket);
				perror( "bind" );
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}
	freeaddrinfo( internet_address_result);
	if(internet_socket == -1)
	{
		fprintf( stderr, "socket: no valid socket adress found\n" );
		exit( 2 );
	}
	return internet_socket;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


void execution(int internet_socket, int *count)
{
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	int number_of_bytes_received = 0;
	char buffer[256];
	
		number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
		if(number_of_bytes_received == -1)
		{
			//perror("recvfrom");
		}
		else
		{
			buffer[number_of_bytes_received] = '\0';
			if(buffer[1]!=0)
			{
				*count++;
			}
		}
	int number_of_bytes_send = 0;
	number_of_bytes_send = sendto( internet_socket, "a", 1, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
	if(number_of_bytes_send == -1)
	{
		//perror("sendto");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void cleanup(int internet_socket)
{
	close( internet_socket );
}