#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

int shsh_cd(char **args);
int shsh_exit(char **args);
int shsh_help(char **args);
int shsh_exec(char ** args);

static const char *BUILTIN_NAMES[] = {
  "cd",
  "exit",
  "help",
  "exec"
};

static const int (*BUILTIN_FUNCTIONS[]) (char**) = {
  &shsh_cd,
  &shsh_exit,
  &shsh_help,
  &shsh_exec
};

int shsh_cd(char **args) {
  chdir(args[1]);

  return 1;
}

#define EXIT_CODE -1
int shsh_exit(char **args) {
  return EXIT_CODE;
}

int shsh_help(char **args) {
  printf("\n------------- Shrug Shell -------------\n");
  printf("The shell that makes you go ¯\\_(ツ)_/¯\n\n");

  return 1;
}

int shsh_exec(char **args) {
  if (execvp(args[1], args + 1) == -1) {
    perror("shsh");
    exit(EXIT_FAILURE);
  } else {
    exit(EXIT_SUCCESS);
  }
}

#define PATH_MAX 1024
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
void shsh_print_prompt() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, PATH_MAX) != NULL) {
    printf("[" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "]", cwd);
    printf(ANSI_COLOR_GREEN "\u00d8 " ANSI_COLOR_RESET);
   } else {
    perror("getcwd error");
   }
}

void shsh_read_line(char **returnLine) {
  char *line;
  size_t capacity = 0;

  if (getline(&line, &capacity, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS); 
    } else {
      perror("shsh");
      exit(EXIT_FAILURE);
    }
  } else if (strlen(line) == 1) {
      *returnLine = NULL;
  } else {
    *returnLine = line;
  }
}

void shsh_continue_read_lines(char** returnLine) {
  char *line = *returnLine;
  int lineLength = strlen(line);
  while(strcmp(&line[lineLength - 2], "\\\n") == 0 ||
        strcmp(&line[lineLength - 3], "||\n") == 0 ||
        strcmp(&line[lineLength - 3], "&&\n") == 0) {
    printf("> ");
    char *lineContinuation;
    shsh_read_line(&lineContinuation);
    
    if (strcmp(&line[lineLength - 2], "\\\n") == 0) {
      strcpy(line + lineLength - 2, lineContinuation);
    } else {
      strcpy(line + lineLength - 1, lineContinuation);
    }
    
    lineLength = strlen(line);
  }
}

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM "\r\a\n\t "
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

int shsh_execute_command(char **args) {
  const int length = sizeof(BUILTIN_NAMES) / sizeof(BUILTIN_NAMES[0]);

  for (int builtinsIndex = 0; builtinsIndex < length; builtinsIndex++) {
    if (!strcmp(args[0], BUILTIN_NAMES[builtinsIndex])) {
      return (*BUILTIN_FUNCTIONS[builtinsIndex])(args);
    }
  }

  pid_t pid = fork();
  int status;

  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror(args[0]);
      exit(EXIT_FAILURE);
    } else {
      exit(EXIT_SUCCESS);
    }
  } else if (pid < 0) {
    perror("shsh");
  } else {
    pid_t wpid;
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    status = !status;
  }

  return status;
}

int shsh_execute(char **tokens) {
  int prevStartIndex = 0;
  int tokenIndex = -1;
  bool conditionalDelimFound = false;
  bool delimFound = false;
  bool delimCondition = false;
  int status;

  while (tokens[tokenIndex] != NULL) {
    tokenIndex++;

    if (tokens[tokenIndex] == NULL) {
      delimFound = true;
      if (tokenIndex == prevStartIndex)
        break;
    } else if(!strcmp(tokens[tokenIndex], ";")) {
      delimFound = true;
    } else if (!strcmp(tokens[tokenIndex], "&&")) {
      conditionalDelimFound = true;
      delimCondition = false;
    } else if (!strcmp(tokens[tokenIndex], "||")) {
      conditionalDelimFound = true;
      delimCondition = true;
    }

    if (delimFound || conditionalDelimFound) {
      int mallocSize = tokenIndex - prevStartIndex + 1;
      if (mallocSize == 1)
        mallocSize = 2;
      char **args = malloc(mallocSize * sizeof(char*));
      memcpy(args, tokens + prevStartIndex, (mallocSize - 1) * sizeof(char*));
      args[mallocSize - 1] = NULL;
      status = shsh_execute_command(args);
      
      if (conditionalDelimFound) {
       if (status == delimCondition) {
         break;
       } 
      } 

      prevStartIndex = tokenIndex + 1;
      delimFound = false;
      conditionalDelimFound = false;
    }
  }

  return status;
}

void shsh_loop() {
  int status = 1;

  while (status != EXIT_CODE) {
    char *line;
    char **tokens;
    
    shsh_print_prompt();
    shsh_read_line(&line);
    if (line != NULL) {
      shsh_continue_read_lines(&line);
      shsh_split_to_tokens(line, &tokens);
      status = shsh_execute(tokens);
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
       
  return EXIT_SUCCESS;
}
