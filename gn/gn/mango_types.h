/*! \file mango_types.h
 * \brief Include data types used in the GN-level Mango platform run-time
 */
#ifndef MANGO_TYPES_H
#define MANGO_TYPES_H

#include <cstdint>
#include <cstddef>

namespace mango {

typedef uint32_t mango_size_t;

#if __x86_64__
	typedef uint64_t mango_addr_t;
#else
	typedef uint32_t mango_addr_t;
#endif

typedef uint32_t mango_id_t;
typedef mango_id_t mango_mem_id_t;
typedef mango_id_t mango_unit_id_t;
typedef mango_id_t mango_cluster_id_t;

/*!
 * \enum mango_event_status_t
 * \brief States for events
 */
typedef enum MangoEventStatus { LOCK=0, READ, WRITE, END_FIFO_OPERATION } mango_event_status_t;

/*!
 * \enum ExitCode
 * \brief Exit codes supported
 */
typedef enum class ExitCode {
	SUCCESS = 0,         /*!< Successfull return */
	ERR_INVALID_VALUE,   /*!< Invalid value specified */
	ERR_INVALID_TASK_ID, /*!< Invalid task id provided */
	ERR_INVALID_KERNEL,  /*!< Invalid kernel structure */
	ERR_FEATURE_NOT_IMPLEMENTED, /*!< The feature is not implemented yet */
	ERR_INVALID_KERNEL_FILE, /*!< Kernel file is of an unknown or invalid type */
	ERR_UNSUPPORTED_UNIT, /*!< Unit type not supported in current configuration */
	ERR_OUT_OF_MEMORY,   /*! < Out of memory error */
	ERR_SEM_FAILED,      /*!< Semaphore open failure */
	ERR_MMAP_FAILED,     /*!< mmap failure */
	ERR_FOPEN,           /*!< file open failure */
	ERR_OTHER            /*!< Generic error */
} mango_exit_code_t;


/*! \brief Memory types
 *
 * Employed to configure memory buffer and scalar types. 
 */
typedef enum class BufferType { 
	NONE, /*!< No type (typically, will cause errors) */
	FIFO, /*!< FIFO behaviour (a single asynchronous read or write from the GN
	* side will result in reading or writing the entire buffer with appropriate
	* synchronization */
	BUFFER, /*!< Buffer behaviour (each read or write is independent) */
	SCALAR /*!< Scalar data type */
} mango_buffer_type_t;

/*! \brief Mango computational unit (device) types
 */
typedef enum class UnitType {
	GN,   /*!< Fall back to GN node if no other option is available */
	X86,
	X86_64,
	ARM_V7,
	ARM_V8,
	NVIDIA,
	PEAK, /*!< PEAK units */
	NUP,  /*!< NU+ units */
	DCT,

	STOP, /*!< Terminator used to close arrays of mango_unit_type_t */
} mango_unit_type_t;


/*! \brief Types of kernel sources/file types
 */
typedef enum class FileType { 
	UNKNOWN_KERNEL_SOURCE_TYPE, /*!< Unknown type, fail. */
	BINARY, /*!< Binary file, ready to load and execute */
	HARDWARE, /*!< Hardware accelerator */
	STRING, /*!< String containing the kernel source code */
	SOURCE  /*!< Text file containing the kernel source code */
} mango_file_type_t;

/*! \brief Memory bank descriptor
 *
 * This provides information about a memory bank, including its size and the
 * starting physical address. This is an immutable class.
 */
class MemoryBank {

public:

	/*! \brief The constructor of the class
	 *  \param id The memory bank identifier
	 *  \param phy_addr The starting physical address for the current bank
	 *  \param size The size in bytes of the memory
	 *  \param tile The tile to which the bank is attached
	 */
	MemoryBank(mango_id_t id, mango_size_t phy_addr, mango_size_t size, mango_id_t tile) noexcept : 
		id(id), phy_addr(phy_addr), size(size), tile(tile) {}
	
	/*! 
	 * \brief Get the memory tile identifier
	 */
	inline mango_id_t get_id() 			const noexcept { return id; }

	/*! 
	 * \brief Get the starting physical address
	 */
	inline mango_size_t get_phy_addr()	const noexcept { return phy_addr; }

	/*! 
	 * \brief Get the memory bank capacity in bytes
	 */
	inline mango_size_t get_size()		const noexcept { return size; }

	/*! 
	 * \brief Get the tile to which the bank is attached
	 */
	inline mango_id_t get_tile()		const noexcept { return tile; }

private:
	const mango_id_t   id;        /*!< The memory bank identifier */
	const mango_size_t phy_addr;  /*!< The starting physical address */
	const mango_size_t size;      /*!< The memory bank capacity */
	const mango_id_t   tile;      /*!< Marks the tile to which the bank is attached */
};

/*!
 * \struct Unit
 * \brief Mango computational unit descriptor
 *
 * For each unit we need to keep track of the architecture type, the id number,
 * the number of cores and the tile the unit belongs to
 */
class Unit {
public:

	/*! \brief The constructor of the unit class
	 *  \param id The unit identifier
	 *  \param arch The architecture of the unit
	 *  \param size The number of available cores
	 */
	Unit(mango_id_t id, UnitType arch, int nr_cores) noexcept :
		arch(arch),
		id(id),
		nr_cores(nr_cores) {

	}

	/*! 
	 * \brief Get the memory tile identifier
	 */
	inline mango_id_t get_id() 			const noexcept { return id; }

	/*! 
	 * \brief Get the architecture
	 */
	inline mango_unit_type_t get_arch()	const noexcept { return arch; }

	/*! 
	 * \brief Get the number of cores
	 */
	inline int get_nr_cores()			const noexcept { return nr_cores; }



private:
	const mango_unit_type_t arch;	/*!< Unit architecture type */
	const mango_id_t id;			/*!< Unique identifier */
	const int nr_cores;				/*!< Number of cores in the unit */

//	TODO Currently not used, we need this?
//	MemoryBank mems[MANGO_NR_MEM_BANKS]; /*!< The memory banks array   */
	
}; 


// TODO UnitStats currently not used, we need this?
#if 0
/*! \brief Unit-level statistics and run-time information
 */
class UnitStats {
	struct {
		uint32_t period_us;  /*!< The update statistics time period */
		uint32_t idle_us;    /*!< Idle time of the unit in the period */
	} time;
	uint32_t nr_tasks; /*!< Number of tasks currently running */
};
#endif




}	// namespace mango
#endif // MANGO_TYPES_H
