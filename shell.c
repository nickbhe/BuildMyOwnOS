#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void split_to_tokens(char *line) {
  char *tmp = line;
  int tokenAmount = 1;
  char previousCharacter = NULL;
  
  while(*tmp) {
    if(*tmp == ' ' && previousCharacter != ' ') {
      tokenAmount++;
    }
    
    printf("%s", *tmp);
    previousCharacter = *tmp;
    tmp++;
  }
 

  char **tokens;
  int tokenIndex = 0;
  
}

void shsh_loop() {
  char *line = NULL;
  int capacity = 0;
  int tokenAmount;
  char previousCharacter = NULL;
  
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
