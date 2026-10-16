#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#include <stdbool.h>

#define List_At(list, index, type) (*(type*)_List_At(list, index))
  
typedef struct {
  size_t size, length;
  void* data; 
} List;

List* List_Create(size_t size);
void List_Destroy(List* list);

int List_FindCustom(List* list, bool (*p_cmp)(size_t size, void* cmp0, void* cmp1), void* cmp);
bool List_Insert(List* list, void* data, size_t insert_length);
bool List_RemoveBack(List* list, size_t remove_length);
char* _List_At(List* list, size_t index);

#endif
