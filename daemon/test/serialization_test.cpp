#include <random>
#include <assert.h>
#include "serialization.h"

#define MAX_NUM_ELEMENTS 5

std::mt19937 random_number_generator;
std::uniform_int_distribution<int> value_distribution(0, 100);
std::uniform_int_distribution<int> element_number_distribution(0, MAX_NUM_ELEMENTS);

int random_value() {
    return value_distribution(random_number_generator);
}

int random_number_of_elements() {
    return element_number_distribution(random_number_generator);
}

hhal::gn_kernel random_kernel() {
    hhal::gn_kernel k;
    k.id = random_value();
    k.cluster_id = random_value();
    k.mem_tile = random_value();
    k.physical_addr = random_value();
    k.size = random_value();
    k.termination_event = random_value();
    k.unit_id = random_value();
    k.task_events = std::vector<int>();
    for (int i = 0; i < random_number_of_elements(); i++) {
        k.task_events.push_back(random_value());
    }
    return k;
}



int main(int argc, char const *argv[])
{
    hhal::gn_kernel kernel = random_kernel();
    hhal_daemon::serialized_object obj = hhal_daemon::serialize(kernel);
    hhal::gn_kernel deserialized_kernel = hhal_daemon::deserialize_gn_kernel(obj);

    assert(kernel.id == deserialized_kernel.id && "Kernel ids are different");
    assert(kernel.cluster_id == deserialized_kernel.cluster_id && "Kernel cluster_ids are different");
    assert(kernel.mem_tile == deserialized_kernel.mem_tile && "Kernel mem tiles are different");
    assert(kernel.physical_addr == deserialized_kernel.physical_addr && "Kernel physical addresses are different");
    assert(kernel.size == deserialized_kernel.size && "Kernel sizes are different");
    assert(kernel.termination_event == deserialized_kernel.termination_event && "Kernel termination events are different");
    assert(kernel.unit_id == deserialized_kernel.unit_id && "Kernel unit ids are different");
    assert(kernel.task_events.size() == deserialized_kernel.task_events.size() && "Kernel task events have different number of elements");
    for (int i = 0; i < kernel.task_events.size(); i++) {
        assert(kernel.task_events[i] == deserialized_kernel.task_events[i] && "A kernel element is different");
    }

    printf("Kernel serialization performed correctly!\n");
    
    return 0;
}
