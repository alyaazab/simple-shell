#include <iostream>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>

int splitLine(char *argv[], char *line);
void createNewProcess(char *argv[], bool isBackgroundProcess);
void signalHandler(int signal);
bool isBackgroundProcess(char *argv[], int i);
char *getCurrentWorkingDirectory();
bool isChangeDirectory(char *argv[]);
void displayPrompt(char *hostname);
void setTextColorToGreen();
void setTextColorToBlue();
void setTextColorToDefault();


// holds value of the initial directory where the log file will be created
char *initialDirectory;
// holds value of the current working directory to be printed at each prompt
char *currentDirectory;

int main()
{
  char line[200], hostname[HOST_NAME_MAX];
  char *argv[100];
  int numberOfTokens;

  // get the value of the hostname
  gethostname(hostname, HOST_NAME_MAX);

  // set initial and current working directories
  initialDirectory = getCurrentWorkingDirectory();
  currentDirectory = getCurrentWorkingDirectory();

  while (1)
  {
    // set signalHandler to handle the SIGCHLD signal
    signal(SIGCHLD, signalHandler);

    displayPrompt(hostname);

    // read in a line from the user and remove the trailing newline
    fgets(line, 200, stdin);
    line[strlen(line) - 1] = '\0';

    // if the line is empty, display the prompt again
    if (strlen(line) == 0)
      continue;

    // split the line and retrieve the number of tokens
    numberOfTokens = splitLine(argv, line);

    // if command is not 'cd', create a new process
    if (!isChangeDirectory(argv))
      createNewProcess(argv, isBackgroundProcess(argv, numberOfTokens));
  }

  // deallocate memory previously allocated to currentDirectory and initialDirectory by malloc
  free(currentDirectory);
  free(initialDirectory);

  return 0;
}

// displays prompt in the format username@hostname:~currentDirectory$
void displayPrompt(char *hostname)
{
  // set text color of username@hostname to green
  setTextColorToGreen();
  printf("%s@%s", getpwuid(getuid())->pw_name, hostname);
  setTextColorToDefault();
  printf(":");

  // set text color of directory name to blue
  setTextColorToBlue();
  printf("~%s", currentDirectory);
  setTextColorToDefault();
  printf("$ ");
}

// this function returns a string containing the current working directory, or NULL if an error occurs
char *getCurrentWorkingDirectory()
{
  char currentWorkingDirectory[PATH_MAX];
  char *directory = (char *)malloc(PATH_MAX);

  if (getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) != NULL)
  {
    strcpy(directory, currentWorkingDirectory);
    return directory;
  }

  return NULL;
}

// this function returns true if a command ends with an & (background process)
bool isBackgroundProcess(char *argv[], int i)
{
  if (*argv[i - 1] == '&')
  {
    argv[i - 1] = NULL;
    return true;
  }

  return false;
}

// this function splits a string and returns the number of tokens found
int splitLine(char *argv[], char *line)
{
  char delimiter[] = " ";

  // strtok splits 'line' by space
  char *ptr = strtok(line, delimiter);
  int i = 0;

  // keep retrieving tokens until strtok returns null, indicating that the string has ended
  // store the tokens in argv
  // set the last element of argv to NULL to indicate the end of the array
  while (ptr != NULL)
  {
    argv[i++] = ptr;
    ptr = strtok(NULL, delimiter);
  }
  argv[i] = NULL;

  // if entered command is 'exit', exit from program
  if (strcmp(argv[0], "exit") == 0 && argv[1] == NULL)
    exit(0);

  return i;
}

// returns true if current command is 'cd' and changes the current directory accordingly
// returns false otherwise
bool isChangeDirectory(char *argv[])
{
  if (strcmp(argv[0], "cd") != 0)
    return false;

  if (chdir(argv[1]) == 0)
  {
    // deallocate the previously allocated memory to currentDirectory
    free(currentDirectory);
    currentDirectory = getCurrentWorkingDirectory();
  }
  else
  {
    printf("bash: %s: %s: No such file or directory\n", argv[0], argv[1]);
  }
  return true;
}

// creates a new child process using fork() and executes
void createNewProcess(char *argv[], bool isBackgroundProcess)
{
  pid_t processId = fork();

  if (processId < 0)
  {
    printf("\nFailed to create a new process.\n");
  }
  // if we are in child process, execute the command
  else if (processId == 0)
  {
    // if execvp fails, print the error message
    if (execvp(argv[0], argv) < 0)
    {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  }
  else
  {
    // if command is a background process, do not wait for child to terminate
    // otherwise, wait for termination of child
    if (!isBackgroundProcess)
      wait(NULL);
  }
}

// handles SIGCHLD signal
// prints into log_file whenever a child terminates
void signalHandler(int signal)
{
  // set the filePath as a file named log_file.txt and place it in initial directory
  char filePath[PATH_MAX];
  sprintf(filePath, "%s/log_file.txt", initialDirectory);

  // open the file in append mode
  FILE *filePtr = fopen(filePath, "a");

  // get the current date and time
  time_t t = time(NULL);
  struct tm currentTime = *localtime(&t);

  // print timestamp and a message to the file
  fprintf(filePtr, "LOG: %d-%d-%d %d:%d:%d ----- Child has terminated\n", currentTime.tm_year + 1900,
          currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec);

  fclose(filePtr);
}

void setTextColorToGreen() {
  printf("\033[1;32m");
}

void setTextColorToBlue() {
  printf("\033[1;34m");
}

void setTextColorToDefault() {
  printf("\033[0m");
}