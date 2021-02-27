#include <regex>
#include <fstream>
#include <iostream>

#include "hnemu.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "hn_include/hn_errcode.h"
//#include <sstream>//4macos

using namespace rapidxml;

namespace mango {

    HNemu *HNemu::instance() {
        static HNemu _instance;
        return &_instance;
    }

    HNemu::HNemu() {
        fill_config();
    }

    HNemu::~HNemu() {
        for (uint32_t cluster_id = 0; cluster_id < num_clusters; cluster_id++) {
            for (uint32_t i = 0; i < configuration[cluster_id].num_tiles; i++) {
                if (configuration[cluster_id].tile_info[i].memory_size > 0) {
                    auto del = configuration[cluster_id].tile_info[i].first_memory_slot;
                    while (del != nullptr) {
                        auto tmp = del->next_memory_slot;
                        free(del);
                        del = (hn_rscmgt_memory_slot_t *) tmp;
                    }
                }
            }
        }
    }

    void HNemu::fill_default_config() {
        log.Info("[HNemu] default configuration");
        uint32_t cluster_id = 0;
        this->num_clusters = 1;
        configuration.reserve(this->num_clusters);
        unsigned long long last_addr = 0;
        configuration[cluster_id].arch_id = 0;
        configuration[cluster_id].num_tiles = NUM_TILES_X * NUM_TILES_Y;
        configuration[cluster_id].num_cols = NUM_TILES_X;
        configuration[cluster_id].num_rows = NUM_TILES_Y;
        configuration[cluster_id].read_cluster_bw = MBinBYTES(BW_CL);
        configuration[cluster_id].avail_read_cluster_bw = MBinBYTES(BW_CL);
        configuration[cluster_id].write_cluster_bw = MBinBYTES(BW_CL);
        configuration[cluster_id].avail_write_cluster_bw = MBinBYTES(BW_CL);
        for (uint32_t i = 0; i < configuration[cluster_id].num_tiles; i++) {
            configuration[cluster_id].tile_info[i].type = HN_TILE_FAMILY_GN;
            configuration[cluster_id].tile_info[i].subtype = HN_TILE_MODEL_NONE;
            configuration[cluster_id].tile_info[i].assigned = 0;
            configuration[cluster_id].tile_info[i].preassigned = 0;
            if ((i == 0) || (i == (NUM_TILES_X - 1)) || (i == (NUM_TILES_X * (NUM_TILES_Y - 1))) ||
                (i == (NUM_TILES_X * NUM_TILES_Y - 1))) {
                configuration[cluster_id].tile_info[i].memory_size = MBinBYTES(MEM_SIZE_MB);
                configuration[cluster_id].tile_info[i].free_memory = MBinBYTES(MEM_SIZE_MB);
                configuration[cluster_id].tile_info[i].read_memory_bw = MBinBYTES(BW_MEM);
                configuration[cluster_id].tile_info[i].avail_read_memory_bw = MBinBYTES(BW_MEM);
                configuration[cluster_id].tile_info[i].write_memory_bw = MBinBYTES(BW_MEM);
                configuration[cluster_id].tile_info[i].avail_write_memory_bw = MBinBYTES(BW_MEM);
                configuration[cluster_id].tile_info[i].first_memory_slot = (hn_rscmgt_memory_slot_t *) malloc(
                        sizeof(hn_rscmgt_memory_slot_t));
                configuration[cluster_id].tile_info[i].first_memory_slot->size = MBinBYTES(MEM_SIZE_MB);
                configuration[cluster_id].tile_info[i].first_memory_slot->start_address = last_addr;
                last_addr += MBinBYTES(MEM_SIZE_MB);
                configuration[cluster_id].tile_info[i].first_memory_slot->end_address = last_addr - 1;
                configuration[cluster_id].tile_info[i].first_memory_slot->next_memory_slot = nullptr;
            } else {
                configuration[cluster_id].tile_info[i].memory_size = 0;
                configuration[cluster_id].tile_info[i].free_memory = 0;
                configuration[cluster_id].tile_info[i].read_memory_bw = 0;
                configuration[cluster_id].tile_info[i].avail_read_memory_bw = 0;
                configuration[cluster_id].tile_info[i].write_memory_bw = 0;
                configuration[cluster_id].tile_info[i].avail_write_memory_bw = 0;
                configuration[cluster_id].tile_info[i].first_memory_slot = nullptr;
            }
            configuration[cluster_id].tile_info[i].north_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].avail_north_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].west_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].avail_west_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].east_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].avail_east_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].south_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].avail_south_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].local_port_bw = MBinBYTES(BW);
            configuration[cluster_id].tile_info[i].avail_local_port_bw = MBinBYTES(BW);
        }
    }
    uint32_t str_to_uint(const char *str) {
        std::string::size_type sz;
        if (strlen(str) != 0) {
            return (uint32_t) std::stoul(str, &sz);
        }
        return 0;
    }

    unsigned long long str_to_ull(const char *str) {
        std::string::size_type sz;
        if (strlen(str) != 0) {
            return std::stoull(str, &sz);
        }
        return 0;
    }

    uint32_t read_num(int *number, char *result) {
        std::string input;
        std::string::size_type sz;
        std::getline(std::cin, input);
        if (!input.empty()) {
            *number = std::stoi(input, &sz);
        }
        sprintf(result, "%d", *number);
        return 0;
    }


    void HNemu::fill_config() {
        log.Info("[HNemu] Configuration");
        uint32_t cluster_id = 0;
        xml_document<> doc;
        std::string conf_file(MANGO_ROOT);  // MANGO_ROOT compile-time defined
        conf_file += MANGO_CONFIG_XML;
        std::ifstream file(conf_file);
        if (!file.is_open()) {
            fill_default_config();
            return;
        }
        log.Info("[HNemu] using configuration file %s", conf_file.c_str());
        uint32_t last_addr = 0;
        std::stringstream buffer;
        buffer << file.rdbuf();

        std::string content(buffer.str());
        doc.parse<0>(&content[0]);

        xml_node<> *root_node = doc.first_node("system");
        std::string arch_id = root_node->first_attribute("arch_id")->value();
        auto arch = str_to_uint(arch_id.c_str());

        std::string c_num_clusters = root_node->first_attribute("num_clusters")->value();
        this->num_clusters = str_to_uint(c_num_clusters.c_str());

        configuration.reserve(this->num_clusters);

        for (xml_node<> *cluster_node = root_node->first_node(); cluster_node; cluster_node = cluster_node->next_sibling(), cluster_id++) {
            configuration[cluster_id].arch_id = arch;
            std::string read_cluster_bw = cluster_node->first_attribute("read_cluster_bw")->value();
            configuration[cluster_id].read_cluster_bw = MBinBYTES(str_to_ull(read_cluster_bw.c_str()));
            std::string write_cluster_bw = cluster_node->first_attribute("write_cluster_bw")->value();
            configuration[cluster_id].write_cluster_bw = MBinBYTES(str_to_ull(write_cluster_bw.c_str()));
            std::string num_rows = cluster_node->first_attribute("num_rows")->value();
            configuration[cluster_id].num_rows = str_to_uint(num_rows.c_str());
            std::string num_cols = cluster_node->first_attribute("num_cols")->value();
            configuration[cluster_id].num_cols = str_to_uint(num_cols.c_str());
            configuration[cluster_id].num_tiles =
                    configuration[cluster_id].num_rows * configuration[cluster_id].num_cols;
            uint32_t tile = 0;
            for (xml_node<> *tile_node = cluster_node->first_node(); tile_node; tile_node = tile_node->next_sibling(), tile++) {
                configuration[cluster_id].tile_info[tile].type = tile;
                std::string type = tile_node->first_attribute("family")->value();
                configuration[cluster_id].tile_info[tile].type = str_to_uint(type.c_str());
                configuration[cluster_id].tile_info[tile].assigned = configuration[cluster_id].tile_info[tile].preassigned = 0;
                std::string subtype = tile_node->first_attribute("model")->value();
                configuration[cluster_id].tile_info[tile].subtype = str_to_uint(subtype.c_str());
                std::string mem_size = tile_node->first_attribute("mem_size")->value();
                configuration[cluster_id].tile_info[tile].memory_size = configuration[cluster_id].tile_info[tile].free_memory
                        = MBinBYTES(str_to_uint(mem_size.c_str()));
                std::string read_mem_bw = tile_node->first_attribute("read_mem_bw")->value();
                configuration[cluster_id].tile_info[tile].read_memory_bw = configuration[cluster_id].tile_info[tile].avail_read_memory_bw
                        = MBinBYTES(str_to_ull(read_mem_bw.c_str()));
                std::string write_mem_bw = tile_node->first_attribute("write_mem_bw")->value();
                configuration[cluster_id].tile_info[tile].write_memory_bw = configuration[cluster_id].tile_info[tile].avail_write_memory_bw
                        = MBinBYTES(str_to_ull(write_mem_bw.c_str()));
                std::string north_port_bw = tile_node->first_attribute("north_port_bw")->value();
                configuration[cluster_id].tile_info[tile].north_port_bw = configuration[cluster_id].tile_info[tile].avail_north_port_bw
                        = MBinBYTES(str_to_ull(north_port_bw.c_str()));
                std::string west_port_bw = tile_node->first_attribute("west_port_bw")->value();
                configuration[cluster_id].tile_info[tile].west_port_bw = configuration[cluster_id].tile_info[tile].avail_west_port_bw
                        = MBinBYTES(str_to_ull(west_port_bw.c_str()));
                std::string east_port_bw = tile_node->first_attribute("east_port_bw")->value();
                configuration[cluster_id].tile_info[tile].east_port_bw = configuration[cluster_id].tile_info[tile].avail_east_port_bw
                        = MBinBYTES(str_to_ull(east_port_bw.c_str()));
                std::string south_port_bw = tile_node->first_attribute("south_port_bw")->value();
                configuration[cluster_id].tile_info[tile].south_port_bw = configuration[cluster_id].tile_info[tile].avail_south_port_bw
                        = str_to_ull(south_port_bw.c_str());
                std::string local_port_bw = tile_node->first_attribute("local_port_bw")->value();
                configuration[cluster_id].tile_info[tile].local_port_bw = configuration[cluster_id].tile_info[tile].avail_local_port_bw
                        = MBinBYTES(str_to_ull(local_port_bw.c_str()));
                if (configuration[cluster_id].tile_info[tile].memory_size > 0) {
                    configuration[cluster_id].tile_info[tile].first_memory_slot = (hn_rscmgt_memory_slot_t *) malloc(
                            sizeof(hn_rscmgt_memory_slot_t));
                    configuration[cluster_id].tile_info[tile].first_memory_slot->size = configuration[cluster_id].tile_info[tile].memory_size;
                    configuration[cluster_id].tile_info[tile].first_memory_slot->start_address = last_addr;
                    configuration[cluster_id].tile_info[tile].first_memory_slot->end_address =
                            configuration[cluster_id].tile_info[tile].first_memory_slot->start_address +
                            configuration[cluster_id].tile_info[tile].memory_size - 1;
                    last_addr += configuration[cluster_id].tile_info[tile].memory_size;
                    configuration[cluster_id].tile_info[tile].first_memory_slot->next_memory_slot = nullptr;
                } else {
                    configuration[cluster_id].tile_info[tile].first_memory_slot = nullptr;
                }
            }
        }
        file.close();
    }

    uint32_t HNemu::isTile (uint32_t cluster, uint32_t tile){
        if (cluster < this->num_clusters || tile < configuration[cluster].num_tiles) {
            return HN_SUCCEEDED;
        } else{
            log.Error("[HNemu] cluster %d, tile %d does not exist in the current configuration", cluster, tile);
            return HN_TILE_DOES_NOT_EXIST;
        }
    }

    uint32_t HNemu::isCluster (uint32_t cluster){
        if (cluster < this->num_clusters) {
            return HN_SUCCEEDED;
        } else{
            log.Error("[HNemu] cluster %d does not exist in the current configuration", cluster);
            return HN_CLUSTER_DOES_NOT_EXIST;
        }
    }

    uint32_t HNemu::get_num_clusters(uint32_t *clusters){
        *clusters = this->num_clusters;
        log.Debug("[HNemu] number of clusters %d", this->num_clusters);
        return HN_SUCCEEDED;;
    }

    uint32_t
    HNemu::get_num_tiles(uint32_t cluster, uint32_t *num_tiles, uint32_t *num_tiles_x, uint32_t *num_tiles_y) {
        if (cluster >= this->num_clusters || configuration[cluster].num_tiles == 0
            || configuration[cluster].num_cols == 0 || configuration[cluster].num_rows == 0
            || configuration[cluster].num_tiles != configuration[cluster].num_cols * configuration[cluster].num_rows) {
            log.Error("[HNemu] cluster %d configuration is broken", cluster);
            return HN_TILE_DOES_NOT_EXIST;
        }
        *num_tiles = configuration[cluster].num_tiles;
        *num_tiles_x = configuration[cluster].num_cols;
        *num_tiles_y = configuration[cluster].num_rows;
        log.Debug("[HNemu] cluster %d, number of tiles %d, number of columns %d, number of rows %d",
                 cluster, *num_tiles, *num_tiles_x, *num_tiles_y);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::get_tile_info(uint32_t tile, hn_rscmgt_tile_info_t *data, uint32_t cluster){
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        *data = configuration[cluster].tile_info[tile];
        return HN_SUCCEEDED;
    }

    const hn_rscmgt_info_t* HNemu::get_info(uint32_t cluster) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return nullptr;
        }
        return &configuration[cluster];
    }

    uint32_t HNemu::get_memory_size(uint32_t cluster, uint32_t tile,  uint32_t *size) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        *size = configuration[cluster].tile_info[tile].memory_size;
        log.Debug("[HNemu] cluster %d, tile %d, memory size %llu",cluster, tile, *size);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::get_memory_size(uint32_t cluster, unsigned long long *size) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        unsigned long long mem_size = 0;
        for (uint32_t i = 0; i < configuration[cluster].num_tiles; i++) {
            mem_size += configuration[cluster].tile_info[i].memory_size;
        }
        *size = mem_size;
        log.Debug("[HNemu] cluster %d, memory size %llu", cluster, mem_size);
        return HN_SUCCEEDED;
    }

    uint32_t
    HNemu::get_memory(uint32_t cluster, uint32_t tile, uint32_t *size, uint32_t *free,
                                 uint32_t *starting_addr) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        *size = configuration[cluster].tile_info[tile].memory_size;
        *free = configuration[cluster].tile_info[tile].free_memory;
        *starting_addr = configuration[cluster].tile_info[tile].first_memory_slot->start_address;
        log.Debug("[HNemu] cluster %d, tile %d, memory size %d, free memory %d, starting address 0x%x",
                cluster, tile, *size, *free, *starting_addr);
        return HN_SUCCEEDED;
    }

    uint32_t
    HNemu::get_free_memory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *starting_addr) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].tile_info[tile].memory_size == 0) {
            log.Error("[HNemu] get_free_memory: cluster %d, tile %d does not have memory attached", cluster, tile);
            return HN_MEMORY_NOT_PRESENT_IN_TILE;
        }
        //First Fit
        auto mem = configuration[cluster].tile_info[tile].first_memory_slot;
        while ((mem != nullptr) && (mem->size < size)) {
            mem = (hn_rscmgt_memory_slot_t *) mem->next_memory_slot;
        }
        if (mem == nullptr || mem->size < size) {
            *starting_addr = 0;
            log.Debug("[HNemu] cluster %d, tile %d, error occured while free memory slot of size %d",
                     cluster, tile, size);
            return HN_NOT_ENOUGH_MEMORY_AVAILABLE;
        } else {
            *starting_addr = mem->start_address;
            log.Debug("[HNemu] cluster %d, tile %d, get free memory slot of size %d starting address 0x%x",
                     cluster, tile, size, *starting_addr);
            return HN_SUCCEEDED;
        }
    }


    uint32_t HNemu::is_tile_assigned(uint32_t cluster, uint32_t tile, uint32_t *avail) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            avail = 0;
            return status;
        }
        *avail = configuration[cluster].tile_info[tile].assigned;
        log.Debug("[HNemu] cluster %d, tile %d, status: %s assigned",
                 cluster, tile, (*avail == 0) ? "not": "");
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::set_tile_assigned(uint32_t cluster, uint32_t tile) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].tile_info[tile].assigned = 1;
        log.Debug("[HNemu] cluster %d, tile %d, is assigned successfully",
                 cluster, tile);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::set_tile_avail(uint32_t cluster, uint32_t tile) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].tile_info[tile].assigned = 0;
        log.Debug("[HNemu] cluster %d, tile %d, is reassigned successfully",
                 cluster, tile);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::get_available_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long *bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            *bw = 0;
            return status;
        }
        *bw = configuration[cluster].tile_info[tile].avail_read_memory_bw;
        log.Debug("[HNemu] cluster %d, tile %d, available read memory bandwidth %d",
                 cluster, tile, *bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::get_available_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long *bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            *bw = 0;
            return status;
        }
        *bw = configuration[cluster].tile_info[tile].avail_write_memory_bw;
        log.Debug("[HNemu] cluster %d, tile %d, available write memory bandwidth %d",
                 cluster, tile, *bw);
        return HN_SUCCEEDED;
    }

    uint32_t
    HNemu::get_available_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long *bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            *bw = 0;
            return status;
        }
        switch (port) {
            case HN_NORTH_PORT:
                *bw = configuration[cluster].tile_info[tile].avail_north_port_bw;
                break;
            case HN_EAST_PORT:
                *bw = configuration[cluster].tile_info[tile].avail_east_port_bw;
                break;
            case HN_WEST_PORT:
                *bw = configuration[cluster].tile_info[tile].avail_west_port_bw;
                break;
            case HN_SOUTH_PORT:
                *bw = configuration[cluster].tile_info[tile].avail_south_port_bw;
                break;
            case HN_LOCAL_PORT:
                *bw = configuration[cluster].tile_info[tile].avail_local_port_bw;
                break;
        }
        log.Debug("[HNemu] cluster %d, tile %d, port %d, available router bandwidth %d",
                 cluster, tile, port, *bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::get_available_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long *bw){
        log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d",
                  cluster, tile_src, tile_dst);
        auto status_src = isTile (cluster, tile_src);
        auto status_dst = isTile (cluster, tile_dst);
        if (status_src != HN_SUCCEEDED || status_dst != HN_SUCCEEDED) {
            return (status_src!= HN_SUCCEEDED) ? status_src : status_dst;
        }
        uint32_t num_tiles_x = configuration[cluster].num_cols;
        uint32_t tile_cur = tile_src, tile_cur_x, tile_cur_y;
        uint32_t tile_dst_x = tile_dst % num_tiles_x;
        uint32_t tile_dst_y = tile_dst / num_tiles_x;
        uint32_t port = 0, status, found = 0;

        unsigned long long bw_cur = 0, bw_avail = 0;
        do {
            tile_cur_x = tile_cur % num_tiles_x;
            tile_cur_y = tile_cur / num_tiles_x;

            if (tile_cur_x < tile_dst_x) port = HN_EAST_PORT;
            else if (tile_cur_x > tile_dst_x) port = HN_WEST_PORT;
            else if (tile_cur_y < tile_dst_y) port = HN_SOUTH_PORT;
            else if (tile_cur_y > tile_dst_y) port = HN_NORTH_PORT;
            else port = HN_LOCAL_PORT;

            status =  get_available_router_bw(cluster, tile_cur, port, &bw_cur);
            if (status!=HN_SUCCEEDED) {
                return status;
            }

            if (found == 0) {
                bw_avail = bw_cur;
                found = 1;
            }
            if (bw_cur < bw_avail)
                bw_avail = bw_cur;

            if (port == HN_EAST_PORT) tile_cur++;
            else if (port == HN_WEST_PORT) tile_cur--;
            else if (port == HN_NORTH_PORT) tile_cur -= num_tiles_x;
            else if (port == HN_SOUTH_PORT) tile_cur += num_tiles_x;
        } while (tile_cur != tile_dst);
        *bw = bw_avail;
        log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d, available network bandwidth %d",
                 cluster, tile_src, tile_dst, *bw);
        return HN_SUCCEEDED;
    }


    uint32_t HNemu::reserve_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].tile_info[tile].avail_read_memory_bw < bw) {
            log.Debug("[HNemu] cluster %d, tile %d has not enough bandwidth available", cluster, tile);
            return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
        }
        configuration[cluster].tile_info[tile].avail_read_memory_bw -= bw;
        log.Debug("[HNemu] cluster %d, tile %d, read memory bandwidth reserved %d",
                 cluster, tile, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::reserve_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].tile_info[tile].avail_write_memory_bw < bw)  {
            log.Debug("[HNemu] cluster %d, tile %d has not enough bandwidth available", cluster, tile);
            return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
        }
        configuration[cluster].tile_info[tile].avail_write_memory_bw -= bw;
        log.Debug("[HNemu] cluster %d, tile %d, write memory bandwidth reserved %d",
                 cluster, tile, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::reserve_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        switch (port) {
            case HN_NORTH_PORT :
                if (configuration[cluster].tile_info[tile].avail_north_port_bw < bw)  {
                    log.Debug("[HNemu] cluster %d, tile %d, port %d has not enough bandwidth available",
                            cluster, tile, port);
                    return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
                }
                configuration[cluster].tile_info[tile].avail_north_port_bw -= bw;
                break;
            case HN_EAST_PORT :
                if (configuration[cluster].tile_info[tile].avail_east_port_bw < bw)  {
                    log.Debug("[HNemu] cluster %d, tile %d, port %d has not enough bandwidth available",
                             cluster, tile, port);
                    return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
                }
                configuration[cluster].tile_info[tile].avail_east_port_bw -= bw;
                break;
            case HN_WEST_PORT :
                if (configuration[cluster].tile_info[tile].avail_west_port_bw < bw)  {
                    log.Debug("[HNemu] cluster %d, tile %d, port %d has not enough bandwidth available",
                             cluster, tile, port);
                    return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
                }
                configuration[cluster].tile_info[tile].avail_west_port_bw -= bw;
                break;
            case HN_SOUTH_PORT :
                if (configuration[cluster].tile_info[tile].avail_south_port_bw < bw)  {
                    log.Debug("[HNemu] cluster %d, tile %d, port %d has not enough bandwidth available",
                             cluster, tile, port);
                    return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
                }
                configuration[cluster].tile_info[tile].avail_south_port_bw -= bw;
                break;
            case HN_LOCAL_PORT:
                if (configuration[cluster].tile_info[tile].avail_local_port_bw < bw)  {
                    log.Debug("[HNemu] cluster %d, tile %d, port %d has not enough bandwidth available",
                             cluster, tile, port);
                    return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
                }
                configuration[cluster].tile_info[tile].avail_local_port_bw -= bw;
                break;
        }
        log.Debug("[HNemu] cluster %d, tile %d, port %d, router bandwidth reserved %d",
                 cluster, tile, port, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::reserve_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long bw){

        unsigned long long bw_avail = 0;
        auto err = get_available_network_bw(cluster, tile_src, tile_dst, & bw_avail);
        if (err!=HN_SUCCEEDED)
            return err;
        if (bw_avail < bw){
            log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d, not enough network bandwidth available",
                     cluster, tile_src, tile_dst);
            return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
        }

        uint32_t num_tiles_x = configuration[cluster].num_cols;
        uint32_t tile_cur = tile_src, tile_cur_x, tile_cur_y;
        uint32_t tile_dst_x = tile_dst % num_tiles_x;
        uint32_t tile_dst_y = tile_dst / num_tiles_x;
        uint32_t port = 0;
        do {
            tile_cur_x = tile_cur % num_tiles_x;
            tile_cur_y = tile_cur / num_tiles_x;
            if (tile_cur_x < tile_dst_x) port = HN_EAST_PORT;
            else if (tile_cur_x > tile_dst_x) port = HN_WEST_PORT;
            else if (tile_cur_y < tile_dst_y) port = HN_SOUTH_PORT;
            else if (tile_cur_y > tile_dst_y) port = HN_NORTH_PORT;

            auto err =  reserve_router_bw(cluster, tile_cur, port, bw);
            if (err!=HN_SUCCEEDED) {
                release_network_bw (cluster, tile_src, tile_cur, bw);
                return err;
            }

            if (port == HN_EAST_PORT) tile_cur++;
            else if (port == HN_WEST_PORT) tile_cur--;
            else if (port == HN_NORTH_PORT) tile_cur -= num_tiles_x;
            else if (port == HN_SOUTH_PORT) tile_cur += num_tiles_x;
        } while (tile_cur != tile_dst);

        log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d, network bandwidth reserved %d",
                 cluster, tile_src, tile_dst, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::release_read_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].tile_info[tile].avail_read_memory_bw += bw;
        log.Debug("[HNemu] cluster %d, tile %d, read memory bandwidth released %d",
                 cluster, tile, bw);
        if (configuration[cluster].tile_info[tile].avail_read_memory_bw > configuration[cluster].tile_info[tile].read_memory_bw) {
            return HN_WRONG_BANDWIDTH_SETTING;
        }
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::release_write_memory_bw(uint32_t cluster, uint32_t tile, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].tile_info[tile].avail_write_memory_bw += bw;
        log.Debug("[HNemu] cluster %d, tile %d, write memory bandwidth released %d",
                 cluster, tile, bw);
        if (configuration[cluster].tile_info[tile].avail_write_memory_bw > configuration[cluster].tile_info[tile].write_memory_bw) {
            return HN_WRONG_BANDWIDTH_SETTING;
        }
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::release_router_bw(uint32_t cluster, uint32_t tile, uint32_t port, unsigned long long bw) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        switch (port) {
            case HN_NORTH_PORT :
                configuration[cluster].tile_info[tile].avail_north_port_bw += bw;
                if (configuration[cluster].tile_info[tile].avail_north_port_bw > configuration[cluster].tile_info[tile].north_port_bw) {
                    log.Error("[HNemu] cluster %d, tile %d, port %d wrong bandwidth settings",
                             cluster, tile, port);
                    return HN_WRONG_BANDWIDTH_SETTING;
                }
                break;
            case HN_EAST_PORT :
                configuration[cluster].tile_info[tile].avail_east_port_bw += bw;
                if (configuration[cluster].tile_info[tile].avail_east_port_bw > configuration[cluster].tile_info[tile].east_port_bw) {
                    log.Error("[HNemu] cluster %d, tile %d, port %d wrong bandwidth settings",
                             cluster, tile, port);
                    return HN_WRONG_BANDWIDTH_SETTING;
                }
                break;
            case HN_WEST_PORT :
                configuration[cluster].tile_info[tile].avail_west_port_bw += bw;
                if (configuration[cluster].tile_info[tile].avail_west_port_bw > configuration[cluster].tile_info[tile].west_port_bw) {
                    log.Error("[HNemu] cluster %d, tile %d, port %d wrong bandwidth settings",
                             cluster, tile, port);
                    return HN_WRONG_BANDWIDTH_SETTING;
                }
                break;
            case HN_SOUTH_PORT :
                configuration[cluster].tile_info[tile].avail_south_port_bw += bw;
                if (configuration[cluster].tile_info[tile].avail_south_port_bw > configuration[cluster].tile_info[tile].south_port_bw) {
                    log.Error("[HNemu] cluster %d, tile %d, port %d wrong bandwidth settings",
                             cluster, tile, port);
                    return HN_WRONG_BANDWIDTH_SETTING;
                }
                break;
            case HN_LOCAL_PORT :
                configuration[cluster].tile_info[tile].avail_local_port_bw += bw;
                if (configuration[cluster].tile_info[tile].avail_local_port_bw > configuration[cluster].tile_info[tile].local_port_bw) {
                    log.Error("[HNemu] cluster %d, tile %d, port %d wrong bandwidth settings",
                             cluster, tile, port);
                    return HN_WRONG_BANDWIDTH_SETTING;
                }
                break;
        }
        log.Debug("[HNemu] cluster %d, tile %d, port %d, router bandwidth released %llu",
                 cluster, tile, port, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::release_network_bw(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, unsigned long long bw){
        auto status_src = isTile (cluster, tile_src);
        auto status_dst = isTile (cluster, tile_dst);
        if (status_src != HN_SUCCEEDED || status_dst != HN_SUCCEEDED) {
            return (status_src!= HN_SUCCEEDED)? status_src : status_dst;
        }

        uint32_t num_tiles_x = configuration[cluster].num_cols;
        uint32_t tile_cur = tile_src, tile_cur_x, tile_cur_y;
        uint32_t tile_dst_x = tile_dst % num_tiles_x;
        uint32_t tile_dst_y = tile_dst / num_tiles_x;
        uint32_t port=0;

        do {
            tile_cur_x = tile_cur % num_tiles_x;
            tile_cur_y = tile_cur / num_tiles_x;

            if (tile_cur_x < tile_dst_x) port = HN_EAST_PORT;
            else if (tile_cur_x > tile_dst_x) port = HN_WEST_PORT;
            else if (tile_cur_y < tile_dst_y) port = HN_SOUTH_PORT;
            else if (tile_cur_y > tile_dst_y) port = HN_NORTH_PORT;

            auto err =  release_router_bw(cluster, tile_cur,port, bw);
            if (err!=HN_SUCCEEDED) {
                return err;
            }

            if (port == HN_EAST_PORT) tile_cur++;
            else if (port == HN_WEST_PORT) tile_cur--;
            else if (port == HN_NORTH_PORT) tile_cur -= num_tiles_x;
            else if (port == HN_SOUTH_PORT) tile_cur += num_tiles_x;
        } while (tile_cur != tile_dst);

        log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d, network bandwidth released %llu",
                 cluster, tile_src, tile_dst, bw);
        return HN_SUCCEEDED;
    }


    uint32_t HNemu::get_available_read_cluster_bw(uint32_t cluster, unsigned long long *bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        *bw = configuration[cluster].avail_read_cluster_bw;
        log.Debug("[HNemu] cluster %d, available read cluster bandwidth %llu",
                 cluster, *bw);
        return HN_SUCCEEDED;

    }

    uint32_t HNemu::get_available_write_cluster_bw(uint32_t cluster, unsigned long long *bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        *bw = configuration[cluster].avail_write_cluster_bw;
        log.Debug("[HNemu] cluster %d, available write cluster bandwidth %llu",
                 cluster, *bw);
        return HN_SUCCEEDED;
    }


    uint32_t HNemu::reserve_read_cluster_bw(uint32_t cluster, unsigned long long bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].avail_read_cluster_bw < bw)  {
            log.Debug("[HNemu] cluster %d has not enough read bandwidth available", cluster);
            return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
        }
        configuration[cluster].avail_read_cluster_bw -= bw;
        log.Debug("[HNemu] cluster %d, read cluster bandwidth reserved %llu",
                 cluster, bw);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::reserve_write_cluster_bw(uint32_t cluster, unsigned long long bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].avail_write_cluster_bw < bw)  {
            log.Debug("[HNemu] cluster %d has not enough write bandwidth available", cluster);
            return HN_NOT_ENOUGH_BANDWIDTH_AVAILABLE;
        }
        configuration[cluster].avail_write_cluster_bw -= bw;
        log.Debug("[HNemu] cluster %d, write cluster bandwidth reserved %llu",
                 cluster, bw);
        return HN_SUCCEEDED;
    }


    uint32_t HNemu::release_read_cluster_bw(uint32_t cluster, unsigned long long bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].avail_read_cluster_bw += bw;
        log.Debug("[HNemu] cluster %d, read cluster bandwidth released %llu",
                 cluster, bw);
        if (configuration[cluster].avail_write_cluster_bw > configuration[cluster].write_cluster_bw) {
            log.Error("[HNemu] cluster %d wrong read bandwidth settings",
                     cluster);
            return HN_WRONG_BANDWIDTH_SETTING;
        }
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::release_write_cluster_bw(uint32_t cluster, unsigned long long bw) {
        auto status = isCluster (cluster);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        configuration[cluster].avail_write_cluster_bw += bw;
        log.Debug("[HNemu] cluster %d, write cluster bandwidth released %llu",
                 cluster, bw);
        if (configuration[cluster].avail_write_cluster_bw > configuration[cluster].write_cluster_bw) {
            log.Error("[HNemu] cluster %d wrong write bandwidth settings",
                     cluster);
            return HN_WRONG_BANDWIDTH_SETTING;
        }
        return HN_SUCCEEDED;
    }


    uint32_t HNemu::checkMemory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *tile_mem,
                                           uint32_t *starting_addr) {
        log.Debug("[HNEmu]: checkMemory");
        auto err = get_free_memory(cluster, tile, size, starting_addr);
        if (err == HN_SUCCEEDED) {
            *tile_mem = tile;
            return 1;
        } else
            return 0;
    }

    inline uint32_t isValid(uint32_t tile_x, uint32_t tile_y, uint32_t num_tiles_x, uint32_t num_tiles_y) {
        if (tile_x < num_tiles_x && tile_y < num_tiles_y)
            return 1;
        else
            return 0;
    }


    uint32_t HNemu::find_memory(uint32_t cluster, uint32_t tile, uint32_t size, uint32_t *tile_mem,
                                           uint32_t *starting_addr) {
        log.Debug("[HNEmu]: find_memory");
        uint32_t tile_cur = tile;
        auto err = get_free_memory(cluster, tile, size, starting_addr);

        if (err == HN_SUCCEEDED) {
            *tile_mem = tile_cur;
            return HN_SUCCEEDED;
        }
        uint32_t num_tiles_x = configuration[cluster].num_cols,
                 num_tiles_y = configuration[cluster].num_rows;

        //von Neumann Neighborhood for HN_MAX_ROUTER_PORTS = 4
        uint32_t d, j, d_max = num_tiles_x + num_tiles_y - 2;
        uint32_t tile_x = tile % num_tiles_x;
        uint32_t tile_y = tile / num_tiles_x;
        int i;
        unsigned long long bw;

        for (d = 1; d < d_max; d++) {
            for (i = d, j = 0; i >= 0 && j <= d; i--, j++) {
                bw = 0;
                if (i > 0 && j > 0) {
                    if (isValid(tile_x + j, tile_y - i, num_tiles_x, num_tiles_y)) {
                        tile_cur = tile - i * num_tiles_x + j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }
                    if (isValid(tile_x - j, tile_y - i, num_tiles_x, num_tiles_y)) {
                        bw = 0;
                        tile_cur = tile - i * num_tiles_x - j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }

                    if (isValid(tile_x - j, tile_y + i, num_tiles_x, num_tiles_y)) {
                        bw = 0;
                        tile_cur = tile + i * num_tiles_x - j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }

                    if (isValid(tile_x + j, tile_y + i, num_tiles_x, num_tiles_y)) {
                        bw = 0;
                        tile_cur = tile + i * num_tiles_x + j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }
                } else if (j == 0) {
                    if (isValid(tile_x, tile_y - i, num_tiles_x, num_tiles_y) ) {
                        tile_cur = tile - i * num_tiles_x;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }

                    if (isValid(tile_x, tile_y + i, num_tiles_x, num_tiles_y)) {
                        bw = 0;
                        tile_cur = tile + i * num_tiles_x;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }
                } else if (i == 0) {
                    if (isValid(tile_x + j, tile_y, num_tiles_x, num_tiles_y)) {
                        tile_cur = tile + j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }
                    if(isValid(tile_x - j, tile_y, num_tiles_x, num_tiles_y)) {
                        bw = 0;
                        tile_cur = tile - j;
                        get_available_network_bw(cluster, tile, tile_cur, &bw);
                        if (bw > 0 &&
                            checkMemory(cluster, tile_cur, size, tile_mem, starting_addr)) {
                            log.Debug("[HNemu] cluster %d, tile %d, free memory slot size %d, starting address 0x%x",
                                      cluster, *tile_mem, size, *starting_addr);
                            return HN_SUCCEEDED;
                        }
                    }
                }
            }
        }
        *starting_addr = 0;
        *tile_mem = 0;
        log.Debug("[HNemu] cluster %d not enough memory available", cluster);
        return HN_NOT_ENOUGH_MEMORY_AVAILABLE;
    }

    uint32_t HNemu::allocate_memory(uint32_t cluster, uint32_t tile, uint32_t addr, uint32_t size) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].tile_info[tile].memory_size == 0) {
            log.Error("[HNemu] allocate_memory: cluster %d, tile %d does not have memory attached", cluster, tile);
            return HN_MEMORY_NOT_PRESENT_IN_TILE;
        }
        auto mem = configuration[cluster].tile_info[tile].first_memory_slot;
        auto prev = configuration[cluster].tile_info[tile].first_memory_slot;
        //find memory slot with start_address = addr
        while (mem->start_address != addr) {
            prev = mem;
            mem = (hn_rscmgt_memory_slot_t *) mem->next_memory_slot;
            if (mem == nullptr) {
                log.Error("[HNemu] cluster %d, tile %d  memory slot size %d, starting address 0x%x does not exist",
                cluster, tile, size, addr);
                return HN_FIND_MEMORY_ERROR;
            }
        }
        if (mem->size < size) {
            log.Debug("[HNemu] cluster %d, tile %d not enough memory available", cluster, tile);
            return HN_NOT_ENOUGH_MEMORY_AVAILABLE;
        }
        configuration[cluster].tile_info[tile].free_memory -= size;
        //delete full slot
        if ((prev != mem) && (mem->size == size)) {
            prev->next_memory_slot = mem->next_memory_slot;
            free(mem);
        } else { //change slot (no free space: start = end+1, size = 0
            mem->start_address += size;
            mem->size -= size;
        }
        log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x allocated successfully",
            cluster, tile, size, addr);
        return HN_SUCCEEDED;

    }


    uint32_t HNemu::release_memory(uint32_t cluster, uint32_t tile, uint32_t addr, uint32_t size) {
        auto status = isTile (cluster, tile);
        if (status != HN_SUCCEEDED) {
            return status;
        }
        if (configuration[cluster].tile_info[tile].memory_size == 0) {
            log.Error("[HNemu] release_memory: cluster %d, tile %d does not have memory attached", cluster, tile);
            return HN_MEMORY_NOT_PRESENT_IN_TILE;
        }
        auto mem = configuration[cluster].tile_info[tile].first_memory_slot;
        auto prev = configuration[cluster].tile_info[tile].first_memory_slot;
        configuration[cluster].tile_info[tile].free_memory += size;
        while (mem != nullptr) {
            if ((prev->start_address < addr) && (mem->start_address > addr)) {
                if ((prev->end_address + 1) == addr && (addr + size) != mem->start_address) {
                    prev->size += size;
                    prev->end_address += size;
                    log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                        cluster, tile, size, addr);
                    return HN_SUCCEEDED;
                } else if ((prev->end_address + 1) != addr && (addr + size) == mem->start_address) {
                    mem->size += size;
                    mem->start_address = addr;
                    log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                        cluster, tile, size, addr);
                    return HN_SUCCEEDED;
                } else if ((prev->end_address + 1) == addr && (addr + size) == mem->start_address) {
                    prev->size += size + mem->size;
                    prev->end_address = mem->end_address;
                    prev->next_memory_slot = mem->next_memory_slot;
                    free(mem);
                    log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                        cluster, tile, size, addr);
                    return HN_SUCCEEDED;
                } else if ((prev->end_address + 1) != addr && (addr + size) != mem->start_address) {
                    auto slot = (hn_rscmgt_memory_slot_t *) malloc(sizeof(hn_rscmgt_memory_slot_t));
                    slot->size = size;
                    slot->start_address = addr;
                    slot->end_address = addr + size - 1;
                    slot->next_memory_slot = mem;
                    prev->next_memory_slot = slot;
                    log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                        cluster, tile, size, addr);
                    return HN_SUCCEEDED;
                }
            } else {
                prev = mem;
                mem = (hn_rscmgt_memory_slot_t *) mem->next_memory_slot;
            }
        }
        if ((addr + size) == configuration[cluster].tile_info[tile].first_memory_slot->start_address) {
            configuration[cluster].tile_info[tile].first_memory_slot->size += size;
            configuration[cluster].tile_info[tile].first_memory_slot->start_address = addr;
            log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                cluster, tile, size, addr);
            return HN_SUCCEEDED;
        } else if ((prev->end_address + 1) == addr) {
            prev->size += size;
            prev->end_address += size;
            log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                cluster, tile, size, addr);
            return HN_SUCCEEDED;
        } else {
            auto slot = (hn_rscmgt_memory_slot_t *) malloc(sizeof(hn_rscmgt_memory_slot_t));
            slot->size = size;
            slot->start_address = addr;
            slot->end_address = addr + size - 1;
            if (addr > prev->start_address) {
                prev->next_memory_slot = slot;
                slot->next_memory_slot = nullptr;
            } else {
                mem = configuration[cluster].tile_info[tile].first_memory_slot;
                configuration[cluster].tile_info[tile].first_memory_slot = slot;
                slot->next_memory_slot = mem;
            }
            log.Debug("[HNemu] cluster %d, tile %d, memory slot size %d, starting address 0x%x released successfully",
                cluster, tile, size, addr);
            return HN_SUCCEEDED;
        }

    }

    uint32_t HNemu::get_network_distance(uint32_t cluster, uint32_t tile_src, uint32_t tile_dst, uint32_t *dst) {
        //XY route for HN_MAX_ROUTER_PORTS = 4
        auto status_src = isTile (cluster, tile_src);
        auto status_dst = isTile (cluster, tile_dst);
        if (status_src != HN_SUCCEEDED || status_dst != HN_SUCCEEDED) {
            return (status_src!= HN_SUCCEEDED)? status_src : status_dst;
        }
        uint32_t tile_cur_x = tile_src % configuration[cluster].num_cols;
        uint32_t tile_cur_y = tile_src / configuration[cluster].num_cols;
        uint32_t tile_dst_x = tile_dst % configuration[cluster].num_cols;
        uint32_t tile_dst_y = tile_dst / configuration[cluster].num_cols;
        *dst = ((tile_cur_x > tile_dst_x) ? (tile_cur_x - tile_dst_x) :
                (tile_dst_x - tile_cur_x)) + ((tile_cur_y > tile_dst_y) ?
                                              (tile_cur_y - tile_dst_y) : (tile_dst_y - tile_cur_y));
        log.Debug("[HNemu] cluster %d, source tile %d, destination tile %d, distance (hops) %d",
                 cluster, tile_src, tile_dst, *dst);
        return HN_SUCCEEDED;
    }

    uint32_t HNemu::checkType(uint32_t cluster, uint32_t tile_cur, uint32_t *num_tiles_cur, uint32_t num_tiles,
                                         uint32_t *tiles_cur, const uint32_t *types) {

        if (*num_tiles_cur >= num_tiles) return 0;
        for (uint32_t i = 0; i < num_tiles; i++) {
            if (configuration[cluster].tile_info[tile_cur].assigned == 0 &&
                configuration[cluster].tile_info[tile_cur].preassigned == 0 &&
                configuration[cluster].tile_info[tile_cur].type == types[i] &&
                tiles_cur[i] == configuration[cluster].num_tiles) {
                tiles_cur[i] = tile_cur;
                configuration[cluster].tile_info[tile_cur].preassigned = 1;
                *num_tiles_cur += 1;
                return 1;
            }
        }
        return 0;
    }

    inline void update_bounds(uint32_t cur_x, uint32_t cur_y, uint32_t *start_x, uint32_t *start_y, uint32_t *end_x,
                       uint32_t *end_y) {
        *start_x = (cur_x < *start_x) ? cur_x : *start_x;
        *start_y = (cur_y < *start_y) ? cur_y : *start_y;
        *end_x = (cur_x > *end_x) ? cur_x : *end_x;
        *end_y = (cur_y > *end_y) ? cur_y : *end_y;
    }

    uint32_t HNemu::find_single_units_set(uint32_t cluster, uint32_t tile, uint32_t num_tiles, uint32_t types[],
                                                     uint32_t **tiles_dst, uint32_t *dst) {
        //von Neumann Neighborhood for HN_MAX_ROUTER_PORTS = 4
        uint32_t *tiles_dst_cur = *tiles_dst;
        uint32_t num_tiles_x = configuration[cluster].num_cols,
                num_tiles_y = configuration[cluster].num_rows,
                d, j, d_max = num_tiles_x + num_tiles_y - 2, sum_dst = *dst;
        int i;
        uint32_t num_tiles_cur = 0, tile_cur, tile_cur_x, tile_cur_y;
        uint32_t tile_x = tile % num_tiles_x;
        uint32_t tile_y = tile / num_tiles_x;
        for (d = 0; d < d_max && num_tiles_cur < num_tiles; d++) {
            for (i = d, j = 0; i >= 0 && j <= d && num_tiles_cur < num_tiles; i--, j++) {
                if (i > 0 && j > 0) {
                    tile_cur = tile - i * num_tiles_x + j;
                    tile_cur_x = tile_x + j;
                    tile_cur_y = tile_y - i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }

                    tile_cur = tile - i * num_tiles_x - j;
                    tile_cur_x = tile_x - j;
                    tile_cur_y = tile_y - i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }

                    tile_cur = tile + i * num_tiles_x - j;
                    tile_cur_x = tile_x - j;
                    tile_cur_y = tile_y + i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }

                    tile_cur = tile + i * num_tiles_x + j;
                    tile_cur_x = tile_x + j;
                    tile_cur_y = tile_y + i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }
                } else if (j == 0) {
                    tile_cur = tile - i * num_tiles_x;
                    tile_cur_x = tile_x;
                    tile_cur_y = tile_y - i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }

                    tile_cur = tile + i * num_tiles_x;
                    tile_cur_x = tile_x;
                    tile_cur_y = tile_y + i;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }
                } else if (i == 0) {
                    tile_cur = tile + j;
                    tile_cur_x = tile_x + j;
                    tile_cur_y = tile_y;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }

                    tile_cur = tile - j;
                    tile_cur_x = tile_x - j;
                    tile_cur_y = tile_y;
                    if (isValid(tile_cur_x, tile_cur_y, num_tiles_x, num_tiles_y) &&
                        checkType(cluster, tile_cur, &num_tiles_cur, num_tiles, tiles_dst_cur, types)) {
                        sum_dst += d;
                    }
                }
            }
        }
        *dst = sum_dst;
