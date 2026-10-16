#include "lexer.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer_Token* Lexer_Token_Create(const char* name, Lexer_TokenType type, size_t row, size_t col) {
  if (!name) {
    Util_ErrorMessage("Argument name Is Zero.");
    return NULL;
  }

  Lexer_Token* token = malloc(sizeof(Lexer_Token));
  if (!token) {
    Util_ErrorMessage("Failed Create Token.");
    return NULL;
  }

  token->name = Util_StrDupe(name);
  if (!token->name) {
    free(token);
    return NULL;
  }

  token->type = type;
  token->row = row;
  token->col = col;

  return token;
}

void Lexer_Token_Destroy(Lexer_Token* token) {
  if (!token) return;

  free((void*)token->name);
  free(token);
}

const char* Lexer_Token_TypeToString(Lexer_TokenType type) {
  switch (type) {
    case Lexer_TokenType_Character: return "Character";
    case Lexer_TokenType_Identifier: return "Identifier";
    case Lexer_TokenType_String: return "String";
    case Lexer_TokenType_Eof: return "Eof";
  }

  return "Uknown";
}

Lexer* Lexer_Create(const char* source) {
  if (!source) {
    Util_ErrorMessage("Argument source Is Zero.");
    return NULL;
  }

  Lexer* lexer = malloc(sizeof(Lexer));
  if (!lexer) {
    Util_ErrorMessage("Failed Create Lexer.");
    return NULL;
  }

  lexer->source = Util_StrDupe(source);
  if (!source) {
    free(lexer);
    return NULL;
  }

  lexer->row = 1;
  lexer->col = 1;
  lexer->position = 0;
  lexer->chrs = 0;


  lexer->list_line = List_Create(sizeof(const char*));
  if (!lexer->list_line) {
    free((void*)lexer->source);
    free(lexer);
    return NULL;
  }

  lexer->list_token = List_Create(sizeof(Lexer_Token*));
  if (!lexer->list_token) {
    List_Destroy(lexer->list_line);
    free((void*)lexer->source);
    free(lexer);
    return NULL;
  }

  return lexer;
}

void Lexer_Destroy(Lexer* lexer) {
  if (!lexer) return;

  for (size_t i = 0; i < lexer->list_line->length; i++) {
    free(List_At(lexer->list_line, i, char*));
  }
  List_Destroy(lexer->list_line);

  for (size_t i = 0; i < lexer->list_token->length; i++) {
    Lexer_Token_Destroy(List_At(lexer->list_token, i, Lexer_Token*));
  }
  List_Destroy(lexer->list_token);

  free((void*)lexer->source);
  free(lexer);
}

size_t Lexer_StringCountToDelimiter(const char* str, const char* delimiter) {
  if (!str) {
    Util_ErrorMessage("Argument str Is Zero.");
    return 0;
  }

  if (!delimiter) {
    Util_ErrorMessage("Argument delimiter Is Zero.");
    return 0;
  }

  return strcspn(str, delimiter);
}

void Lexer_CurrentChar(Lexer* lexer) {
  lexer->chrs = lexer->source[lexer->position];
}

void Lexer_Skip(Lexer* lexer, size_t count) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  if (((lexer->position + 1) + count) > (strlen(lexer->source) + 1)) {
    //Util_ErrorMessage("Trying Skip OverFlow.");
    return;
  }

  lexer->position += count;
  Lexer_CurrentChar(lexer);
  lexer->col += count;
}

const char* Lexer_Current(Lexer* lexer) {
  return lexer->position + lexer->source;
}

void Lexer_AddToken(Lexer* lexer, size_t length, Lexer_TokenType type) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  char name[length + 1];
  memset(name, 0, length + 1);
  memcpy(name, Lexer_Current(lexer), length);

  Lexer_Token* token = Lexer_Token_Create(name, type, lexer->row, lexer->col);
  if (!token) return;

  if (!List_Insert(lexer->list_token, (Lexer_Token*[]){token}, 1)) {
    Lexer_Token_Destroy(token);
  }
}

void Lexer_ParseIdentifier(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  size_t count = Lexer_StringCountToDelimiter(Lexer_Current(lexer), " \";{}!=\t\r\n\0");
  Lexer_AddToken(lexer, count, Lexer_TokenType_Identifier);

  Lexer_Skip(lexer, count);
}

void Lexer_ParseCharacter(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  Lexer_AddToken(lexer, 1, Lexer_TokenType_Character);
  Lexer_Skip(lexer, 1);
}

void Lexer_ParseString(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  Lexer_ParseCharacter(lexer);

  size_t count = Lexer_StringCountToDelimiter(Lexer_Current(lexer), "\t\r\"\0");
  Lexer_AddToken(lexer, count, Lexer_TokenType_String);

  Lexer_Skip(lexer, count);

  Lexer_ParseCharacter(lexer);
}

void Lexer_ParseComment(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  Lexer_Skip(lexer, Lexer_StringCountToDelimiter(Lexer_Current(lexer), "\n\0"));
}

void Lexer_Start(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return;
  }

  const char* line_pos = lexer->source;
  while (*line_pos != 0) {
    size_t count = Lexer_StringCountToDelimiter(line_pos, "\n\0");

    char* line = Util_StrNDupe(line_pos, count);
    List_Insert(lexer->list_line, (char**){&line}, 1);
    line_pos += count + 1;
  }

  char* line = Util_StrDupe("///Di Akhir File.///");
  List_Insert(lexer->list_line, (char**){&line}, 1);

  lexer->chrs = lexer->source[0];
  do {
    switch (lexer->chrs) {
      case '0'...'9': 
      case 'A'...'Z': 
      case 'a'...'z':
        Lexer_ParseIdentifier(lexer);
        break;
      
      case '"':
        Lexer_ParseString(lexer);
        break;

      case '#':
        Lexer_ParseComment(lexer);
        break;

      case '\n':
        Lexer_Skip(lexer, 1);
        lexer->row++;
        lexer->col = 1;
        break;

      case ' ':
        Lexer_Skip(lexer, 1);
        break;

      default: 
        Lexer_AddToken(lexer, 1, Lexer_TokenType_Character);
        Lexer_Skip(lexer, 1);
    }

  } while (lexer->chrs);

  Lexer_AddToken(lexer, 1, Lexer_TokenType_Eof);
}
