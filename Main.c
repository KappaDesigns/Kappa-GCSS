//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

//DEFINES
#define TRUE 1
#define FALSE 0

//FUNCTION DECLERATION
int verifyPath(char const * path, int writePath);
int fileExists(char const * path, int writePath);
int lastIndexOf(char const * string, char ch);
int getPathWithOutFile(char path[], char const * originalPath, int length);
int putGCSSHeader(FILE * file);
//END FUNCTION DECLERATION

int main(int argc, char const * argv[]) {
  int isValidInputPath = verifyPath(argv[1], FALSE);
  int outputFileExists = verifyPath(argv[2], TRUE);

  FILE * inputIndex = fopen(argv[1], "w+");
  FILE * outputFile = fopen(argv[2], "w+");
  putGCSSHeader(outputFile);

  return 0;
}

// [API]
// Verifies that a file path is valid and exists, otherwise the program either
// exits if the path is not to be written, otherwise the path and file is
// written and the progam continues
int verifyPath(char const * path, int writePath) {
  int exists = fileExists(path, writePath);
  int errorCode = 0;
  if (exists == FALSE) {
    if (writePath == TRUE) {
      int dirStringLength = lastIndexOf(path, '/');
      char pathWithOutFile[dirStringLength + 1];
      getPathWithOutFile(pathWithOutFile, path, dirStringLength);
      mkdir(pathWithOutFile, S_IRWXU);
      FILE * file = fopen(path, "w+");
      fclose(file);
    } else {
      errorCode = -1;
    }
  }
  if (errorCode == -1) {
    printf("Input File \"%s\" Does Not Exist. Exiting...\n", path);
    exit(0);
  }
  return TRUE;
}

int fileExists(char const * path, int writePath) {
  int exists = TRUE;
  FILE * file = fopen(path, "r");
  if (!file) {
    exists = FALSE;
  }
  fclose(file);
  return exists;
}

int lastIndexOf(char const * string, char ch) {
  int index = 0;
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == ch) {
      index = i;
    }
  }
  return index;
}

int getPathWithOutFile(char path[], char const * originalPath, int length) {
  for (int i = 0; i < length; i++) {
    path[i] = originalPath[i];
  }
  path[length] = '\0';
  return 0;
}

int putGCSSHeader(FILE * file) {
  fputs("//KAPPA-GCSS CONSTRAINT BASED CSS \n//CREATED WITH 💖 @ https://github.com/KappaDesigns/Kappa-GCSS", file);
  return 0;
}
