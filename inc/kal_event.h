#ifndef _KAL_EVENT_H_
#define _KAL_EVENT_H_

#include <stdint.h>

#define KAL_EVENT_ALL 0xFF

void kal_event_open(uint8_t nb_events);
uint8_t kal_event_wait(uint8_t event);
void kal_event_set(uint8_t event);
void kal_event_clear(uint8_t event);

#endif // _KAL_EVENT_H_
