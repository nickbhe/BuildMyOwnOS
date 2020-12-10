#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

bool shsh_cd(char **args);
bool shsh_exit(char **args);
bool shsh_help(char **args);

static const char *BUILTIN_NAMES[] = {
  "cd",
  "exit",
  "help"
};

static const bool (*BUILTIN_FUNCTIONS[]) (char**) = {
  &shsh_cd,
  &shsh_exit,
  &shsh_help
};

bool shsh_cd(char **args) {
  chdir(args[1]);

  return true;
}

bool shsh_exit(char **args) {
  return false;
}

bool shsh_help(char **args) {
  printf("------------- Shrug Shell -------------\n");
  printf("The shell that makes you go ¯\\_(ツ)_/¯\n");

  return true;
}

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

bool shsh_execute(char **args) {
  int length = sizeof(BUILTIN_NAMES) / sizeof(BUILTIN_NAMES[0]);

  for (int builtinsIndex = 0; builtinsIndex < length; builtinsIndex++) {
    if (!strcmp(args[0], BUILTIN_NAMES[builtinsIndex])) {
      return (*BUILTIN_FUNCTIONS[builtinsIndex])(args);
    }
  }
  
  pid_t pid = fork();
      
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("shsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("shsh");
  } else {
    pid_t wpid;
    int status;
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return true;
}

void shsh_loop() {
  int status = true;

  while (status) {
    char *line;
    char **tokens;

    printf("\u2660 -> ");
    shsh_read_line(&line);
    shsh_split_to_tokens(line, &tokens);
    status = shsh_execute(tokens);
  }
}

int main(int argc, char **argv) {
  shsh_loop();
       
  return EXIT_SUCCESS;
}
