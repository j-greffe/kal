#include "hal.h"
#include "kal_event.h"

static uint8_t g_events[32] = {0};
static uint8_t g_nb_events = 0;

void kal_event_open(uint8_t nb_events)
{
    g_nb_events = nb_events;
}

uint8_t kal_event_wait(uint8_t event)
{
    // Wait for any event
    if (KAL_EVENT_ALL == event)
    {
        uint8_t i = 0;
        while (1)
        {
            if (g_events[i])
            {
                event = i;
                break;
            }

            i++;

            if (i >= g_nb_events)
            {
                i = 0;
                hal_lpm_enter(LPM_0);
            }
        }
    }
    // Wait for specific event
    else
    {
        while (!(g_events[event]))
        {
            // TODO: Go to low power
        }
    }
    
    // Clear this event
    kal_event_clear(event);
    
    return event;
}

void kal_event_set(uint8_t event)
{
    if (g_events[event] < 0xFF) g_events[event]++;
}

void kal_event_clear(uint8_t event)
{
    if (g_events[event]) g_events[event]--;
}
