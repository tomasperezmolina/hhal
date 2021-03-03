#ifndef __HN_RESOURCE_MANAGER_H__
#define __HN_RESOURCE_MANAGER_H__
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (c) Copyright 2012 - 2018  Parallel Architectures Group (GAP)
// Department of Computing Engineering (DISCA)
// Universitat Politecnica de Valencia (UPV)
// Valencia, Spain
// All rights reserved.
//
// All code contained herein is, and remains the property of
// Parallel Architectures Group. The intellectual and technical concepts
// contained herein are proprietary to Parallel Architectures Group and
// are protected by trade secret or copyright law.
// Dissemination of this code or reproduction of this material is
// strictly forbidden unless prior written permission is obtained
// from Parallel Architectures Group.
//
// THIS SOFTWARE IS MADE AVAILABLE "AS IS" AND IT IS NOT INTENDED FOR USE
// IN WHICH THE FAILURE OF THE SOFTWARE COULD LEAD TO DEATH, PERSONAL INJURY,
// OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE.
//
// contact: jflich@disca.upv.es
//---------------------------------------------------------------------------------------------------------------------
//
// Company:   GAP (UPV)
// Engineer:  J. Flich (jflich@disca.upv.es)
//
// Create Date: January 25, 2018
// File Name: hn_resource_manager.c
// Design Name:
// Module Name: Heterogeneous node
// Project Name:
// Target Devices:
// Tool Versions:
// Description:
//
//        Provides resource management functionality and keeps resource configuration
//
//
// Dependencies: NONE
//
// Revision:
//   Revision 0.01 - File Created
//
// Additional Comments: NONE
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#define HN_RSCMGT_MAX_TILES  256
#define HN_RSCMGT_MAX_INTS   256
#define HN_RSCMGT_MAX_DMAS   256

typedef enum
{
  HN_RESOURCE_MANAGER_INIT_MODE_NO_FPGA = 0,
  HN_RESOURCE_MANAGER_INIT_MODE_FPGA
}hn_rscmgt_init_mode_t;

typedef enum
{
  HN_RESOURCE_MANAGER_STATUS_NOT_INITIALIZED = 0,
  HN_RESOURCE_MANAGER_STATUS_RESETING,
  HN_RESOURCE_MANAGER_STATUS_RESET_COMPLETED,
  HN_RESOURCE_MANAGER_STATUS_GETTING_ARCH_ID,
  HN_RESOURCE_MANAGER_STATUS_RUNNING
} hn_rscmgt_status_t;

// structures for the resouce manager
typedef struct hn_rscmgt_memory_slot_st
{
  unsigned long long       size;
  unsigned int             start_address;
  unsigned int             end_address;
  void                    *next_memory_slot;
} hn_rscmgt_memory_slot_t;

typedef struct hn_rscmgt_tile_info_st
{
  unsigned int             type;                       // unit type
  unsigned int             subtype;                    // subtype or specific configuration id of the unit
  unsigned int             num_cores;                  // number of cores of the unit
  unsigned int             assigned;                   // whether this tile is assigned or not
  unsigned int             preassigned;                // for search tiles operation
  unsigned long long       memory_size;                // memory size (0 if not present)
  unsigned long long       free_memory;                // memory available
  unsigned long long       read_memory_bw;             // read memory bandwidth
  unsigned long long       avail_read_memory_bw;       // available read memory bandwidth
  unsigned long long       write_memory_bw;            // write memory bandwidth
  unsigned long long       avail_write_memory_bw;      // available write memory bandwidth
  unsigned long long       north_port_bw;              // north port bandwidth
  unsigned long long       avail_north_port_bw;        // available north port bandwidth
  unsigned long long       west_port_bw;               //
  unsigned long long       avail_west_port_bw;         //
  unsigned long long       east_port_bw;               //
  unsigned long long       avail_east_port_bw;         //
  unsigned long long       south_port_bw;              //
  unsigned long long       avail_south_port_bw;        //
  unsigned long long       local_port_bw;              //
  unsigned long long       avail_local_port_bw;        //
  hn_rscmgt_memory_slot_t *first_memory_slot;          // free memory available for assignment
} hn_rscmgt_tile_info_t;


typedef struct hn_rscmgt_info_st
{
  unsigned int          arch_id;                 // architecture id
  unsigned int          num_tiles;               // number of tiles in the HN cluster
  unsigned int          num_rows;                // number of rows of tiles in the HN cluster
  unsigned int          num_cols;                // number of columns of tiles in the HN cluster
  unsigned int          num_vns;                 // number of virtual networks in the HN cluster
  hn_rscmgt_tile_info_t tile_info[HN_RSCMGT_MAX_TILES];    // info for each tile
  unsigned long long    read_cluster_bw;         // read cluster bandwidth (FPGA->Host)
  unsigned long long    avail_read_cluster_bw;   // available read cluster bandwidth
  unsigned long long    write_cluster_bw;        // write cluster bandwidth (Host->FPGA)
  unsigned long long    avail_write_cluster_bw;  // available write cluster bandwidth
} hn_rscmgt_info_t;

