#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


#define COM_PORT 12345
#define BUFF_SIZE 4096
#define REQUEST_SIZE 256

int createAndConnectSocket(struct sockaddr_in address, int network_socket);
int errorMessage(char *errorType, int returnValue);
void printMenu();

int getText(int socket);
int getJpeg(int socket);
int getGif(int socket);

void GETfile(int socket, char *filePath);

int main()
{

    int network_socket, choice = 1;
    struct sockaddr_in address;
    char textFile[] = {"textfile.txt"};
    char jpegFile[] = {"image.jpeg"};
    char gifFile[] = {"movingImage.gif"};

    while(choice)
    {
        
        printMenu();
        printf("\nYour input: ");
        scanf("%i", &choice);
        switch(choice)
        {
            case 1:
                network_socket = createAndConnectSocket(address, network_socket);
                GETfile(network_socket, textFile);
            break;

            case 2:
                network_socket = createAndConnectSocket(address, network_socket);
                GETfile(network_socket, jpegFile);
            break;

            case 3:
                network_socket = createAndConnectSocket(address, network_socket);
                GETfile(network_socket, gifFile);
            break;

            case 4:
                printf("\nExiting..");
                choice = 0;
            break;

            default:
                printf("\nUnknown command. Please try again");
            

        }
    }


    return 0;
}

int createAndConnectSocket(struct sockaddr_in address, int network_socket)
{
    if(!(errorMessage("Socket", network_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))))
    {
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons( COM_PORT );
    address.sin_addr.s_addr = ( INADDR_ANY );

    if(!(errorMessage("Connection", connect(network_socket, (struct sockaddr *) &address, sizeof(address)))))
    {
        exit(EXIT_FAILURE);
    }

    return network_socket;
}

void closeSocket(int socket)
{
    close(socket);
    socket = -1;
}

void GETfile(int socket, char *fileName)
{
    char textGetter[REQUEST_SIZE] = {"GET /"};
    char version[] = {" HTTP/1.1\r\n"};

    strcat(textGetter, fileName);
    strcat(textGetter, version);

    printf("\nGet command is: %s with size: %li", textGetter, strlen(textGetter));

    send(socket, &textGetter, strlen(textGetter), 0);

    char recBuffer[BUFF_SIZE];

    recv(socket, &recBuffer, sizeof(recBuffer), 0);
    printf("\nRecieved message is: %s\n", recBuffer);

    //CHECK STATUS HERE
    //RECIEVE FILE HERE

    closeSocket(socket);
}





int errorMessage(char *errorType, int returnValue)
{
    if(returnValue < 0 )
    {
        perror(errorType);
        return 0;
    }
    return 1;
}


void printMenu()
{
    printf("\n1. Request .txt from server\n2. Request .jpeg from server\n3. Reqeest .gif from server\n\n4. Exit\n");
}
