#include <stdlib.h>
#include <stdio.h>

void ush_loop() {
  char *line = NULL;
  int capacity = 0;
  while(1) {
    printf("\u2660 -> ");
    getline(&line, &capacity, stdin);
    printf(line);
  }
}

int main(int argc, char **argv) {
  ush_loop();
  
  return EXIT_SUCCESS;
}