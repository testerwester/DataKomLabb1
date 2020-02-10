#include "server.h"

/*
DISCLAIMER: 
Inspiration for code found from mthe playlist:
https://www.youtube.com/watch?v=bdIiTxtMaKA&list=PL9IEJIKnBJjH_zM5LnovnoaKlXML5qh17



*/

int main()
{
     
    int server_socket = 0; //Server socket
    int client_socket = 0; //Client socket

    int opt = 1; //Option level
    struct sockaddr_in address; 
    int addressSize = sizeof(address);

    //Buffers
    char buffer[BUFF_SIZE] = {""};
    char requestBuffer[BUFF_SIZE] = {""};
    char *methodLine = NULL; 
    char *resourceLine = NULL; 
    char *fileTypeLine = NULL; 

    FILE *fp;
    
    long *fileSize = malloc(sizeof(long));
    char *fileBuffer = NULL;

    char *returnMessage = NULL;
    char *httpText = NULL;
                    
    int okSize = 0;
    int contentSize = 0;
    long *totalSize = malloc(sizeof(long));

    //1. Create socket
    errorMessage("Create socket", server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    
    //Makes sure that server port and adress are reusable
    errorMessage("SetsocketOpt", setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)));
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
            printf("Successfully accepted request\n");

            //Reads buffer from socket
            read(client_socket, buffer, BUFF_SIZE-1);

            //Gets requestline and makes sure that it is fully read
            if(getRequestLine(buffer, requestBuffer))
            {
                //Makes shure that method is GET
                if(strncmp(requestBuffer, "GET", 3) == 0)
                {
                    /* Extracts file name and checks if it exists on server */
                    resourceLine = getFileName(requestBuffer); //Gets file name
                    fileBuffer = findReadFile(resourceLine, fileSize); //Attempts to read file


                    if(fileBuffer == NULL){
                        printf("File not FOUND\n");
                        prepNotFound(&httpText, &fileBuffer, fileSize);
                    }else
                    {
                        /* Extracts file type if it exists */
                        if(hasFileType(resourceLine, strlen(resourceLine)))
                        {
                            fileTypeLine = getFileType(resourceLine, strlen(resourceLine));
                            okSize = strlen(HTTP_OK_MESSAGE);
                            httpText = calloc(okSize + 1, sizeof(char));
                            strcat(httpText, HTTP_OK_MESSAGE);

                            /* Checks filetype and runns correct HTTP structure*/
                            if(strcmp(fileTypeLine, "jpg") == 0 | strcmp(fileTypeLine, "jpeg") == 0)
                            {
                                /* IMAGES */
                                printf("Sending image %s\n", resourceLine);
                                contentSize = strlen(HTTP_CONTENT_JPEG);
                                httpText = realloc(httpText, okSize + contentSize + 1);
                                strcat(httpText, HTTP_CONTENT_JPEG);

                            }else if(strcmp(fileTypeLine, "gif") == 0)
                            {
                                /* GIFS */
                                printf("Sending GIF %s\n", resourceLine);
                                contentSize = strlen(HTTP_CONTENT_GIF);
                                httpText = realloc(httpText, okSize + contentSize +1);
                                strcat(httpText, HTTP_CONTENT_GIF);

                            }
                            else if(strcmp(fileTypeLine, "txt") == 0 | strcmp(fileTypeLine, "html") == 0)
                            {
                                /* TEXT */
                                printf("Sending file %s\n", resourceLine);
                                contentSize = strlen(HTTP_CONTENT_TEXTFILE);
                                httpText = realloc(httpText, okSize + contentSize + 1);
                                strcat(httpText, HTTP_CONTENT_TEXTFILE);

                            } else
                            {
                                /* NOT FOUND */
                                printf("Filetype not supported\n");
                                prepNotFound(&httpText, &fileBuffer, fileSize);
                            }

                        /*If file has no filetype*/
                        }else
                        {
                        
                            printf("File has no filetype, what to do here\n");
                            prepNotFound(&httpText, &fileBuffer, fileSize);
                        }
                    }
                }
                /* If requestline is not of GETTER format - Not supported */
                else
                {
                    printf("Requestline is not a getter. Closing socket - Returning %s\n", HTTP_INTERNAL_ERROR_RESPONSE);
                    //Sends error message if request method is not a GET
                    prepNotFound(&httpText, &fileBuffer, fileSize);
                }
            } else
            {
                printf("Unable to read requestline\n");
                //Internal server error
                prepNotFound(&httpText, &fileBuffer, fileSize);

            }

            if(httpText != NULL)
            {
                returnMessage = buildReturnMessage(httpText, fileBuffer, fileSize, totalSize);
                free(httpText);
            }
            else
            {
                printf("Unable to create HTTP response..\n");
            }
            

            
            send(client_socket, returnMessage, *totalSize, 0);
            free(fileBuffer);
            free(returnMessage);
            httpText = NULL;
            free(resourceLine);
            free(fileTypeLine);

            //Closes client side socket
            printf("\nClosing connection\n");
            printf("-----------------------------------\n");
            close(client_socket);
        }

    }
    shutdown(server_socket, SHUT_RDWR);
    printf("\nRELEASE ME\n"); 
    free(methodLine);
    free(resourceLine);
    free(fileTypeLine);
    free(fileSize);
    
    return 0;
}


