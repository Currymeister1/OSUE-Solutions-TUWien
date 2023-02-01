/**
  * @file generator.c
  * @author 
  * @date 13.11.2022

  * @brief The generator module
  * @details A generator writes the result using the 3 colouring algorithm to the shared memory
**/
#include "circularBuffer.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

char *progname;

/**
  * Usage function
  * @brief explains how to use the programm
  * @details If the user provides wrong argument, show usage message and 
  * exit on EXIT_FAILURE
**/
void usage(void){
    fprintf(stderr, "Usage: %s requires at least one edge where an edge is in form [d-d] where d is a number.\n", progname);
    exit(EXIT_FAILURE);
}

/**
  * Is Node Aleady Coloured function
  * @brief check if the parsed node is already coloured
  * @details Checking if the node is already coloured helps with not assigning different
  * colour values to the same node. 
  * @param node Node to be checked
  * @param parsedEdges Edge array containing the parsed edges and their nodes
  * @param size size of the array
  * @return the colour of the node or -1 indicating that node is still not coloured
**/
int isNodeAlreadyColoured(struct Node node, struct Edge *parseEdges, int size){
    
    for(int i = 0; i <= size; i++){
       
        if(parseEdges[i].n1.value == node.value){
            return parseEdges[i].n1.colour;
        }

        if(parseEdges[i].n2.value == node.value){
            return parseEdges[i].n2.colour;
        }
    }

    return -1;
}

/**
  * Colour Node function
  * @brief colours the node
  * @param node Node to be coloured
**/
void colour_node(struct Node *node){
    node->colour = (rand() % (3));
}

/**
  * Parse Graph function
  * @brief Parses the input graph
  * @details Parses the input graph and if the input isn't in the form of d-d,
  * prints usage and exit on EXIT_FAILURE. Else colour the node and save 
  * it to the parsedEdge array.
  * @param edge Edge of the input graph
  * @param parsedEdges Array to store edges
  * @param size size of the graph
**/
void parse_graph(char *edge, struct Edge *parseEdges, int size){
    struct Node node1;
    struct Node node2;
    
    if (sscanf(edge, "%d-%d", &node1.value, &node2.value) != 2){
			usage();
	}

   if((node1.colour = isNodeAlreadyColoured(node1, parseEdges, size)) == -1){
        colour_node(&node1);
    }

    if((node2.colour = isNodeAlreadyColoured(node2, parseEdges, size)) == -1){
        colour_node(&node2);
    }

    parseEdges[size].n1 = node1;
    parseEdges[size].n2 = node2;

}

/**
  * Generate Result function
  * @brief Use for generating results
  * @details Generate result by removing the neigbouring nodes with
  * the same colour.
  * @param parsedEdges the edges from the input graph
  * @param graphSize size of the graph = number of edges
  * @return result containg the amount of edges removed and the removed edges
**/
struct Result generate_result(struct Edge *parsedEdges, int graphSize){
    struct Result result;
    int removedEdges = 0;
    for(int i = 0; i < graphSize; i++){
        if(removedEdges >= MAX_EDGES){
            result.amount = INT_MAX;
            return result;
        }
        if(parsedEdges[i].n1.colour == parsedEdges[i].n2.colour){
            result.edges[removedEdges] = parsedEdges[i];
            removedEdges++;
        }
    }

    result.amount = removedEdges;
    return result;
}

/**
  * Main function
  * @brief entry point to the program
  * @details A generator produces results using the 3 colouring algorithm.
  * These results are then written to the shared circular buffer. 
  * @param argc
  * @param argv
  * @return EXIT_SUCCESS or EXIT_FAILURE
**/
int main(int argc, char **argv){    
   
    progname = argv[0];

    if(argc < 2){
        usage();
    }
    
    bool isSupervisor = false;
  
    int graphSize = argc-1;
    struct Edge *savedEdges = malloc(sizeof(struct Edge) * graphSize);
    
    //For different colouring of the graph
    srand(time(0));
    for(int i = 1; i <= graphSize; i++){
      
        parse_graph(argv[i],savedEdges, i-1);
    }
    
    open_buffer(isSupervisor);

    while(get_state() == false){
       
        struct Result result = generate_result(savedEdges, graphSize);
       
        savedEdges = NULL;
        savedEdges = malloc(sizeof(struct Edge) * graphSize);
        for(int i = 1; i <= graphSize; i++){
            parse_graph(argv[i],savedEdges, i-1);
        }
       
        write_to_buffer(&result);
    } 

    free(savedEdges);
    clean_exit(isSupervisor);

}