typedef struct hn_rscmgt_registered_interrupt_st
{
  uint8_t  in_use;
  uint32_t client_id;
  uint32_t tile;
  uint16_t vector_mask;
  uint16_t received_interrupts;
  uint16_t wait_on_interrupts;
} hn_rscmgt_registered_interrupt_t;

typedef struct hn_rscmgt_registered_dma_st
{
  uint8_t  in_use;
  uint32_t  client_id;
  uint32_t  tile_src;
  uint32_t  addr_src;
  unsigned long long size;
  uint32_t  tile_dst;
  uint32_t  addr_dst;
  uint32_t  to_unit;
  uint32_t  to_mem;
  uint32_t  to_ext;
  uint32_t  notify;
  uint32_t  completed;
} hn_rscmgt_registered_dma_t;
// -------------------------------------

/*
 * \brief This function initializes the resource manager
 */
void hn_rscmgt_initialize(hn_rscmgt_init_mode_t mode);

/*
 * \brief This function ends the resource manager
 */
void hn_rscmgt_end();

/*!
 * \brief Sets the reset status
 */
void hn_rscmgt_reset();

/*!
 * \brief Gets the resource manager lock
 */
uint32_t hn_rscmgt_get_lock();

/*!
 * \brief Sets the resource manager lock
 */
void hn_rscmgt_set_lock();

/*!
 * \brief Reset the resource manager lock
 */
void hn_rscmgt_reset_lock();




/*!
 * \brief Gets the underlaying architecture Id
 *
 * \returns the arch id or -1 if not architecture initialized
 */
int hn_rscmgt_get_arch_id();

/*!
 * \brief Sets the architecture id
 */
void hn_rscmgt_set_arch_id(unsigned int arch_id);


/*!
 * \brief Get the current status
 */
hn_rscmgt_status_t hn_rscmgt_get_status();


/*!
 * \brief Gets the info structure
 *
 * \return a constant data pointer to the internal structure
 */
const hn_rscmgt_info_t *hn_rscmgt_get_info();

/*
 * \brief This function delivers information about the closest memory (to a tile) with the available memory size
 */
uint32_t hn_rscmgt_find_memory(uint32_t tile, unsigned long long size, uint32_t *tile_mem, uint32_t *starting_addr);

/*
 * \brief This function delivers information about the memories with the available memory size
 */
void hn_rscmgt_find_memories(unsigned long long size, uint32_t **tile_mem, uint32_t **starting_addr, uint32_t *num_memories);

/*
 * \brief This function allocates a memory segment
 */
uint32_t hn_rscmgt_allocate_memory(uint32_t tile, uint32_t addr, unsigned long long size);

/*
 * \brief This function releases a memory segment
 */
uint32_t hn_rscmgt_release_memory(uint32_t tile, uint32_t addr, unsigned long long size);

/*
 * \brief This function delivers the available network bandwidth between two tiles
 */
unsigned long long hn_rscmgt_get_available_network_bandwidth(uint32_t tile_src, uint32_t tile_dst);

/*
 * \brief This function delivers the available read memory bandwidth in a tile
 */
unsigned long long hn_rscmgt_get_available_read_memory_bandwidth(uint32_t tile);

/*
 * \brief This function delivers the available write memory bandwidth in a tile
 */
unsigned long long hn_rscmgt_get_available_write_memory_bandwidth(uint32_t tile);

/*
 * \brief This function delivers the available read cluster bandwidth
 */
unsigned long long hn_rscmgt_get_available_read_cluster_bandwidth();

/*
 * \brief This function delivers the available write cluster bandwidth
 */
unsigned long long hn_rscmgt_get_available_write_cluster_bandwidth();

/*
 * \brief This function reserves network bandwidth between two tiles
 */
uint32_t hn_rscmgt_reserve_network_bandwidth(uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);

/*
 * \brief This function reserves read memory bandwidth in a tile
 */
uint32_t hn_rscmgt_reserve_read_memory_bandwidth(uint32_t tile, unsigned long long bw);

/*
 * \brief This function reserves write memory bandwidth in a tile
 */
uint32_t hn_rscmgt_reserve_write_memory_bandwidth(uint32_t, unsigned long long bw);

/*
 * \brief This function reserves read cluster bandwidth
 */
uint32_t hn_rscmgt_reserve_read_cluster_bandwidth(unsigned long long bw);

/*
 * \brief This function reserves write cluster bandwidth
 */
uint32_t hn_rscmgt_reserve_write_cluster_bandwidth(unsigned long long bw);

/*
 * \brief This function releases network bandwidth between two tiles
 */
