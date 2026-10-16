#include "eval.h"
#include "list.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

char* Eval_EvaluateString(Parser* parser, Parser_Node* body);

typedef struct {
  char* name;
  char* value;
} var_info;

List* info_var_stack = NULL;
char* run_label = "pidato";

Parser_Node* Eval_Evaluate(Parser* parser, Parser_Node* node);

void Eval_AddStackPointer(var_info info) {
  info.value = Util_StrDupe(info.value);
  if (!info.value) return;

  List_Insert(info_var_stack, (var_info[]){info}, 1);
}

void Eval_RemoveStackPointer(size_t count) {
  if (count <= 0 || info_var_stack->length <= 0) return;

  size_t final_length = (info_var_stack->length - 1) - count;

  for (size_t i = final_length;  i < info_var_stack->length; i++) {
    free(List_At(info_var_stack, i, var_info).value);
  }

  List_RemoveBack(info_var_stack, count);
}

bool Eval_CmpSearchVarStack(size_t length, var_info* info, const char* var_name) {
  if (strcmp(info->name, var_name) == 0) return true;

  return false;
}

var_info Eval_SearchVarStack(Parser* parser, const char* var_name) {
  if (!var_name) {
    Util_ErrorMessage("Argument var_name Is Zero.");
    return (var_info){};
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return (var_info){};
  }

  int find_res = List_FindCustom(
    info_var_stack, (void*)Eval_CmpSearchVarStack, (void*)var_name
  );

  if (find_res == -1) return (var_info){};

  return List_At(info_var_stack, find_res, var_info);
}

bool Eval_CmpSearchLabelRoot(size_t length, Parser_Node** node, const char* label_name) {
  if ((*node)->type == Parser_NodeType_LabelDecl) {
    if (strcmp((*node)->value.vstr, label_name) == 0) return true;
  }

  return false;
}

Parser_Node* Eval_SearchLabelRoot(Parser* parser, const char* label_name) {
  if (!label_name) {
    Util_ErrorMessage("Argument label_name Is Zero.");
    return NULL;
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL;
  }

  int find_res = List_FindCustom(
    parser->root->value.vlist, (void*)Eval_CmpSearchLabelRoot, (void*)label_name
  );

  if (find_res == -1) return NULL;

  Parser_Node* node = List_At(
      parser->root->value.vlist, 
      find_res, 
      Parser_Node*
  );

  if (node) return node;

  return NULL;
}

void Eval_EvaluateStringBodyModeC(Parser* parser, Parser_Node* body, char** command) {
  if (!body) {
    Util_ErrorMessage("Argument body Is Zero.");
    return;
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!command) {
    Util_ErrorMessage("Argument command Is Zero.");
    return;
  }

  char* body_command = Eval_EvaluateString(parser, body);
  if (body_command) Util_StrCat(command, body_command);
}

void Eval_EvaluateStringBodyModeF(Parser* parser, Parser_Node* body, char** command) {
  if (!body) {
    Util_ErrorMessage("Argument body Is Zero.");
    return;
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!command) {
    Util_ErrorMessage("Argument command Is Zero.");
    return;
  }

  List* body_list = body->value.vlist;

  for (size_t i = 0; i < body_list->length; i++) {

    Parser_Node* search_node = List_At(body_list, i, Parser_Node*);

    char* search_str = search_node->value.vstr;
    if (search_node->type == Parser_NodeType_Var) {
      var_info info = Eval_SearchVarStack(parser, search_node->value.vstr);

      if (info.value) search_str = info.value;
    } 

    glob_t result = {};
    if (glob(search_str, 0, 0, &result) == 0) {
      for (size_t j = 0; j < result.gl_pathc; j++) {
        if (!Util_StrCat(command, result.gl_pathv[j])) return;
        if (!Util_StrCat(command, " ")) return; 
      }
    }

    globfree(&result);
  }

}

