#ifndef MM_H
#define MM_H

#include <vector>
#include <map>

#include "gn/gn/mango_types.h"
#include "gn/types.h"
#include "gn/manager.h"

/*! \file mm.h
 * \brief Local memory manager for virtual address resolution
 */ 

using namespace mango;

namespace hhal {

/*! \brief Stub of a Memory Manager
 * Currently, it only manages in a very rough way the virtual addresses
 */
class MM {


public:
	MM() noexcept;

	~MM();


	mango_exit_code_t set_vaddr_kernels(GNManager &manager, std::vector<gn_kernel> &kernels) noexcept;
		
	mango_exit_code_t set_vaddr_buffers(GNManager &manager, std::vector<gn_buffer> &buffers) noexcept;

	mango_exit_code_t set_vaddr_events(GNManager &manager, std::vector<gn_event> &events) noexcept;


private:

	std::map<mango_id_t, int> entries;
	std::map<mango_id_t, mango_addr_t> virtual_address_pool;    /** This is used to keep track of
                                                                    used virtual addresses of buffers.
                                                                    It maps kernel id to next free
                                                                    virtual address. */

	void set_tlb_kb(GNManager &manager, mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
			mango_size_t size, mango_addr_t phy_addr, int entry,
			uint32_t cluster_id) const noexcept;

	virtual void set_buff_tlb(GNManager &manager, gn_kernel &k, gn_buffer &b) noexcept;
	virtual void set_event_tlb(GNManager &manager, gn_kernel &k, gn_event &e) noexcept;

};

}
#endif /* MM */
