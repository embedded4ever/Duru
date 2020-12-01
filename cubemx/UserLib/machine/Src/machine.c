//volkan.unal@asisct.com wrote this file.

#include "machine.h"
#include "stdio.h"
#include "string.h"

#define CB_TIMEOUT 2000
#define TIMEOUT_COUNTER_MAX 10

extern bool gsm_reinit_flag;
tgsm_uart gsm_uart;

void clear_gsm_uart(tgsm_uart *self)
{
	memset(self -> buffer, 0, sizeof(self -> buffer));
	
	self -> buffer_index  = 0;
}
/**
 * \brief           Initialize Command Window
 * \param[in]       self: Pointer to command_window
 * \param[in] 			table: Array of pointer for table
 * \param[in]       parent: Pointer to parent for current command_window
 * \param[in]				size : Size of given array of pointer table
 * \param[in]				is_repeatable 
 * \return          error code
 */
 
error_code command_window_init(struct command_window *self, struct command** table, struct command_window* parent, uint8_t size, uint16_t is_repeatable, bool low_power_mode)
{
	if (self == NULL)
	{
		gsm_machine_debug("NULL[command_window_init]");
		return NULL_PTR;
	}
	
	self -> command_table  = table;
	
	self -> command_window_parent = parent;
	
	self -> command_window_size = size;
	
	self -> command_window_is_repetable = is_repeatable;
	
	self -> low_power_flag = low_power_mode;
	
	return OK;
}

/**
 * \brief           Show us details of given command window
 * \param[in]       self: Pointer to command machine
 * \return          error code
 */
error_code printf_command_window_details (const struct command_machine *self)
{
	if (self == NULL)
	{
		gsm_machine_debug("NULL[printf_command_window_details]");
		
		return NULL_PTR;
	}
	
	for (int i = 0; i < self -> current_command_window -> command_window_size; ++i)
	{
		gsm_machine_debug("\r\n Command Name = %s", self -> current_command_window -> command_table[i] -> command_name);
	}
	
	return OK;
}

/**
 * \brief           Go child window for each machine start
 * \param[in]       self: Pointer to command_machine
 * \return          error code
 */
error_code machine_start (struct command_machine* self)
{	
	if (self == NULL)
	{
		gsm_machine_debug("NULL[machine_start]");
		
		return NULL_PTR;
	}
	
	gsm_machine_debug("\nMachine Start ->  %s\r\n", self -> current_command_window -> command_table[self -> command_window_index] -> command_name);
	
	self -> command_index = 0;
	
	clear_gsm_uart(&gsm_uart);

	if (self -> current_command_window -> command_table[self -> command_window_index] -> child_command_window != NULL)
	{
		self -> current_command_window = self -> current_command_window -> command_table[self -> command_window_index] -> child_command_window;

		if (self -> current_command_window -> command_table[self -> command_index] -> command_cb != NULL)
		{
			self -> current_command_window -> command_table[self -> command_index] -> command_cb(self);
		}

		else
		{ 
			self -> uart_tx_cb ((uint8_t *)self -> current_command_window -> command_table[self -> command_index] -> command_name, 
									strlen(self -> current_command_window -> command_table[self -> command_index] -> command_name));
		}
		
		self -> timer = 1;
		
		return OK;
	}
	
	else
	{
		return NULL_CHILD;
	}
}

//For systick proceses
static bool is_callback = false;
static int callback_timer = 0;

//Timeout Counter
static uint8_t timeout_counter = 0;
extern struct command_machine command_machine_t;

/**
 * \brief           Call this function inside of your systick!
 * \param[in]       self: Pointer to command_machine
 * \return          error code
 */
