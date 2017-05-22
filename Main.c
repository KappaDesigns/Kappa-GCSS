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
  TokenType_Class_Selector,
  TokenType_Left_Bracket,
  TokenType_Right_Bracket,
  TokenType_Equals,
  TokenType_Equals_Equals,
  TokenType_Number,
  TokenType_Semi_Colon,
  TokenType_Colon,
  TokenType_Identifier,
  TokenType_String,
  TokenType_Global_Selector,
  TokenType_WhiteSpace,
  TokenType_EOF,
  TokenType_Unknown
} TokenType;

typedef enum {
  NodeType_Identifier,
  NodeType_Global_Selector,
  NodeType_Selector,
  NodeType_Class,
  NodeType_Id,
  NodeType_Psuedo,
  NodeType_Equals_Operator,
  NodeType_String
} NodeType;
//END ENUMS


//STRUCTS
typedef struct {
  TokenType type;
  char* chars;
} Token;

typedef struct {
  Token currentToken;
  char* chars;
  int offset;
  int length;
} TokenStream;

typedef struct _SyntaxNode {
  NodeType type;
  Token token;
  struct _SyntaxNode* left;
  struct _SyntaxNode* right;
} SyntaxNode;

typedef struct {
  SyntaxNode* root;
} SyntaxTree;
//END STRUCTS

