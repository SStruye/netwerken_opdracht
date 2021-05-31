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

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length, int *count, char p );
void cleanup( int internet_socket, struct sockaddr * internet_address );


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	
//////////////////
//Initialization//
//////////////////

	OSInit();
	
	struct sockaddr * internet_address = NULL;
	socklen_t internet_address_length = 0;
	int internet_socket = initialization( &internet_address, &internet_address_length );

/////////////
//Execution//
/////////////
	while(1)
	{
		int y =0;
		char p ='\0';
		char q;
		printf("\n||How many packets do you want to send?: ");
		scanf(" %d", &y);
		int count = 0;
		for(int x = 1; x<=y;x++)
		{
			p = x;
			execution( internet_socket, internet_address, internet_address_length, &count ,p);
			//sleep(0.5);
		}
		
		printf("\n|| Packets received: %d\n",count);
		fflush(stdin);
		printf("\n|| Want to send more packets?(y/n): ");
		q = getchar();
		
		if(q =='n')
		{
			return 0;
		}
		
	}

////////////
//Clean up//
////////////
	
	cleanup( internet_socket, internet_address );
	
	OSCleanup();
	
	//printf("compiler works!");
	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result = NULL;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	int getaddrinfo_return = getaddrinfo( "192.168.2.99", "24040", &internet_address_setup, &internet_address_result );
	
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
			*internet_address_length = internet_address_result_iterator->ai_addrlen;
			*internet_address = (struct sockaddr *) malloc( internet_address_result_iterator->ai_addrlen );
			memcpy( *internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			break;
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


void execution(int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length, int *count, char p)
{
	char x[10];
	x[0] = p;
	int number_of_bytes_send = 0;
	number_of_bytes_send = sendto( internet_socket, x, sizeof x , 0, internet_address, internet_address_length );
	if(number_of_bytes_send == -1)
	{
		perror("sendto");
	}
	
	int number_of_bytes_received = 0;
	char buffer[1000];
	number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, internet_address, &internet_address_length );
	if(number_of_bytes_received == -1)
	{
		perror("recvfrom");
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		if(number_of_bytes_received != 0)
		{
			if(buffer[0] != p)
			{
				(*count)++;
			}
		}//printf( "Received : %s\n", buffer );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void cleanup(int internet_socket, struct sockaddr * internet_address)
{
	free( internet_address );
	close( internet_socket );
}