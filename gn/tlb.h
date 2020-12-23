/*! \file
 *  \brief Synchronization events
 */
#ifndef TLB_H
#define TLB_H

#include "gn/gn/mango_types.h"
#include "gn/types.h"

#include <map>

using namespace mango;

namespace hhal {

/*! \brief HN Event descriptor 
 *
 * These events are also used at the GN level.
 */
class TLB {
public:	

	TLB() noexcept {}

	inline mango_size_t get_virt_addr(const gn_buffer &buffer) const { 
		return this->tlb_buffers.at(buffer.id);
	}

	inline mango_size_t get_virt_addr(const gn_event &event) const {
		return this->tlb_events.at(event.id);
	}

	inline mango_size_t get_virt_addr_buffer(const mango_id_t buffer_id) const { 
		return this->tlb_buffers.at(buffer_id);
	}

	inline mango_size_t get_virt_addr_event(const mango_id_t event_id) const {
		return this->tlb_events.at(event_id);
	}

	inline mango_size_t get_virt_addr_kernel(const mango_id_t kernel_id) const {
		return this->tlb_kernels.at(kernel_id);
	}

	inline void set_virt_addr(const gn_buffer &buffer, mango_size_t virt_addr) noexcept {
		this->tlb_buffers[buffer.id] = virt_addr;
	}

	inline void set_virt_addr(const gn_event &event, mango_size_t virt_addr)   noexcept {
		this->tlb_events[event.id] = virt_addr;
	}

	inline void set_virt_addr(const gn_kernel &kernel, mango_size_t virt_addr)   noexcept {
		this->tlb_events[kernel.id] = virt_addr;
	}


private:
	std::map<mango_id_t, mango_size_t> tlb_buffers; /*!< map addresses of buffers */
	std::map<mango_id_t, mango_size_t> tlb_events;  /*!< map addresses of events */
	// libmango originally does not have this here, but there seems to be no reason to have the virtual address on the kernel and not here
	// In the future we probably remove the TLB entirely
	std::map<mango_id_t, mango_size_t> tlb_kernels;  /*!< map addresses of kernels */ 

};


}
#endif /* TLB_H */
