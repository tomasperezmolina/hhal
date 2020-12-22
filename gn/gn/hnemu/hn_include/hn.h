#ifndef __HN_H__
#define __HN_H__
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (c) Copyright 2012 - 2017  Parallel Architectures Group (GAP)
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
// Engineer:  J. Martinez (jomarm10@gap.upv.es)
//
// Create Date: february 21, 2017
// Design Name:
// Module Name: Heterogeneous node library
// Project Name: MANGO
// Target Devices:
// Tool Versions:
// Description:
//
//    hn library
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

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************************************************************
 **
 ** Included files
 **
 **********************************************************************************************************************
 **/
#include <stdint.h>

#include "hn_filter.h"
#include "hn_peak.h"
#include "hn_nuplus.h"
#include "hn_dct.h"
#include "hn_tetrapod.h"

/**********************************************************************************************************************
 **
 ** macros
 **
 **********************************************************************************************************************
 **/

/*!
 * \brief Defines for maximum values, this helps in defining structures sizes
 */
#define HN_MAX_VNS            10        //<! maximum number of virtual networks supported
#define HN_MAX_TILES         256        //<! maximum number of tiles supported
#define HN_MAX_ROUTER_PORTS    4        //<! maximum number of router ports (NEWS)
#define HN_MAX_LOCAL_PORTS     6        //<! maximum number of local ports (NIr NIw Ur Uw MCr MCw)
#define HN_MAX_RSC            10        //<! maximum number of computing (and memory buffer) resources requested
#define HN_MAX_REGISTERS      32        //<! maximum number of registers in TILEREG structures
#define HN_MAX_SAVED_REQ      30        //<! maximum number of saved request

/*!
 * Unit family types available in MANGO
 */
#define HN_TILE_FAMILY_NONE         255
#define HN_TILE_FAMILY_PEAK         0
#define HN_TILE_FAMILY_NUPLUS       1
#define HN_TILE_FAMILY_DCT          2
#define HN_TILE_FAMILY_TETRAPOD     3

/*!
 * \brief Identify a None Model for units without model classification
 */
#define HN_TILE_MODEL_NONE          255


/*!
 * \brief Partition strategies that can manage the HN library
 */
#define NO_PARTITION_STRATEGY   0
#define UPV_PARTITION_STRATEGY  1

/*!
 * \brief Ids for Tiles
 */
#define HN_TILE_0     0
#define HN_TILE_1     1
#define HN_TILE_2     2
#define HN_TILE_3     3
#define HN_TILE_4     4
#define HN_TILE_5     5
#define HN_TILE_6     6
#define HN_TILE_7     7
#define HN_TILE_8     8
#define HN_TILE_9     9
#define HN_TILE_10    10
#define HN_TILE_11    11
#define HN_TILE_12    12
#define HN_TILE_13    13
#define HN_TILE_14    14
#define HN_TILE_15    15
#define HN_TILE_16    16
#define HN_TILE_17    17
#define HN_TILE_18    18
#define HN_TILE_19    19
#define HN_TILE_20    20
#define HN_TILE_21    21
#define HN_TILE_22    22
#define HN_TILE_23    23
#define HN_TILE_24    24
#define HN_TILE_25    25
#define HN_TILE_26    26
#define HN_TILE_27    27
#define HN_TILE_28    28
#define HN_TILE_29    29
#define HN_TILE_30    30
#define HN_TILE_31    31
#define HN_TILE_32    32
#define HN_TILE_33    33
#define HN_TILE_34    34
#define HN_TILE_35    35
#define HN_TILE_36    36
#define HN_TILE_37    37
#define HN_TILE_38    38

/*!
 * \brief Architecture Register Id
 */
#define HN_ARCH_ID_REGISTER                0
#define HN_MASK_ARCH_ID                    0
#define HN_SHIFT_ARCH_ID                   0


/*!
 * \brief Register Ids
 */
