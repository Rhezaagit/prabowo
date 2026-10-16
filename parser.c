#include "parser.h"
#include "parsererror.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

Parser_Node* Parser_ParseBodyMode(Parser* parser);
Parser_Node* Parser_ParseLabelCallArgument(Parser* parser);
Parser_Node* Parser_Parse(Parser* parser, Lexer_Token* brackets);
Parser_Node* Parser_ParseBody(Parser* parser);

Parser_Node* Parser_Node_Create(Parser_NodeValue value, Parser_NodeType type, Parser_Node* left, Parser_Node* right) {
  Parser_Node* node = malloc(sizeof(Parser_Node));
  if (!node) {
    Util_ErrorMessage("Failed Create Node.");
    return NULL;
  }

  switch (type) {
    case Parser_NodeType_Var:
    case Parser_NodeType_VarDecl:
    case Parser_NodeType_Label:
    case Parser_NodeType_LabelDecl:
    case Parser_NodeType_String:
    case Parser_NodeType_LabelCall:
      node->value = Parser_NodeValue_Create(Util_StrDupe(value.vstr), str);
      if (!node->value.vstr) {
        free(node);
        return NULL;
      }
      break;

    case Parser_NodeType_Argument:
      node->value = Parser_NodeValue_Create(List_Create(sizeof(char*)), list);
      if (!node->value.vlist) {
        free(node);
        return NULL;
      }
      break;

    case Parser_NodeType_Body:
      node->value = Parser_NodeValue_Create(List_Create(sizeof(Parser_Node*)), list);
      if (!node->value.vlist) {
        free(node);
        return NULL;
      }
      break;
      
    default:
      node->value = value;
  }

  node->left = left;
  node->right = right;
  node->type = type;

  return node;
}

void Parser_Node_Destroy(Parser_Node* node) {
  if (!node) return;

  if (node->left) Parser_Node_Destroy(node->left);
  if (node->right) Parser_Node_Destroy(node->right);

  switch (node->type) {
    case Parser_NodeType_Var:
    case Parser_NodeType_VarDecl:
    case Parser_NodeType_Label:
    case Parser_NodeType_LabelDecl:
    case Parser_NodeType_String:
    case Parser_NodeType_LabelCall:
      free(node->value.vstr);
      break;

    case Parser_NodeType_Argument:
      for (size_t i = 0; i < node->value.vlist->length; i++) {
        free(List_At(node->value.vlist, i, char*));
      }
      List_Destroy(node->value.vlist);
      break;

    case Parser_NodeType_Body:
      for (size_t i = 0; i < node->value.vlist->length; i++) {
        Parser_Node_Destroy(List_At(node->value.vlist, i, Parser_Node*));
      }

      List_Destroy(node->value.vlist);
      
    default: break;
  }

  free(node);
}

const char* Parser_Node_TypeToString(Parser_NodeType type) {
  switch (type) {
    case Parser_NodeType_Body: return "Body";
    case Parser_NodeType_String: return "String";
    case Parser_NodeType_Var: return "Var";
    case Parser_NodeType_VarDecl: return "VarDeclaration";
    case Parser_NodeType_Label: return "Label";
    case Parser_NodeType_Argument: return "Argument";
    case Parser_NodeType_LabelCall: return "LabelCall";
    case Parser_NodeType_LabelDecl: return "LabelDeclaration";
  }

  return "Uknown";
}

void __Parser_Node_Print(Parser_Node* node, size_t i) {
  if (!node) return;

  char space[i + 1];
  memset(space, ' ', i);
  space[i] = 0; 

  printf("%sN: ", space);

  const char* str_type = Parser_Node_TypeToString(node->type);

  switch (node->type) {
    case Parser_NodeType_String:
    case Parser_NodeType_Var:
    case Parser_NodeType_Label:
    case Parser_NodeType_VarDecl:
    case Parser_NodeType_LabelCall:
    case Parser_NodeType_LabelDecl:
      printf("%s <= %s", node->value.vstr, str_type);
      break;

    case Parser_NodeType_Argument:
      printf("{ <= %s\n%s", str_type, space);
      for (size_t j = 0; j < node->value.vlist->length; j++) {
        printf("  %s", List_At(node->value.vlist, j, char*));
      }
      printf("\n%s}", space);
      break;

    case Parser_NodeType_Body:
      printf("{ <= %s\n", str_type);
      for (size_t j = 0; j < node->value.vlist->length; j++) {
        __Parser_Node_Print(List_At(node->value.vlist, j, Parser_Node*), i + 2);
      }
      printf("%s}", space);
      break;

    default: break;
  }

  putchar('\n');

  if (node->left) __Parser_Node_Print(node->left, i + 2);
  if (node->right) __Parser_Node_Print(node->right, i + 2);
}

