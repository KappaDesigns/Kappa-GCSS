//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

//DEFINES
#define TRUE 1
#define FALSE 0
//END DEFINES

//ENUMS
typedef enum {
  TokenType_Pound,
  TokenType_Class_Selector,
  TokenType_Left_Bracket,
  TokenType_Right_Bracket,
  TokenType_Left_Curly,
  TokenType_Right_Curly,
  TokenType_Greater_Than,
  TokenType_Less_Than,
  TokenType_Equals,
  TokenType_Contains_Value,
  TokenType_Contains_Value_In_Space_List,
  TokenType_Value_Starts_With,
  TokenType_Contains_Value_In_Dash_List,
  TokenType_Value_Ends_With,
  TokenType_Equals_Equals,
  TokenType_Number,
  TokenType_Semi_Colon,
  TokenType_Colon,
  TokenType_Comma,
  TokenType_Important,
  TokenType_Identifier,
  TokenType_String,
  TokenType_Plus,
  TokenType_Dash,
  TokenType_Astrix,
  TokenType_Slash,
  TokenType_Global_Selector,
  TokenType_WhiteSpace,
  TokenType_EOF,
  TokenType_Unknown
} TokenType;

typedef enum {
  NodeType_Ruleset,
  NodeType_Selector,
  NodeType_Simple_Selector,
  NodeType_Class,
  NodeType_Id,
  NodeType_Psuedo,
  NodeType_Data_Attribute,
  NodeType_Attribute_Assigner,
  NodeType_Declaration,
  NodeType_Combinator,
  NodeType_String,
  NodeType_Identifier,
  NodeType_Expression
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

typedef struct {
  void** items;
  int length;
} Array;


typedef struct _SyntaxNode {
  NodeType type;
  Token token;
  Array list;
  struct _SyntaxNode* left;
  struct _SyntaxNode* right;
} SyntaxNode;

typedef struct {
  SyntaxNode* root;
} SyntaxTree;

typedef struct _LinkedListNode {
  void* data;
  struct _LinkedListNode* next;
} LinkedListNode;

typedef struct {
  struct _SyntaxNode* property;
  struct _SyntaxNode* expression;
} Declaration;

typedef struct _Selector {
  struct _SyntaxNode* simpleSelector;
  struct _SyntaxNode* combinator;
  struct _Selector* selector;
} Selector;
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
  {TokenType_Class_Selector, "."},
  {TokenType_Contains_Value, "*="},
  {TokenType_Contains_Value_In_Dash_List, "|="},
  {TokenType_Contains_Value_In_Space_List, "~="},
  {TokenType_Value_Ends_With, "$="},
  {TokenType_Value_Starts_With, "^="},
  {TokenType_Important, "!important"},
  {TokenType_Left_Curly, "{"},
  {TokenType_Right_Curly, "}"},
  {TokenType_Comma, ","},
  {TokenType_Less_Than, "<"},
  {TokenType_Greater_Than, ">"},
  {TokenType_Plus, "+"},
  {TokenType_Dash, "-"},
  {TokenType_Astrix, "*"},
  {TokenType_Slash, "/"}
};
Token token_eof = {TokenType_EOF, ""};
//END TOKEN TABLE

