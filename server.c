#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Used by read()
#include <string.h> //String manipulation
#include <stdbool.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


#define COM_PORT 12345
#define BUFF_SIZE 4096
#define BUFF_SIZE_SMALL 1024
#define BUFF_MAX_SIZE 2000000000 // 2MB
#define QUEUE_SIZE 10
#define START_INDEX 5

#define HTTP_OK_MESSAGE "HTTP/1.1 200 OK\r\n"
#define HTTP_NOT_FOUND_RESPONSE "HTTP/1.1 404 NOT FOUND\r\n"
#define HTTP_INTERNAL_ERROR_RESPONSE "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"

#define HTTP_CONTENT_JPEG "Content-Type: image/jpeg\r\n\r\n"
#define HTTP_CONTENT_GIF "Content-Type: image/gif\r\n\r\n"
#define HTTP_CONTENT_TEXTFILE "Content-Type: text/html; charset=UTF-8\r\n\r\n"

#define tr_shoot 1 //Troubleshooting, 1 - Activated -> Extra prints in terminal

char *buildReturnMessage(char *statusLine, char *contentType, char *fileBuffer, long *fileSize);
char * getFileName(char *requestLine);
bool hasFileType(char *resourceLine, int sizeOfresource);
char *getFileType(char *fileName, int sizeFileName);
int errorMessage(char *errorType, int error);
int getRequestLine(char *buffer, char *requestBuffer);
int closeSocket(int socket);
void troubleShoot(char * typeText, char *text);
char *readFile(FILE *fp, long *size);

