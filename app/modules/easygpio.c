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
//
// easygpio.init(5, 6, 0)
// easygpio.read(3, 1)

unsigned int PIN_CLK;
unsigned int PIN_DATA;
unsigned int STATE_INDICATOR;



bool is_ready() {
	if (STATE_INDICATOR == 0)
		return platform_gpio_read(PIN_DATA) == LOW;
	return platform_gpio_read(PIN_DATA) == HIGH;
}

long read(int pulses, int after_ticks) {
	unsigned long Count;
	unsigned char i;
	Count=0;

	if (STATE_INDICATOR == 0) {
		platform_gpio_write(PIN_CLK, LOW );
	}
	else {
		platform_gpio_write(PIN_CLK, HIGH );
	}

	while (!is_ready());

	for (i=0;i<pulses*8;i++){
		platform_gpio_write(PIN_CLK, HIGH );
		Count=Count<<1;

		//uint32_t t = system_get_time();
		//while ((system_get_time() - t) < 40)
		//{
		//	os_delay_us(1);
		//}
		os_delay_us(1);

		if (platform_gpio_read(PIN_DATA) == HIGH) Count++;
		platform_gpio_write(PIN_CLK, LOW );

		//t = system_get_time();
		//while ((system_get_time() - t) < 40)
		//{
		//	os_delay_us(1);
		//}
		os_delay_us(1);
	}

	for (i = 1; i <= after_ticks; i++) {
		platform_gpio_write(PIN_CLK, HIGH);
		os_delay_us(1);
		platform_gpio_write(PIN_CLK, LOW);
		os_delay_us(1);
	}

	Count=Count^0x800000;
	return(Count);
}

//initialize GPIO's
static int easygpio_init( lua_State* L )
{
	PIN_CLK = luaL_checkinteger( L, 1 );
	PIN_DATA = luaL_checkinteger( L, 2 );
	STATE_INDICATOR = luaL_checkinteger(L, 3);

	platform_gpio_mode(PIN_CLK, OUTPUT, PULLUP);
	platform_gpio_mode(PIN_DATA, INPUT, PULLUP);

	return 1;
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
	lua_Integer pulses;
	pulses = luaL_checkinteger( L, 1 );
	lua_Integer after_tick;
	after_tick = luaL_checkinteger( L, 2 );
	long int result;
	result = read((int)pulses, (int)after_tick);
	lua_pushnumber(L, (lua_Number)result);
	return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE easygpio_map[] =
{
  { LSTRKEY( "init" ), LFUNCVAL( easygpio_init ) },
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