char* Eval_EvaluateString(Parser* parser, Parser_Node* body) {
  if (!body) {
    Util_ErrorMessage("Argument body Is Zero.");
    return NULL; 
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return NULL; 
  }

  char* command = Util_StrCreate(1);
  if (!command) return NULL;

  Parser_Node* str = List_At(body->value.vlist, 0, Parser_Node*);
  if (str->type == Parser_NodeType_Var) {
    var_info info_var = Eval_SearchVarStack(parser, str->value.vstr);

    Util_StrCat(&command, info_var.value);
  } else {
    Util_StrCat(&command, str->value.vstr);
  }

  size_t length = body->value.vlist->length;
  bool should_stop = false;

  for (size_t i = 1; i < length; i++) {
    if (should_stop) break;

    Parser_Node* node = List_At(body->value.vlist, i, Parser_Node*);
    switch (node->type) {
        case Parser_NodeType_Var: {
          var_info info_var = Eval_SearchVarStack(parser, node->value.vstr);
          if (info_var.value) {
              Util_StrCat(&command, info_var.value);
          }
          break;
        }

        case Parser_NodeType_String: {
          Util_StrCat(&command, node->value.vstr);
          break;
        }

        case Parser_NodeType_Body: {

          if (node->right) {
            if (node->right->value.vstr[0] == 'c') {
              Eval_EvaluateStringBodyModeC(parser, node, &command);

            } else if (node->right->value.vstr[0] == 'f') {
              Eval_EvaluateStringBodyModeF(parser, node, &command);
            }
            break;
          }
        }

        default:
          should_stop = true; 
          break;
    }
  }

  return command;
}

void Eval_EvaluateBodyModeC(Parser* parser, Parser_Node* body) {
  if (!body) {
    Util_ErrorMessage("Argument body Is Zero.");
    return;
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  char* command = Eval_EvaluateString(parser, body);
  if (command) {
    if (system(command)) {
      free(command);
      exit(-1);
    }

    free(command);
  }
}

void Eval_EvaluateBody(Parser* parser, Parser_Node* body) {
  if (!body) {
    Util_ErrorMessage("Argument body Is Zero.");
    return;
  }

  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (body->value.vlist->length) {
    if (body->right) {
      char* mode = body->right->value.vstr;

      if (mode[0] == 'c') {
        Eval_EvaluateBodyModeC(parser, body);
        return;
      }
    } 
  } 

  for (size_t i = 0; i < body->value.vlist->length; i++) {
    Eval_Evaluate(parser, List_At(body->value.vlist, i, Parser_Node*));
  }
}

void Eval_EvaluateLabel(Parser* parser, Parser_Node* label, Parser_Node* call) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!label) {
    Util_ErrorMessage("Argument label Is Zero.");
    return;
  }

  if (label->type != Parser_NodeType_LabelDecl) {
    Util_ErrorMessage("Argument label Is Not Label Is Zero.");
    return;
  }

  size_t stack_count = 0;
  if (call) {
    List* call_left_list = call->left->value.vlist;
    List* label_left_list = label->left->value.vlist;

    if (call_left_list->length == label_left_list->length) {
      for (size_t i = 0; i < label_left_list->length; i++) {
        char* name = List_At(label_left_list, i, char*);
        Parser_Node* value = List_At(call_left_list, i, Parser_Node*);

        switch (value->type) {
          case Parser_NodeType_Var: {}
            var_info info_var = Eval_SearchVarStack(parser, value->value.vstr);

            if (info_var.value) {
              info_var.name = name;
              Eval_AddStackPointer(info_var);
              stack_count++;
            }
            break;

          case Parser_NodeType_String: {}
            Eval_AddStackPointer((var_info){.name = name, .value = value->value.vstr});
            stack_count++;
            break;

          default: break;
        }
      }
    }
  }
  
  Eval_EvaluateBody(parser, label->right);

  if (stack_count) Eval_RemoveStackPointer(stack_count);
}

