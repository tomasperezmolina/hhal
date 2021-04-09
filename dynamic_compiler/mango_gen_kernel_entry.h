#include <string>
#include "types.h"

/**
 * Generates an entrypoint for a kernel.
 * Requires the pragma: #pragma mango_gen_entrypoint
 *  which needs to be present before the mango_kernel pragma.
 * Returns true if an entrypoint was generated.
 */
bool generate_entrypoint(std::string file_path, std::string output_path, hhal::Unit type);
