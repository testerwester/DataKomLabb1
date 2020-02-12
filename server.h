#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Used by read()
#include <string.h> //String manipulation
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


/*  Integer constants */
#define COM_PORT 12345
#define BUFF_SIZE 4096
#define BUFF_SIZE_SMALL 1024
#define BUFF_MAX_SIZE 2000000000 // 2MB
#define QUEUE_SIZE 10
#define START_INDEX 5


/* HTTP RESPONSES */
#define FILE_NOT_FOUND "notfound.txt"
#define HTML_INDEX_PAGE "index.html"

#define HTTP_OK_MESSAGE "HTTP/1.1 200 OK\r\n"
#define HTTP_NOT_FOUND_RESPONSE "HTTP/1.1 404 NOT FOUND\r\n"
#define HTTP_INTERNAL_ERROR_RESPONSE "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"

#define HTTP_CONTENT_JPEG "Content-Type: image/jpeg\r\n\r\n"
#define HTTP_CONTENT_GIF "Content-Type: image/gif\r\n\r\n"
#define HTTP_CONTENT_TEXTFILE "Content-Type: text/html; charset=UTF-8\r\n\r\n"

/*  Trouble shooting */
#define tr_shoot 0 //Troubleshooting, 1 = Activated -> More information in terminal


/*  Function declarations */

void prepNotFound(char **requestPointer, char**filePointer, long *fileSize);
char *buildReturnMessage(char *stateAndContent, char *fileBuffer, long *fileSize, long *totalSize);
char * getFileName(char *requestLine);
bool hasFileType(char *resourceLine, int sizeOfresource);
char *getFileType(char *fileName, int sizeFileName);
int errorMessage(char *errorType, int error);
int getRequestLine(char *buffer, char *requestBuffer);
int closeSocket(int socket);
void troubleShoot(char * typeText, char *text);
char *findReadFile(char *fileName, long *size);