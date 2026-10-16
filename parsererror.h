#ifndef __PARSERERROR_H__
#define __PARSERERROR_H__

#include "parser.h"

void ParserError_Syntax(Parser* parser, Lexer_Token wrong, Lexer_Token right);
void ParserError_SyntaxString(Parser* parser, Lexer_Token wrong, const char* error);

#endif
