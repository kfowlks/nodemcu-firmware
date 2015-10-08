-- Lua 5.1+ hx711 2015 by Roman Chyla <roman.chyla@gmail.com>

local moduleName = ... 
local M = {}
_G[moduleName] = M

-- variables that affect HX711 reading (we read data with easygpio module)
local DOUT = 6;
local SCLK = 5;
local BITS_READ = 24;
local GAIN = 1;
local DELAY_US = 1;
local READY_STATE = gpio.LOW;
local INITIAL_VAL = gpio.HIGH;
local OFFSET = 0;
local SCALE_UNIT = 1;



-- HX711 accepts gain of 128, 64, and 32; the default is 128 (1)
function M.set_gain(b)
    if b == 128 then
        GAIN = 1;
    elseif b == 64 then 
        GAIN = 2;
    elseif b == 32 then
        GAIN = 3;
    else then
        throw("Error, unknown gain value: " .. b);
    end
end

-- public function, returns averaged readings (minus scale offset)
function M.read(times)
    return read_average(times) - OFFSET;
end

-- offset is individual to the load-cell
function M.set_offset(off)
    OFFSET = off;
end

-- you can discover the units by measuring a known object (i.e. 1kg)
function M.set_scale_unit(u)
    SCALE_UNIT = u;
end

-- reset measured weight to zero
function M.tare(times)
    M.set_offset(read_average(times));
end

-- return weight in the units (instead of raw reading)
function read_units(times)
    return M.read(times) / SCALE_UNIT;
end


-- read raw data from the sensor (apply appropriate bitmasks)
local function read()
    val = easygpio.read(CLK, OUT, BITS_READ, GAIN, DELAY_US, INITIAL_VAL, READY_STATE);
    return bit.bxor(val, 0x80);
end

-- average value after reading 'num_try' times
local function read_average(num_try)
    s = 0;
    for i=1, num_try do
        s = s + read();
    end
    return s / num_try;
end

return M