#define HN_MANGO_TILEREG_CONF              0
#define HN_MANGO_TIMESTAMP_LOW_REG         1
#define HN_MANGO_TIMESTAMP_HIGH_REG        2
#define HN_MANGO_CONNECTIVITY_REG          3
#define HN_MANGO_MC_ACCESS_REG             4
#define HN_MANGO_TILEREG_TO_UNIT           5
#define HN_MANGO_MS_INJECT_DST_REG         7
#define HN_MANGO_MS_INJECT_OTH_REG         8
#define HN_MANGO_TILEREG_WEIGHTS           9
#define HN_MANGO_TILEREG_NETSTATS          10
#define HN_MANGO_TILEREG_TLB               11
#define HN_MANGO_TILEREG_TEMPERATURE       12
#define HN_MANGO_TILEREG_SYNCH0            16              // SYNCH registers start at 16 and end at 31




/*!
 * \brief Router ports
 */
#define HN_NORTH_PORT                      0
#define HN_EAST_PORT                       1
#define HN_WEST_PORT                       2
#define HN_SOUTH_PORT                      3
#define HN_LOCAL_PORT                      4

/*!
 * \brief HN lib return values for functions
 */
#define HN_SUCCEEDED                         0
#define HN_DEVICE_NOT_FOUND                  1
#define HN_NOT_INITIALIZED                   2
#define HN_TILE_DOES_NOT_EXIST               3
#define HN_PARTITION_NOT_FOUND               4
#define HN_PARTITION_NOT_DEFINED             5
#define HN_ALTERNATIVE_PARTITION_NOT_FOUND   6
#define HN_MEMORY_NOT_PRESENT_IN_TILE        7
#define HN_REGISTER_DOES_NOT_EXIST           8
#define HN_PEAK_NOT_FOUND_IN_TILE            9
#define HN_NUPLUS_NOT_FOUND_IN_TILE          10
#define HN_ACCELERATOR_NOT_FOUND_IN_TILE     11
#define HN_SYNCH_RESOURCE_DOES_NOT_EXIST     12
#define HN_REGISTERS_DOES_NOT_EXIST          13
#define HN_WRITE_ONLY_REGISTER               14
#define HN_READ_ONLY_REGISTER                15
#define HN_WRONG_BANDWIDTH_SETTING           16
#define HN_ARCHITECTURE_NOT_FOUND            17
#define HN_TILE_TYPE_NOT_RECOGNIZED          18
#define HN_UNALIGNED_READ                    19
#define HN_IMAGE_FILE_NOT_FOUND              20
#define HN_PROTOCOL_FILE_NOT_FOUND           21
#define HN_WRONG_REGISTER_READ               22
#define HN_SOCKET_ERROR                      23
#define HN_ARCHITECTURE_NOT_SUPPORTED        24
#define HN_READ_REGISTER_ACCESS_NOT_ALLOWED  25
#define HN_NO_FREE_REGISTERS_AVAILABLE       26
#define HN_NOT_ENOUGH_MEMORY_AVAILABLE       27
#define HN_NOT_ENOUGH_PARTITIONS_AVAILABLE   28
#define HN_UNKNOWN_PARTITION_STRATEGY        29
#define HN_WRONG_REGISTER_TYPE               30
#define HN_WRONG_ARGUMENTS                   31
#define HN_FIND_MEMORY_ERROR                 32
#define HN_RSC_ERROR                         33
#define HN_DMA_ERROR                         34

/*!
 * \brief Types of synchronization registers
 */
#define HN_READRESET_REG_TYPE               0
#define HN_READRESET_INCRWRITE_REG_TYPE     1
#define HN_REGULAR_REG_TYPE                 2

/**********************************************************************************************************************
 **
 ** Data structures
 **
 **********************************************************************************************************************
 **/

/*!
 * \brief Tile information
 */
typedef struct {
  uint32_t unit_family;        //!< The family of the unit in the tile (PEAK, NUPLUS, DCT...)
  uint32_t unit_model;         //!< Implemented model of the unit in the tile (PEAK_0...)
  uint32_t memory_attached;    //1< memory attached to the tile (0 = no memory attached)
} hn_tile_info_t;


/*!
 * \brief Tile stats
 */
typedef struct hn_tile_stats_st {
  uint32_t flits_ejected_north[HN_MAX_VNS];   //!< flits counter for north port
  uint32_t flits_ejected_east[HN_MAX_VNS];    //!< flits counter for east port
  uint32_t flits_ejected_west[HN_MAX_VNS];    //!< flits counter for west port
  uint32_t flits_ejected_south[HN_MAX_VNS];   //!< flits counter for south port
  uint32_t flits_ejected_local[HN_MAX_VNS];   //!< flits counter for local port
  uint32_t temperature;                       //!< temperature
  uint32_t unit_utilization;                  //!< performance counter
}hn_tile_stats_t;


