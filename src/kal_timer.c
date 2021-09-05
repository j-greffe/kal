#include <stdio.h>
#include "hal.h"
#include "kal_timer.h"

typedef struct {
    kal_timer_t* first;
} kal_timer_ctx_t;

static kal_timer_ctx_t g_timer = {
    .first = NULL,
};

static void kal_timer_wrap(void* param);
static void kal_timer_expired(void* param);

// Add a timer in the list if no already in
__inline static void kal_timer_add(kal_timer_t* timer_to_add)
{
    kal_timer_t* timer = g_timer.first;

    if (!g_timer.first)
    {
        // List is empty
        g_timer.first = timer_to_add;
        return;
    }

    while (timer != NULL)
    {
        if (timer_to_add == timer)
        {
            // Already in list
            return;
        }

        if (!timer->next)
        {
            // Reached end of list, append
            timer->next = timer_to_add;
            return;
        }
    }
}

__inline bool kal_timer_is_expired(int32_t wraps, uint16_t ti, uint16_t hal_time)
{
    return (((wraps < 0) || (!wraps && ti <= hal_time)));
}

__inline bool kal_timer_is_schedulable(int32_t wraps, uint16_t ti, uint16_t hal_time)
{
    return (!wraps && (ti > hal_time));
}

static void kal_timer_execute(void)
{
    kal_timer_t* timer = g_timer.first;

    // Look up all expired timers
    while (timer != NULL)
    {
        // Timer is expired
        if (KAL_TIMER_STATE_EXPIRED == timer->state)
        {
            timer->state = KAL_TIMER_STATE_STOPPED;

            // Execute action
            if (timer->action)
            {
                timer->action(timer->param);
            }
        }

        timer = timer->next;
    }
}

// Schedule next expiring timer
static void kal_timer_schedule_next(void)
{
    kal_timer_t* timer = g_timer.first;
    kal_timer_t* timer_to_schedule = NULL;
    bool timer_expired = false;
    uint16_t hal_time;

    // Stop ongoing timer
    hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_1);

    hal_time = hal_timer_get_time(HAL_TIMER_0);

    // TODO: Remove stopped timers from list?

    // Search for timer to schedule
    while (timer != NULL)
    {
        if (KAL_TIMER_STATE_RUNNING == timer->state)
        {
            // Save value to avoid wrap interrupt changing it while we check it
            int32_t wraps = timer->wraps;

            // Timer is expired
            if (kal_timer_is_expired(wraps, timer->ti, hal_time))
            {
                timer->state = KAL_TIMER_STATE_EXPIRED;
                timer_expired = true;
            }
            else if (kal_timer_is_schedulable(wraps, timer->ti, hal_time))
            {
                // No timer scheduled yet
                if (!timer_to_schedule)
                {
                    timer_to_schedule = timer;
                }
                // Current timer expires before scheduled timer
                else if (timer->ti < timer_to_schedule->ti)
                {
                    timer_to_schedule = timer;
                }
            }
        }

        timer = timer->next;
    }

    // At least one timer is expired
    if (timer_expired)
    {
        // Execute all expired timers
        kal_timer_execute();
    }

    if (timer_to_schedule)
    {
        hal_timer_start(HAL_TIMER_0, HAL_TIMER_INT_1, HAL_TIMER_MODE_ONE_SHOT, kal_timer_expired, timer_to_schedule, timer_to_schedule->ti - hal_time);
    }
}

// Callback on hal timer wrap
static void kal_timer_wrap(void* param)
{
    kal_timer_t* timer = g_timer.first;
    bool reschedule = false;

    while (timer != NULL)
    {
        timer->wraps--;

        if (!timer->wraps)
        {
            // Timer completed its wraps
            reschedule = true;
        }

        timer = timer->next;
    }

    if (reschedule)
    {
        kal_timer_schedule_next();
    }
}

// Callback on expired timer
static void kal_timer_expired(void* param)
{
    // This will execute expired timers and schedule the next one
    kal_timer_schedule_next();
}

void kal_timer_open(void)
{
    // Empty timer list
    g_timer.first = NULL;

    // Start wrap timer
    hal_timer_start(HAL_TIMER_0, HAL_TIMER_INT_0, HAL_TIMER_MODE_WRAP, kal_timer_wrap, NULL, 0);
}

void kal_timer_close(void)
{
    // Stop all hardware timers
    hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_0);
    hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_1);
}

void kal_timer_start(kal_timer_t* timer, hal_isr_t action, void* param, uint32_t ti)
{
    // Add the number of elapsed ticks since last wrap
    ti += hal_timer_get_time(HAL_TIMER_0);

    // Save action and parameter
    timer->action = action;
    timer->param = param;
    timer->state = KAL_TIMER_STATE_RUNNING;

    // Number of timer wraps before timer can be scheduled
    timer->wraps = (ti / 0x10000);

    // Remaining ticks after all the timer wraps
    timer->ti = (ti % 0x10000);

    // Add it
    kal_timer_add(timer);

    // Schedule
    kal_timer_schedule_next();
}

void kal_timer_stop(kal_timer_t* timer)
{
    // Flag as stopped
    timer->state = KAL_TIMER_STATE_STOPPED;

    // Reschedule
    kal_timer_schedule_next();
}

