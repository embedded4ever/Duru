//volkan.unal@asisct.com wrote this file.
#ifndef MACHINE_H
#define MACHINE_H

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOW_POWER_MODE

typedef bool (*uart_tx)(uint8_t* data, uint16_t size);
typedef void* (*parser)(void* self, const char* buf, char* record_buf);
typedef uint8_t (*command_callback)(void* self);
typedef void (*error_callback)(void* self);
typedef void (*success_callback)(void *self);

	

#define DEBUG 0

#if DEBUG
#define gsm_machine_debug(...) do{printf(__VA_ARGS__);}while(0);
#else
#define gsm_machine_debug(...)
#endif

typedef enum e_error_code
{
		OK  = 0,
		NULL_PTR = -1,
		IN_PROCESS = -2,
		NULL_CHILD = -3,
	
}error_code;

struct command
{   
    const char* command_name;
    const char* expected_reply[2];
    struct command_window* child_command_window;
		parser parser_cb;
		uint16_t timeout;
		command_callback command_cb;
		error_callback error_cb;
		success_callback success_cb;
		
};

struct command_window
{
    struct command** command_table;
    struct command_window* command_window_parent;
    bool command_window_status;
    bool command_window_is_repetable;
    uint8_t command_window_size;
		#ifdef LOW_POWER_MODE
			bool low_power_flag;
		#endif
	
};

struct command_machine
{
		struct command_window* current_command_window;
    uint8_t command_index;
    uint8_t command_window_index;
		uint16_t timer;
		uint16_t timer_cnt;
		bool is_state_transition_available;
		bool is_all_command_completed;
		bool is_process;
    uart_tx uart_tx_cb;
    void (*gsm_power_reset)(void);
		#ifdef LOW_POWER_MODE
		 void (*low_power_cb)(int time);
		#endif
};

#define MAX_NUM_OF_BUFFER 350

typedef struct _gsm_uart
{
	char data;
	char buffer[MAX_NUM_OF_BUFFER];
	char gsm_imei[15];
	char utc_time[7];
	char latitude[12];
	char longitude[13];
	uint16_t buffer_index;
	uint8_t new_line_flag;
	char gnss_buffer[100];
	char gnss_data;	
     
}tgsm_uart;


void clear_gsm_uart(tgsm_uart *self);

extern tgsm_uart gsm_uart;

error_code command_window_init(struct command_window *self, struct command** table, struct command_window* parent, uint8_t size, uint16_t is_repeatable, bool low_power_mode);
	
error_code printf_command_window_details(const struct command_machine* self);

error_code machine_start(struct command_machine* self);

error_code machine_systick(struct command_machine* self);

error_code machine_loop(struct command_machine* self);

error_code machine_repeated_task(struct command_machine* self);

error_code machine_trans(struct command_machine* self, const char* tran_command_window_name);

struct command_window* get_current_window(struct command_machine* self);

error_code machine_stop_current(struct command_machine* self);


bool get_command_window_status(struct command_machine* self, const char* name);

#ifdef __cplusplus
}
#endif

#endif