/**********************************************************************************************************************
 **
 ** Public function prototypes
 **
 **********************************************************************************************************************
 **/

// --------------------------------------------------------------------------------------------------------------------
// Functions to initialize and deinitialize the library
// --------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// hn_initialize function.
//   short: This function initializes the library. As a first action, it tries to connect
//          to the daemon server. Then, it gets the architecture ID and initializes all
//          the configuration structures
//   argument filter: Filter to be used when connecting to the daemon server
//   argument partition_strategy: partition strategy to be used
//   argument socket_connection: \deprecated
//   argument reset: if set to 0, the system will not perform a reset, otherwise a reset will be done \deprecated
//   argument uint32_t capture_signals_enable: if set to 0, library will not capture terminal signals (to terminate lib in a controlled manner)
uint32_t hn_initialize(hn_filter_t filter, uint32_t partition_strategy, uint32_t socket_connection, uint32_t reset, uint32_t capture_signals_enable);

// -------------------------------------------------------------------------------------------------
// hn_end function. This functions deletes all the internal structures and concludes the library
//   short: Ends the library. As a single action it disconnects from the daemon server
//
uint32_t hn_end();

// ---------------------------------------------------------------------------------------------------
// hn_load_architecture
//   short: Loads an architecture on the FPGA system
//   argument arch_id: Architecture ID to be loaded
uint32_t hn_load_architecture(uint32_t arch_id);




// --------------------------------------------------------------------------------------------------------------------
// Functions to access registers
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------
// hn_read_register
//   short: Reads a registers from a tile
//   argument tile: Tile where the register is located
//   argument reg: Register to read
//   argument data: the value of the register is returned on this argument
uint32_t hn_read_register(uint32_t tile, uint32_t reg, uint32_t *data);

// --------------------------------------------------------------------------------------------------
// hn_write_register
//   short: Writes a register in a tile
//   argument tile: Tile where the register is located
//   argument reg: Register to write
//   argument data: the value to write on the register
uint32_t hn_write_register(uint32_t tile, uint32_t reg, uint32_t data);

// ---------------------------------------------------------------------------------------------------
// hn_receive_item
//   argument item: Item received
uint32_t hn_receive_item(uint32_t *item);




// --------------------------------------------------------------------------------------------------------------------
// Functions to retrieve configuration information and statistics
// --------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Function to get the architecture id
uint32_t hn_get_arch_id(uint32_t *arch_id);

// -------------------------------------------------------------------------------------------------
// hn_get_num_tiles
//   short: Returns the number of tiles in the HN system as well as the number of rows and columns
//   argument num_tiles: Pointer to the variable where the number of tiles is written
//   argument num_tiles_x: Pointer to the variable where the number in X dimension is written
//   argument num_tiles_y: Pointer to the variable where the number in Y dimension is written
uint32_t hn_get_num_tiles(uint32_t *num_tiles, uint32_t *num_tiles_x, uint32_t *num_tiles_y);

// -------------------------------------------------------------------------------------------------
// hn_get_num_vns
//   short: Returns the number of VNs present in the system
//   argument num_vns: Pointer to the variable where the number of VNs is written
uint32_t hn_get_num_vns(uint32_t *num_vns);

// ------------------------------------------------------------------------------------------------
// hn_get_memory_size
//   short: Returns the size (in bytes) of the memory attached to a given tile
//          If no memory attached the return value is 0
//   argument tile: The tile from where the functions returns the memory size
//   argument size: Pointer to the value returned
uint32_t hn_get_memory_size(uint32_t tile, uint32_t *size);

// -------------------------------------------------------------------------------------------------
// hn_get_tile_info
//   short: Returns the configuration info for a tile
//   argument tile: Tile from which info will be provided
//   argument data: Pointer to the variable where info will be written
uint32_t hn_get_tile_info(uint32_t tile, hn_tile_info_t *data);

// -------------------------------------------------------------------------------------------------
// hn_to_str_unit_family
//   short: Returns an string representation for the unit family given
//   argument family: The family the string representation will be for
const char *hn_to_str_unit_family(uint32_t family);

