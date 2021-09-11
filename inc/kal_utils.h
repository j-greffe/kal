#ifndef _KAL_UTILS_H_
#define _KAL_UTILS_H_


__inline uint8_t min_u8(uint8_t a, uint8_t b)
{
    return (a < b)? a: b;
}

__inline uint8_t max_u8(uint8_t a, uint8_t b)
{
    return (a > b)? a: b;
}

__inline uint16_t min_u16(uint16_t a, uint16_t b)
{
    return (a < b)? a: b;
}

__inline uint16_t max_u16(uint16_t a, uint16_t b)
{
    return (a > b)? a: b;
}

__inline uint32_t min_u32(uint32_t a, uint32_t b)
{
    return (a < b)? a: b;
}

__inline uint32_t max_u32(uint32_t a, uint32_t b)
{
    return (a > b)? a: b;
}

__inline int8_t min_s8(int8_t a, int8_t b)
{
    return (a < b)? a: b;
}

__inline int8_t max_s8(int8_t a, int8_t b)
{
    return (a > b)? a: b;
}

__inline int16_t min_s16(int16_t a, int16_t b)
{
    return (a < b)? a: b;
}

__inline int16_t max_s16(int16_t a, int16_t b)
{
    return (a > b)? a: b;
}

__inline int32_t min_s32(int32_t a, int32_t b)
{
    return (a < b)? a: b;
}

__inline int32_t max_s32(int32_t a, int32_t b)
{
    return (a > b)? a: b;
}


#endif // _KAL_UTILS_H_
