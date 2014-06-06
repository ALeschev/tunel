#ifndef CALLER_TREE_H
#define CALLER_TREE_H

#include "ivr_def.h"

void tree_add_caller (int callref, caller_t *caller);
void tree_del_caller  (int callref);
void tree_clean_all   (void);

int  tree_find_caller (int callref);

int  tree_get_caller_cnt (void);

int  tree_get_state   (int callref);
int  tree_get_ivr_idx (int callref);
int  tree_set_state   (int callref, int state);

int  tree_set_digit       (int callref, uint8_t dig);
int  tree_set_gitits_need (int callref, int num);
char *tree_get_digits     (int callref);
char *tree_get_caller_sym (int callref);

void tree_print (void);

void tree_clean (void);

#endif // CALLER_TREE_H