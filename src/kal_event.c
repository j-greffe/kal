#include "hal.h"
#include "kal_event.h"

static uint32_t g_events = 0;

uint8_t kal_event_wait(uint8_t event)
{
    // Wait for any event
    if (KAL_EVENT_ALL == event)
    {
        while (!g_events)
        {
            hal_lpm_enter();
        }
    }
    // Wait for specific event
    else
    {
        while (!(g_events & (1 << event)))
        {

            hal_lpm_enter();
        }
    }
    
    // Get first event
    event = 0;
    while (!(g_events & (1 << event)))
    {
        event++;
    }

    // Clear this event
    kal_event_clear(event);
    
    return event;
}

void kal_event_set(uint8_t event)
{
    g_events |= (1 << event);
}

void kal_event_clear(uint8_t event)
{
    g_events &=~ (1 << event);
}