void Parser_Node_Print(Parser_Node* node) {
  if (!node) return;
  __Parser_Node_Print(node, 0);
}

Parser* Parser_Create(Lexer* lexer) {
  if (!lexer) {
    Util_ErrorMessage("Argument lexer Is Zero.");
    return NULL;
  }

  Parser* parser = malloc(sizeof(Parser));
  if (!parser) {
    Util_ErrorMessage("Failed Create Parser.");
    return NULL;
  }

  parser->success = true;
  parser->root = NULL;
  parser->current = NULL;
  parser->lexer = lexer;
  parser->position = 0;

  return parser;
}

void Parser_Destroy(Parser* parser) {
  if (!parser) return;

  Parser_Node_Destroy(parser->root);
  free(parser);
}

bool Parser_IsCharacterOf(Parser* parser, char chr) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return false;
  }

  if (parser->current->type != Lexer_TokenType_Character) return false;
  if (parser->current->name[0] != chr) return false;

  return true;
}

bool Parser_IsTypeOf(Parser* parser, Lexer_TokenType type) {
    if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return false;
  }

  if (parser->current->type != type) return false;
  return true;
}

bool Parser_IsNameOf(Parser* parser, const char* name) {
    if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return false;
  }

  if (parser->current->type == Lexer_TokenType_Character) return false;
  if (strcmp(parser->current->name, name) != 0) return false;

  return true;
}

void Parser_Scan(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  parser->current = List_At(parser->lexer->list_token, parser->position, Lexer_Token*);
}

void Parser_Expect(Parser* parser, Lexer_TokenType type, const char* name) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!name) {
    Util_ErrorMessage("Argument name Is Zero.");
    return;
  }

  if (!parser->current->name) {
    parser->current->name = "";
  }

  if (parser->current->type != type) {
    ParserError_Syntax(parser, *parser->current, (Lexer_Token){.type = type, .name = name});
  } else if (strcmp(name, "") != 0 && strcmp(parser->current->name, name) != 0) {
    ParserError_Syntax(parser, *parser->current, (Lexer_Token){.type = type, .name = name});
  }
}

void Parser_Next(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if ((parser->position + 1) >= parser->lexer->list_token->length) {
    return;
  }

  parser->position++;
  Parser_Scan(parser);
}

Parser_Node* Parser_ParseVarDecl(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Identifier, "");
  Parser_Node* var = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str), 
    Parser_NodeType_VarDecl, 
    NULL, NULL
  );

  if (!var) {
    Util_ErrorMessage("Failed Create var Node.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Character, "=");
  Parser_Next(parser);

  if (Parser_IsTypeOf(parser, Lexer_TokenType_Identifier)) {
    Parser_Node* mode = Parser_Node_Create(
      Parser_NodeValue_Create((char*)parser->current->name, str), 
      Parser_NodeType_String, 
      NULL, NULL
    );

    if (!mode) {
      Parser_Node_Destroy(var);
      return NULL;
    }

    var->left = mode;

    Parser_Next(parser);
    var->right = Parser_ParseLabelCallArgument(parser);
    return var;
  } 

  Parser_Expect(parser, Lexer_TokenType_Character, "\"");
  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_String, "");
  Parser_Node* value = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str), 
    Parser_NodeType_String, 
    NULL, NULL
  );

  if (!value) {
    Util_ErrorMessage("Failed Create value Node.");
    Parser_Node_Destroy(var);
    return NULL;
  }

  var->right = value;
  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Character, "\"");

  if (!parser->success) {
    Parser_Node_Destroy(var);
    return NULL;
  }

  return var;
}

bool Parser_CmpArgumentName(size_t size, void* cmp0, void* cmp1) {
  if (strcmp(*(char**)cmp0, (char*)cmp1) == 0) return true;

  return false;
}