uint32_t hn_rscmgt_release_network_bandwidth(uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);

/*
 * \brief This function releases read memory bandwidth in a tile
 */
uint32_t hn_rscmgt_release_read_memory_bandwidth(uint32_t tile, unsigned long long bw);

/*
 * \brief This function releases write memory bandwidth in a tile
 */
uint32_t hn_rscmgt_release_write_memory_bandwidth(uint32_t, unsigned long long bw);

/*
 * \brief This function releases read cluster bandwidth
 */
uint32_t hn_rscmgt_release_read_cluster_bandwidth(unsigned long long bw);

/*
 * \brief This function releases write cluster bandwidth
 */
uint32_t hn_rscmgt_release_write_cluster_bandwidth(unsigned long long bw);

/*
 * \brief This function returns a set of tiles according to the requested types and number of tiles
 */
uint32_t hn_rscmgt_find_units_set(uint32_t tile, uint32_t num_tiles, uint32_t types[], uint32_t *tiles_dst, uint32_t *types_dst, uint32_t reset_preassignments, uint32_t external_use);

/*
 * \brief This function returns all possible (disjoint) set of tiles according to the request types and number of tiles
 */
uint32_t hn_rscmgt_find_units_sets(uint32_t tile, uint32_t num_tiles, uint32_t types[], uint32_t ***tiles_dst, uint32_t ***types_dst, uint32_t *num);

/*
 * \brief This function reserves a set of tiles
 */
void hn_rscmgt_reserve_units_set(uint32_t num_tiles, uint32_t *tiles);

/*
 * \brief This function releases a set of tiles
 */
void hn_rscmgt_release_units_set(uint32_t num_tiles, uint32_t *tiles);

/*
 * \brief This function delivers the number of tiles
 */
void hn_rscmgt_get_num_tiles(uint32_t *num_tiles, uint32_t *num_tiles_x, uint32_t *num_tiles_y);

/*!
 * \brief This function delivers the memory size for a tile or 0 if there is not any memory attached to
 */
void hn_rscmgt_get_memory_size(uint32_t tile, uint32_t *mem_size);

/*!
 * \brief This function delivers the number of vns
 */
void hn_rscmgt_get_num_vns(uint32_t *num_vns);

/*!
 * \brief This function delivers the tile info for a tile
 */
void hn_rscmgt_get_tile_info(uint32_t tile, uint32_t *tile_type, uint32_t *tile_subtype, uint32_t *mem_size);


/*
 * \brief This function registers an interrupt service
 */
uint32_t hn_rscmgt_register_int(uint32_t tile, uint16_t vector_mask, uint32_t client_id, uint32_t *id);

/*
 * \brief This function provides associated info to an interrupt entry
 */
uint32_t hn_rscmgt_get_interrupt_info(uint32_t id, uint32_t *tile, uint16_t *vector);


/*
 * \brief This function releases an interrupt entry
 */
uint32_t hn_rscmgt_release_int(uint32_t id);

/*
 * \brief This function provides whether an interrupt associated to an entry and vector has been received
 */
uint32_t hn_rscmgt_received_interrupt(uint32_t id, uint16_t vector);

/*
 * \brief This function cancels an interrupt associated to an entry and vector has been received
 */
void hn_rscmgt_cancel_interrupt(uint32_t id, uint16_t vector);

/*
 * \brief This function annotates a wait on an interrupt associated to an entry and vector has been received
 */
void hn_rscmgt_annotate_wait_on_interrupt(uint32_t id, uint16_t vector);

/*
 * \brief This functions returns the client that is waiting for a given interrupt
 */
uint32_t hn_rscmgt_find_waiting_client_to_interrupt(uint32_t tile, uint16_t vector_mask, uint32_t *client_id, uint32_t *id);

/*
 * \brief This function releases an interrupt entry
 */
uint32_t hn_rscmgt_release_int(uint32_t id);

/*
 * \brief This function registers a DMA service
 */
uint32_t hn_rscmgt_register_dma(uint32_t *id, uint32_t client_id);

/*
 * \brief This function writes the dma data into an entry
 */
uint32_t hn_rscmgt_write_dma_operation(uint32_t id, uint32_t tile_src, uint32_t addr_src, unsigned long long size, uint32_t tile_dst, uint32_t addr_dst, uint32_t to_unit, uint32_t to_mem, uint32_t to_ext, uint32_t notify);

/*
 * \brief This function finds a client waiting on a completion of a DMA device
 */

uint32_t hn_rscmgt_find_waiting_client_to_dma(uint32_t tile, uint32_t *client_id, uint32_t *id);

/*
 * \brief This function annotates a wait on a dma
 */
void hn_rscmgt_annotate_wait_on_dma(uint32_t id, uint32_t value);

/*
 * \brief This function releases a DMA entry
 */
uint32_t hn_rscmgt_release_dma(uint32_t id);


#endif
