#ifndef HNEMU_H_
#define HNEMU_H_

#include <vector>

#include "logger.h"
#include "hn_include/hn_resource_manager.h"

#define MANGO_ROOT "/opt/mango"
#define MANGO_CONFIG_XML  "/usr/local/share/config.xml"
#define NUM_TILES_X 5
#define NUM_TILES_Y 4
#define MEM_SIZE_MB ( 1024UL )
#define BW  (40 * 64 / 8)
#define BW_MEM  (40 * 64)
#define BW_CL  1024

#define MBinBYTES(x) ((x)*1024UL*1024UL)

namespace hhal {

    /*!
     * \brief This class delivers local resources manager.
     * HNemu is implemented as a Singleton that holds the HN configuration and
     * allows to request, reserve and release resources by using HN API functions.
     * \see HN API in hn_include/hn.h
     */
    class HNemu {

    public:

        static HNemu *instance();

        HNemu(HNemu const &) = delete;

        void operator=(HNemu const &)  = delete;

        virtual ~HNemu() noexcept;

/*!
 * \brief Create default configuration
 */
        void fill_default_config();
/*!
 * \brief Create configuration from config.xml
 */
        void fill_config();
/*!
 * \brief Provide the number of clusters in the current architecture
 * \param clusters Pointer to the variable where the number of clusters is written
 * \return status code
 */
        uint32_t get_num_clusters(uint32_t *clusters);

/*!
 * \brief Provide the number of tiles in the HN system as well as the number of rows and columns
 * \param cluster Cluster for which info will be provided
 * \param num_tiles Pointer to the variable where the number of tiles is written
 * \param num_tiles_x Pointer to the variable where the number in X dimension is written
 * \param num_tiles_y Pointer to the variable where the number in Y dimension is written
 * \return status code
 */
         uint32_t get_num_tiles(uint32_t cluster, uint32_t *num_tiles, uint32_t *num_tiles_x, uint32_t *num_tiles_y);
/*!
 * \brief Provide the configuration info for a cluster
 * \param cluster Cluster for which info will be provided
 * \return: Pointer to the variable where the info will be written
 */
        const hn_rscmgt_info_t* get_info(uint32_t cluster);
/*!
 * \brief Provide the configuration info for the tile
 * \param tile Tile for which info will be provided
 * \param data Pointer to the variable where the info will be written
 * \param cluster Cluster where the target tile is located
 * \return status code
 */
        uint32_t get_tile_info(uint32_t tile, hn_rscmgt_tile_info_t *data, uint32_t cluster);
/*!
 * \brief Provide the size (in bytes) of the memory attached to a given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile for which info will be provided
 * \param size Pointer to the variable where the size will be written. If no memory attached the value is 0
 * \return status code
 */
        uint32_t get_memory_size(uint32_t cluster, uint32_t tile, uint32_t *size);
/*!
 * \brief Provide the size (in bytes) of the memory attached to a given cluster
 * \param cluster Cluster for which info will be provided
 * \param size Pointer to the variable where the size will be written. If no memory attached the value is 0
 * \return status code
 */
        uint32_t get_memory_size(uint32_t cluster,unsigned long long *size);

/*!
 * \brief Provide the size (in bytes) of the memory attached to a given cluster
 * \param cluster Cluster for which info will be provided
 * \param size Pointer to the variable where the size will be written. If no memory attached the value is 0
 * \return status code
 */
        uint32_t
        get_memory(uint32_t cluster, uint32_t tile, uint32_t *size, uint32_t *free, uint32_t *starting_addr);
/*!
 * \brief Provide a free memory slot of the requested size
 * \param cluster Cluster where the target tile is located
 * \param tile Tile on which the free memory slot is requested
 * \param size Requested size of memory slot
 * \param starting_addr Pointer to the variable where the starting address of the free memory slot will be written.
 * If no memory slot found then the value is 0 and the return value is HN_NOT_ENOUGH_MEMORY_AVAILABLE
 * \return status code
 */
        uint32_t get_free_memory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *starting_addr);
/*!
 * \brief Provide the status of the tile, i.e. assigned or not assigned
 * \param cluster Cluster where the target tile is located
 * \param tile Tile for which info will be provided
 * \param avail Pointer to the variable where the status be written. If tile is assigned the value = 1 otherwise 0
 * \return status code
 */
        uint32_t is_tile_assigned(uint32_t cluster, uint32_t tile, uint32_t *avail);
/*!
 * \brief Reserve the tile, i.e set the tile assigned
 * \param cluster Cluster where the target tile is located
 * \param tile Target tile for assignment
 * \return status code
 */
        uint32_t set_tile_assigned(uint32_t cluster, uint32_t tile);
/*!
 * \brief Release the tile, i.e set the tile available
 * \param cluster Cluster where the target tile is located
 * \param tile Target tile for release
 * \return status code
 */
        uint32_t set_tile_avail(uint32_t cluster, uint32_t tile);
/*!
 * \brief Provide the available read bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Pointer to the variable where the read memory bandwidth will be written.
 * If tile is not attached to the memory the value =  0
 * \return status code
 */
        uint32_t get_available_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long *bw);
