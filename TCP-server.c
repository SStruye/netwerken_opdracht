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
	#include <poll.h>
	#include <pthread.h>
#endif

#define PORT "24024"

int initialization();
void execution(int internet_socket,struct pollfd *pfds[], int *fd_count, int i);
void add_to_pfds(struct pollfd *pfds[], int client_internet_socket, int *fd_count, int *fd_size);
void *get_in_addr(struct sockaddr *sa);
int connection(int internet_socket, struct pollfd *pfds[], int *fd_count, int *fd_size);
void cleanup( int internet_socket, int client_internet_socket);


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	int client_internet_socket = 0;
	int fd_count	= 0;
	int fd_size		= 16;
	
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
	
	//Initialization
	int internet_socket = initialization();
	
	
	pfds[0].fd 		= internet_socket;
	pfds[0].events 	= POLLIN;
	

	
	fd_count	= 1;
	pthread_t tid[60];
	int k = 0;
	for(;;)
	{
		int poll_count = poll(pfds, fd_count, -1);
		
		if (poll_count == -1)
		{
			perror("poll");
			exit(1);
		}
		
		for(int i = 0; i < fd_count; i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				if(pfds[i].fd == internet_socket)
				{
					client_internet_socket = connection(internet_socket, &pfds, &fd_count, &fd_size);
				}
				else
				{
					execution( internet_socket, &pfds, &fd_count, i);
				}
			}
		}
	}
	
	//Clean up
	cleanup( internet_socket, client_internet_socket);
	
	
	printf("compiler works!");
	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int initialization()
{
    int listener;
    int yes=1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
			
            continue;
        }
		
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
		
        break;
    }

    freeaddrinfo(ai);
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int connection(int internet_socket,struct pollfd *pfds[], int *fd_count, int *fd_size)
{
	char remoteIP[INET6_ADDRSTRLEN];
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	int client_socket = accept(internet_socket, (struct sockaddr *) &client_internet_address, &client_internet_address_length);
	
	if(client_socket == -1)
	{
		perror("accept");
		//close(internet_socket);
		exit( 3 );
	}
	else
	{
		if (*fd_count == *fd_size) 
		{
			*fd_size *= 2; // Double it

			*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
		}
		(*pfds)[*fd_count].fd = client_socket;
		(*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
		(*fd_count)++;
		
		printf("|| New connection from %s on socket %d\n", 
			inet_ntop(client_internet_address.ss_family,
				get_in_addr((struct sockaddr*)&client_internet_address),
				remoteIP, INET6_ADDRSTRLEN),
			client_socket);
	}
	return client_socket;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void execution(int internet_socket,struct pollfd *pfds[], int *fd_count, int i)
{
	char buffer[256];
	int x = 0;
	int number_of_bytes_received = recv((*pfds)[i].fd, buffer, sizeof buffer, 0 );
	int sender_fd = (*pfds)[i+1].fd;
	
	if(number_of_bytes_received <= 0)
	{
		if(number_of_bytes_received == 0)
		{
			printf("||SERVER: socket %d left\n", sender_fd);
		}
		else
		{
			perror("recv");
		}
		
		close((*pfds)[i].fd);
		
		(*pfds)[i] = (*pfds)[*fd_count-1];
		(*fd_count)--;
	}
	else
	{
		for(int x = 0; x < *fd_count; x++)
		{
			int dest_fd = (*pfds)[x].fd;
			
			if (dest_fd != internet_socket && dest_fd != sender_fd) 
			{
				if (send(dest_fd, buffer, number_of_bytes_received, 0) == -1) 
				{
					perror("send");
				}
			}
		}
		
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void cleanup(int internet_socket, int client_internet_socket)
{
	int shutdown_return = shutdown(client_internet_socket, 2);
	if(shutdown_return == -1)
	{
		perror( "shutdown" );
	}
	close( client_internet_socket );
	close( internet_socket );
}