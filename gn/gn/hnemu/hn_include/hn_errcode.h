#ifndef HN_ERRCODE_H_
#define HN_ERRCODE_H_

/*!
 * \brief This header provides error codes and HN API constants
 * \see HN API in hn_include/hn.h
 */


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
#define HN_TILE_FAMILY_GN           255

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

#define HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE    35
#define HN_CLUSTER_DOES_NOT_EXIST            36


#endif //HN_ERRCODE_H_

