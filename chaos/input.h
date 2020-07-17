#ifndef input_h_seen
#define input_h_seen
/* input.h */
enum wait_type {
	NO_WAIT,
	WAIT_FOR_LETGO,
	WAIT_FOR_KEYPRESS
};
void handle_keys(void);
void wait_for_keypress(void);
void wait_for_letgo(void);
void keypress_waiting(intptr_t arg);

#endif