// -------------------------------------------------------------------------------------------------
// hn_to_str_unit_model
//   short: Returns an string representation for the unit model given
//   argument model: The model the string representation will be for
const char *hn_to_str_unit_model(uint32_t model);

// ------------------------------------------------------------------------------------------------
// hn_get_tile_temperature
//   short: Returns the temperature (in celcius) of the tile
//   argument tile: The tile from where the functions returns the memory size
//   argument temp: Pointer to the value returned
uint32_t hn_get_tile_temperature(uint32_t tile, float *temp);

// -------------------------------------------------------------------------------------------------
// hn_get_tile_stats
//   short: Returns the statistics for a tile
//   argument tile: Tile from which stats will be provided
//   argument data: Pointer to the variable where stats will be written

uint32_t hn_get_tile_stats(uint32_t tile, hn_tile_stats_t *data);

// -----------------------------------------------------------------------------------------------
// hn_get_router_port_bandwidth
//   short: Returns the bandwidth of a port in the router located in a given tile
//   argument tile: Tile where the router is located
//   argument port: Port to get bandwidth from
//   argument bandwidth: Pointer to the variable to return the bandwidth
uint32_t hn_get_router_port_bandwidth(uint32_t tile, uint32_t port, uint32_t *bandwidth);


// --------------------------------------------------------------------------------------------------------------------
// Functions to set router port bandwidths
// --------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// hn_set_router_port_vn_weights
//   short: Sets the router port bandwidth for each VN
//   argument tile: Tile where the router is located
//   argument port: Port to set the bandwidth
//   argument weights: Vector of weights, one for each possible vn
uint32_t hn_set_router_port_vn_weights(uint32_t tile, uint32_t port, uint32_t *weights);

// ------------------------------------------------------------------------------------------------
// hn_set_vn_weights_along_path
//   short: Sets the router port bandwidth for each VN along a path
//   argument src_tile: Source tile
//   argument dst_tile: Destination tile
//   argument weights: Vector of weights, one for each possible vn
//
//   Currently unused function!!
//   TODO TOMAS: verificar que el codi es correcte
uint32_t hn_set_vn_weights_along_path(uint32_t src_tile, uint32_t dst_tile, uint32_t *weights);


// --------------------------------------------------------------------------------------------------------------------
// Functions to manage TLBs
// --------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
// hn_set_tlb
//   short: Configures a TLB entry in a given tile
//   argument tile: Tile where the TLB is located
//   argument entry: Entry of the TLB
//   argument vaddr_ini: Initial virtual address
//   argument vaddr_end: Final virtual address
//   argument paddr: Initial Physical address
//   argument mem_res: If set indicates the TLB entry is for memory resource
//   argument tilereg_rsc: If set indicates the TLB entry is for TILEREG resource
//   argument reg_rsc: TILEREG register (when tilereg_rsc == 1)
//   argument tile_rsc: Tile where the resource (memory or TILEREG) is located
uint32_t hn_set_tlb(uint32_t tile, uint32_t entry, uint32_t vaddr_ini, uint32_t vaddr_end,
                  uint32_t paddr, uint32_t mem_rsc, uint32_t tilereg_rsc, uint32_t reg_rsc, uint32_t tile_rsc);



// --------------------------------------------------------------------------------------------------------------------
// Functions to boot the system, units and kernels
// --------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// hn_boot_unit
//   short: boots a unit located in a tile
//   argument tile: Tile where the unit is located
//   argument tile_memory: Tile where the memory image (if needed) is located
//   argument addr: Starting address of the memory image (if needed)
//   argument protocol_img_path: path of image file of memory coherence protocol
//   argument kernel_img_path: kernel image file path
uint32_t hn_boot_unit(uint32_t tile, uint32_t tile_memory, uint32_t addr, const char *protocol_img_path, const char *kernel_img_path);

// -----------------------------------------------------------------------------------------------------
// hn_load_kernel
//  short: loads a kernel image into a given tile and address
//  argument tile: Tile where the memory attached will be written
//  argument addr: Starting address for the kernel image in memory
//  argument kernel_size: size of the kernel in bytes
//  argument kernel_image: pointer to the kernel image buffer
uint32_t hn_load_kernel(uint32_t tile, uint32_t addr, uint32_t kernel_size, char *kernel_image);

