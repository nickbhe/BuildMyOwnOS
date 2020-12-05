#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void shsh_read_line(char **returnLine) {
  char *line = NULL;
  size_t capacity = 0;

  if (getline(&line, &capacity, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS); 
    } else {
      perror("shsh: Readline Failure");
      exit(EXIT_FAILURE);
    }
  } else {
    *returnLine = line;
  }
}

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM " \n\a\r\t"
void shsh_split_to_tokens(char *line, char ***returnTokens) {
  int buffer = TOKEN_BUFSIZE;
  char **tokens = malloc(buffer * sizeof(char*));

  if (!tokens) {
    printf("shsh: Allocation Failure");
    exit(EXIT_FAILURE);
  }

  char *saveptr;
  char *token = strtok_r(line, TOKEN_DELIM, &saveptr);
  
  if (token == NULL) {
    *returnTokens = NULL;
  } else {
    int tokenAmount = 0;

    while (token != NULL) {
      tokens[tokenAmount] = token;
      tokenAmount++;
      
      if (tokenAmount >= TOKEN_BUFSIZE) {
        buffer += TOKEN_BUFSIZE;
        tokens = realloc(tokens, buffer * sizeof(char*));
        if (!tokens) {
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
  while (1) {
    char *line;
    char **tokens;

    printf("\u2660 -> ");
    shsh_read_line(&line);
    shsh_split_to_tokens(line, &tokens);
    
    if (tokens == NULL) {
      printf("No Input was provided");
    } else {
      execvp(tokens[0], tokens);
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
  
  return EXIT_SUCCESS;
}