/*!
 * \brief Provide the available write bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Pointer to the variable where the write memory bandwidth will be written.
 * If tile is not attached to the memory the value =  0
 * \return status code
 */
        uint32_t get_available_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long *bw);
/*!
 * \brief Provide the available network bandwidth from the source tile to the destination tile
 * \param cluster Cluster where the target tiles are located
 * \param tile_src Source tile
 * \param tile_dst Destination tile
 * \param bw Pointer to the variable where the available network bandwidth will be written.
 * If tile is not attached to the memory the value =  0
 * \return status code
 * \note cross-cluster connection is not provided
 */
        uint32_t get_available_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long *bw);
/*!
 * \brief Provide the available bandwidth of a port in the router located in a given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile where the router is located
 * \param port Port to get bandwidth from
 * \param bw Pointer to the variable where the write available router bandwidth will be written.
 * \return status code
 * \note XY route for HN_MAX_ROUTER_PORTS = 4
 */
        uint32_t get_available_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long *bw);
/*!
 * \brief Reserve the read memory bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Value of the read memory bandwidth to be reserved
 * \return status code
 */
        uint32_t reserve_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw);
/*!
 * \brief Reserve the write memory bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Value of the write memory bandwidth to be reserved
 * \return status code
 */
        uint32_t reserve_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw);
/*!
 * \brief Reserve the bandwidth of a port in the router located in a given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile where the router is located
 * \param port Port to reserve bandwidth from
 * \param bw Value of the router bandwidth to be reserved.
 * \return status code
 */
        uint32_t reserve_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long bw);
/*!
 * \brief Reserve the network bandwidth from the source tile to the destination tile
 * \param cluster Cluster where the target tiles are located
 * \param tile_src Source tile
 * \param tile_dst Destination tile
 * \param bw Value of the network bandwidth to be reserved
 * \return status code
 * \note cross-cluster connection is not provided
 */
        uint32_t reserve_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);
/*!
 * \brief Release the read memory bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Value of the read memory bandwidth to be released
 * \return status code
 * \note XY route for HN_MAX_ROUTER_PORTS = 4
 */
        uint32_t release_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw);
/*!
 * \brief Release the write memory bandwidth to the memory attached to the given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile which attached to the memory
 * \param bw Value of the write memory bandwidth to be released
 * \return status code
 */
        uint32_t release_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw);
/*!
 * \brief Release the bandwidth of a port in the router located in a given tile
 * \param cluster Cluster where the target tile is located
 * \param tile Tile where the router is located
 * \param port Port to release bandwidth from
 * \param bw Value of the router bandwidth to be released.
 * \return status code
 */
        uint32_t release_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long bw);
/*!
 * \brief Release the network bandwidth from the source tile to the destination tile
 * \param cluster Cluster where the target tiles are located
 * \param tile_src Source tile
 * \param tile_dst Destination tile
 * \param bw Value of the network bandwidth to be released
 * \return status code
 * \note cross-cluster connection is not provided
 */
        uint32_t release_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long bw);
/*!
 * \brief Provide the available read bandwidth to the given cluster
 * \param cluster Target cluster
 * \param bw Pointer to the variable where the read cluster bandwidth will be written.
 * \return status code
 * \note XY route for HN_MAX_ROUTER_PORTS = 4
 */
        uint32_t get_available_read_cluster_bw(uint32_t cluster, unsigned long long *bw);
