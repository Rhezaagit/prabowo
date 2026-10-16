#include "parsererror.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

void ParserError_Syntax(Parser* parser, Lexer_Token wrong, Lexer_Token right) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!parser->success) return;

  char space[wrong.col + 1];
  memset(space, ' ', wrong.col);
  space[wrong.col] = 0;;

  printf("%lu:%lu %s %s(%s)\n|\n-> %s\n  %s^\n", 
    wrong.row, wrong.col, 
    "Tapi Membutuhkan", 
    Lexer_Token_TypeToString(right.type), right.name,
    List_At(parser->lexer->list_line, wrong.row - 1, char*),
    space
  );

  parser->success = false;
}


void ParserError_SyntaxString(Parser* parser, Lexer_Token wrong, const char* error) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!parser->success) return;

  char space[wrong.col + 1];
  memset(space, ' ', wrong.col);
  space[wrong.col] = 0;;

  printf("%lu:%lu %s\n|\n-> %s\n  %s^\n", 
    wrong.row, wrong.col, 
    error, 
    List_At(parser->lexer->list_line, wrong.row - 1, char*),
    space
  );

  parser->success = false;
}
