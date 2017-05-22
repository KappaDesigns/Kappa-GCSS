//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>

//DEFINES
#define TRUE 1
#define FALSE 0
#define COMMENT "//KAPPA-KCSS CONSTRAINT BASED CSS \n//CREATED WITH 💖 @ https://github.com/KappaDesigns/Kappa-KCSS"
//END DEFINES

//ENUMS
typedef enum {
  TokenType_Pound,
  TokenType_Left_Bracket,
  TokenType_Right_Bracket,
  TokenType_Equals_Equals,
  TokenType_Number,
  TokenType_Semi_Colon,
  TokenType_Colon,
  TokenType_Identifier,
  TokenType_WhiteSpace,
  TokenType_EOF,
  TokenType_Unknown
} TokenType;
//END ENUMS

//STRUCTS
typedef struct {
  TokenType type;
  char* chars;
} Token;

typedef struct {
  char* chars;
  int offset;
  int length;
} TokenStream;

typedef struct {
  Token token;
  struct SyntaxNode* left;
  struct SyntaxNode* right;
} SyntaxNode;

typedef struct {
  SyntaxNode root;
} SyntaxTree;
//END STRUCTS

//TOKEN TABLE
Token tokens[] = {
  {TokenType_Pound, "#"},
  {TokenType_Left_Bracket, "["},
  {TokenType_Right_Bracket, "]"},
  {TokenType_Semi_Colon, ";"},
  {TokenType_Equals_Equals, "=="},
  {TokenType_Colon, ":"},
};
Token token_eof = {TokenType_EOF, ""};
//END TOKEN TABLE

//FUNCTION DECLERATION
int verifyPath(char const* path, int writePath);
int fileExists(char const* path, int writePath);
int lastIndexOf(char const* string, char ch);
int checkFileEnding(char const* path);
int handleVerificationErrors(int errorCode, char const* path);
int haveFilesChanged(char const* path);
int printWatchingFiles();
int printDetectedChanges(char const* path);
int checkForCompilationErrors(char const* path);
TokenStream* readFile(FILE* file);
Token readToken(TokenStream* stream);
Token createToken(char* chars, int length, TokenType tokenType);
Token readWhiteSpaceToken(TokenStream* stream);
Token readIndentifierToken(TokenStream* stream);
Token readNumberToken(TokenStream* stream);
Token readStringToken(TokenStream* stream, char quoteType);
//END FUNCTION DECLERATION

int main(int argc, char const* argv[]) {
  const char* inputPath = argv[1];
  int isValidInputPath = verifyPath(inputPath, FALSE);
  FILE * file = fopen(inputPath, "r");
  TokenStream* tokenStream = readFile(file);

  printWatchingFiles();

  while(TRUE) {
    // int hasFileUpdated = haveFilesChanged(inputPath);
    // if (shouldCompile == FALSE) {
      // printDetectedChanges(inputPath);
    // }
    Token token = readToken(tokenStream);
    if (token.type == TokenType_EOF) {
      break;
    }
    if (token.type == TokenType_Unknown) {
      printf("Unknown: %s\n", token.chars);
    } else {
      printf("Token: %s\n", token.chars);
    }
  }

  return 0;
}

TokenStream* readFile(FILE* file) {
  TokenStream* stream = (TokenStream*)malloc(sizeof(TokenStream));
  int size;
  int ch;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char* chars = (char*)malloc(sizeof(char) * size);
  for (int i = 0; i < size; i++) {
    ch = fgetc(file);
    chars[i] = (char)ch;
  }
  stream -> chars = chars;
  stream -> offset = 0;
  stream -> length = size;
  return stream;
}

//[Tokens] readToken
Token readToken(TokenStream* stream) {
  if (stream -> offset == stream -> length) {
    return token_eof;
  }

  char ch = stream->chars[stream->offset];
  if (isspace(ch)) {
    return readWhiteSpaceToken(stream);
  }
  if (ch == '\"' || ch == '\'') {
    return readStringToken(stream, ch);
  }
  if (isalpha(ch)) {
    return readIndentifierToken(stream);
  }
  if (isdigit(ch)) {
    return readNumberToken(stream);
  }
  for (int i = 0; i < sizeof(tokens) / sizeof(Token); i++) {
    if (strncmp(tokens[i].chars, &stream -> chars[stream -> offset], strlen(tokens[i].chars)) == 0) {
      stream -> offset += strlen(tokens[i].chars);
      return tokens[i];
    }
  }
  Token token = createToken(&stream -> chars[stream -> offset], 1, TokenType_Unknown);
  stream->offset ++;
  return token;
}

