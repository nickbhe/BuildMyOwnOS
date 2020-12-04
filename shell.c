#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char** split_to_tokens(char *line) {
  int lineLength = strlen(line);
  char lineCopy[lineLength];
  strcpy(lineCopy, line);
  char *saveptr;
  int tokenAmount = 0;

  char* tmp = strtok_r(lineCopy, " ", &saveptr);
  while ((tmp != NULL && strcmp(tmp, "\n") != 0)) {
    tokenAmount++;
    tmp = strtok_r(NULL, " ", &saveptr);
  }

  if(tokenAmount == 0) {
    return NULL;
  } else {
    char *tokens[tokenAmount];
    strcpy(lineCopy, line);

    tokens[0] = strtok_r(lineCopy, " ", &saveptr);
    for(int tokenCount = 1; tokenCount < tokenAmount; tokenCount++) {
      tokens[tokenCount] = strtok_r(NULL, " ", &saveptr);
    }
    
    return tokens;
  }
}

void shsh_loop() {
  char *line = NULL;
  int capacity = 0;
  int tokenAmount;
  
  while(1) {
    printf("\u2660 -> ");
    getline(&line, &capacity, stdin);
    
    split_to_tokens(line);
  }
}

int main(int argc, char **argv) {
  shsh_loop();
  
  return EXIT_SUCCESS;
}