//      clean preassigment
        for (uint32_t k = 0; k < num_tiles_cur; k++) {
            configuration[cluster].tile_info[tiles_dst_cur[k]].preassigned = 0;
        }
        if (num_tiles_cur == num_tiles) {
            return HN_SUCCEEDED;
        } else {
            return HN_PARTITION_NOT_FOUND;
        }
    }

    uint32_t HNemu::find_units_set(uint32_t cluster, uint32_t num_tiles, uint32_t types[], uint32_t **tiles_dst){
        if (num_tiles == 0) {
            return HN_SUCCEEDED;
        }
        uint32_t **tiles_cur, **tiles_min = nullptr;
        auto tiles1 = (uint32_t *) malloc(sizeof(*tiles_dst) * (num_tiles));
        auto tiles2 = (uint32_t *) malloc(sizeof(*tiles_dst) * (num_tiles));

        tiles_cur = &tiles1;
        uint32_t i, sum_dst = 0, min_dst = 0, found = 0;

        for (uint32_t tile_cur = 0; tile_cur < configuration[cluster].num_tiles; tile_cur++) {
            if (configuration[cluster].tile_info[tile_cur].memory_size == 0) continue;
            if (configuration[cluster].tile_info[tile_cur].assigned == 1) continue;

            for (i = 0; i < (num_tiles); i++) {
                (*tiles_cur)[i] = configuration[cluster].num_tiles;
            }

            sum_dst = 0;
            auto err = find_single_units_set(cluster, tile_cur, num_tiles, types, tiles_cur, &sum_dst);
            if (err == HN_PARTITION_NOT_FOUND) continue;

            if (found == 0) {
                min_dst = sum_dst;
                tiles_min = tiles_cur;
                tiles_cur = &tiles2;
                found = 1;
            } else if (min_dst > sum_dst) {
                min_dst = sum_dst;
                auto tmp = tiles_min;
                tiles_min = tiles_cur;
                tiles_cur = tmp;
            }
        }
        if (found) {
            *tiles_dst = *tiles_min;
            if (&tiles1 != tiles_min) {
                free(tiles1);
                tiles1= nullptr;
            } else {
                free(tiles2);
                tiles2= nullptr;
            }
            log.Debug("[HNemu] find_units_set cluster %d, number of requested tiles %d, set found",
                     cluster, num_tiles);
            return HN_SUCCEEDED;
        } else {
            free(tiles1);
            tiles1= nullptr;
            free(tiles2);
            tiles2= nullptr;
            log.Debug("[HNemu] find_units_set cluster %d, number of requested tiles %d, set not found",
                     cluster, num_tiles);
            return HN_PARTITION_NOT_FOUND;
        }
    }

    uint32_t HNemu::find_units_sets(uint32_t cluster, uint32_t num_tiles, uint32_t types[], uint32_t ***tiles_dst,
            uint32_t *num){

        if (num_tiles == 0) {
            return HN_SUCCEEDED;
        }
        uint32_t i, sum_dst = 0, num_cur = 0;
        uint32_t **tiles_dst_cur = nullptr;
        auto tiles_cur = (uint32_t *) malloc(sizeof(**tiles_dst) * (num_tiles));

        for (i = 0; i < num_tiles; i++)
            tiles_cur[i] = configuration[cluster].num_tiles;

        for (uint32_t tile_cur = 0; tile_cur < configuration[cluster].num_tiles; tile_cur++) {
            if (configuration[cluster].tile_info[tile_cur].memory_size == 0) continue;
            if (configuration[cluster].tile_info[tile_cur].assigned == 1) continue;

            sum_dst = 0;
            auto err = find_single_units_set(cluster, tile_cur, num_tiles, types, &tiles_cur, &sum_dst);
            if (err == HN_PARTITION_NOT_FOUND) continue;

            num_cur += 1;
            tiles_dst_cur = (uint32_t **) realloc(tiles_dst_cur, sizeof(*tiles_dst_cur) * num_cur);
            if (tiles_dst_cur != nullptr) {
                tiles_dst_cur[num_cur - 1] = (uint32_t *) malloc(num_tiles * sizeof(uint32_t));
            } else {
                for (i = 0; i < num_cur; i++) {
                    free(tiles_dst_cur[i]);
                }
                free(tiles_dst_cur);
                return HN_PARTITION_NOT_FOUND;
            }
            for (i = 0; i < num_tiles; i++) {
                tiles_dst_cur[num_cur - 1][i] = tiles_cur[i];
                tiles_cur[i] = configuration[cluster].num_tiles;
            }
        }
        *num = num_cur;
        if (num_cur > 0) {
            *tiles_dst = tiles_dst_cur;
            log.Debug("[HNemu] find_units_sets cluster %d, number of requested tiles %d, number of available sets %d",
                     cluster, num_tiles, num_cur);
            return HN_SUCCEEDED;
        } else {
            free(tiles_dst_cur);
            log.Debug("[HNemu] find_units_sets cluster %d, number of requested tiles %d, sets not found",
                     cluster, num_tiles);
            return HN_PARTITION_NOT_FOUND;
        }
    }

