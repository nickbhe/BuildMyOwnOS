#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

enum builtins {
  CD
};

static const char *BUILTINS[] = {
  "cd"
};

void shsh_read_line(char **returnLine) {
  char *line = NULL;
  size_t capacity = 0;

  if (getline(&line, &capacity, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS); 
    } else {
      perror("shsh");
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
    fprintf(stderr, "%s", "shsh: Allocation Failure\n");
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

bool execute_builtin(char **tokens) {
  int length = sizeof(BUILTINS) / sizeof(BUILTINS[0]);

  for (int builtinsIndex = 0; builtinsIndex < length; builtinsIndex++) {
    if (!strcmp(tokens[0], BUILTINS[builtinsIndex])) {
      switch (builtinsIndex) {
        case CD:
          chdir(tokens[1]);
          break;
      }
      return true;
    }
  }

  return false;
}

void shsh_loop() {
  while (1) {
    char *line;
    char **tokens;

    printf("\u2660 -> ");
    shsh_read_line(&line);
    shsh_split_to_tokens(line, &tokens);
    
    if (!execute_builtin(tokens)) {
      int pid = fork();
      
      if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
          perror("shsh");
        }
      } else if (pid < 0) {
        perror("shsh");
      } else {
        wait(NULL);
      }
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
       
  return EXIT_SUCCESS;
}