/*
    Prepares statusline, contentype and filebuffer for 404 Not found
*/
void prepNotFound(char **requestPointer, char**filePointer, long *fileSize)
{
    int errorSize = strlen(HTTP_NOT_FOUND_RESPONSE);
    int contentSize = strlen(HTTP_CONTENT_TEXTFILE);

    if(*requestPointer == NULL)
    {
        troubleShoot("PNF NULL", *requestPointer);
        *requestPointer = calloc(errorSize + contentSize + 1, sizeof(char));
    }else
    {
        troubleShoot("PNF NOT NULL", *requestPointer);
        printf("\nPointer is: %p", *requestPointer);
        *requestPointer = realloc(*requestPointer, errorSize + contentSize + 1);
        
    }
    
    strcat(*requestPointer, HTTP_NOT_FOUND_RESPONSE);
    strcat(*requestPointer, HTTP_CONTENT_TEXTFILE);


    *filePointer = findReadFile(FILE_NOT_FOUND, fileSize);
    if(*filePointer == NULL)
    {
        printf("\nError file not found");
    }

}



/*
    Concatenates statusline, headerlines and file to one pointer to be sent in socket. 
    stateAndContent: Status line + all header lines
    fileBuffer: Allready read file
    fileSize: Size of filebuffer
    totalSize: Recievied total size for socket send
*/
char *buildReturnMessage(char *stateAndContent, char *fileBuffer, long *fileSize, long *totalSize)
{
    long textSize = strlen(stateAndContent);
    *totalSize = textSize + *fileSize;

    if(*totalSize < BUFF_MAX_SIZE)
    {
        troubleShoot("Build return message", "Size allowed by threshold");
        char *returnMessage = malloc(*totalSize);
        if(returnMessage == NULL)
        {
            printf("\nUnable to allocate returnM\n");
        }
        
        memcpy(returnMessage, stateAndContent, textSize); //Adds http message
        memcpy(returnMessage + textSize, fileBuffer, *fileSize); //Adds file content

        return returnMessage;

    }else
    {
        troubleShoot("Build return message", "Size not allowed by threshold");
        return NULL;
    }
    
    

}


/*
    Returnes whole filename
*/
char * getFileName(char *requestLine)
{
    int i = 5; //Magic number for resource
    char *resourceLine = malloc(BUFF_SIZE_SMALL);
    memset(resourceLine, 0, BUFF_SIZE_SMALL);
    strcpy(resourceLine, "");
    char *middleMan = " ";

    while(strncmp((requestLine+i), middleMan, 1) != 0)
    {
        resourceLine[i-5] = requestLine[i];
        i++;
    }

    resourceLine[i] = '\0';
    troubleShoot("Get file name", resourceLine);
    return resourceLine;
}

/*
    Returns true if filename is separated by "." to indicate that a filetype exists
*/
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

/*
    gets the specific filetype. Returns 
*/
char *getFileType(char *fileName, int sizeFileName)
{
    char *returnBuff = malloc(BUFF_SIZE_SMALL + 1);
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


/*
    Finds and returns allocated pointer with file. Returns NULL if not found
*/
char *findReadFile(char *fileName, long *size)
{

    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    long localSize = ftell(fp);
    rewind(fp);

    char *fileBuffer = calloc(localSize + 1, sizeof(char));
    *size = localSize+1;


    int result = fread(fileBuffer, 1, localSize, fp); 

    fclose(fp);

    if(result != localSize)
    {
        troubleShoot("Readfile", "Unable to read file - NULL");
        return NULL;
    }else
    {
        troubleShoot("Readfile", "Successfully read file");
        return fileBuffer;
    }
    
}

