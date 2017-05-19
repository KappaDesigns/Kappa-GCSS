//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

//DEFINES
#define TRUE 1
#define FALSE 0
#define COMMENT "//KAPPA-GCSS CONSTRAINT BASED CSS \n//CREATED WITH 💖 @ https://github.com/KappaDesigns/Kappa-GCSS"

//FUNCTION DECLERATION
int verifyPath(char const * path, int writePath);
int fileExists(char const * path, int writePath);
int lastIndexOf(char const * string, char ch);
int getPathWithOutFile(char path[], char const * originalPath, int length);
int putGCSSHeader(FILE * file);
int checkFileEnding(int isOutputFile, char const * path);
int handleVerificationErrors(int errorCode, char const * path);
int haveFilesChanged(char const * path);
int complieGCSS();
int printWatchingFiles();
//END FUNCTION DECLERATION

int main(int argc, char const * argv[]) {
  const char * path1 = argv[1];
  const char * path2 = argv[2];
  int isValidInputPath = verifyPath(path1, FALSE);
  int outputFileExists = verifyPath(path2, TRUE);

  FILE * inputIndex = fopen(path1, "w+");
  FILE * outputFile = fopen(path2, "w+");
  putGCSSHeader(outputFile);
  printWatchingFiles();
  while(TRUE) {
    int shouldCompile = haveFilesChanged(path1);
    if (shouldCompile) {
      // printDetectedChanges();
      complieGCSS();
    }
  }

  return 0;
}

// [API]
// Verifies that a file path is valid and exists, otherwise the program either
// exits if the path is not to be written, otherwise the path and file is
// written and the progam continues
int verifyPath(char const * path, int writePath) {
  int exists = fileExists(path, writePath);
  int properEnding = checkFileEnding(writePath, path);
  int errorCode = 0;
  if (properEnding == FALSE) {
    if (writePath == TRUE) {
      errorCode = -3;
    } else {
      errorCode = -2;
    }
  }
  if (exists == FALSE) {
    if (writePath == TRUE && errorCode != -3) {
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
  handleVerificationErrors(errorCode, path);
  return TRUE;
}

// Checks if file exists, returns either TRUE or FALSE
int fileExists(char const * path, int writePath) {
  int exists = TRUE;
  FILE * file = fopen(path, "r");
  if (!file) {
    exists = FALSE;
  }
  fclose(file);
  return exists;
}

// Checks if file mathches it proper ending, .gcss if it is an input file and
// .js if output file. Returns TRUE if correct ending, returns FALSE otherwise
int checkFileEnding(int isOutputFile, char const * path) {
  int length = strlen(path);
  int prefixLength = isOutputFile == TRUE ? 2 : 4;
  int dotIndex = length - prefixLength;
  if (path[dotIndex - 1] != '.') {
    return FALSE;
  }
  for (int i = dotIndex; i < length; i++) {
    if (prefixLength == 2 && path[i] != 'j' && path[i] != 's') {
      return FALSE;
    }
    if (prefixLength == 4 && path[i] != 'g' && path[i] != 'c' && path[i] != 's') {
      return FALSE;
    }
  }
  return TRUE;
}

// Gets the last index of a passed character in a given char const *. returns
// The last index of passed char as an int.
int lastIndexOf(char const * string, char ch) {
  int index = 0;
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == ch) {
      index = i;
    }
  }
  return index;
}

// Gets the Directory Path with out the output file.
int getPathWithOutFile(char path[], char const * originalPath, int length) {
  for (int i = 0; i < length; i++) {
    path[i] = originalPath[i];
  }
  path[length] = '\0';
  return 0;
}

// Puts Custom GCSS COMMENT into output JS file
int putGCSSHeader(FILE * file) {
  fputs(COMMENT, file);
  return 0;
}


//Handles all errors from the verification process
int handleVerificationErrors(int errorCode, char const * path) {
  if (errorCode == -1) {
    printf("Input File \"%s\" Does Not Exist. Exiting...\n", path);
    exit(0);
  }
  if (errorCode == -2) {
    printf("Input GCSS File At \"%s\" Does Not End In .gcss. Exiting...\n", path);
    exit(0);
  }
  if (errorCode == -3) {
    printf("Output JS File at \"%s\" Does Not End In .js. Exiting...\n", path);
    exit(0);
  }
  return 0;
}

int printWatchingFiles() {
  printf("%s\n", ">>> GCSS Is Watching For Changes. Press Ctrl+C To Stop.");
  return 0;
}

// Returns whether or not there have been changes to the GCSS files. IF there
// have TRUE is returned otherwise FALSE is returned.
int haveFilesChanged(char const * path) {
  return FALSE;
}

//Complies GCSS to JS
int complieGCSS() {
  return 0;
}