/*!
 * \brief Provide the available write bandwidth to the given cluster
 * \param cluster Target cluster
 * \param bw Pointer to the variable where the write cluster bandwidth will be written.
 * \return status code
 */
        uint32_t get_available_write_cluster_bw(uint32_t cluster, unsigned long long *bw);
/*!
 * \brief Reserve the read bandwidth to the given cluster
 * \param cluster Target cluster
 * \param bw Value of the read cluster bandwidth to be reserved
 * \return status code
 */
        uint32_t reserve_read_cluster_bw(uint32_t cluster, unsigned long long bw);
/*!
 * \brief Reserve the write bandwidth to the given cluster
 * \param cluster Target cluster
 * \param bw Value of the write cluster bandwidth to be reserved
 * \return status code
 */
        uint32_t reserve_write_cluster_bw(uint32_t cluster, unsigned long long bw);

/*!
* \brief Release the read bandwidth to the given cluster
* \param cluster Target cluster
* \param bw Value of the read cluster bandwidth to be released
* \return status code
*/
        uint32_t release_read_cluster_bw(uint32_t cluster, unsigned long long bw);
/*!
 * \brief Release the write bandwidth to the given cluster
 * \param cluster Target cluster
 * \param bw Value of the write cluster bandwidth to be released
 * \return status code
 */
        uint32_t release_write_cluster_bw(uint32_t cluster, unsigned long long bw);
/*!
 * \brief Provide a free memory slot of the requested size located close to the given tile
 * \param cluster Cluster where the source tile is located
 * \param tile Tile close to which the free memory slot is requested
 * \param size Requested size of the memory slot
 * \param tile_mem Pointer to the variable where the target will be written
 * If no memory slot found then the value is 0 and the return value is HN_NOT_ENOUGH_MEMORY_AVAILABLE
 * \param starting_addr Pointer to the variable where the starting size of the free memory slot will be written.
 * If no memory slot found then the value is 0 and the return value is HN_NOT_ENOUGH_MEMORY_AVAILABLE
 * \return status code
 */
        uint32_t
        find_memory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *tile_mem, uint32_t *starting_addr);
/*!
 * \brief Allocate a memory slot on the given tile of the requested size
 * \param cluster Cluster where the target tile is located
 * \param tile Tile on which the memory slot is located
 * \param addr Starting address of the memory slot to be allocated.
 * \param size Size of the memory slot
 * \return status code
 */
        uint32_t allocate_memory(uint32_t cluster, uint32_t tile, uint32_t addr, uint32_t size);
/*!
 * \brief Release a memory slot on the given tile of the requested size
 * \param cluster Cluster where the target tile is located
 * \param tile Tile on which the memory slot is located
 * \param addr Starting address of the memory slot to be released.
 * \param size Size of the memory slot
 * \return status code
 */
        uint32_t release_memory(uint32_t cluster, uint32_t tile, uint32_t addr, uint32_t size);
/*!
 * \brief Provide the distance between the source tile and the destination tile (in hops)
 * \param cluster Cluster where the target tiles are located
 * \param tile_src Source tile
 * \param tile_dst Destination tile
 * \param bw Pointer to the variable where the distance will be written
 * \return status code
 * \note cross-cluster connection is not provided
 */
        uint32_t get_network_distance(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, uint32_t *dst);
/*!
 * \brief Provide the set of tiles matching the requested types
 * \param cluster Cluster where the tiles could be located
 * \param num_tiles Number of requested tiles to be found
 * \param types Array of requested tiles architectures
 * \param tiles_dst Pointer to the array of variables where the appropriate tiles will be written
 * \return status code
 * \note The returned set of tiles is supplemented by the additional tiles to form the rectangular area
 * Returned set is selected from several available by smallest distance between tiles and memory
 */
        uint32_t find_units_set(uint32_t cluster, uint32_t num_tiles, uint32_t types[], uint32_t **tiles_dst);