//FUNCTION DECLARATION
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
Token nextToken(TokenStream* stream, TokenType type);
void advance(TokenStream* stream);
int runWhiteSpace(TokenStream* stream);
int isCSSSelector(Token token);
int isElementName(Token token);
int isAttributeAssigner(Token token);
LinkedListNode* createLinkedListNode();
Array linkedListToArray(LinkedListNode* front, SyntaxNode* node, int size);
SyntaxNode* createNode(NodeType type);
SyntaxNode* readIdentifier(TokenStream* stream);
SyntaxNode* readString(TokenStream* stream);
SyntaxNode* readSelectorType(TokenStream* stream);
SyntaxNode* readCSSSelector(TokenStream* stream);
SyntaxNode* readClass(TokenStream* stream);
SyntaxNode* readId(TokenStream* stream);
SyntaxNode* readPsuedo(TokenStream* stream);
SyntaxNode* readSimpleSelector(TokenStream* stream);
SyntaxNode* readAttribute(TokenStream* stream);
SyntaxNode* readAttributeAssignment(TokenStream* stream);
SyntaxNode* readRuleset(TokenStream* stream);
SyntaxNode* readCombinator(TokenStream* stream);
SyntaxNode* readAllDeclarationsInRuleSet(TokenStream* stream);
SyntaxNode* readAllSelectorsInRuleSet(TokenStream* stream);
SyntaxNode* readExpression(TokenStream* stream);
SyntaxNode* readOperator(TokenStream* stream);
SyntaxNode* readTerm(TokenStream* stream);
SyntaxNode* readFunction(TokenStream* stream);
Declaration* readDeclaration(TokenStream* stream);
Selector* readSelector(TokenStream* stream);
//END FUNCTION DECLARATION

char* nodeTypeToString(NodeType type) {
  switch (type) {
    case NodeType_Selector:
      return "NodeType_Selector";
    case NodeType_Class:
      return "NodeType_Class";
    case NodeType_Identifier:
      return "NodeType_Identifier";
    case NodeType_Id:
      return "NodeType_Id";
    case NodeType_Psuedo:
      return "NodeType_Psuedo";
    case NodeType_Data_Attribute:
      return "NodeType_Data_Attribute";
    case NodeType_String:
      return "NodeType_String";
    case NodeType_Attribute_Assigner:
      return "NodeType_Attribute_Assigner";
    case NodeType_Ruleset:
      return "NodeType_Ruleset";
    case NodeType_Declaration:
      return "NodeType_Declaration";
    default:
      return "NodeType_Unknown";
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

  root = readRuleset(tokenStream);
  printf("\n");
  print(root);
  return 0;
}

int isCSSSelector(Token token) {
  return (token.type == TokenType_Pound ||
          token.type == TokenType_Colon ||
          token.type == TokenType_Class_Selector ||
          token.type == TokenType_Left_Bracket);
}

int isElementName(Token token) {
  return (
    token.type == TokenType_Global_Selector ||
    token.type == TokenType_Identifier
  );
}

int isAttributeAssigner(Token token) {
  return (
    token.type == TokenType_Equals ||
    token.type == TokenType_Contains_Value ||
    token.type == TokenType_Contains_Value_In_Dash_List ||
    token.type == TokenType_Contains_Value_In_Space_List ||
    token.type == TokenType_Value_Starts_With ||
    token.type == TokenType_Value_Ends_With
  );
}

int isOperator(Token token) {
  return (
    token.type == TokenType_Plus ||
    token.type == TokenType_Dash ||
    token.type == TokenType_Astrix ||
    token.type == TokenType_Slash
  );
}

int isTerm(Token token) {
  return (
    token.type == TokenType_Identifier ||
    token.type == TokenType_String ||
    token.type == TokenType_Number
  );
}

int runWhiteSpace(TokenStream* stream) {
  int wasWhiteSpace = FALSE;
  while(currentToken(stream).type == TokenType_WhiteSpace) {
    advance(stream);
    wasWhiteSpace = TRUE;
  }
  return wasWhiteSpace;
}

LinkedListNode* createLinkedListNode() {
  LinkedListNode* node = (LinkedListNode*)malloc(sizeof(LinkedListNode));
  node -> data = NULL;
  node -> next = NULL;
  return node;
}

Array linkedListToArray(LinkedListNode* front, SyntaxNode* node, int size) {
  int i = 0;
  Array array;
  LinkedListNode* current = front;
  array.length = size;
  array.items = (void**)malloc(sizeof(void*) * size);
  while(current != NULL) {
    array.items[i] = current -> data;
    current = current -> next;
    i++;
  }
  return array;
}

SyntaxNode* readRuleset(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Ruleset);
  SyntaxNode* selectorNode = readAllSelectorsInRuleSet(stream);
  SyntaxNode* declarationNode = readAllDeclarationsInRuleSet(stream);
  node -> left = selectorNode;
  node -> right = declarationNode;
  return node;
}

