#ifndef _KAL_TIMER_H_
#define _KAL_TIMER_H_

#include <stdint.h>

typedef enum {
    KAL_TIMER_STATE_STOPPED,
    KAL_TIMER_STATE_RUNNING,
    KAL_TIMER_STATE_EXPIRED,
} kal_timer_state_t;

typedef struct kal_timer_t {
    uint32_t ti;
    int32_t wraps;
    kal_timer_state_t state;
    hal_isr_t action;
    void* param;
    struct kal_timer_t* next;
} kal_timer_t;

void kal_timer_open(void);
void kal_timer_close(void);
//void kal_timer_init(kal_timer_t* timer, kal_timer_unit_t unit);
void kal_timer_start(kal_timer_t* timer, hal_isr_t action, void* param, uint32_t ti);
void kal_timer_stop(kal_timer_t* timer);


#endif // _KAL_TIMER_H_
