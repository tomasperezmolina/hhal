#include "rm_common.h"

static int event_id_gen = 0;

namespace rm_common {
    int get_new_event_id() {
        return event_id_gen++;
    }
}