error_code machine_systick (struct command_machine* self)
{		
	if (self == NULL)
	{
		gsm_machine_debug("NULL[machine_systick]");
		
		return NULL_PTR;
	}
	
  if (self -> timer)
  {					
		self -> timer_cnt++;
	
			if (gsm_uart.new_line_flag || self -> timer_cnt >= self -> current_command_window -> command_table[self -> command_index] -> timeout )
			{
				
				if ((strstr(gsm_uart.buffer, self -> current_command_window -> command_table[self -> command_index] -> expected_reply[0]) ||
						strstr(gsm_uart.buffer, self -> current_command_window -> command_table[self -> command_index] -> expected_reply[1])) &&
					 gsm_uart.buffer_index > 0)
				{
					gsm_machine_debug("\n State Ok! = %s \n", self -> current_command_window -> command_table[self -> command_index] -> command_name);
					
					timeout_counter = 0;
					
					if(self -> current_command_window -> command_table[self -> command_index] -> success_cb != NULL)
					{
						self -> current_command_window -> command_table[self -> command_index] -> success_cb(self);
					}
					
					if (self -> current_command_window -> command_table[self -> command_index] -> parser_cb != NULL)
					{
						is_callback = true;
					}
					
					else
					{
						self -> command_index += 1;	
						
						self -> is_state_transition_available = 1;
					}
				}
	
				else
				{	
					if (self -> current_command_window -> command_table[self -> command_index] -> error_cb != NULL)
					{
						self -> current_command_window -> command_table[self -> command_index] -> error_cb(self);
					}
					
					gsm_machine_debug(" \n Timeout State! = %s \n", self -> current_command_window -> command_table[self -> command_index] -> command_name);
					
					self -> is_state_transition_available = 1;
					
					if (self -> command_window_index != 2)
					{
						 timeout_counter++;
					}
				}
				
				self -> timer = 0;
				
				self -> timer_cnt = 0;
				
				gsm_uart.new_line_flag = false;
				
			}
	}
	
	if (timeout_counter >= TIMEOUT_COUNTER_MAX)
	{
		timeout_counter = 0;
		
		gsm_machine_debug("\r\n Modem Reset Because Of Timeout_Counter Overflow \r\n");
		
		self -> timer = 0;
		
		self -> command_index = 0;
		
		self -> command_window_index = 0;

		self -> is_state_transition_available  = 0;
		
		while (self -> current_command_window -> command_window_parent != NULL)
		{
			self -> current_command_window  = self -> current_command_window -> command_window_parent;
		}
		
		gsm_reinit_flag = true;
		
		machine_start(&command_machine_t);
		
	}
	
	if (is_callback)
	{
		callback_timer++;
	}
	
	return OK;
}

/**
 * \brief           Call this function inside of your loop!
 * \param[in]       self: Pointer to command_machine
 * \return          error code
 */
extern bool reconnect;

error_code machine_loop (struct command_machine* self)
{
	if (self == NULL)
	{
		gsm_machine_debug("NULL[machine_loop]");
		
		return NULL_PTR;
	}
	
  if (self -> is_state_transition_available)
  {
		clear_gsm_uart(&gsm_uart);
		
		self -> is_process = false;
		
    if (self -> current_command_window -> command_window_size > self -> command_index)
    {
			if (self -> current_command_window -> command_table[self -> command_index] -> command_cb != NULL)
			{
				self -> current_command_window -> command_table[self -> command_index] -> command_cb(self);
			}

			else
			{
				self -> uart_tx_cb((uint8_t *)self -> current_command_window -> command_table[self -> command_index] -> command_name, 
																strlen(self -> current_command_window -> command_table[self -> command_index] -> command_name));
			}

			self -> is_state_transition_available = 0;
			
			self -> timer = 1;
			
			self -> current_command_window -> command_window_status = 0;
			
		
		}
		
		/* it means, command window has just ended successfully, Now we can pass to parent of current window. Like linkedlist.*/
		else 
    {
			clear_gsm_uart(&gsm_uart);
			
			self -> is_state_transition_available = 0;
			
			self -> command_index = 0;
			
			self -> timer = 0;
			
			self -> current_command_window -> command_window_status = 1;
			
			#ifdef LOW_POWER_MODE
			
				if (self -> current_command_window -> low_power_flag == 1)
				{
					gsm_machine_debug("Entering Low Power Mode ASAP \r\n");
					
					self -> low_power_cb(0);
				}
				
			#endif
			
			while (self -> current_command_window -> command_window_parent != NULL)
			{
				self -> current_command_window  = self -> current_command_window -> command_window_parent;
			}
						
			self -> is_process = true;
		}
  }
	
	if (is_callback && callback_timer > CB_TIMEOUT && self -> current_command_window -> command_table[self -> command_index] -> parser_cb != NULL)
	{
		self -> current_command_window -> command_table[self -> command_index] -> parser_cb(self, gsm_uart.buffer, NULL);
		
		is_callback = false;
	
		clear_gsm_uart(&gsm_uart);
		
		callback_timer = 0;
		
		if (self -> current_command_window -> command_window_size == self -> command_index + 1)
		{
			self -> is_process = true;
			
			while (self -> current_command_window -> command_window_parent != NULL)
			{
				self -> current_command_window  = self -> current_command_window -> command_window_parent;
			}
		}
		
		else
		{
			self -> command_index += 1;
		
			self -> is_state_transition_available = 1;
		}
	}
	
	return OK;
}