Parser_Node* Parser_ParseLabelArgument(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Node* argument = Parser_Node_Create(
    Parser_NodeValue_Create(0, list), 
    Parser_NodeType_Argument, 
    NULL, NULL
  );

  if (!argument) {
    Util_ErrorMessage("Failed Create argument Node.");
    return NULL;
  }

  while (!Parser_IsCharacterOf(parser, '{')) {
    if (Parser_IsTypeOf(parser, Lexer_TokenType_Identifier));
    else Parser_Expect(parser, Lexer_TokenType_Character, "{");

    if (!parser->success) {
      Parser_Node_Destroy(argument);
      return NULL;
    }

    const char* name = Util_StrDupe(parser->current->name);
    if (!name) {
      Util_ErrorMessage("Failed Create Name For Label Argument.");  
      return NULL;
    }

    if (List_FindCustom(argument->value.vlist, Parser_CmpArgumentName, (void*)name) != -1) {
      ParserError_SyntaxString(parser, *parser->current, "Nama Argument Double.");
    }

    List_Insert(argument->value.vlist, (const char**){&name}, 1);
    Parser_Next(parser);
  }

  return argument;
}

Parser_Node* Parser_ParseLabel(Parser* parser, Lexer_Token* brackets) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Node* label = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str), 
    Parser_NodeType_LabelDecl, 
    NULL, NULL
  );

  if (!label) {
    Util_ErrorMessage("Failed Create label Node.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Node* argument = Parser_ParseLabelArgument(parser);

  if (!argument) {
    Parser_Node_Destroy(label);
    return NULL;
  }

  Parser_Node* body = Parser_ParseBody(parser);
  label->right = body;
  label->left = argument;

  if (!parser->success) {
    Parser_Node_Destroy(label);
    return NULL;
  }

  return label;
}

Parser_Node* Parser_ParseVar(Parser* parser) {
  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Identifier, "");
  
  Parser_Node* var = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str), 
    Parser_NodeType_Var, 
    NULL, NULL
  );

  if (!var) {
    Util_ErrorMessage("Failed Create var Node.");
    return NULL;
  }

  return var;
}

Parser_Node* Parser_ParseString(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Next(parser);
  Parser_Expect(parser, Lexer_TokenType_String, "");

  Parser_Node* str = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str), 
    Parser_NodeType_String, 
    NULL, NULL
  );

  if (!str) {
    Util_ErrorMessage("Failed Create str Node.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Character, "\"");
  if (!parser->success) {
    Parser_Node_Destroy(str);
    return NULL;
  }

  return str;
}

Parser_Node* Parser_ExpectLiteral(Parser* parser) {
  if (Parser_IsCharacterOf(parser, '!')) return Parser_ParseVar(parser);
  else if (Parser_IsCharacterOf(parser, '\"')) return Parser_ParseString(parser);
  else if (Parser_IsNameOf(parser, "c")) return Parser_ParseBodyMode(parser);

  else if (Parser_IsNameOf(parser, "f")) return Parser_ParseBodyMode(parser);
  else 
    ParserError_SyntaxString(parser, *parser->current, "Membutuhkan Literal.");

  return NULL;
}

Parser_Node* Parser_ParseLabelCallArgument(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Expect(parser, Lexer_TokenType_Character, "{");
  Lexer_Token* brackets = parser->current;

  Parser_Next(parser);

  Parser_Node* argument = Parser_Node_Create(
    Parser_NodeValue_Create(0, list),
    Parser_NodeType_Body,
    NULL,
    NULL
  );

  if (!argument) {
    Util_ErrorMessage("Failed Create argument Node.");
    return NULL;
  }

  while (!Parser_IsCharacterOf(parser, '}')) {
    Parser_Node* argument_ = Parser_ExpectLiteral(parser);

    if (argument_) {
      List_Insert(argument->value.vlist, (Parser_Node**){&argument_}, 1);
    }

    Parser_Next(parser);
  }

  return argument;
}

