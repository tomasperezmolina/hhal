#include <random>
#include <assert.h>
#include "serialization.h"
#include <cstring>

#define REPETITIONS 5
#define MAX_NUM_ELEMENTS 5

std::mt19937 random_number_generator;
std::uniform_int_distribution<int> value_distribution(0, 100);
std::uniform_int_distribution<int> element_number_distribution(1, MAX_NUM_ELEMENTS);

int random_value() {
    return value_distribution(random_number_generator);
}

int random_number_of_elements() {
    return element_number_distribution(random_number_generator);
}

void test_arguments() {
    for (int i = 0; i < REPETITIONS; i++) {
        hhal::Arguments args;

        int scalar_amount = random_number_of_elements();
        for (int i = 0; i < scalar_amount; i++) {
            hhal::scalar_arg scalar = {hhal::ScalarType::INT, sizeof(int)} ;
            scalar.aint32 = random_value(); 
            args.add_scalar(scalar);
        }

        for (int i = 0; i < random_number_of_elements(); i++) {
            args.add_event({random_value()});
        }

        for (int i = 0; i < random_number_of_elements(); i++) {
            args.add_buffer({random_value()});
        }

        hhal_daemon::serialized_object obj = hhal_daemon::serialize(args);
        hhal_daemon::auxiliary_allocations aux;
        hhal::Arguments deserialized_args = hhal_daemon::deserialize_arguments(obj, aux);

        assert(args.get_args().size() == deserialized_args.get_args().size() && "Arguments objects have different number of arguments");
        for(int i = 0; i < args.get_args().size(); i++) {
            hhal::arg original = args.get_args()[i];
            hhal::arg deserialized = deserialized_args.get_args()[i];
            assert(original.type == deserialized.type && "Arg objects have different types");
            switch (original.type) {
                case hhal::ArgumentType::SCALAR: {
                    hhal::scalar_arg original_scalar = original.scalar;
                    hhal::scalar_arg deserialized_scalar = deserialized.scalar;
                    assert(original_scalar.type == deserialized_scalar.type && "Scalar arguments have different types");
                    assert(original_scalar.size == deserialized_scalar.size && "Scalar arguments have different sizes");
                    assert(original_scalar.aint32 == deserialized_scalar.aint32 && "Scalar arguments have different values");
                    break;
                }
                case hhal::ArgumentType::BUFFER: {
                    hhal::buffer_arg original_buffer = original.buffer;
                    hhal::buffer_arg deserialized_buffer = deserialized.buffer;
                    assert(original_buffer.id == deserialized_buffer.id && "Buffer arguments have different ids");
                    break;
                }
                case hhal::ArgumentType::EVENT: {
                    hhal::event_arg original_event = original.event;
                    hhal::event_arg deserialized_event = deserialized.event;
                    assert(original_event.id == deserialized_event.id && "Event arguments have different ids");
                    break;
                }
            }
        }
    }
    printf("Arguments serialization performed correctly!\n");
}



int main(int argc, char const *argv[])
{
    test_arguments();
    
    return 0;
}
