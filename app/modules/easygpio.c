// Module for interfacing with GPIOs

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"

#include "c_types.h"
#include "c_string.h"

#define PULLUP PLATFORM_GPIO_PULLUP
#define FLOAT PLATFORM_GPIO_FLOAT
#define OUTPUT PLATFORM_GPIO_OUTPUT
#define INPUT PLATFORM_GPIO_INPUT
#define INTERRUPT PLATFORM_GPIO_INT
#define HIGH PLATFORM_GPIO_HIGH
#define LOW PLATFORM_GPIO_LOW

// Usage:
// easygpio.read(PIN_CLK, PIN_OUT, num_bits, extra_ticks, delay_us, initial_val, ready_state)
// -- read 24 bits, from pin 6, pin 5 is CLOCK, hold it for 1us, initial value
// -- written to pin 5 is LOW (0), and ready state is when the pin 6 changes to LOW (0)
// easygpio.read(5, 6, 24, 1, 1, 0, 0)

unsigned int PIN_CLK;
unsigned int PIN_DATA;
unsigned int STATE_INDICATOR;



bool is_ready() {
	if (STATE_INDICATOR == 0)
		return platform_gpio_read(PIN_DATA) == LOW;
	return platform_gpio_read(PIN_DATA) == HIGH;
}

long read(int pin_clk, int pin_data, int num_bits, int after_ticks,
		int delay_us, int initial_clk_write, int ready_state) {
	unsigned long Count;
	unsigned char i;
	Count=0;

	platform_gpio_write(pin_clk, initial_clk_write);

	while (platform_gpio_read(pin_data) != ready_state);

	for (i=0;i<num_bits;i++){
		platform_gpio_write(pin_clk, HIGH );
		Count=Count<<1;

		//uint32_t t = system_get_time();
		//while ((system_get_time() - t) < 40)
		//{
		//	os_delay_us(1);
		//}
		os_delay_us(delay_us);

		if (platform_gpio_read(pin_data) == HIGH) Count++;
		platform_gpio_write(pin_clk, LOW );

		//t = system_get_time();
		//while ((system_get_time() - t) < 40)
		//{
		//	os_delay_us(1);
		//}
		os_delay_us(delay_us);
	}

	for (i = 1; i <= after_ticks; i++) {
		platform_gpio_write(pin_clk, HIGH);
		os_delay_us(delay_us);
		platform_gpio_write(pin_clk, LOW);
		os_delay_us(delay_us);
	}

	Count=Count^0x800000;
	return(Count);
}

static int easygpio_info( lua_State* L )
{
	char buf[255];
	c_sprintf(buf, "pin clock:%d, pin output:%d, state:%d", PIN_CLK, PIN_DATA, STATE_INDICATOR);
	lua_pushfstring(L, buf);
	return 1;
}

static int easygpio_read( lua_State* L )
{
	//int pin_clk, int pin_data, int num_bits, int after_ticks,
	//		int delay_us, int initial_state, int ready_state)

	int pin_clk;
	int pin_data;
	int num_bits;
	int after_ticks;
	int delay_us;
	int initial_clk_write;
	int ready_state;

	pin_clk = (int) luaL_checkinteger( L, 1 );
	pin_data = (int) luaL_checkinteger( L, 2 );
	num_bits = (int) luaL_checkinteger( L, 3 );
	after_ticks = (int)luaL_checkinteger( L, 4 );
	delay_us = (int) luaL_checkinteger( L, 5 );
	initial_clk_write = (int) luaL_checkinteger( L, 6 );
	ready_state = (int) luaL_checkinteger( L, 7 );

	long int result;
	result = read(pin_clk, pin_data, num_bits, after_ticks,
				delay_us, initial_clk_write, ready_state);

	lua_pushnumber(L, (lua_Number)result);
	return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE easygpio_map[] =
{
  { LSTRKEY( "read" ), LFUNCVAL( easygpio_read ) },
  { LSTRKEY( "info" ), LFUNCVAL( easygpio_info ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_easygpio( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_EASYGPIO, easygpio_map );
  return 1;
#endif
}
