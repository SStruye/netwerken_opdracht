#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7 //select minimal legacy support, needed for inet_pton, inet_ntop
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	
	#include <windows.h>
	

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
//void execution( int internet_socket);
void cleanup( int socket_send, int socket_recv);

// struct myStruct{
        // char name[50];
        // int client_send;
// };



/////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI client_send(void *socket_send)
{
	printf("\n|| Type your name: ");
	
	char name[256], c;
		int y = 0;
		//sleep(0.5);
		c = getchar();
		//fflush(stdin);
		while(c!='\n')
		{
			name[y] = c;
			y++;
			c = getchar();
		}
		
		
		name[strlen(name)]=':';
		name[strlen(name)]=' ';
		int name_l = strlen(name)+1;
	
	printf("\n|| Start chatting! ||\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~\n");
	
	for(;;)
	{
		char msg[256];
		for(int z =0; z<=name_l;z++)
		{
			msg[z]=name[z];
		}
		char ch;
		printf("|| You: ");
		int e = name_l-1;
		//sleep(0.5);
		ch = getchar();
		//fflush(stdin);
		while(ch!='\n')
		{
			msg[e] = ch;
			e++;
			ch = getchar();
		}
		int number_of_bytes_send = 0;
		number_of_bytes_send = send( *((int *) socket_send), msg, strlen(msg), 0 );
		if(number_of_bytes_send == -1)
		{
			perror("send");
			exit(1);
		}	
	}
}

DWORD WINAPI client_recv(void *socket_recv)
{
	for(;;)
	{
		sleep(0.5);
		int number_of_bytes_received = 0;
		char buffer[256];
		number_of_bytes_received = recv( *((int *) socket_recv), buffer, sizeof buffer, 0 );
		if(number_of_bytes_received == -1)
		{
			perror("recv");
			exit(1);
		}
		else
		{
			buffer[number_of_bytes_received] = '\0';
			printf("\r|| %s\n", buffer );
			printf("|| You: ");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	
//////////////////
//Initialization//
//////////////////
	//DWORD ThreadId[1];
    // myStruct s1;
    // myStruct * s1_address;
    
	// printf("\n|| Type your name: ");
	
	
	OSInit();
	
	//int internet_socket = initialization();
	HANDLE ThreadHandle[5];
	int socket_send = initialization();
	int socket_recv = initialization();
	
	// for(;;)
	// {
		// sleep(1);
		
		ThreadHandle[0] =  CreateThread(NULL, 0, client_send, &socket_send, 0, NULL);
		if (ThreadHandle[0] == NULL)
		{
			fprintf( stderr, "Could not create SEND socket" );
			ExitProcess(3);
		}
		
		
		ThreadHandle[1] =  CreateThread(NULL, 0, client_recv, &socket_recv, 0, NULL);
		if (ThreadHandle[1] == NULL)
		{
			fprintf( stderr, "Could not create RECV socket" );
			ExitProcess(3);
		}
	// }
/////////////
//Execution//
/////////////

	//execution( internet_socket);

////////////
//Clean up//
////////////
	
	cleanup( socket_send, socket_recv);
	
	OSCleanup();
	
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
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "192.168.2.99", "24024", &internet_address_setup, &internet_address_result );
	
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
			int connect_return = connect(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
			if(connect_return == -1)
			{
				perror( "connect" );
				close(internet_socket);
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

//-lws2_32

/*void execution(int internet_socket)
{
	int x;
	char msg[256];
	
	while(x)
	{
		printf("\nYou: ");
		gets(msg);
		
		int number_of_bytes_send = 0;
		number_of_bytes_send = send( internet_socket, msg, strlen(msg), 0 );
		if(number_of_bytes_send == -1)
		{
			perror("send");
		}
		
		int number_of_bytes_received = 0;
		char buffer[256];
		number_of_bytes_received = recv( internet_socket, buffer, sizeof buffer, 0 );
		if(number_of_bytes_received == -1)
		{
			perror("recv");
		}
		else
		{
			buffer[number_of_bytes_received] = '\0';
			printf("\nReceived : %s\n", buffer );
		}
	}
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void cleanup(int socket_send, int socket_recv)
{
	int shutdown_return  = shutdown(socket_send, SD_SEND);
	int shutdown_return1 = shutdown(socket_recv, SD_SEND);
	
	if(shutdown_return == -1)
	{
		perror( "shutdown" );
	}
	if(shutdown_return1 == -1)
	{
		perror( "shutdown" );
	}
	
	close( socket_send);
	close( socket_recv);
}