SyntaxNode* readAllSelectorsInRuleSet(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Selector);
  LinkedListNode* front = NULL;
  LinkedListNode* prev = NULL;
  int listSize = 0;
  runWhiteSpace(stream);
  while(currentToken(stream).type != TokenType_Left_Curly || currentToken(stream).type == TokenType_Comma) {
    listSize++;
    LinkedListNode* current = createLinkedListNode();
    runWhiteSpace(stream);
    Selector* selector = readSelector(stream);
    current -> data = selector;
    if (front == NULL) {
      front = current;
      prev = front;
    } else {
      prev -> next = current;
      prev = current;
    }
  }
  Array array = linkedListToArray(front, node, listSize);
  node -> list = array;
  return node;
}

SyntaxNode* readAllDeclarationsInRuleSet(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Declaration);
  LinkedListNode* front = NULL;
  LinkedListNode* prev = NULL;
  int listSize = 0;
  runWhiteSpace(stream);
  nextToken(stream, TokenType_Left_Curly);
  while(currentToken(stream).type != TokenType_Right_Curly) {
    listSize++;
    LinkedListNode* current = createLinkedListNode();
    runWhiteSpace(stream);
    Declaration* declaration = readDeclaration(stream);
    current -> data = declaration;

    if (front == NULL) {
      front = current;
      prev = front;
    }
    else {
      assert(prev->next == NULL);
      prev->next = current;
      prev = current;
    }
  }
  Array array = linkedListToArray(front, node, listSize);
  node -> list = array;
  return node;
}

Declaration* readDeclaration(TokenStream* stream) {
  Declaration* declaration = (Declaration*)malloc(sizeof(Declaration));
  declaration -> property = readIdentifier(stream);
  runWhiteSpace(stream);
  nextToken(stream, TokenType_Colon);
  runWhiteSpace(stream);
  declaration -> expression = readIdentifier(stream);
  runWhiteSpace(stream);
  return declaration;
}

Selector* readSelector(TokenStream* stream) {
  Selector* selector = (Selector*)malloc(sizeof(Selector));
  selector -> simpleSelector = readSimpleSelector(stream);
  runWhiteSpace(stream);
  if (currentToken(stream).type == TokenType_Plus || currentToken(stream).type == TokenType_Greater_Than) {
    selector -> combinator = readCombinator(stream);
    runWhiteSpace(stream);
    if (isCSSSelector(currentToken(stream)) || isElementName(currentToken(stream))) {
      selector -> selector = readSelector(stream);
    } else {
      exit(1);
    }
  } else if (isCSSSelector(currentToken(stream)) || isElementName(currentToken(stream))) {
    selector -> selector = readSelector(stream);
  }
  return selector;
}

SyntaxNode* readSimpleSelector(TokenStream* stream) {
  SyntaxNode* node = NULL;
  runWhiteSpace(stream);
  if (isElementName(currentToken(stream))) {
    node = readIdentifier(stream);
  }
  runWhiteSpace(stream);
  while (isCSSSelector(currentToken(stream))) {
    SyntaxNode* parent = readCSSSelector(stream);
    parent -> left = node;
    node = parent;
    int wasWhiteSpace = runWhiteSpace(stream);
    if (wasWhiteSpace && currentToken(stream).type == TokenType_Colon) {
      break;
    }
  }
  return node;
}

SyntaxNode* readCSSSelector(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Simple_Selector);
  node -> right = readSelectorType(stream);
  return node;
}