Token readWhiteSpaceToken(TokenStream* stream) {
  Token token = createToken(&stream -> chars[stream -> offset], 1, TokenType_WhiteSpace);
  stream -> offset ++;
  return token;
}

Token readIndentifierToken(TokenStream* stream) {
  int offset = stream->offset;
  while (isalnum(stream->chars[offset]) || stream->chars[offset] == '-') {
    offset++;
  }
  Token token = createToken(&stream->chars[stream->offset], offset - stream -> offset, TokenType_Identifier);
  stream -> offset = offset;
  return token;
}

Token readNumberToken(TokenStream* stream) {
  int offset = stream->offset;
  while(isdigit(stream->chars[offset])) {
    offset++;
  }
  Token token = createToken(&stream->chars[stream->offset], offset - stream -> offset, TokenType_Number);
  stream -> offset = offset;
  return token;
}

Token readStringToken(TokenStream* stream, char quoteType) {
  int offset = stream->offset + 1;
  while(stream->chars[offset] != quoteType) {
    offset++;
  }
  offset++;
  Token token = createToken(&stream->chars[stream->offset], offset - stream -> offset, TokenType_Number);
  stream -> offset = offset;
  return token;
}

//[Token]createToken
Token createToken(char* chars, int length, TokenType tokenType) {
  Token token;
  char* str = (char*)malloc(sizeof(char) * (length + 1));
  for (int i = 0; i < length; i++) {
    str[i] = chars[i];
  }
  str[length] = '\0';
  token.chars = str;
  token.type = tokenType;
  return token;
}

// [API]verifyPath
// Verifies that a file path is valid and exists, otherwise the program either
// exits if the path is not to be written, otherwise the path and file is
// written and the progam continues
int verifyPath(char const* path, int writePath) {
  int exists = fileExists(path, writePath);
  int properEnding = checkFileEnding(path);
  int errorCode = 0;
  if (properEnding == FALSE) {
    errorCode = -2;
  }
  if (exists == FALSE) {
    errorCode = -1;
  }
  handleVerificationErrors(errorCode, path);
  return TRUE;
}

// [API]fileExists
// Checks if file exists, returns either TRUE or FALSE
int fileExists(char const* path, int writePath) {
  int exists = TRUE;
  FILE* file = fopen(path, "r");
  if (!file) {
    exists = FALSE;
  }
  fclose(file);
  return exists;
}

// [API]checkFileEnding
// Checks if file mathches it proper ending, .KCSS if it is an input file and
// .js if output file. Returns TRUE if correct ending, returns FALSE otherwise
int checkFileEnding(char const* path) {
  int length = strlen(path);
  int dotIndex = length - 4;
  if (path[dotIndex - 1] != '.') {
    return FALSE;
  }
  for (int i = dotIndex; i < length; i++) {
    if (path[i] != 'k' && path[i] != 'c' && path[i] != 's') {
      return FALSE;
    }
  }
  return TRUE;
}

// [API]lastIndexOf
// Gets the last index of a passed character in a given char const *. returns
// The last index of passed char as an int.
int lastIndexOf(char const* string, char ch) {
  int index = 0;
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == ch) {
      index = i;
    }
  }
  return index;
}

// [API]handleVerificationErrors
//Handles all errors from the verification process
int handleVerificationErrors(int errorCode, char const* path) {
  if (errorCode == -1) {
    printf("Input File \"%s\" Does Not Exist. Exiting...\n", path);
    exit(0);
  }
  if (errorCode == -2) {
    printf("Input KCSS File At \"%s\" Does Not End In .KCSS. Exiting...\n", path);
    exit(0);
  }
  return 0;
}

int printWatchingFiles() {
  printf("%s\n", ">>> KCSS Is Watching For Changes. Press Ctrl+C To Stop.");
  return 0;
}

// [API]haveFilesChanged
// Returns whether or not there have been changes to the KCSS files. IF there
// have TRUE is returned otherwise FALSE is returned.
int haveFilesChanged(char const* path) {
  return FALSE;
}

// [API]printDetectedChanges
int printDetectedChanges() {
  return 0;
}