// ---------------------------------------------------------------------------------------------------
// hn_run_kernel
//  short: sets a kernel to run into a given unit, the kernel image must be already loaded in
//         memory and the TLB properly configured
//  argument tile: Tile where the unit is located
//  argument addr: Virtual address where the image will be found by the unit
//  argument args: string including all arguments to be sent
uint32_t hn_run_kernel(uint32_t tile, uint32_t addr, char *args);




// --------------------------------------------------------------------------------------------------------------------
// Functions to access memory
// --------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// hn_write_memory
//   short: Writes a buffer into memory to a given memory located in a tile and from a starting address
//   argument tile: Tile where the memory is located
//   argument addr: Starting address to write to
//   argument size: Size of the write operation (in bytes)
//   argument data: pointer to the data to be written into memory
uint32_t hn_write_memory(uint32_t tile, uint32_t addr, uint32_t size, const char *data);

// -----------------------------------------------------------------------------------------------
// hn_write_image_into_memory
//   short: Writes a memory image located in a file into a given memory starting in a given address
//   argument file_name: Name of the file with the memory image to be written
//   argument tile: tile where the memory is located
//   argument addr: Starting address where the image will be copied to
uint32_t hn_write_image_into_memory(const char *file_name, uint32_t tile, uint32_t addr);

// -----------------------------------------------------------------------------------------------------
// hn_read_memory
//   short: reads memory data and puts it in a buffer
//          All reads must be block-aligned
//   argument tile: Tile where the memory attached will be read
//   argument addr: Address from where the memory will be accessed
//   argument size: Size of the data to be read (in bytes)
//   argument data: Pointer to the variable to put the data read
uint32_t hn_read_memory(uint32_t tile, uint32_t addr, uint32_t size, char *data);

// ---------------------------------------------------------------------------------------------
// hn_write_memory_byte
//   short: Writes one byte in a specific addres (not memory aligned) in a memory located in a given tile
//   argument tile: Tile where the memory is located
//   argument addr: Address where the write operation is performed
//   argument data: data to be written
uint32_t hn_write_memory_byte(uint32_t tile, uint32_t addr, uint32_t data) __attribute__((unused));

// ---------------------------------------------------------------------------------------------
// hn_write_memory_halfword
//   short: Writes 2 bytes in a specific addres (not memory aligned) in a memory located in a given tile
//   argument tile: Tile where the memory is located
//   argument addr: Address where the write operation is performed
//   argument data: data to be written
uint32_t hn_write_memory_halfword(uint32_t tile, uint32_t addr, uint32_t data) __attribute__((unused));

// ---------------------------------------------------------------------------------------------
// hn_write_memory_word
//   short: Writes one word (4 bytes) in a specific addres (not memory aligned) in a memory located in a given tile
//   argument tile: Tile where the memory is located
//   argument addr: Address where the write operation is performed
//   argument data: data to be written
uint32_t hn_write_memory_word(uint32_t tile, uint32_t addr, uint32_t data) __attribute__((unused));

// ---------------------------------------------------------------------------------------------
// hn_write_memory_block
//   short: Writes a memory block (64 bytes) in a block-aligned address in a memory located in a given tile
//   argument tile: Tile where the memory is located
//   argument addr: Address where the write operation is performed
//   argument data: Buffer with the block to be written
uint32_t hn_write_memory_block(uint32_t tile, uint32_t addr, const char *data);

// -----------------------------------------------------------------------------------------------------
// hn_read_memory_block
//   short: reads a block-aligned memory block
//   argument tile: Tile where the memory attached will be read
//   argument addr: Address from where the memory will be accessed
//   argument data: Pointer to the variable to put the data read
uint32_t hn_read_memory_block(uint32_t tile, uint32_t addr, char *data);

// --------------------------------------------------------------------------------------------------------------------
// Functions for semaphores implementations
// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// hn_get_synch_id
//   short: Returns an id associated with a synchronization resource (a TILEREG register) from a tile
//   argument id:   pointer to the value where the id will be returned to
//   argument tile: Tile where the register should be assigned (however a different tile may be used if no available ones)
//   argument type: Type of synchronization register
//
uint32_t hn_get_synch_id(uint32_t *id, uint32_t tile, uint32_t type);

