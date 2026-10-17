#define UTIL_DEFINITION
#include "util.h"

#include "eval.h"
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

  Parser* parser = Parser_Create(lexer);
  if (!parser) {
    free((void*)files_buffer);
    Lexer_Destroy(lexer);
    return 4;
  }

  Parser_Start(parser);
  if (parser->success) {
    Eval(parser, (argc > 2) ? NULL : argv[1]);
  }

  Parser_Destroy(parser);
  Lexer_Destroy(lexer);
  free((char*)files_buffer);
  return 0;
}
