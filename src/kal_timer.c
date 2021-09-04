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

// Add a timer in the list
static void kal_timer_add(kal_timer_t* timer_to_add)
{
    // Add it in first place for simplicity's sake
    timer_to_add->next = g_timer.first;
    g_timer.first = timer_to_add;
}

// Remove specified timer from the list
static void kal_timer_remove(kal_timer_t* timer_to_remove)
{
    kal_timer_t* timer = g_timer.first;
    kal_timer_t* timer_previous = NULL;

    while (timer != NULL)
    {
        if (timer == timer_to_remove)
        {
            // First timer in list
            if (!timer_previous)
            {
                g_timer.first = timer->next;
            }
            // Other timer
            else
            {
                timer_previous->next = timer->next;
            }
            break;
        }

        timer_previous = timer;
        timer = timer->next;
    }
}

// Schedule next expiring timer
static void kal_timer_schedule_next(void)
{
    kal_timer_t* timer = g_timer.first;
    kal_timer_t* timer_to_schedule = NULL;
    uint16_t hal_time = hal_timer_get_time(HAL_TIMER_0);

    while (timer != NULL)
    {
        // Timer is available for scheduling
        if (!timer->wraps)
        {
            // Timer is expired
            if (timer->ti <= hal_time)
            {
                // Execute action
                if (timer->action)
                {
                    timer->action(timer->param);
                }

                kal_timer_remove(timer);
            }
            // No timer scheduled yet
            else if (!timer_to_schedule)
            {
                timer_to_schedule = timer;
            }
            // Current timer expires before scheduled timer
            else if (timer->ti < timer_to_schedule->ti)
            {
                timer_to_schedule = timer;
            }
        }

        timer = timer->next;
    }

    if (timer_to_schedule)
    {
        hal_timer_start(HAL_TIMER_0, HAL_TIMER_INT_1, HAL_TIMER_MODE_ONE_SHOT, kal_timer_expired, timer_to_schedule, timer_to_schedule->ti - hal_time);
    }
    else
    {
        hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_1);
    }
}

// Callback on hal timer wrap
static void kal_timer_wrap(void* param)
{
    kal_timer_t* timer = g_timer.first;
    bool reschedule = false;

    while (timer != NULL)
    {
        if (timer->wraps)
        {
            timer->wraps--;

            if (!timer->wraps)
            {
                reschedule = true;
            }
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
    kal_timer_t* timer = (kal_timer_t*)param;

    // Execute action
    if (timer->action)
    {
        timer->action(timer->param);
    }

    kal_timer_stop(timer);
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

    // Number of timer wraps before timer can be scheduled
    timer->wraps = (ti / 0x10000);

    // Remaining ticks after all the timer wraps
    timer->ti = (ti % 0x10000);

    // Remove timer form list in case it is already here
    kal_timer_remove(timer);

    // Add it
    kal_timer_add(timer);

    // Schedule next
    kal_timer_schedule_next();
}

void kal_timer_stop(kal_timer_t* timer)
{
    // Remove from list
    kal_timer_remove(timer);

    // Reschedule
    kal_timer_schedule_next();
}

