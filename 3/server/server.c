/**
  * @file server.c
  * @author 
  * @date 15.01.2023
  *
  * @brief Server for hosting files 
**/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
char *PROG_NAME;

volatile sig_atomic_t quit = 0;

/**
  * Handle Signal Function
  * @brief Function for handling signal  
**/
void handle_signal(int signal) { quit = 1; }

/**
  * Usage function
  * @brief If user provide wrong arguments, display the right usage and exit
  * @details Close the program with EXIT_FAILURE
**/
void usage(void){
    fprintf(stderr, "%s [-p PORT] [-i index] DOC_ROOT\n", PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
  * Error Exit function
  * @brief Display error to the user
  * @details If an error occurs, display it to the user
  * @param msg String containing the error message
**/
void error_exit(char *msg){
    fprintf(stderr, "%s Error occured %s\n", PROG_NAME, msg);
    exit(EXIT_FAILURE);
}

/**
  * Send Response Function
  * @brief Send a response to the client
  * @details If write failes, close the connection.
  * @param connfd File discriptor write to
  * @param resp Response message for the client
**/
void send_response(int connfd, char *resp){
    if(write(connfd, resp, strlen(resp)) == -1){
        close(connfd);
        error_exit("Couldn't send the response to the client.");
    }
}

/**
  * Server Setup function
  * @brief Setting up the server
  * @param port Port on which the server should listen to
**/
int setup_server(int port){
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Converting Port (int) To A Char
    char service[12];
    sprintf(service, "%d", port);

    int res = getaddrinfo(NULL, service, &hints, &ai);

    if(res != 0){
        error_exit("getaddrinfo failed");
    }
    int sockfd = socket(ai->ai_family, ai->ai_socktype,ai->ai_protocol);

	int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    if(sockfd < 0){
        freeaddrinfo(ai);
        error_exit("Sockfd failed!");
    }

    if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
        freeaddrinfo(ai);
        error_exit("Bind failed!");
    }

    freeaddrinfo(ai);
    return sockfd;
}

/**
  * Request Header function
  * @brief Reads the request header
  * @details If the request is invalid, sends the client a response 400
  * if the request header contains invalid request method, return 501
  * @param request File used for reading the request
  * @param connfd File discriptor used for writing back to the client
  * @param file_name Use to save the name of the requested file
  * @return On failure -1, on success 0
**/
int request_header(FILE *request, int connfd, char **file_name){
    char buffer[1024];
    
    if(fgets(buffer, sizeof(buffer), request) == NULL){
        fgets(buffer, sizeof(buffer), request);
   }
  
  

    char *req_method = NULL;
    char *req_file = NULL;
    char *req_protocol = NULL;
    char *extra = NULL;

    req_method = strtok(buffer, " ");
    req_file = strtok(NULL, " ");
    req_protocol = strtok(NULL, " ");
    extra = strtok(NULL, " ");

    if(req_method == NULL || req_file == NULL || req_protocol == NULL || extra != NULL){
        char *string = "HTTP/1.1 400 (Bad Request)\r\n";
        send_response(connfd, string);
        return -1;     
    }

    if(strcmp(req_method,"GET") != 0){
        char *string = "HTTP/1.1 501 (Not Implemented)\r\n";
        send_response(connfd, string);
        return -1;
    }

    if(strstr(req_protocol, "HTTP/1.1") == NULL){
        char *string = "HTTP/1.1 400 (Bad Request)\r\n";
        send_response(connfd, string);
        return -1;     
    }

    
    *file_name = strdup(req_file);
    return 0;
}

/**
  * Send Header function
  * @brief Send the header to the client
  * @details Send the header to the client if the request header was valid
  * The fields: HTTP Protocol, Date, Content-Length and Connection are set.
  * @param connfd Connection file descriptor use for writing back to client
  * @param requested_file File which was requested by the client.
**/
void send_header(int connfd, FILE *requested_file){
   
    char *string = "HTTP/1.1 200 OK\r\n";

    send_response(connfd, string);

    char line[500];

	time_t rtime;
	struct tm* timeinfo;

	time(&rtime);
	timeinfo = gmtime(&rtime);

	strftime(line, sizeof(line), "Date: %a, %d %b %y %H:%M:%S GMT\r\n", timeinfo);
	
    send_response(connfd, line);

   
   //File Size
    char content_length[500];
    fseek(requested_file, 0L, SEEK_END);
    int filesize = ftell(requested_file);
	rewind(requested_file);
    sprintf(content_length, "Content-Length: %d\r\n", filesize);
	send_response(connfd, content_length);

    //Connection closed   
    char *con_close = "Connection: close\r\n\r\n";
    send_response(connfd, con_close);
}

/**
  * Send File Content Function
  * @brief Send the content of the file
  * @details The content of the files are sent to the client after sending a header with
  * status code 200.
  * @param connfd Connection file descriptor use for writing back to client
  * @param requested_file File which was requested by the client.
**/
void send_file_content(int connfd, FILE *requested_file){
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, rueqested_file)) != -1) {
        write(connfd, line, strlen(line));
    }
    
    close(connfd);
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
    bool i_flag = false;

    //default settings
    int port = 8080;
    char *index_file_name = "index.html";

    
    char *wrongPortInput;
    int c = 0;

    while((c=getopt(argc, argv , "p:i:")) != -1){
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
            case 'i':
                if(i_flag){
                    usage();
                    break;
                }
                i_flag = true;
                index_file_name = optarg; 
                break;
            case '?':
                error_exit("Invalid flag!");
                break; 
            default:
                usage();
                break;   
        }
    }

    char *doc_root = NULL;
    doc_root = argv[optind++];

    if(doc_root == NULL || argc > optind){
        usage();
    }


    int sockfd = setup_server(port);

    if(listen(sockfd,1) < 0){
        error_exit("Listen Failed.");
    }

    //Signal Handling
   struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, 0) == -1 || sigaction(SIGTERM, &sa, 0) == -1){
        error_exit("Sig failed.");
    }
    
    
    //Accepting Connections
    while(!quit){
        int connfd = accept(sockfd,NULL,NULL);

        if(connfd < 0){
            if (errno == EINTR){
                continue;
            }
                
            close(connfd);
            close(sockfd);
            break;
        }

        FILE *request = fdopen(connfd, "r+");
        if(request == NULL){
            error_exit("fdopen failed!");
        }

        char *file_name;
        if(request_header(request, connfd,&file_name) == -1){
            char *con_close = "Connection: close\r\n\r\n";
            send_response(connfd, con_close);
            close(connfd);
            continue;
        }
        char *path ;

        if(file_name[strlen(file_name)-1] == '/'){
            if((path = malloc(strlen(file_name)+strlen(doc_root)+strlen("index.html")+1)) != NULL){
                path[0] = '\0';   // ensures the memory is an empty string
                strcat(path,doc_root);
                strcat(path,file_name);
                strcat(path,index_file_name);
            } else {
                close(sockfd);
                error_exit("Error occured");
            }
        }
        else{
            if((path = malloc(strlen(file_name)+strlen(doc_root)+1)) != NULL){
                path[0] = '\0';   // ensures the memory is an empty string
                strcat(path,doc_root);
                strcat(path,file_name);
            
            } else {
                close(sockfd);
                error_exit("Error occured");
            }
        }

        FILE *requested_file = fopen(path, "r");
        free(path);
        free(file_name);

        if(requested_file == NULL){
            char *string = "HTTP/1.1 404 (Not Found)\r\n";
            send_response(connfd, string);
            char *con_close = "Connection: close\r\n\r\n";
            send_response(connfd, con_close);
            close(connfd);
            continue;
        } 
        send_header(connfd, requested_file);
        send_file_content(connfd, requested_file);
    }
   
    close(sockfd);
}
