#include "platform_specific.h"
#include "stdlib.h"
#include "time.h"
#include "rtc.h"

extern int ultrasonic_val;

extern RTC_HandleTypeDef hrtc;
extern char get_buffer_for_send[600];

static void update_rtc(RTC_TimeTypeDef *sTime,	RTC_DateTypeDef *sDate)
{
	HAL_RTC_SetDate(&hrtc,sDate,RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc,sTime,RTC_FORMAT_BIN);
}

//We have 4 seperated window for now
struct command_window main_all_command_list_t;
struct command_window at_command_window;
struct command_window tcp_command_window;
struct command_window put_command_window;
struct command_window put_transparent_command_window;
struct command_window close_connection_window;

//For "main window"
struct command at_window_t = {"INIT", "", NULL, &at_command_window};
struct command put_window_t = {"PUT", "", NULL, &put_command_window};
struct command put_transparent_command_window_t = {"SEND", "", NULL, &put_transparent_command_window};
struct command close_connection_window_t = {"CLOSE", "", NULL, &close_connection_window};

//"Main window" command_table.
struct command* main_all_command_list_table[] =
{
    &at_window_t,
	&put_window_t,
	&put_transparent_command_window_t,
	&close_connection_window_t,
};

/**************************************************************************************************************************/

/**************************************************************************************************************************/

//For "INIT" window.
void* get_imei (void* self, const char* buf, char* record_buf)
{
	static char ret_buf[15] = {0};
	
	if (self != NULL && strlen(buf) > 0)
	{
		char* ret = NULL;
		
		ret = strstr(buf, "\n");
		
		memset (ret_buf, 0, sizeof(ret_buf));
		
		if (ret != NULL)
		{
			ret++;
			
			memcpy(ret_buf, ret, 15);
			
			memcpy(gsm_uart.gsm_imei, ret, 15);
			
		}	
	}	
	
	return ret_buf;
}

void* get_clock (void* self, const char* buf, char* record_buf)
{
	RTC_TimeTypeDef stimeststuctureget = {0};
	RTC_DateTypeDef sdatastuctureget = {0};
	
	struct tm tm;
	
	if (self != NULL && strlen(buf) > 0)
	{
		
		char* ret = NULL;
		
		ret = strstr(buf, "+QLTS:");
		
		if (ret != NULL)
		{
			ret = ret + sizeof("+QLTS:") + 1;
					sscanf(ret, "%d/%d/%d,%d:%d:%d",
						&tm.tm_year,&tm.tm_mon,&tm.tm_mday,
							&tm.tm_hour,&tm.tm_min,&tm.tm_sec);
			
			sdatastuctureget.Date = tm.tm_mday;
			sdatastuctureget.Year = tm.tm_year;
			sdatastuctureget.Month = tm.tm_mon;
			
			stimeststuctureget.Hours = tm.tm_hour;
			stimeststuctureget.Minutes = tm.tm_min;
			stimeststuctureget.Seconds = tm.tm_sec;
			
			update_rtc(&stimeststuctureget, &sdatastuctureget);
						
		}	
	}	
	
	return NULL;
}

struct command at_echo = {"ATE0\r\n", "OK\r\n", NULL, NULL, NULL, 1000 };
struct command at = {"AT\r\n", "OK\r\n", NULL, NULL, NULL, 3000 };
struct command at_cgact_activate = {"AT+CGACT=1,1\r\n", "OK\r\n",NULL, NULL, NULL,  4000};
struct command at_cgact_deactivate = {"AT+CGACT=0,1\r\n", "OK\r\n", NULL, NULL, NULL, 4000};
struct command at_cping = {"AT+CPIN?\r\n", "READY\r\n", NULL, NULL, NULL, 2000 };
struct command at_cgreg = {"AT+CGREG?\r\n", "0,1\r\n", "0,2\r\n", NULL, NULL, 3500 };
struct command at_creg = {"AT+CREG?\r\n", "0,1\r\n", "0,2\r\n", NULL, NULL, 3000 };
struct command at_qindi = {"AT+QINDI=1\r\n", "OK\r\n", NULL, NULL, NULL, 3000 };
struct command cgdcont = {"AT+CGDCONT=1,\"IP\",\"mgb\",\"\",\r\n", "OK\r\n",NULL, NULL, NULL, 3000 };
struct command at_cpgaddr = {"AT+CGPADDR=1\r\n", "OK\r\n", NULL, NULL, NULL, 1000 };
struct command at_qiact = {"AT+QIACT=1\r\n", "OK\r\n", NULL, NULL, NULL, 4500 };
struct command at_get_imei = {"AT+GSN\r\n", "OK\r\n", NULL, NULL, get_imei, 1000 };
struct command at_get_time = {"AT+QLTS\r\n", "OK", NULL, NULL, &get_clock, 1000 };
struct command at_gps_session = {"AT+QGPS=1\r\n", "OK", NULL, NULL, NULL, 1500 };
struct command at_configure_gnss_second = {"AT+QGPSCFG=\"outport\",\"uartdebug\"\r\n", "OK", NULL, NULL, NULL, 1500 };