bool Eval_EvaluateVarDeclModeF(Parser* parser, Parser_Node* decl, char** value) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return false;
  }

  if (!decl) {
    Util_ErrorMessage("Argument decl Is Zero.");
    return false;
  }

  if (!value) {
    Util_ErrorMessage("Argument value Is Zero.");
    return false;
  }

  char* files = Util_StrCreate(1);
  if (!files) return false;

  List* body_list = decl->right->value.vlist;

  for (size_t i = 0; i < body_list->length; i++) {

    Parser_Node* search_node = List_At(body_list, i, Parser_Node*);

    char* search_str = search_node->value.vstr;
    if (search_node->type == Parser_NodeType_Var) {
      var_info info = Eval_SearchVarStack(parser, search_node->value.vstr);

      if (info.value) search_str = info.value;
    } 

    glob_t result = {};
    if (glob(search_str, 0, 0, &result) == 0) {
      for (size_t j = 0; j < result.gl_pathc; j++) {
        if (!Util_StrCat(&files, result.gl_pathv[j])) return false;
        if (!Util_StrCat(&files, " ")) return false;
      }
    }

    globfree(&result);
  }

  *value = files;
  return true;
}

bool Eval_EvaluateVarDeclModeC(Parser* parser, Parser_Node* decl, char** value) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return false;
  }

  if (!decl) {
    Util_ErrorMessage("Argument decl Is Zero.");
    return false;
  }

  if (!value) {
    Util_ErrorMessage("Argument value Is Zero.");
    return false;
  }

  char* str = Util_StrCreate(1);
  if (!str) return false;

  List* body_list = decl->right->value.vlist;

  for (size_t i = 0; i < body_list->length; i++) {

    Parser_Node* search_node = List_At(body_list, i, Parser_Node*);

    char* result_str = search_node->value.vstr;
    if (search_node->type == Parser_NodeType_Var) {
      var_info info = Eval_SearchVarStack(parser, search_node->value.vstr);

      if (info.value) result_str = info.value;
    } 

    if (!Util_StrCat(&str, result_str)) return false;
  }

  *value = str;
  return true;
}

void Eval_EvaluateVarDecl(Parser* parser, Parser_Node* decl) {
  if (!parser) {
    Util_ErrorMessage("Argument parser Is Zero.");
    return;
  }

  if (!decl) {
    Util_ErrorMessage("Argument decl Is Zero.");
    return;
  }

  char* value = decl->right->value.vstr;
  char* mode = NULL;

  if (decl->left) {
    mode = decl->left->value.vstr;

    if (mode[0] == 'f') Eval_EvaluateVarDeclModeF(parser, decl, &value);
    else if (mode[0] == 'c') Eval_EvaluateVarDeclModeC(parser, decl, &value);
  }

  Eval_AddStackPointer((var_info){
    .name = decl->value.vstr, 
    .value = value
    });

  if (mode) {
    if (mode[0] == 'f' || mode[0] == 'c') free(value);
  }
}

Parser_Node* Eval_Evaluate(Parser* parser, Parser_Node* node) {
  if (!node) return NULL;

  switch (node->type) {
    case Parser_NodeType_LabelDecl: {} 
      if (strcmp(node->value.vstr, run_label) == 0) {
        Eval_EvaluateLabel(parser, node, NULL);
      }
      break;

    case Parser_NodeType_Body:  
      Eval_EvaluateBody(parser, node);
      break;

    case Parser_NodeType_String:
      if (system(node->value.vstr)) exit(-1);
      break;

    case Parser_NodeType_Var: {}
      var_info info_var = Eval_SearchVarStack(parser, node->value.vstr);

      if (info_var.value) system(info_var.value);
      break;

    case Parser_NodeType_LabelCall: {}
      Parser_Node* label = Eval_SearchLabelRoot(parser, node->value.vstr);
      if (label) {
        Eval_EvaluateLabel(parser, label, node);
      }
      break;

    case Parser_NodeType_VarDecl: {}
      Eval_EvaluateVarDecl(parser, node);
      break;

    default: break;
  }

  return NULL;
}

bool Eval(Parser* parser, char* run) {
  if (run) run_label = run;

  info_var_stack = List_Create(sizeof(var_info));
  if (!info_var_stack) {
    Util_ErrorMessage("Failed Create Var Stack.");
    return false;
  }

  Eval_Evaluate(parser, parser->root);

//  for (size_t i = 0; i < info_var_stack->length; i++) {
//    var_info info = List_At(info_var_stack, i, var_info);
//    printf("%s: %s\n", info.name, info.value);
//  }

  Eval_RemoveStackPointer(info_var_stack->length - 1);
  List_Destroy(info_var_stack);
  return true;
}
