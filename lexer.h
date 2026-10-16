#ifndef __LEXER_H__
#define __LEXER_H__

#include <stddef.h>

#include "list.h"

typedef enum {
  Lexer_TokenType_String,
  Lexer_TokenType_Identifier,
  Lexer_TokenType_Character,
  Lexer_TokenType_Eof
} Lexer_TokenType;

typedef struct {
  const char* name;
  Lexer_TokenType type;
  size_t row, col;
} Lexer_Token;

typedef struct {
  const char* source;
  List* list_token, *list_line;
  size_t row, col, position;
  char chrs;
} Lexer;

Lexer_Token* Lexer_Token_Create(const char* name, Lexer_TokenType type, size_t row, size_t col);
void Lexer_Token_Destroy(Lexer_Token* token);

const char* Lexer_Token_TypeToString(Lexer_TokenType type);

Lexer* Lexer_Create(const char* source);
void Lexer_Destroy(Lexer* lexer);

void Lexer_Start(Lexer* lexer);

#endif