// -----------------------------------------------------------------------------------------------
// hn_get_synch_id_array
//   short: Returns an id associated with a synchronization resource array (a set of consecutive TILEREG registers) from a tile
//   argument id:   pointer to the value where the id will be returned to
//   argument n:    number of registers to reserve
//   argument tile: Tile where the register should be assigned (however a different tile may be used if no available ones)
//   argument type: Type of registers to reserve
//
uint32_t hn_get_synch_id_array(uint32_t *id, uint32_t n, uint32_t tile, uint32_t type);

// ------------------------------------------------------------------------------------------------
// hn_release_synch_id
//   short: Releases a synch register
//   argument id: ID of the register to release
//
uint32_t hn_release_synch_id(uint32_t id);

// ------------------------------------------------------------------------------------------------
// hn_release_synch_id_array
//   short: Releases a synch register array
//   argument id: ID of the register to release
//   argument n:  number of consecutive registers to release
//
uint32_t hn_release_synch_id_array(uint32_t id, uint32_t n);

// ------------------------------------------------------------------------------------------------
// hn_read_synch_register
//   short: reads a synchronization register
//   argument id: ID of the register to read
//   argument v : pointer to the variable to write with the read data
uint32_t hn_read_synch_register(uint32_t id, uint32_t *v);

// ------------------------------------------------------------------------------------------------
// hn_write_synch_register
//   short: writes to a synchronization register
//   argument id: ID of the register to write
//            v : value to write on the register
//
uint32_t hn_write_synch_register(uint32_t id, uint32_t v);


// ------------------------------------------------------------------------------------------------
// Data burst transfers between FPGA DDR <-> Host Application
//
/*!
 * \brief Register a buffer in shared memory region
 *
 * \param[in] size in bytes of requested buffer
 * \param[out] ptr pointer to new created buffer in shared memory region (base address of the buffer)
 * \return 0 if the buffer has been succesfully allocated, otherwise return error code
 */
uint32_t hn_shm_malloc(uint32_t size, void **ptr);

/*!
 * \brief Release a buffer in shared memory region
 *
 * \param[in] ptr pointer to buffer in shared memory region (base address of the buffer)
 * \return 0 if the buffer has been succesfully released, otherwise return error code
 */
uint32_t hn_shm_free(void *ptr);

/*!
 * \brief Write the content of a buffer in shared memory region of the Host PC to DDR in HN system. High Performace Data Transfer by means of burst transfer functions
 *
 *The application has to provide the base address of the shared memory are buffer otherwise, the function will return an error
 *
 * \param[in] ptr to base address of the transfer, not necessarily the base address of the buffer
 * \param[in] size number of bytes to transfer (*ptr + size must be in the range of the space address of the buffer)
 * \param[in] address_dst base address at destination memory on HN system
 * \param[in] tile_dst tile where destination memory is attached
 * \param[in] blocking_transfer specifies if operation is blocking ( != 0 ) and the function does not return
 *            until the write operation is finished, or not ( 0 ) the application will have to check whether the operation is finished or not
 * \return 0 if the buffer has been succesfully released, otherwise return error code
 */
uint32_t hn_shm_write_memory(void *ptr, uint32_t size, uint32_t address_dst, uint32_t tile_dst, uint32_t blocking_transfer);

/*!
 * \brief Read data from DDR memory in HN system and write it into a buffer in shared memory region of the Host PC. High Performace Data Transfer by means of burst transfer functions
 *
 * \param[in] ptr to base address of buffer where read data will be stored, not necessarily the base address of the buffer in shared memory
 * \param[in] size number of bytes to read from memory in HN system (*ptr + size must be in the range of the space address of the buffer)
 * \param[in] address_src base address of data to read from memory in HN system
 * \param[in] tile_src tile where memory is attached
 * \param[in] blocking_transfer specifies if operation is blocking ( != 0 ) and the function does not return
 *            until the read operation is finished, or not ( 0 ) the application will have to check whether the operation is finished or not
 * \return 0 if the buffer has been succesfully released, otherwise return error code
 */
uint32_t hn_shm_read_memory(void *ptr, uint32_t size, uint32_t address_src, uint32_t tile_src, uint32_t blocking_transfer);


