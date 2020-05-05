#ifndef SYS_UTILS_H
#define SYS_UTILS_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#define CLR_BIT(reg,bit) (reg &= ~(1L << bit))
#define GET_BIT(reg,bit) (reg & (1L << bit)? 1 : 0)

#define SET_BITS(reg,bit) (reg |=  bit)
#define CLR_BITS(reg,bit) (reg &= ~bit)
#define GET_BITS(reg,bit) (reg & bit)
        
#define ABS(a) ((a) > 0 ? (a) : (-a))

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
#define LAST_ITEM_NUBER(array) (ARRAY_SIZE(array) - 1)
        
#ifdef __cplusplus
    }
#endif

#endif /** SYS_UTILS */
