#include "event.h"

class Event_Item_t {
public:
    int8_t     id;
    EventMsg_t msg;
};

static osMessageQId eventId;

bool MX_Event_Init()
{
    osMessageQDef(event, 16, Event_Item_t*);
    eventId = osMessageCreate(osMessageQ(event), osThreadGetId());
    return true;
}

bool MX_Event_Peek()
{
    osEvent evt = osMessagePeek(eventId, 0);
    if (evt.status != osEventMessage)
        return false;
    return true;
}

bool MX_Event_Get(EventId_t& id, EventMsg_t& msg, uint32_t timeout)
{
    osEvent evt = osMessageGet(eventId, timeout);
    if (evt.status != osEventMessage)
        return false;
    auto* ptr = static_cast<Event_Item_t*>(evt.value.p);
    if (!EventId_t::_is_valid(ptr->id))
        id = EventId_t::null;
    else
        id = EventId_t::_from_integral_unchecked(ptr->id);
    msg = ptr->msg;
    delete ptr;
    return true;
}

bool MX_Event_Put(const EventId_t id, const EventMsg_t msg, uint32_t timeout)
{
    Event_Item_t* ptr = new Event_Item_t;
    ptr->id           = id._to_integral();
    ptr->msg          = msg;
    osMessagePut(eventId, reinterpret_cast<uint32_t>(ptr), timeout);
    return true;
}
