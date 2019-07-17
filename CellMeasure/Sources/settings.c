#ifdef __cplusplus
    extern "C"
    {
#endif

#include "settings.h"
#include <string.h>
        
settings_t settings;

int16_t toshiba_curve[2][CELL_CURVE_SIZE] = {{2070, 2120,2150,2180,2210,2260,2320,2350,2400,2480,2600},   
                                             { 0,   100, 200,  300, 400, 500, 600, 700, 800, 900, 1000}};


settings_t * settings_memory_get(void)
{
    return &settings;
}

void settings_restore(void)
{
    settings.amp_per_volt       = 160;
    settings.capacity_ah        = 23;
    settings.cell_number        = 5;
    settings.discharge_start_level_mv = 2400;
    settings.switch_off_time    = 30;
    memcpy(settings.cell_curve, toshiba_curve, sizeof(toshiba_curve)); 

    settings.overtmp_level      = 50;
    settings.undertmp_level     = -20;
    settings.charge_stop_level  = 2700;
    settings.undervoltage_level = 1500;
    settings.current_limit      = 100;
    
    
}

#ifdef __cplusplus
    }
#endif
