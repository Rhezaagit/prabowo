#ifndef __UTIL_H__
#define __UTIL_H__

#include <stddef.h>
#include <signal.h>
#include <stdio.h>

char* Util_StrDupe(const char* str);
char* Util_StrNDupe(const char* str, size_t n);
char* Util_StrCreate(size_t length);
char* Util_StrCat(char** str, const char* cat);
char* Util_ReadFile(const char* path);

void Util_InfoMessage(const char* message);

#define TODO fprintf(stderr, "TODO:%s:%u\n", __func__, __LINE__); raise(SIGINT)

#endif

#ifndef UTIL_NO_ERRORS 
#include <stdlib.h>

    #define Util_ErrorMessage(message) \
        fprintf(stderr, "(%s) Error: %s\n", __func__, message) //abort() 

#else
  #define Util_ErrorMessage(message) ;

#endif

#ifdef UTIL_DEFINITION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Util_InfoMessage(const char* message) {
  printf("Info: %s\n", message);
}

char* Util_StrNDupe(const char* str, size_t n) {
  if (!str) {
    Util_ErrorMessage("Argument str Is Zero.");
    return NULL;
  }

  size_t str_length = strlen(str);
  if (str_length < n) {
    Util_ErrorMessage("Argument n Bigger Than The Actual String.");
    return NULL;
  }

  char* duped_str = malloc(n + 1);
  if (!duped_str) {
    Util_ErrorMessage("Failed Duped str.");
    return NULL;
  }

  memcpy(duped_str, str, n);
  duped_str[n] = 0;

  return duped_str;
}

char* Util_StrDupe(const char* str) {
  if (!str) {
    Util_ErrorMessage("Argument str Is Zero.");
    return NULL;
  }

  size_t str_length = strlen(str);
  char* duped_str = malloc(str_length + 1);
  if (!duped_str) {
    Util_ErrorMessage("Failed Duped str.");
    return NULL;
  }

  memcpy((char*)duped_str, str, str_length);
  duped_str[str_length] = 0;

  return duped_str;
}

char* Util_StrCreate(size_t length) {
  if (!length) {
    Util_ErrorMessage("Argument length Is Zero.");
    return NULL;
  }

  char* alloc_str = malloc(length + 1);
  if (!alloc_str) {
    Util_ErrorMessage("Failed Create str.");
    return NULL;
  }

  memset(alloc_str, 0, length + 1);
  return alloc_str;
}

char* Util_ReadFile(const char* path) {
  if (!path) {
    Util_ErrorMessage("Argument path Is Zero.");
    return NULL;
  }

  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }

  fseek(f, 0, SEEK_END); 
  size_t read_length = ftell(f);
  rewind(f);


  char* buf = Util_StrCreate(read_length + 1);
  if (!buf) {
    fclose(f);
  }
  memset(buf, 0, read_length + 1);
  fread(buf, 1, read_length, f);

  fclose(f);
  return buf;
}

char* Util_StrCat(char** str, const char* cat) {
  if (!str) {
    Util_ErrorMessage("Argument str Is Zero.");
    return NULL;
  }

  if (!cat) {
    Util_ErrorMessage("Argument cat Is Zero.");
    return NULL;
  }

  size_t current_length = strlen(*str);
  size_t cat_length = strlen(cat);
  size_t total_length = current_length + cat_length + 1;

  char* new_str = realloc(*str, total_length);
  if (!new_str) {
    Util_ErrorMessage("failed Realloc str.");
    return NULL; 
  }

  *str = new_str;
  strcpy(new_str + current_length, cat);

  return new_str;
}

#endif