int main()
{
     
    int server_socket; //Server socket
    int client_socket; //Client socket

    int opt = 1; //Option level
    struct sockaddr_in address; 
    int addressSize = sizeof(address);

    //Buffers
    char buffer[BUFF_SIZE];
    char requestBuffer[BUFF_SIZE];
    char *methodLine; //NEEDS FREE
    char *resourceLine; //NEEDS FREE
    char *fileTypeLine; //NEEDS FREE

    FILE *fp;

    //1. Create socket
    errorMessage("Create socket", server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    
    //Makes sure that server port and adress are reusable
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    //Create address for server socket
    address.sin_family = AF_INET;
    address.sin_port = htons( COM_PORT );
    address.sin_addr.s_addr = (INADDR_ANY); //Adress = 0.0.0.0 to all interfaces, instead of hostent struct


    //2. Binds socket with adress
    errorMessage("Binding", bind(server_socket, (struct sockaddr*) &address, addressSize));


    //3. Listen
    errorMessage("Listen", listen(server_socket, QUEUE_SIZE));
    while(1)
    {
        printf("Waiting for requests..\n");
        //4. Accept
        if(errorMessage("Accept", client_socket = accept(server_socket, (struct sockaddr*) &address, (socklen_t*)&addressSize)))
        {
            //Clears buffers
            memset(buffer, 0, BUFF_SIZE-1);
            memset(requestBuffer, 0, BUFF_SIZE-1);

            printf("-----------------------------------\n");
            printf("\nSuccessfully accepted request\n");

            //Reads buffer from socket
            read(client_socket, buffer, BUFF_SIZE-1);

            //Gets requestline and makes sure that it is fully read
            if(getRequestLine(buffer, requestBuffer))
            {
                //Makes shure that method is GET
                if(strncmp(requestBuffer, "GET", 3) == 0)
                {
                    /* Extracts file name and checks if it exists on server */
                    resourceLine = getFileName(requestBuffer);
                    if(strlen(resourceLine) == 0){
                        //No resource asked for
                        //RETURNA 404
                    } 
                    fp = fopen(resourceLine, "rb");
                    if(fp == NULL)
                    {
                        printf("\nUnable to open file\n");
                        //RETURNA 404
                    }

                    /* Extracts file type if it exists */
                    if(hasFileType(resourceLine, strlen(resourceLine)))
                    {
                        fileTypeLine = getFileType(resourceLine, strlen(resourceLine));
                    }else
                    {
                        
                        printf("\nFile has no filetype, what to do here");
                        //RETURNA 404
                    }

                    long *fileSize = malloc(sizeof(long));
                    char *fileBuffer = readFile(fp, fileSize);
                    char *returnMessage;
                    
                    int okSize = strlen(HTTP_OK_MESSAGE);
                    int contentSize;
                    long totalSize;

                    /* Checks filetype and runns correct HTTP structure*/
                    if(strcmp(fileTypeLine, "jpg") == 0 | strcmp(fileTypeLine, "jpeg") == 0)
                    {
                        /* IMAGES */
                        printf("Sending image %s\n", resourceLine);


                        contentSize = strlen(HTTP_CONTENT_JPEG);
                        totalSize = *fileSize + okSize + contentSize;
                        returnMessage = malloc(totalSize);
                        memcpy(returnMessage, HTTP_OK_MESSAGE, okSize); //Adds status line
                        memcpy(returnMessage + okSize, HTTP_CONTENT_JPEG, contentSize); //Adds header content line
                        memcpy(returnMessage + (okSize + contentSize), fileBuffer, *fileSize); //Adds file binary content
                        
                        buildReturnMessage(HTTP_OK_MESSAGE, HTTP_CONTENT_JPEG, fileBuffer, fileSize);

                        send(client_socket, returnMessage, totalSize+1, 0); //Sends whole entity of http response

                    }else if(strcmp(fileTypeLine, "gif") == 0)
                    {
                        /* GIFS */
                        printf("Sending GIF %s\n", resourceLine);
                    }
                    else if(strcmp(fileTypeLine, "txt") == 0)
                    {
                        /* TEXT */
                        printf("\nSending file %s\n", resourceLine);
                    } else
                    {
                        /* NOT FOUND */
                        printf("\n404 NOT FOUND");
                    }
                    
                    
                    

                    //fileTypeLine = getFileType(resourceLine);

                    /*
                    * 1. Kontrollera vilken fil som eftersöks
                    * 2. Sök efter filen
                    * 3. Om filen finns - Switch case
                    * 4. Om filen inte finns - 404 NOT FOUND
                    */


                    //Create return message
                    //memset(responseBuffer, 0, sizeof(responseBuffer));

                    //strcpy(responseBuffer, okMessage);





                }
                else
                {
                    printf("\nRequestline is not a getter. Closing socket - Returning %s\n", HTTP_INTERNAL_ERROR_RESPONSE);
                    //Sends error message if request method is not a GET
                    send(client_socket, HTTP_INTERNAL_ERROR_RESPONSE, strlen(HTTP_INTERNAL_ERROR_RESPONSE), 0);
                }
            
            } else
            {
                printf("Unable to read requestline\n");
            }

            //Closes client side socket
            printf("\nClosing connection\n");
            printf("-----------------------------------\n");
            closeSocket(client_socket);
        }

    }

    shutdown(server_socket, SHUT_RDWR);
    return 0;
}

char *buildReturnMessage(char *statusLine, char *contentType, char *fileBuffer, long *fileSize)
{
    int statusSize = strlen(statusLine);
    int contentTypeSize = strlen(contentType);

    long totalSize = statusSize + contentTypeSize + *fileSize;
    

    if(totalSize < BUFF_MAX_SIZE)
    {
        troubleShoot("Build return message", "Size allowed by threshold");

    }
    

}




/*  Closes wanted socket*/
int closeSocket(int socket)
{
    close(socket);
}


char * getFileName(char *requestLine)
{
    int i = 5; //Magic number for resource
    char *resourceLine = malloc(BUFF_SIZE_SMALL);
    strcpy(resourceLine, "");
    char *middleMan = " ";

    while(strncmp((requestLine+i), middleMan, 1) != 0)
    {
        resourceLine[i-5] = requestLine[i];
        i++;
    }
    troubleShoot("Get file name", resourceLine);
    return resourceLine;
}

bool hasFileType(char *resourceLine, int sizeOfresource)
{
    int i;
    char *dot = ".";
    bool hasFileType = false;

    for(i=0; i<sizeOfresource; i++)
    {
        if(strncmp(resourceLine+i, dot, 1)==0)
        {
            hasFileType = true;
        }
    }

    return hasFileType;
}

char *getFileType(char *fileName, int sizeFileName)
{
    char *returnBuff = malloc(BUFF_SIZE_SMALL);
    strcpy(returnBuff, "");
    char *dot = ".";
    int i;
    bool foundStart = false;

    for(i=0; i<=sizeFileName; i++)
    {
        if(foundStart)
        {
            strncat(returnBuff, fileName+i, 1);
        }
        if(strncmp((fileName + i), dot, 1) == 0)
        {
            foundStart = true;
        }
        
    }
    troubleShoot("Get file type", returnBuff);
    return returnBuff;

}


/* Return requestline from clientside request
    Returns true if requestBuffer is smaller than the buff. Else false since there is more to read*/
int getRequestLine(char *buffer, char *requestBuffer)
{
    int i = 0;
    char terminator = '\0';

    while(i < BUFF_SIZE && buffer[i] != '\n')
    {
        strncat(requestBuffer, &buffer[i], 1);
        i++; 
    }
    //Adds terminator att end
    strcat(requestBuffer, &terminator);
    
    troubleShoot("getRequest", requestBuffer);
    

    if((sizeof(requestBuffer)/sizeof(char)) < BUFF_SIZE) return 1;
    else return 0;
}



/* Error handling for all functions that return -1 for error
*  Used by Socket(), Bind(), Listen(), Accept()
*/
int errorMessage(char *errorType, int returnValue)
{
    if(returnValue < 0 )
    {
        perror(errorType);
        return 0;
    }
    return 1;
}

void troubleShoot(char * typeText, char *text)
{
    if(tr_shoot)
    {
        printf("Debug - %s: %s\n", typeText, text);
    }
}


char *readFile(FILE *fp, long *size)
{
    fseek(fp, 0, SEEK_END);
    long localSize = ftell(fp);
    rewind(fp);

    char *fileBuffer = malloc(localSize+1);
    *size = localSize+1;

    int result = fread(fileBuffer, 1, localSize, fp);

    if(result != localSize)
    {
        printf("\nUnable to read file");
        return "error";
    }else
    {
        printf("\nReturning file");
        return fileBuffer;
    }
    
}



//TEXT FILE TRANSFER
                    /*
                    strcat(responseBuffer, contentText);
                    //strcat(responseBuffer, "Message goes here\r\n");

                    printf("\nResponseBuffer: \n%s\n", responseBuffer);
                    FILE *fd = fopen("text.txt", "r");
                    char fileread[BUFF_SIZE];

                    int result = fread(fileread, BUFF_SIZE, 1, fd);
                    printf("\nResult is: %s\n", fileread);

                    printf("\nRead %i bytes", result);
                    fclose(fd);

                    strcat(responseBuffer, fileread);
                    */



/*
                    }else
                    {
                        fseek(fp, 0, SEEK_END); //Moves to end of file for size
                        long fileSize = ftell(fp);
                        fseek(fp, 0, SEEK_SET); //Moves pointer to the beginning again
                        printf("\nSize of file is: %li", fileSize);

                        char *fileBuffer = malloc(fileSize + 1);

                        int result = fread(fileBuffer, 1, fileSize, fp);
                        printf("\nResult from read is: %i", result);

                        if(result != fileSize)
                        {
                            printf("\nFile was not read correctly");
                        } else
                        {
                            printf("\nFile was read correctly. Putting together file");
                            printf("\nRefixing size of responseBuffer to: %li", fileSize);
                            long totalSize = (fileSize + strlen(okMessage) + strlen(contentImageJpeg));
                            char *responseBuffer = malloc(totalSize);
                            printf("\nTotal size is: %li", totalSize);

                            memcpy(responseBuffer, okMessage, strlen(okMessage));
                            memcpy(responseBuffer + strlen(okMessage), contentImageJpeg, strlen(contentImageJpeg));
                            printf("\nBuffer is: \n%s\n", responseBuffer);

                            memcpy(responseBuffer +(strlen(okMessage) + strlen(contentImageJpeg)), fileBuffer, fileSize);

                            send(client_socket, responseBuffer, totalSize, 0);
                        }
                    }
                    */