Parser_Node* Parser_ParseLabelCall(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Expect(parser, Lexer_TokenType_Identifier, "");

  Parser_Node* call = Parser_Node_Create(
    Parser_NodeValue_Create((char*)parser->current->name, str),
    Parser_NodeType_LabelCall,
    NULL,
    NULL
  );

  if (!call) {
    Util_ErrorMessage("Failed Create call Node.");
    return NULL;
  }

  Parser_Next(parser);

  Parser_Node* argument = Parser_ParseLabelCallArgument(parser);
  if (!argument) {
    Parser_Node_Destroy(call);
    return NULL;
  }

  call->left = argument;

  return call;
}

Parser_Node* Parser_ParseBodyMode(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  char* mode = (char*)parser->current->name;
  Parser_Next(parser);

  Parser_Node* argument = Parser_ParseLabelCallArgument(parser);
  if (!argument) return NULL;

  Parser_Node* argument_mode = Parser_Node_Create(
    Parser_NodeValue_Create(mode, str), 
    Parser_NodeType_String,
    NULL, 
    NULL
  );

  if (!argument_mode) {
    Parser_Node_Destroy(argument);
    return NULL;
  }

  argument->right = argument_mode;

  return argument;
}

Parser_Node* Parser_Parse(Parser* parser, Lexer_Token* brackets) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Scan(parser);

  if (brackets) {
    if (Parser_IsCharacterOf(parser, '$'))
      return Parser_ParseLabelCall(parser);
    else if (Parser_IsCharacterOf(parser, '!')) 
      return Parser_ParseVar(parser);

    else if (Parser_IsCharacterOf(parser, '\"')) 
      return Parser_ParseString(parser);

    else if (Parser_IsNameOf(parser, "c")) 
      return Parser_ParseBodyMode(parser);

    else if (Parser_IsNameOf(parser, "f")) 
      return Parser_ParseBodyMode(parser);

    else if (Parser_IsCharacterOf(parser, '{')) 
      return Parser_ParseLabelCallArgument(parser);

    else {
      ParserError_SyntaxString(parser, *parser->current, "Token Tidak Terduga.");
    }
    
  } else {
    if (Parser_IsCharacterOf(parser, '!')) 
      return Parser_ParseVarDecl(parser);

    else if (Parser_IsTypeOf(parser, Lexer_TokenType_Identifier)) 
      return Parser_ParseLabel(parser, brackets);

    else 
      ParserError_SyntaxString(parser, *parser->current, "Token Tidak Terduga.");
  }

  return NULL;
}

Parser_Node* Parser_ParseBody(Parser* parser) {
  Parser_Expect(parser, Lexer_TokenType_Character, "{");
  if (!parser->success) return NULL;

  Lexer_Token* brackets = parser->current;
  Parser_Next(parser);
 

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  Parser_Node* root = Parser_Node_Create(
    Parser_NodeValue_Create(0, list), 
    Parser_NodeType_Body, 
    NULL, NULL
  );

  if (!root) {
    Util_ErrorMessage("Failed Create Root Node.");
    return NULL;
  }

  Parser_Scan(parser);
  while (!Parser_IsCharacterOf(parser, '}')) {
    if (parser->current->type == Lexer_TokenType_Eof) {
      ParserError_SyntaxString(parser, *brackets, "Membutuhkan Penutup.");
      Parser_Node_Destroy(root);
      return NULL;
    }

    Parser_Node* new_root = Parser_Parse(parser, brackets);

    if (new_root) {
      List_Insert(root->value.vlist, (Parser_Node**){&new_root}, 1);
    }

    Parser_Next(parser);
  }

  return root;
}

void Parser_Start(Parser* parser) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  Parser_Node* root = Parser_Node_Create(
    Parser_NodeValue_Create(0, list), 
    Parser_NodeType_Body, 
    NULL, NULL
  );

  if (!root) {
    Util_ErrorMessage("Failed Create Root Node.");
    return;
  }

  Parser_Scan(parser);
  while (parser->current->type != Lexer_TokenType_Eof) {
    if (Parser_IsCharacterOf(parser, '}')) {
      ParserError_SyntaxString(parser, *parser->current, "Tidak Ada Pembuka.");
      Parser_Node_Destroy(root);
      return;
    }

    Parser_Node* new_root = Parser_Parse(parser, NULL);

    if (new_root) {
      List_Insert(root->value.vlist, (Parser_Node**){&new_root}, 1);
    }

    Parser_Next(parser);
  }

  parser->root = root;
}
