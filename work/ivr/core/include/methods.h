#ifndef METHODS_H
#define METHODS_H

#include "ivr_def.h"

int ivr_mutex_init();
int ivr_mutex_destroy();

void ivr_add_caller_proc (int callref, caller_t *new_call);
void ivr_del_caller_proc (int callref);
void ivr_rem_all_caller  (void);


void ivr_palay_file_proc (int callref, int start);
void ivr_play_file_stop_proc (int callref);

void ivr_make_call_proc  (int callref, char *call_num);

void ivr_collect_digit_proc (int callref, digits_t *dig);

void ivr_send_command (int module, int command);




// void ivr_show_caller_tree();

// void ivr_add_caller (int callref, caller_t *new_call);
// void ivr_del_caller (int callref);
// void ivr_remove_all_caller(void);

// int ivr_get_next_callref (void);

// int  ivr_get_caller_count ();
// void ivr_set_caller_state (int callref, int state);
// int  ivr_get_caller_state (int callref);
// int  ivr_set_digit_to_caller (int callref, uint8_t dig);

// void ivr_collect_digit (int callref, int needed);
// int  ivr_validate_digit_ok (int callref, char *base);

// //void stop_mgapp_proc ();
// //void send_mgapp_answer (int command, int state);
// void ivr_send_message (int module, int command, const char *_data, int size);

#endif // METHODS_H