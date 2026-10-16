#include "list.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

List* List_Create(size_t size) {
  if (size == 0) {
    Util_ErrorMessage("Argument size Cannot Be Zero.");
  }

  List* list = malloc(sizeof(List));
  if (!list) {
    Util_ErrorMessage("Failed Create List.");
    return NULL;
  }

  list->data = NULL;
  list->length = 0;
  list->size = size;

  return list;
}

void List_Destroy(List* list) {
  if (!list) return;

  free(list->data);
  free(list);
}

bool List_Insert(List* list, void* data, size_t insert_length) {
  if (!list) {
    Util_ErrorMessage("Argument list Cannot Be Zero.");
    return false;
  }

  if (!data) {
    Util_ErrorMessage("Argument data Cannot Be Zero.");
    return false;
  }

  if (!insert_length) {
    Util_ErrorMessage("Argument insert_length Cannot Be Zero.");
    return false;
  }

  list->length += insert_length;
  void* new_data = realloc(list->data, list->length * list->size);
  if (!new_data) {
    Util_ErrorMessage("Failed ReAlloc List.");
    list->length -= insert_length;
    return false;
  }

  memcpy(new_data + ((list->length - insert_length) * list->size), data, insert_length * list->size);
  list->data = new_data;

  return true;
}

char* _List_At(List* list, size_t index) {
  return list->data + (list->size * index);
}

int List_FindCustom(List* list, bool (*p_cmp)(size_t size, void* cmp0, void* cmp1), void* cmp) {
  if (!list) {
    Util_ErrorMessage("Argument list Is Zero.");
    return -1;
  }

  if (!p_cmp) { 
    Util_ErrorMessage("Argument p_cmp Is Zero.");
    return -1;
  }

  size_t i = list->length;
  while (i > 0) {
    i--;
    void* element_ptr = (void*)&List_At(list, i, char);

    if (p_cmp(list->size, element_ptr, cmp)) return i;
  }

  return -1;  
}

bool List_RemoveBack(List* list, size_t remove_length) {
  if (!list) {
    Util_ErrorMessage("Argument list Is Zero.");
    return false;
  }

  if (!remove_length) {
    Util_ErrorMessage("Argument remove_length Is Zero.");
    return false;
  }

  if (list->length == 0) return true;

  if (list->length == 1) {
    free(list->data);
    list->data = NULL;
    list->length -= remove_length;
    return true;
  }

  list->length -= remove_length;
  void* new_data = realloc(list->data, list->length * list->size);
  if (!new_data) {
    Util_ErrorMessage("Failed ReAlloc List.");
    list->length += remove_length;
    return false;
  }

  list->data = new_data;
  return true;
}