/**
 * \brief           [Not Impelemented]
 * \param[in]       self: Pointer to command_machine
 * \return          error code
 */
error_code machine_repeated_task (struct command_machine* self)
{	
	if (self == NULL)
	{
		gsm_machine_debug("NULL[machine_repeated_task]");
		
		return NULL_PTR;
	}
	
	if (self -> is_all_command_completed)
	{
		while(self -> current_command_window -> command_window_parent != NULL)
		{
			self -> current_command_window = self -> current_command_window -> command_window_parent;
		} 
		
		self -> is_all_command_completed = 0;
				
		for (int i = 0; i < self -> current_command_window -> command_window_size; i++)
		{
			gsm_machine_debug("\r\n self -> current_command_window -> command name = %s", self -> current_command_window -> command_table[0] -> command_name);
			
			if (self -> current_command_window -> command_table[i] -> child_command_window != NULL)
			{
				self -> current_command_window = self -> current_command_window -> command_table[i] -> child_command_window;
				
				if (self -> current_command_window -> command_window_is_repetable)
				{
					self -> current_command_window = self -> current_command_window -> command_window_parent;
					
					self -> command_window_index = i;
					
					machine_start(self);
					
					break;
				}
				
				self -> current_command_window = self -> current_command_window -> command_window_parent;
			}
		}
	}
	
	return OK;
}

/**
 * \brief           You can change your process
 * \param[in]       self: Pointer to command_machine
 * \param[in]       tran_command_window_name: Name of the trans command_window_name's we want
 * \return          error code
 */
error_code machine_trans (struct command_machine* self, const char* tran_command_window_name)
{
	if (self == NULL)
	{
		gsm_machine_debug("NULL[machine_trans]");
		
		return NULL_PTR;
	}
	
	if (self -> is_process)
	{
		for (int i = 0;  i < self -> current_command_window -> command_window_size; ++i)
		{
			gsm_machine_debug("Machine Trans = %s", self -> current_command_window -> command_table[i] -> command_name);
			
			if (strstr(self -> current_command_window -> command_table[i] -> command_name, tran_command_window_name) != NULL)
			{
				self -> command_window_index = i;
				
				machine_start(self);
				
				break;
			}
		} 
	}
	
	else
	{
		return IN_PROCESS;
	}
	
	return OK;
}

/**
 * \brief           Get current window
 * \param[in]       self: Pointer to command_machine
 * \return          error code
 */
struct command_window* get_current_window (struct command_machine* self)
{
	static struct command_window* ret  = NULL;
	
	if (self == NULL)
	{
		gsm_machine_debug("NULL[get_current_window\r\n");
		
		return NULL;
	}
	
	ret = self -> current_command_window;
	
	return ret;
}

/**
 * \brief           get command window tatus
 * \param[in]       self: Pointer to command_machine
 * \param[in]       self: related command_window's name
 * \return          related command_window's status
 */
bool get_command_window_status(struct command_machine* self, const char* name)
{
	bool status = false;
	
	if (self -> is_process)
	{		 
		for (int i = 0;  i < self -> current_command_window -> command_window_size; ++i)
		{
			gsm_machine_debug("Machine Trans = %s", self -> current_command_window -> command_table[i] -> command_name);
			
			if (strstr(self -> current_command_window -> command_table[i] -> command_name, name) != NULL)
			{
					status = self -> current_command_window -> command_table[i] -> child_command_window -> command_window_status;
				
					self -> current_command_window -> command_table[i] -> child_command_window -> command_window_status = false;
				
					break;
			}
		}
	}
	
	return status;
}
