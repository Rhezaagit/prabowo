#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"
#include "list.h"

#include <stddef.h>

typedef union {
  char* vstr;
  List* vlist;
} Parser_NodeValue;

typedef enum {
  Parser_NodeType_Label,
  Parser_NodeType_LabelDecl,
  Parser_NodeType_LabelCall,
  Parser_NodeType_String,
  Parser_NodeType_Var,
  Parser_NodeType_VarDecl,
  Parser_NodeType_Body,
  Parser_NodeType_Argument,
} Parser_NodeType;

typedef struct Parser_Node {
  struct Parser_Node* left, *right;
  Parser_NodeType type;
  Parser_NodeValue value;
} Parser_Node;

typedef struct {
  Parser_Node* root;
  Lexer* lexer;
  Lexer_Token* current;
  size_t position;
  bool success;
} Parser;

Parser_Node* Parser_Node_Create(Parser_NodeValue value, Parser_NodeType type, Parser_Node* left, Parser_Node* right);
void Parser_Node_Destroy(Parser_Node* node);

const char* Parser_Node_TypeToString(Parser_NodeType type);
void Parser_Node_Print(Parser_Node* node);

#define Parser_NodeValue_Create(value, type) (Parser_NodeValue){.v##type = value} 

Parser* Parser_Create(Lexer* lexer);
void Parser_Destroy(Parser* parser);

void Parser_Start(Parser* parser);

#endif