//"INIT window" command table.
struct command* at_commands_list_table[] = 
{
	//&at_echo,
    &at,
	&at_cping,
	&at_creg,
	&at_cgreg,
	&at_qindi,
	&at,
	&at_cgact_deactivate,
	&cgdcont,
    &at_cgact_activate,
	&at_cpgaddr,
	&at,
	&at_qiact,
	&at_get_imei,
	&at_get_time,
	&at,
		
};

/**************************************************************************************************************************/
//For "PUT" window.
uint8_t put_connect_host(void* self)
{
	
	gsm_machine_debug("\r\n Host Connected !! \r\n");

	struct command_machine *cm = (struct command_machine*)self;
	
	char cr = {0x0D};
	
	char cmd[60];	
	
	snprintf(cmd, sizeof(cmd),"AT+QIOPEN=1,2,\"UDP\",\"%s\",%i,0,2%c","youripadress", <port>, cr);

	cm -> uart_tx_cb((uint8_t*)cmd, strlen(cmd));
	
	return true;
}

uint8_t close_conn(void* self)
{
	gsm_machine_debug("\r\n Data Sended !! \r\n");
	
	char cr = {0x0D};
	
	struct command_machine* cm = (struct command_machine*)self;
	
	char cmd[30];
	
	snprintf(cmd, sizeof(cmd), "AT+QICLOSE=2%c", cr);

	cm -> uart_tx_cb((uint8_t*)cmd, strlen(cmd));	
	
	return true;
}

uint8_t switch_to_command_mode(void* self)
{	
	struct command_machine* cm = (struct command_machine*)self;
	
	cm -> uart_tx_cb((uint8_t*)"+++", 3);
	
	return true;
}

extern uint8_t Time[3];
extern uint8_t Date[3];

uint8_t send_data_to_host(void* self)
{	
	gsm_machine_debug("\r\n Data Sended !! \r\n");
	
	struct command_machine* cm = (struct command_machine*)self;
	
	snprintf(get_buffer_for_send, sizeof(get_buffer_for_send), "%s", "test");
	
	cm -> uart_tx_cb((uint8_t*)get_buffer_for_send, strlen(get_buffer_for_send));	
	
	return true;
}

struct command at_qiopen_put = {"AT+QIOPEN\r\n", "CONNECT", "ALREADY CONNECT\r\n", NULL, NULL, 3000, put_connect_host, NULL, NULL};
struct command at_qisend_data_put = {"AT+QISEND", "+++", NULL, NULL, NULL, 1000, send_data_to_host};

//For "PUT Window" command table.
struct command* put_command_list_table[] = 
{
	&at_qiopen_put,
};

struct command* put_transparent_command_list_table[] = 
{
	&at_qisend_data_put,
};


struct command switch_to_command_mode_command = {"AT+QISEND", "OK\r\n", NULL, NULL, NULL, 1500, switch_to_command_mode};
struct command at_close = {"AT+QISEND", "OK", NULL, NULL, NULL, 1500, close_conn};

struct command* close_command_list_table[] = 
{
	&switch_to_command_mode_command,
	&at_close,
};

void init_all_command_window(void)
{	
	command_window_init(&main_all_command_list_t, main_all_command_list_table, NULL, sizeof(main_all_command_list_table) / sizeof(main_all_command_list_table[0]), 0, 0);

	command_window_init(&at_command_window, at_commands_list_table, &main_all_command_list_t, sizeof(at_commands_list_table) / sizeof(at_commands_list_table[0]), 0, 0);
		
	command_window_init(&put_command_window, put_command_list_table, &main_all_command_list_t, sizeof(put_command_list_table) / sizeof(put_command_list_table[0]), 0, 0);
	
	command_window_init(&put_transparent_command_window, put_transparent_command_list_table, &main_all_command_list_t, sizeof(put_transparent_command_list_table) / sizeof(put_transparent_command_list_table[0]), 0, 0);
	
	command_window_init(&close_connection_window, close_command_list_table, &main_all_command_list_t, sizeof(close_command_list_table) / sizeof(close_command_list_table[0]), 0, 0);

}




