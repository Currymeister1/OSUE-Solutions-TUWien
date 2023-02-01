/**
  * @file client.c
  * @author 
  * @date 15.01.2023
  *
  * @brief Client for requesting files from a server supporting HTTP
**/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

char *PROG_NAME;


/**
  * Usage function
  * @brief If user provide wrong arguments, display the right usage and exit
  * @details Close the program with EXIT_FAILURE
**/
void usage(void){
    fprintf(stderr, "USAGE: %s [-p PORT] [-o FILE | -d DIR] URL \n", PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
  * Error Exit function
  * @brief Display error to the user
  * @details If an error occurs, display it to the user
  * @param msg String containing the error message
**/
void error_exit(char *msg){
    fprintf(stderr, "%s: Error occured: %s \n",PROG_NAME ,msg);
    exit(EXIT_FAILURE);
}


/**
  * Parse Host function
  * @brief Parse host name from the given URL
  * @param URL needed to be parsed
  * @param HOST_NAME pointer in which the host name will be saved
  * @param size size of HOST_NAME
  * @return the length of host name 
**/
int parse_host(char *URL, char *HOST_NAME, const size_t size){
    const char *p;
	char *q = HOST_NAME;
	int n = 0;
	
	HOST_NAME[0] = '\0';
 
	if ((p = strstr(URL, "http://")) == NULL){
        error_exit("The URL must start with http");
    } 

	for (p += 7; *p; p++ ){
		if (n >= (int)size) break;
		if (*p == ':' || *p == '/' || *p == ';' || *p == '?' || *p == '@' || *p == '=' || *p == '&'){
           break;
        }
		*q++ = *p;
		n++;
	}
	*q = '\0';

    return n;
}

/**
  * Parse File function
  * @brief Parse file name from the given URL
  * @param URL needed to be parsed
  * @param FILE_NAME pointer in which the host name will be saved
  * @param starting_pos starting position in URL for parsing the file name 
  * @param size size of FILE_NAME
  * @return the length of file name 
**/
int parse_file(char *URL, char *FILE_NAME, int starting_pos,const size_t size){
    const char *p;
    char *q = FILE_NAME;
    int n = 0;

    FILE_NAME[0] = '\0';

    if ((p = strstr(URL, "http://")) == NULL){
        error_exit("Error occured");
    } 

    for(p += (starting_pos+7); *p; p++){
        if(n>=(int)size) break;
        *q++ = *p;
        n++;
        
    }
   
    *q = '\0';
    return n;
   
}



/**
  * Setup Connection function
  * @brief Creates a connection to the serveer
  * @param host_name name of the host to be connected to
  * @param port port number on which the server is listening
  * @return socket file descriptor 
**/
int setup_connection(char *host_name, int port){
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    //Converting Port (int) To A Char
    char service[12];
    sprintf(service, "%d", port);

    int res = getaddrinfo(host_name, service, &hints, &ai); 

    if(res != 0){
        error_exit("getaddrinfo failed %s");
    }

    int sockfd = socket(ai->ai_family, ai->ai_socktype,ai->ai_protocol);

    if(sockfd < 0){
        error_exit("socket failed");
    }

    if(connect(sockfd, ai->ai_addr, ai->ai_addrlen)<0){
        error_exit("connect failed");
    }
    freeaddrinfo(ai);
    return sockfd;
}


/**
  * Request Header function
  * @brief Sending the request header to the server.
  * @param sockfile use for sending data
  * @param host_name name of the host
  * @param file_name requested file from the server
**/
void request_header(FILE *sockfile, char *host_name, char *file_name){
    if(fprintf(sockfile, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", file_name, host_name) < 0){
        error_exit("fprintf failed");
    }
    if(fflush(sockfile) == EOF){
        error_exit("fflush failed");
    }
}

/**
  * Response Header function
  * @brief Reading the response header from the server.
  * @details Checking if the response header is valid. 
  * Invalid header causes the client to shutdown with exit code 2 or 3.
  * @param sockfile use for sending data
**/
void response_header(FILE *sockfile){
    char buffer[5000];

    if(fgets(buffer,sizeof(buffer),sockfile) == NULL){
        error_exit("Response fgets failed ");
   }
    
   

    char *http = NULL;
    char *status_code = NULL;
    char *status_msg = NULL;

    http = strtok(buffer, " ");
    status_code = strtok(NULL, " ");
    status_msg = strtok(NULL, "");

    if(http == NULL || status_code == NULL || status_msg == NULL){
        fprintf(stderr, "%s: Protocol error! \n", PROG_NAME);
        exit(2);
    }
    char *endptr;
    int status_code_int = strtol(status_code, &endptr, 10);
    if(strcmp(http, "HTTP/1.1") != 0 || strlen(endptr) > 0){
        fprintf(stderr, "%s: Protocol error!", PROG_NAME);
        exit(2);
    }

    if(status_code_int != 200){
        fprintf(stderr, "%s %s \n", status_code, status_msg);
        exit(3);
    }
      
}

/**
  * Reading Content function
  * @brief Readig the content from the server.
  * @param sockfile use for reading the data
  * @param output_file the file, to which the content must be stored
**/

void read_content(FILE *sockfile, FILE *output_file){
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

  
    bool start_printing = false;
    while((nread = getline(&line, &len, sockfile)) != -1){
        if(start_printing){
            fprintf(output_file, "%s", line);
        }
         if(strcmp(line, "\r\n") == 0){
            start_printing = true;
        }
    }
    free(line);
}

/**
  * Main Function
  * @brief Entry point to the program
**/
int main(int argc, char **argv){
    PROG_NAME = argv[0];

    if(argc < 2){
        usage();
    }

    //FLAGS
    bool isPortGiven = false;
    bool o_flag = false;
    bool dir_flag = false;

    //default settings
    int port = 80;
    char *output_file_name = NULL;
    char *directory_name = NULL;
    
    char *wrongPortInput;
    int c = 0;

    while((c=getopt(argc, argv , "p:o:d:")) != -1){
        switch (c) {
            case 'p':
                if(isPortGiven == true){
                    error_exit("Port flag must be given once!");
                    break;
                }
                isPortGiven = true;
                port = strtol(optarg, &wrongPortInput, 10);

                if(strlen(wrongPortInput) > 0){
                    error_exit("Port number must be numerical!");   
                    break;
                }
                break;
            case 'o':
                if(o_flag == true || dir_flag == true){
                    usage();
                    break;
                }
                o_flag = true;
                output_file_name = optarg; 
                break;
            case 'd':
                   if(o_flag == true || dir_flag == true){
                    usage();
                    break;
                }
                dir_flag = true;
                directory_name = optarg;
                break;
            case '?':
                error_exit("Invalid flag!");
                break; 
            default:
                usage();
                break;   
        }
    }

    //Getting the URL
    char *URL = argv[optind++];

    //Checking if the user supplies additional arguments after the URL
    if(argc > optind){  
        usage();
    }

    //Parsing URL
    char HOST_NAME[300];
    char FILE_NAME[500];
    
    int parse_host_len = parse_host(URL, HOST_NAME, sizeof(HOST_NAME));
    int parse_file_len = parse_file(URL, FILE_NAME, parse_host_len, sizeof(FILE_NAME));

    int sockfd = setup_connection(HOST_NAME, port);

    //Connection established
    FILE *sockfile = fdopen(sockfd, "r+");
    if(sockfile == NULL){
        error_exit("fdopen failed");
    }

    request_header(sockfile, HOST_NAME, FILE_NAME);
    response_header(sockfile);

    //Setting output;
    FILE *output = stdout;
    fprintf(stdout,"File path: %s\n", FILE_NAME);

    if(directory_name != NULL){
        //mkdir(directory_name, S_IRWXU | S_IRWXG | S_IRWXO);
        char *filepath;
        if(parse_file_len == 1){
            char *ih = "/index.html"; 
            filepath = malloc(strlen(directory_name) + strlen(ih) + 2);
            filepath = strcpy(filepath, directory_name);
            filepath = strcat(filepath, ih);
        }
        else{
            filepath = malloc(strlen(directory_name) + strlen(FILE_NAME) + 2);
            filepath = strcpy(filepath, directory_name);
            filepath = strcat(filepath, FILE_NAME);
        }
       
        output = fopen(filepath, "w+");
        free(filepath);
    }

    if(output_file_name != NULL){
        output = fopen(output_file_name, "w+");
    }

    if(output == NULL){
        error_exit("Fail to open output file");
    }


    read_content(sockfile, output);
  

    fclose(output);
    fclose(sockfile);
    return EXIT_SUCCESS;
}
