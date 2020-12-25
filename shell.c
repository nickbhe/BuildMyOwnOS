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
int execute_tokens(char **tokens);

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

  return EXIT_SUCCESS;
}

int shsh_exit(char **args) {
  exit(EXIT_SUCCESS);
}

int shsh_help(char **args) {
  printf("\n------------- Shrug Shell -------------\n");
  printf("The shell that makes you go ¯\\_(ツ)_/¯\n\n");

  return EXIT_SUCCESS;
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
void print_prompt() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, PATH_MAX) != NULL) {
    printf("[" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "]", cwd);
    printf(ANSI_COLOR_GREEN "\u00d8 " ANSI_COLOR_RESET);
   } else {
    perror("getcwd error");
   }
}

void read_lines(char **returnLine) {
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
    int lineLength = strlen(line);
    if (strcmp(&line[lineLength - 2], "\\\n") == 0 ||
        strcmp(&line[lineLength - 3], "||\n") == 0 ||
        strcmp(&line[lineLength - 3], "&&\n") == 0) {
          printf("> ");
          char *lineContinuation;
          read_lines(&lineContinuation);

          if (strcmp(&line[lineLength - 2], "\\\n") == 0) {
            strcpy(line + lineLength - 2, lineContinuation);
          } else {
            strcpy(line + lineLength - 1, lineContinuation);
          }
    }

    *returnLine = line;
  }
}

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM "\r\a\n\t "
void split_to_tokens(char *line, char ***returnTokens) {
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
      int tokenInnerIndex = 0;
      while (token[tokenInnerIndex] != '\0') {
        if (token[tokenInnerIndex] == ';' ||
            token[tokenInnerIndex] == '(' ||
            token[tokenInnerIndex] == ')' ||
            (token[tokenInnerIndex] == '|' && token[tokenInnerIndex + 1 ] == '|') ||
            (token[tokenInnerIndex] == '&' && token[tokenInnerIndex + 1 ] == '&')) {
          char *tmpToken = malloc((tokenInnerIndex + 4) * sizeof(char));
          if(tokenInnerIndex != 0) {
            strncpy(tmpToken, token, tokenInnerIndex);
            tmpToken[tokenInnerIndex + 1] = '\0';
            tokens[tokenAmount] = tmpToken;
            tokenAmount++;
          }

          tmpToken[tokenInnerIndex + 2] = token[tokenInnerIndex];
          if (token[tokenInnerIndex] == '|' || token[tokenInnerIndex] == '&') {
            tmpToken[tokenInnerIndex + 3] = token[tokenInnerIndex + 1];
            tmpToken[tokenInnerIndex + 4] = '\0';
            token = token + tokenInnerIndex + 2;
          } else {
            tmpToken[tokenInnerIndex + 3] = '\0';
            token = token + tokenInnerIndex + 1;
          }

          tokens[tokenAmount] = tmpToken + tokenInnerIndex + 2;
          tokenAmount++;
          
          tokenInnerIndex = 0; 
        } else {
          tokenInnerIndex++;
        }     
      }

      if (tokenInnerIndex != 0) {
        tokens[tokenAmount] = token;
        tokenAmount++;
      }
      
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

int execute_command(char **args) {
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
    if (status > 1)
      status = 1;
  }

  return status;
}

int execute_subshell(char **args) {
  pid_t pid = fork();
  int status;

  if (pid == 0) {
   if (execute_tokens(args)) {
     exit(EXIT_SUCCESS);
   } else {
     exit(EXIT_FAILURE);
   }
  } else if (pid < 0) {
    perror("shsh");
  } else {
    pid_t wpid;
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    if (status > 1)
      status = 1;
  }

  return status;
}

int execute_sub_tokens(char **tokens, int startIndex, int finishIndex) {
      int mallocSize = finishIndex - startIndex + 1;
      if (mallocSize == 1)
        mallocSize = 2;
      char **args = malloc(mallocSize * sizeof(char*));
      memcpy(args, tokens + startIndex, (mallocSize - 1) * sizeof(char*));
      args[mallocSize - 1] = NULL;
      return execute_command(args);
}

int execute_tokens(char **tokens) {
  int prevStartIndex = 0;
  int tokenIndex = 0;
  bool conditionalDelimFound = false;
  bool delimCondition = false;
  bool didSubshellStart = false;
  int status;

  while (tokens[tokenIndex] != NULL) {
    if (tokens[tokenIndex][0] == '(') {
      didSubshellStart = true;
      status = execute_subshell(tokens + tokenIndex + 1);
    } else if(tokens[tokenIndex][0] == ')') {
      if (didSubshellStart) {
        didSubshellStart = false;
        prevStartIndex = tokenIndex + 1;
      } else {
        return execute_sub_tokens(tokens, prevStartIndex, tokenIndex);
      }
    } 
    
    if(!didSubshellStart) {
      if (tokens[tokenIndex][0] == ';') {
        if (tokenIndex != prevStartIndex) {
          status = execute_sub_tokens(tokens, prevStartIndex, tokenIndex);
        }
        prevStartIndex = tokenIndex + 1;
      } else if (!strcmp(tokens[tokenIndex], "&&")) {
        conditionalDelimFound = true;
        delimCondition = false;
      } else if (!strcmp(tokens[tokenIndex], "||")) {
        conditionalDelimFound = true;
        delimCondition = true;
      }

      if (conditionalDelimFound) {
        if (tokenIndex != prevStartIndex) {
          status = execute_sub_tokens(tokens, prevStartIndex, tokenIndex);
        }
        if (status != delimCondition)
          break;

        prevStartIndex = tokenIndex + 1;
        conditionalDelimFound = false;
      }    
    }
    
    tokenIndex++;
  }

  if (tokens[tokenIndex] == NULL) {
    if (tokenIndex != prevStartIndex)
      status = execute_sub_tokens(tokens, prevStartIndex, tokenIndex);
  }

  return status;
}

void shsh_loop() {
  int status = EXIT_SUCCESS;

  while (true) { 
    char *line;
    char **tokens;
    
    print_prompt();
    read_lines(&line);
    if (line != NULL) {
      split_to_tokens(line, &tokens);
      status = execute_tokens(tokens);
    }
  }
}

int main(int argc, char **argv) {
  shsh_loop();
       
  return EXIT_SUCCESS;
}