// ------------------------------------------------------------------------------------------------
// DMA transfer functions (FPGA DDR -> UNIT)
//
uint32_t hn_register_dma_operation(uint32_t *id);
uint32_t hn_dma_to_unit(uint32_t id, uint32_t addr_src, uint32_t tile_src, uint32_t size, uint32_t tile_dst, uint32_t blocking_transfer);
uint32_t hn_dma_to_mem(uint32_t id, uint32_t addr_src, uint32_t tile_src, uint32_t size, uint32_t addr_dst, uint32_t tile_dst, uint32_t blocking_transfer);
uint32_t hn_release_dma_operation(uint32_t id);
uint32_t hn_get_dma_status(uint32_t id, uint32_t *status);
// --------------------------------------------------------------------------------------------------
// Interrupt-related functions
//
uint32_t hn_register_int(uint32_t tile, uint16_t vector_mask, uint32_t *id);
uint32_t hn_interrupt(uint32_t id, uint16_t vector);
uint32_t hn_wait_int(uint32_t id, uint16_t vector_mask);
uint32_t hn_release_int(uint32_t id);

// --------------------------------------------------------------------------------------------------
// Resource allocation (memories, units and bandwidth) functions
uint32_t hn_lock_resources_access();
uint32_t hn_unlock_resources_access();
uint32_t hn_find_memory(uint32_t tile, unsigned long long size, uint32_t *tile_mem, uint32_t *starting_addr);
uint32_t hn_find_memories(uint32_t size, uint32_t *tiles_mem, uint32_t *starting_addr, uint32_t *num_memories);
uint32_t hn_allocate_memory(uint32_t tile, uint32_t addr, uint32_t size);
uint32_t hn_release_memory(uint32_t tile, uint32_t addr, uint32_t size);
uint32_t hn_get_available_network_bandwidth(uint32_t tile_src, uint32_t tile_dst, unsigned long long *bw);
uint32_t hn_get_available_read_memory_bandwidth(uint32_t tile, unsigned long long *bw);
uint32_t hn_get_available_write_memory_bandwidth(uint32_t tile, unsigned long long *bw);
uint32_t hn_get_available_read_cluster_bandwidth(unsigned long long *bw);
uint32_t hn_get_available_write_cluster_bandwidth(unsigned long long *bw);
uint32_t hn_reserve_network_bandwidth(uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);
uint32_t hn_reserve_read_memory_bandwidth(uint32_t tile, unsigned long long bw);
uint32_t hn_reserve_write_memory_bandwidth(uint32_t tile, unsigned long long bw);
uint32_t hn_reserve_read_cluster_bandwidth(unsigned long long bw);
uint32_t hn_reserve_write_cluster_bandwidth(unsigned long long bw);
uint32_t hn_release_network_bandwidth(uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);
uint32_t hn_release_read_memory_bandwidth(uint32_t tile, unsigned long long bw);
uint32_t hn_release_write_memory_bandwidth(uint32_t tile, unsigned long long bw);
uint32_t hn_release_read_cluster_bandwidth(unsigned long long bw);
uint32_t hn_release_write_cluster_bandwidth(unsigned long long bw);
uint32_t hn_find_units_set(uint32_t tile, uint32_t num_tiles, uint32_t types[], uint32_t *tiles_dst, uint32_t *types_dst);
uint32_t hn_find_units_sets(uint32_t tile, uint32_t num_tiles, uint32_t types[], uint32_t ***tiles_dst, uint32_t ***types_dst, uint32_t *num);
uint32_t hn_reserve_units_set(uint32_t num_tiles, uint32_t *tiles);
uint32_t hn_release_units_set(uint32_t num_tiles, uint32_t *tiles);


// ---------------------------------------------------------------------------------------------------
// hn_print_error
//   short: prints an string describing the error passed as parameter
//   argument error_code: error code
void hn_print_error(uint32_t error_code);

//----------------------------------------------------------------------------------------------------
// hn_reset
//   short: resets the architecture
//   argument tile: tile that has to process the reset command (not important)
uint32_t hn_reset(uint32_t tile);


//*********************************************************************************************************************
//
// Variable definition
//
//*********************************************************************************************************************

#ifndef __HN_ENTRY_POINT
extern uint32_t hn_initialized;
#else
uint32_t hn_initialized = 0;
#endif



#ifdef __cplusplus
}
#endif

#endif
//*********************************************************************************************************************
// end of file hn.h
//*********************************************************************************************************************