SyntaxNode* readSelectorType(TokenStream* stream) {
  Token token = currentToken(stream);
  if (token.type == TokenType_Class_Selector) {
    return readClass(stream);
  }
  if (token.type == TokenType_Pound) {
    return readId(stream);
  }
  if (token.type == TokenType_Colon) {
    return readPsuedo(stream);
  }
  if (token.type == TokenType_Left_Bracket) {
    return readAttribute(stream);
  }
  return NULL;
}


//TODO
//1. readExpression
//2. read semi colons in readAllDeclarationsInRuleSet
//3. readFunction
//4. readHexColor

SyntaxNode* readExpression(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Expression);
  Token token = currentToken(stream);
  return node;
}

SyntaxNode* readTerm(TokenStream* stream) {
  return NULL;
}

SyntaxNode* readOperator(TokenStream* stream) {
  return NULL;
}

SyntaxNode* readFunction(TokenStream* stream) {
  return NULL;
}

SyntaxNode* readCombinator(TokenStream* stream) {
  SyntaxNode* node = createNode(NodeType_Combinator);
  Token token = currentToken(stream);
  advance(stream);
  node -> token = token;
  return node;
}

SyntaxNode* readClass(TokenStream* stream) {
  nextToken(stream, TokenType_Class_Selector);
  Token token = nextToken(stream, TokenType_Identifier);
  SyntaxNode* node = createNode(NodeType_Class);
  node->token = token;
  return node;
}

SyntaxNode* readId(TokenStream* stream) {
  nextToken(stream, TokenType_Pound);
  Token token = nextToken(stream, TokenType_Identifier);
  SyntaxNode* node = createNode(NodeType_Id);
  node->token = token;
  return node;
}

SyntaxNode* readPsuedo(TokenStream* stream) {
  nextToken(stream, TokenType_Colon);
  Token token = nextToken(stream, TokenType_Identifier);
  SyntaxNode* node = createNode(NodeType_Psuedo);
  node->token = token;
  return node;
}

SyntaxNode* readAttribute(TokenStream* stream) {
  nextToken(stream, TokenType_Left_Bracket);
  runWhiteSpace(stream);
  SyntaxNode* node = readAttributeAssignment(stream);
  if (node != NULL) {
    runWhiteSpace(stream);
    if (currentToken(stream).type == TokenType_Identifier) {
      nextToken(stream, TokenType_Identifier);
      node -> left = readIdentifier(stream);
    }
    if (currentToken(stream).type == TokenType_String) {
      nextToken(stream, TokenType_String);
      node -> left =  readString(stream);
    }
  }
  nextToken(stream, TokenType_Right_Bracket);
  runWhiteSpace(stream);
  return node;
}

SyntaxNode* readAttributeAssignment(TokenStream* stream) {
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  runWhiteSpace(stream);
  Token token = currentToken(stream);
  advance(stream);
  node -> type = NodeType_Attribute_Assigner;
  if (isAttributeAssigner(token)) {
    node -> token = token;
    return node;
  }
  return NULL;
}

SyntaxNode* readString(TokenStream* stream) {
  Token token = nextToken(stream, TokenType_String);
  SyntaxNode* node = createNode(NodeType_String);
  node -> token = token;
  return node;
}

SyntaxNode* readIdentifier(TokenStream* stream) {
  Token token = nextToken(stream, TokenType_Identifier);
  SyntaxNode* node = createNode(NodeType_Identifier);
  node -> token = token;
  return node;
}

SyntaxNode* createNode(NodeType type) {
  SyntaxNode* node = (SyntaxNode*)malloc(sizeof(SyntaxNode));
  node -> type = type;
  return node;
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

Token nextToken(TokenStream* stream, TokenType type) {
  Token token = currentToken(stream);
  if (token.type != type) {
    printf("[error] expected:%d actual:%d offset:%d \n", type, token.type, stream -> offset);
    exit(0);
  }
  advance(stream);
  return token;
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