/*!
 * \brief Provide several available sets of tiles matching the requested types
 * \param cluster Cluster where the tiles could be located
 * \param num_tiles Number of requested tiles to be found
 * \param types Array of requested tiles architectures
 * \param tiles_dst Pointer to the 2-dimensional array of variables where the sets of appropriate tiles will be written
 * \param num Pointer to the variable where the value of available sets of tiles matching the requested types
 * will be written
 * \return status code
 * \note The returned sets of tiles are supplemented by the additional tiles to form the rectangular area
 */
        uint32_t find_units_sets(uint32_t cluster, uint32_t num_tiles, uint32_t types[], uint32_t ***tiles_dst,
                uint32_t *num);
/*!
 * \brief Reserve sets of tiles
 * \param cluster Cluster where the tiles could be located
 * \param num_tiles Number of requested tiles to be reserved
 * \param types Pointer to the set of tiles to be reserved
 * \return status code
 * \note The requested sets of tiles are supplemented by the additional tiles to form the rectangular area
 */
        uint32_t reserve_units_set(uint32_t cluster, uint32_t num_tiles, const uint32_t *tiles);
/*!
 * \brief Release sets of tiles
 * \param cluster Cluster where the tiles could be located
 * \param num_tiles Number of requested tiles to be released
 * \param types Pointer to the set of tiles to be released
 * \return status code
 * \note The requested sets of tiles are supplemented by the additional tiles to form the rectangular area
 */
        uint32_t release_units_set(uint32_t cluster, uint32_t num_tiles, const uint32_t *tiles);

    private:
        ConsoleLogger log;

        HNemu();
/*!
 * \brief Array of configurations for each cluster
 */
        std::vector <hn_rscmgt_info_st> configuration;
/*!
 * \brief Number of clusters
 * */
        uint32_t num_clusters;

/*!
 * \brief Provide the verification of cluster id and tile id
 * \param cluster Cluster where the target tiles are located
 * \param tile Tile to be checked
 * \return status code
 */
        inline uint32_t isTile (uint32_t cluster, uint32_t tile);
/*!
 * \brief Provide the verification of cluster id
 * \param cluster Cluster to be checked
 * \return status code
 */
        inline uint32_t isCluster (uint32_t cluster);
/*!
 * \brief Check if the tile matches one of the requested tile architectures
 * \param cluster Cluster where the target tiles are located
 * \param tile_cur Tile to be checked
 * \param num_tiles_cur Number of tiles currenly preassigned
 * \param num_tiles Number of requested tiles
 * \param tiles_cur Pointer to the set of currently preassigned tiles
 * \param types Array of requested tiles architectures
 * \return status code
 * \details Perform check if tile is not assigned or preassigned,
 * check if tile matches one of requested but not preassigned architectures.
 * In case of successfull matching tile_cur is set preassigned,
 * includes to the tiles_cur and num_tiles_cur is incremented
 */
        inline uint32_t checkType(uint32_t cluster, uint32_t tile_cur, uint32_t *num_tiles_cur, uint32_t num_tiles, uint32_t *tiles_cur,
                           const uint32_t *types) ;
/*!
 * \brief Check if the memory attached to the given tile have requested size of free memory
 * \param cluster Cluster where the target tiles are located
 * \param tile Tile to which memory is attached
 * \param size Requested size of free memory
 * \param tile_mem Pointer to the variable where the tile to which memory is attached to be written
 * \param starting_addr Pointer to the variable where the starting address of the memory slot to be written
 * \return status code
 * \details Check if there is a free memory slot of the requested size in the memory
 * attached to the given tile.
 * In case of free memory slot, tile_mem and starting_addr are filled
 */
        inline uint32_t checkMemory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *tile_mem, uint32_t *starting_addr) ;
/*!
 * \brief Provide the set of tiles matching the requested types close to the given tile
 * \param cluster Cluster where the tiles could be located
 * \param tile Tile with attached memory close to which the set could be located
 * \param num_tiles Number of requested tiles to be found
 * \param types Array of requested tiles architectures
 * \param tiles_dst Pointer to the array of variables where the appropriate tiles will be written
 * \param dst Pointer to the variable where the distanse between tiles and memory will be written
 * \return status code
 * \note used in find_units_set and find_units_sets
 */
        uint32_t find_single_units_set(uint32_t cluster, uint32_t tile, uint32_t num_tiles, uint32_t *types, uint32_t **tiles_dst,
                                       uint32_t *dst) ;
    };

} // namespace mango

#endif //HNEMU_H_