/*
 * \brief This function reserves a set of tiles
 */
    uint32_t HNemu::reserve_units_set(uint32_t cluster, uint32_t num_tiles, const uint32_t *tiles){

        uint32_t err, status;
        uint32_t num_tiles_x = configuration[cluster].num_cols,
                num_tiles_y = configuration[cluster].num_rows;
        uint32_t tile_x = tiles[0] % num_tiles_x;
        uint32_t tile_y = tiles[0] / num_tiles_x;
        uint32_t start_x = tile_x, start_y = tile_y;
        uint32_t end_x = tile_x, end_y = tile_y;

        for (uint32_t i = 0; i<num_tiles; i++){
            err = is_tile_assigned(cluster, tiles[i], &status);
            if (err!=HN_SUCCEEDED) {
                log.Debug("[HNemu] reserve_units_set cluster %d, number of requested tiles %d, error occured while checking tiles, release %d already reserved tiles",
                         cluster, num_tiles, i);
                for (uint32_t j = 0; j<i; j++){
                        set_tile_avail(cluster, tiles[j]);
                }
               //TODO error code?
                return HN_RSC_ERROR;
            }

            if (status==0){
                set_tile_assigned(cluster, tiles[i]);
                update_bounds(tiles[i] % num_tiles_x, tiles[i] / num_tiles_x, &start_x, &start_y, &end_x, &end_y);
            } else {
                log.Debug("[HNemu] reserve_units_set cluster %d, number of requested tiles %d, tile %d is currently assigned, release %d already reserved tiles",
                         cluster, num_tiles, tiles[i], i);
                for (uint32_t j = 0; j<i; j++){
                    set_tile_avail(cluster, tiles[j]);
                }
                //TODO error code?
                return HN_RSC_ERROR;
            }
        }
//      include all tiles in isolated area
        uint32_t diff_x = (end_x<start_x) ? (start_x-end_x) : (end_x-start_x);
        uint32_t diff_y = (end_y<start_y) ? (start_y-end_y) : (end_y-start_y);
        uint32_t num_tiles_final = (diff_x + 1) * (diff_y + 1);
        if (num_tiles_final > num_tiles) {
            for (uint32_t i = start_y; i <= end_y; i++) {
                for (uint32_t j = start_x; j <= end_x; j++) {
                    if (configuration[cluster].tile_info[i * num_tiles_x + j].assigned == 0) {
                        configuration[cluster].tile_info[i * num_tiles_x + j].assigned = 1;
                    }
                }
            }
        }
        log.Debug("[HNemu] reserve_units_set cluster %d, number of requested tiles %d, reserve %d tiles in isolated area (start: [%d,%d], end: [%d,%d])",
                 cluster, num_tiles, num_tiles_final, start_x, start_y, end_x, end_y);
//      isolate area using bw
        if (start_y > 0){
            for (uint32_t i = start_x; i <= end_x; i++) {
                uint32_t bound_tile = (start_y - 1) * num_tiles_x + i;
                uint32_t cur_tile = (start_y) * num_tiles_x + i;
                configuration[cluster].tile_info[bound_tile].avail_south_port_bw = 0;
                configuration[cluster].tile_info[cur_tile].avail_north_port_bw = 0;
            }
        }
        if (end_y < (num_tiles_y-1)){
            for (uint32_t i = start_x; i <= end_x; i++) {
                uint32_t bound_tile = (end_y + 1) * num_tiles_x + i;
                uint32_t cur_tile = (end_y) * num_tiles_x + i;
                configuration[cluster].tile_info[bound_tile].avail_north_port_bw = 0;
                configuration[cluster].tile_info[cur_tile].avail_south_port_bw = 0;
            }
        }
        if (start_x > 0){
            for (uint32_t i = start_y; i <= end_y; i++) {
                uint32_t bound_tile = i * num_tiles_x + (start_x-1);
                uint32_t cur_tile = i * num_tiles_x + (start_x);
                configuration[cluster].tile_info[bound_tile].avail_east_port_bw = 0;
                configuration[cluster].tile_info[cur_tile].avail_west_port_bw = 0;
            }
        }
        if (end_x < (num_tiles_x-1)){
            for (uint32_t i = start_y; i <= end_y; i++) {
                uint32_t bound_tile = i * num_tiles_x + (end_x+1);
                uint32_t cur_tile = i * num_tiles_x + (end_x);
                configuration[cluster].tile_info[bound_tile].avail_west_port_bw = 0;
                configuration[cluster].tile_info[cur_tile].avail_east_port_bw = 0;
            }
        }
        return HN_SUCCEEDED;
    }

