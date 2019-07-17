#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#include <stdint.h>

#define CELL_CURVE_SIZE 11
        
typedef int16_t settings_type;

typedef struct
{
    settings_type  amp_per_volt;
    settings_type  capacity_ah;
    settings_type  cell_number;
    settings_type  discharge_start_level_mv;
    settings_type  cell_curve[2][CELL_CURVE_SIZE];
    settings_type  current_limit;
    settings_type  charge_stop_level;
    settings_type  undervoltage_level;
    settings_type  overtmp_level;
    settings_type  undertmp_level;
    settings_type  switch_off_time;
}settings_t;



void settings_restore(void);
settings_t * settings_memory_get(void);

#ifdef __cplusplus
    }
#endif

#endif /** SETTINGS */
