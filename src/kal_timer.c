#include <stdio.h>
#include "hal.h"
#include "kal_timer.h"
#include "kal_event.h"

typedef struct {
    kal_timer_t* first;
    kal_timer_t wait;
    uint8_t id;
} kal_timer_ctx_t;

static kal_timer_ctx_t g_timer = {
    .first = NULL,
    .id = 1,
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
        timer_to_add->id = g_timer.id++;
        timer_to_add->next = NULL;
        g_timer.first = timer_to_add;
        return;
    }

    while (timer != NULL)
    {
        if (timer_to_add->id == timer->id)
        {
            // Already in list
            return;
        }

        if (!timer->next)
        {
            // Reached end of list, append
            timer_to_add->id = g_timer.id++;
            timer_to_add->next = NULL;
            timer->next = timer_to_add;
            return;
        }

        timer = timer->next;
    }
}

__inline bool kal_timer_is_expired(int32_t wraps, uint32_t qti, uint16_t hal_time)
{
    return (((wraps < 0) || (!wraps && qti <= ((uint32_t)hal_time))));
}

__inline bool kal_timer_is_schedulable(int32_t wraps, uint32_t qti, uint16_t hal_time)
{
    return (!wraps && (qti > hal_time));
}

// Schedule next expiring timer
static void kal_timer_schedule_next(void)
{
    kal_timer_t* timer = g_timer.first;
    kal_timer_t* timer_to_schedule = NULL;
    uint16_t hal_time;

    // Stop ongoing timer
    hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_1);

    hal_time = hal_timer_get_time(HAL_TIMER_0);

    // Execute all expired timers
    while (timer != NULL)
    {
        // Timer is expired
        if (kal_timer_is_expired(timer->wraps, timer->qti, hal_time))
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

    // TODO: Remove stopped timers from list?

    // Search for timer to schedule
    timer = g_timer.first;
    while (timer != NULL)
    {
        if (KAL_TIMER_STATE_RUNNING == timer->state)
        {
            if (kal_timer_is_schedulable(timer->wraps, timer->qti, hal_time))
            {
                // No timer scheduled yet
                if (!timer_to_schedule)
                {
                    timer_to_schedule = timer;
                }
                // Current timer expires before scheduled timer
                else if (timer->qti < timer_to_schedule->qti)
                {
                    timer_to_schedule = timer;
                }
            }
        }

        timer = timer->next;
    }

    if (timer_to_schedule)
    {
        hal_timer_start(HAL_TIMER_0, HAL_TIMER_INT_1, HAL_TIMER_MODE_ONE_SHOT, kal_timer_expired, timer_to_schedule, timer_to_schedule->qti - hal_time);
    }
}

// Callback on hal timer wrap
static void kal_timer_wrap(void* param)
{
    kal_timer_t* timer = g_timer.first;
    bool reschedule = false;

    while (timer != NULL)
    {
        if (KAL_TIMER_STATE_RUNNING == timer->state)
        {
            timer->wraps--;

            if (!timer->wraps)
            {
                // Timer completed its wraps
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

    // Stop ongoing timer
    hal_timer_stop(HAL_TIMER_0, HAL_TIMER_INT_1);

    // Flag as stopped
    timer->state = KAL_TIMER_STATE_STOPPED;

    // Critical section here because if we wrap after getting the time
    // but before the timer is running, we will have one wrap of added delay
    hal_irq_critical_enter();

    // Save action and parameter
    timer->action = action;
    timer->param = param;

    // Add the number of elapsed qTi since last wrap
    uint16_t hal_time = hal_timer_get_time(HAL_TIMER_0);

    // Convert Tick to qTi
    timer->wraps = (ti >> 14);
    timer->qti = ((ti & 0x00003FFF) << 2) + hal_time;
    timer->wraps += (timer->qti >> 16);
    timer->qti &= 0x0000FFFF;

    // Add it
    kal_timer_add(timer);

    // Flag as running
    timer->state = KAL_TIMER_STATE_RUNNING;

    hal_irq_critical_exit();

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

void kal_timer_wait(uint32_t ti)
{
    kal_timer_start(&g_timer.wait, (hal_isr_t)kal_event_set, (void*)KAL_EVENT_WAIT, ti);
    kal_event_wait(KAL_EVENT_WAIT);
}