//TOKEN TABLE
Token tokens[] = {
  {TokenType_Pound, "#"},
  {TokenType_Left_Bracket, "["},
  {TokenType_Right_Bracket, "]"},
  {TokenType_Semi_Colon, ";"},
  {TokenType_Equals_Equals, "=="},
  {TokenType_Equals, "="},
  {TokenType_Colon, ":"},
  {TokenType_Global_Selector, "*"},
  {TokenType_Class_Selector, "."}
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
Token currentToken(TokenStream* stream);
void advance(TokenStream* stream);
int isSelector(Token token);
void runWhiteSpace(TokenStream* stream);
SyntaxNode* readIdentifier(TokenStream* stream, SyntaxNode* root);
SyntaxNode* readString(TokenStream* stream, SyntaxNode* root);
SyntaxNode* readSelectorType(TokenStream* stream, SyntaxNode* parent);
SyntaxNode* readCSSSelector(TokenStream* stream, SyntaxNode* child);
SyntaxNode* readClass(TokenStream* stream, SyntaxNode* node);
SyntaxNode* readId(TokenStream* stream, SyntaxNode* node);
SyntaxNode* readPsuedo(TokenStream* stream, SyntaxNode* node);
SyntaxNode* readSelector(TokenStream* stream, SyntaxNode* root);
SyntaxNode* readAttribute(TokenStream* stream, SyntaxNode* node);
//END FUNCTION DECLERATION

char* nodeTypeToString(NodeType type) {
  switch (type) {
    case NodeType_Selector:
      return "NodeType_Selector";
    case NodeType_Class:
      return "NodeType_Class";
    case NodeType_Identifier:
      return "NodeType_Identifier";
    case NodeType_Global_Selector:
      return "NodeType_Global_Selector";
    case NodeType_Id:
      return "NodeType_Id";
    case NodeType_Psuedo:
      return "NodeType_Psuedo";
    case NodeType_Equals_Operator:
      return "NodeType_Equals_Operator";
    case NodeType_String:
      return "NodeType_String";
    default:
      return "";
  }
}

void print(SyntaxNode* node) {
  if (node != NULL) {
    printf("%s\n", nodeTypeToString(node -> type));
    print(node -> left);
    print(node -> right);
  }
}

int main(int argc, char const* argv[]) {
  const char* inputPath = argv[1];
  int isValidInputPath = verifyPath(inputPath, FALSE);
  SyntaxNode* root = (SyntaxNode*)malloc(sizeof(SyntaxNode));

  FILE * file = fopen(inputPath, "r");
  TokenStream* tokenStream = readFile(file);

  printWatchingFiles();

  root = readSelector(tokenStream, root);
  print(root);
  return 0;
}

int isSelector(Token token) {
  return (token.type == TokenType_Pound || token.type == TokenType_Colon ||
          token.type == TokenType_Class_Selector || token.type == TokenType_Left_Bracket);
}

void runWhiteSpace(TokenStream* stream) {
  while(currentToken(stream).type == TokenType_WhiteSpace) {
    advance(stream);
  }
}

SyntaxNode* readSelector(TokenStream* stream, SyntaxNode* root) {
  int wasWhiteSpace = FALSE;
  SyntaxNode* node = readIdentifier(stream, root);
  if (currentToken(stream).type == TokenType_WhiteSpace) {
    advance(stream);
  }
  while (isSelector(currentToken(stream))) {
    SyntaxNode* parent = readCSSSelector(stream, node);
    node = parent;
    while (currentToken(stream).type == TokenType_WhiteSpace) {
      wasWhiteSpace = TRUE;
      advance(stream);
    }
    if (wasWhiteSpace && currentToken(stream).type == TokenType_Colon) {
      break;
    }
  }
  return node;
}

SyntaxNode* readCSSSelector(TokenStream* stream, SyntaxNode* child) {
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  node -> left = child;
  node -> type = NodeType_Selector;
  node = readSelectorType(stream, node);
  advance(stream);
  return node;
}

SyntaxNode* readSelectorType(TokenStream* stream, SyntaxNode* parent) {
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  Token token = currentToken(stream);
  if (token.type == TokenType_Class_Selector) {
    advance(stream);
    readClass(stream, node);
  }
  if (token.type == TokenType_Pound) {
    advance(stream);
    readId(stream, node);
  }
  if (token.type == TokenType_Colon) {
    advance(stream);
    readPsuedo(stream, node);
  }
  if (token.type == TokenType_Left_Bracket) {
    advance(stream);
    readAttribute(stream, node);
  }
  parent -> right = node;
  return parent;
}

SyntaxNode* readClass(TokenStream* stream, SyntaxNode* node) {
  Token token = currentToken(stream);
  if (token.type == TokenType_Identifier) {
    node -> type = NodeType_Class;
  }
  return node;
}

SyntaxNode* readId(TokenStream* stream, SyntaxNode* node) {
  Token token = currentToken(stream);
  if (token.type == TokenType_Identifier) {
    node -> type = NodeType_Id;
  }
  return node;
}

SyntaxNode* readPsuedo(TokenStream* stream, SyntaxNode* node) {
  Token token = currentToken(stream);
  if (token.type == TokenType_Identifier) {
    node -> type = NodeType_Psuedo;
  }
  return node;
}

SyntaxNode* readAttribute(TokenStream* stream, SyntaxNode* node) {
  runWhiteSpace(stream);
  node -> left = readIdentifier(stream, node);

  runWhiteSpace(stream);
  if (currentToken(stream).type == TokenType_Equals) {
    node -> type = NodeType_Equals_Operator;
    advance(stream);
  }

  runWhiteSpace(stream);
  if (currentToken(stream).type == TokenType_Identifier) {
    node-> right = readIdentifier(stream, node);
  }
  if (currentToken(stream).type == TokenType_String) {
    node -> right = readString(stream, node);
  }
  return node;
}

SyntaxNode* readString(TokenStream* stream, SyntaxNode* root) {
  Token token = currentToken(stream);
  advance(stream);
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  if (token.type == TokenType_String) {
    node -> token = token;
    node -> type = NodeType_String;
    return node;
  }
  return NULL;
}

SyntaxNode* readIdentifier(TokenStream* stream, SyntaxNode* root) {
  Token token = currentToken(stream);
  advance(stream);
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  if (token.type == TokenType_Identifier || token.type == TokenType_Global_Selector) {
    node -> token = token;
    node -> type = NodeType_Identifier;
    return node;
  }
  return NULL;
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
  advance(stream);
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
  Token token = createToken(&stream->chars[stream->offset], offset - stream -> offset, TokenType_String);
  stream -> offset = offset;
  return token;
}

Token currentToken(TokenStream* stream) {
  return stream -> currentToken;
}

void advance(TokenStream* stream) {
  Token token = readToken(stream);
  stream -> currentToken = token;
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
