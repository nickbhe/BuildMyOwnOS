#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM " \n\a\r\t"
void split_to_tokens(char *line) {
  int buffer = TOKEN_BUFSIZE;
  char **tokens = malloc(buffer * sizeof(char*));

  if (!tokens) {
    printf("shsh: Allocation Failure");
    exit(EXIT_FAILURE);
  }

  char lineCopy[strlen(line)];
  strcpy(lineCopy, line);
  char *saveptr;
  char *token = strtok_r(lineCopy, TOKEN_DELIM, &saveptr);
  
  if(token == NULL) {
    *returnTokens = NULL;
  } else {
    int tokenAmount = 0;

    while(token != NULL) {
      tokens[tokenAmount] = token;
      tokenAmount++;
      
      if(tokenAmount >= TOKEN_BUFSIZE) {
        buffer += TOKEN_BUFSIZE;
        tokens = realloc(tokens, buffer * sizeof(char*));
        if(!tokens) {
          printf("shsh: Reallocation Failure");
          exit(EXIT_FAILURE);
        }
      }

      token = strtok_r(NULL, TOKEN_DELIM, &saveptr);
    }

    tokens[tokenAmount] = NULL;
    *returnTokens = tokens;
  }
}

void shsh_loop() {
  while(1) {
    char *line = NULL;
    size_t capacity = 0;
    char **tokens;

    printf("\u2660 -> ");
    getline(&line, &capacity, stdin);
    split_to_tokens(line, &tokens);
    
    if(tokens == NULL) {
      printf("No Input was provided");
    } else {
      printf("%s %s %s", tokens[0], tokens[1], tokens[2]);
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
  
  return EXIT_SUCCESS;
}
