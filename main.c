//#define UTIL_NO_ERRORS
#include "eval.h"
#define UTIL_DEFINITION
#include "util.h"

#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
  const char* files_buffer = Util_ReadFile("./.prabowo");
  if (!files_buffer) {
    Util_InfoMessage("Tidak Ada '.prabowo'");
    Util_InfoMessage("Bikin Filenya Dahulu.\n");

    return 1;
  }

  Lexer* lexer = Lexer_Create(files_buffer);
  if (!files_buffer) {
    free((char*)files_buffer);
    return 2;
  }

  Lexer_Start(lexer);

//  for (size_t i = 0; i < lexer->list_token->length; i++) {
//    Lexer_Token* token = List_At(lexer->list_token, i, Lexer_Token*);
//
//    printf("(%s) <= %s %lu:%lu\n", token->name, Lexer_Token_TypeToString(token->type), token->row, token->col);
//  }

  Parser* parser = Parser_Create(lexer);
  if (!parser) {
    free((void*)files_buffer);
    Lexer_Destroy(lexer);
    return 4;
  }

  Parser_Start(parser);
  if (parser->success) {
    //Parser_Node_Print(parser->root);
    Eval(parser, (argc > 2) ? NULL : argv[1]);
  }

  Parser_Destroy(parser);
  Lexer_Destroy(lexer);
  free((char*)files_buffer);
  return 0;
}