/*
 * \brief This function releases a set of tiles
 */
    uint32_t HNemu::release_units_set(uint32_t cluster, uint32_t num_tiles, const uint32_t *tiles){
        uint32_t status = 0;
        uint32_t num_tiles_x = configuration[cluster].num_cols,
                num_tiles_y = configuration[cluster].num_rows;
        uint32_t tile_x = tiles[0] % num_tiles_x;
        uint32_t tile_y = tiles[0] / num_tiles_x;
        uint32_t start_x = tile_x, start_y = tile_y;
        uint32_t end_x = tile_x, end_y = tile_y;

        for (uint32_t i = 0; i<num_tiles; i++){
            is_tile_assigned(cluster, tiles[i], &status);
            if (status){
                set_tile_avail(cluster, tiles[i]);
            }
            update_bounds(tiles[i] % num_tiles_x, tiles[i] / num_tiles_x, &start_x, &start_y, &end_x, &end_y);
        }
        //release all tiles in isolated area
        uint32_t diff_x = (end_x<start_x) ? (start_x-end_x) : (end_x-start_x);
        uint32_t diff_y = (end_y<start_y) ? (start_y-end_y) : (end_y-start_y);
        uint32_t num_tiles_final = (diff_x + 1) * (diff_y + 1);
        if (num_tiles_final > num_tiles) {
            for (uint32_t i = start_y; i <= end_y; i++) {
                for (uint32_t j = start_x; j <= end_x; j++) {
                    if (configuration[cluster].tile_info[i * num_tiles_x + j].assigned == 1) {
                        configuration[cluster].tile_info[i * num_tiles_x + j].assigned = 0;
                    }
                }
            }
        }
        log.Debug("[HNemu] release_units_set cluster %d, number of requested tiles %d, release %d tiles in isolated area (start: [%d,%d], end: [%d,%d])",
                 cluster, num_tiles, num_tiles_final, start_x, start_y, end_x, end_y);
//      release bw of bounds of isolated area
        if (start_y > 0){
            for (uint32_t i = start_x; i <= end_x; i++) {
                uint32_t bound_tile = (start_y - 1) * num_tiles_x + i;
                uint32_t cur_tile = (start_y) * num_tiles_x + i;
                if (configuration[cluster].tile_info[bound_tile].assigned == 0) {
                    configuration[cluster].tile_info[bound_tile].avail_south_port_bw = configuration[cluster].tile_info[bound_tile].south_port_bw;
                    configuration[cluster].tile_info[cur_tile].avail_north_port_bw = configuration[cluster].tile_info[cur_tile].north_port_bw;
                }
            }
        }
        if (end_y < (num_tiles_y-1)){
            for (uint32_t i = start_x; i <= end_x; i++) {
                uint32_t bound_tile = (end_y + 1) * num_tiles_x + i;
                uint32_t cur_tile = (end_y) * num_tiles_x + i;
                if (configuration[cluster].tile_info[bound_tile].assigned == 0) {
                    configuration[cluster].tile_info[bound_tile].avail_north_port_bw = configuration[cluster].tile_info[bound_tile].north_port_bw;
                    configuration[cluster].tile_info[cur_tile].avail_south_port_bw = configuration[cluster].tile_info[cur_tile].south_port_bw;
                }
            }
        }
        if (start_x > 0){
            for (uint32_t i = start_y; i <= end_y; i++) {
                uint32_t bound_tile = i * num_tiles_x + (start_x-1);
                uint32_t cur_tile = i * num_tiles_x + (start_x);
                if (configuration[cluster].tile_info[bound_tile].assigned == 0) {
                    configuration[cluster].tile_info[bound_tile].avail_east_port_bw = configuration[cluster].tile_info[bound_tile].east_port_bw;
                    configuration[cluster].tile_info[cur_tile].avail_west_port_bw = configuration[cluster].tile_info[cur_tile].west_port_bw;
                }
            }
        }
        if (end_x < (num_tiles_x-1)){
            for (uint32_t i = start_y; i <= end_y; i++) {
                uint32_t bound_tile = i * num_tiles_x + (end_x+1);
                uint32_t cur_tile = i * num_tiles_x + (end_x);
                if (configuration[cluster].tile_info[bound_tile].assigned == 0) {
                    configuration[cluster].tile_info[bound_tile].avail_west_port_bw = configuration[cluster].tile_info[bound_tile].west_port_bw;
                    configuration[cluster].tile_info[cur_tile].avail_east_port_bw = configuration[cluster].tile_info[cur_tile].east_port_bw;
                }
            }
        }
        return  HN_SUCCEEDED;
    }

}


