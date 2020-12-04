#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void split_to_tokens(char *line, char*** returnTokens) {
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
    returnTokens = NULL;
  } else {
    char *tokens[tokenAmount];
    strcpy(lineCopy, line);

    tokens[0] = strtok_r(lineCopy, " ", &saveptr);
    for(int tokenCount = 1; tokenCount < tokenAmount; tokenCount++) {
      tokens[tokenCount] = strtok_r(NULL, " ", &saveptr);
    }

    printf("hi");
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
      execvp(tokens[0], tokens);
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
  
  return EXIT_SUCCESS;
}
