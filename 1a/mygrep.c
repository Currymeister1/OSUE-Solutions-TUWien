/**
  * @file mygrep.c
  * @author 
  * @date 13.11.2022
  *
  * @brief mygrep, simpler version of linux command mygrep, for finding keywords in file/stdin

**/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *PROG_NAME;

/**
  * Usage function
  * @brief If user provide wrong arguments, display the right usage and exit
  * @details Close the program with EXIT_FAILURE
**/
void usage(void){
    fprintf(stderr, "Usage: %s [-i] [-o file] keyword [file...]\n", PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
  * Display Error function
  * @brief Display error to the user
  * @details If an error occurs, display it to the user
  * @param error_message String containing the error message
**/
void display_error(char *error_message){
    fprintf(stderr, "[%s] ERROR: %s\n", PROG_NAME, error_message);
}


/**
  * To Lower Case function
  * @brief Converts the input to lower case
  * @details Loop over a valid char pointer and convert the chars into lower case till terminate symbol is reached 
  * @param input String, which needed to be lower cased
**/
void to_lower_case(char *input){
    while(*input != '\0'){
        *input = tolower(*input);
        input++;
    }
}

/**
  * Mygrep function
  * @brief Find the keyword in the file and print the whole line containg that keyword
  * @details By default (no input, no output file), mygrep will read the line from stdin 
  * and if the line contains the keyword, it will print it to the stdout. If the input 
  * file(s) are provided it will read from them. And if output file is provided, 
  * it will write to it. Based on the case_sensitive flag, upper and lower
  * case may be distinguish.
  * @param input Input file containing the input (by default stdin)
  * @param output Output file to write the result to (by default stdout)
  * @param keyword Keyword to be looked in the lines of input file
  * @param case_sensitive Boolean flag to check case sensitivity (by default true)
**/

void mygrep(FILE *input, FILE *output, char *keyword, _Bool case_sensitive){
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    
    //loop where we read from the file and write it to the output
    while((read = getline(&line, &len, input)) != -1){
        char *copied_line = strdup(line);
        if(!case_sensitive){
            to_lower_case(copied_line);
            to_lower_case(keyword);
        }
        
        char *compare =  strstr(copied_line,keyword);

        if(compare != NULL){
            fprintf(output, "%s", line);
        }

        free(copied_line);
        
    }
   free(line);
}


/**
  * Main function
  * @brief Entry point to the program
  * @details The flags (case sensitive and output file) are checked and set. 
  * and then the grep function is called.
  * @param argc 
  * @param argv
  * @return EXIT_SUCCESS on success and EXIT_FAILURE on failure
**/

int main(int argc, char **argv){
    PROG_NAME = argv[0];
    char *keyword;
    FILE *input;
    FILE *output;

    _Bool case_sensitive = true;
    char *out_file_name = NULL;

   //Setting the flags 
    int c;
    while((c=getopt(argc, argv , "io:")) != -1){
        switch (c) {
            case 'i':
                if(case_sensitive == false){
                    usage();
                    break;
                }
                case_sensitive = false;
                break;
            case 'o':
                if(out_file_name != NULL){
                    usage();
                    break;
                }
                out_file_name = optarg;
                break;
            case '?':
                usage();
                break; 
            default:
                usage();
                break;   
        }
    }


    //If output flag on, write to output file else to standard output
    output = stdout;
    if(out_file_name){
        output = fopen(out_file_name, "w");
        if(output == NULL){
            display_error("Couldn't open output file.");
        }
    }

    //Get the keyword
    keyword = argv[optind++];

    if(keyword == NULL){
        usage();
    }

    input = stdin;
    
    //Check if input files are given.
    if(optind < argc){
        //Loop over the input files and call for each file mygrep
        while(optind < argc){
            char* in_file_name = argv[optind++]; 
            input = fopen(in_file_name, "r");

            if(input == NULL){
                display_error("Couldn't able to open the input file. \n");
            }

            mygrep(input, output, keyword, case_sensitive);
            
            if(fclose(input) != 0){
                display_error("Couldn't able to close the input file. \n");
            }

        }
    }
    else{
        mygrep(input, output, keyword, case_sensitive);
    }



    if(out_file_name){
        if(fclose(output) != 0){
            display_error("Couldn't able to close output file properly. \n");
        }
    }

   
    exit(EXIT_SUCCESS);
}