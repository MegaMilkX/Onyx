#include "event.h"

std::map<typeindex, std::set<event_dispatcher_base*>> dispatcher_storage::dispatchers;