#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <sys/stat.h>  // For stat() and mkdir()
#include <cstdlib>     // For system()

#include "utils.h"
#include "AlpEncoder.h"
#include "AlpDecoder.h"

#define COMPRESS_ALP
// #define IS_DEBUG

using CoeffPair = std::pair<double, double>;
using CoeffArr = std::vector<std::vector<double>>;
using OrderArr = std::vector<bool>;


// constexpr int L1_LEVEL = 2;
// constexpr int L2_LEVEL = 3;

constexpr int L1_LEVEL = 12;
constexpr int L2_LEVEL = 20;

constexpr int L1_SIZE = 1 << L1_LEVEL; // 4096
constexpr int L1_SIZE_TIMES2 = L1_SIZE*2; // 8192
constexpr int L2_SIZE = 1 << L2_LEVEL; // 1048576


class VariableManager
{
private:
    std::vector<double> L1_min, L1_max;              // decoded aggregate coefficients
    std::vector<double> L2_min, L2_max;              // uncompressed detail coefficients
#ifdef COMPRESS_ALP
    std::vector<CompressedData *> L3_min, L3_max; // compressed detail coefficients
#else
    std::vector<std::vector<double>> L3_min, L3_max; // uncompressed detail coefficients
#endif
    OrderArr bottom_order;
    size_t variable_id;
    size_t data_size;

    void build_multilevel_cache(const CoeffPair& top_aggregate, CoeffArr &input_data, bool is_min);
    CoeffPair minmax_decode_pair(double agg_parent, double detail, bool is_min) const;

public:
    VariableManager(const size_t variable_id) : variable_id(variable_id)
    {
        std::cout << "VariableManager initialized for variable: " << variable_id << std::endl;
        L1_min.resize(L1_SIZE_TIMES2);
        L1_max.resize(L1_SIZE_TIMES2);
        L2_min.resize(L2_SIZE - L1_SIZE);
        L2_max.resize(L2_SIZE - L1_SIZE);
    };
    ~VariableManager()
    {
#ifdef COMPRESS_ALP
        for (auto &compressed_data : L3_min)
        {
            delete compressed_data;
        }
        for (auto &compressed_data : L3_max)
        {
            delete compressed_data;
        }
#endif
    };
    
    bool load_minmax_data(const std::string &filename, const size_t original_data_size);
    bool load_order_data(const std::string &path_string, OrderArr &data);
    std::vector<double> get_all_decoded_data(size_t original_data_size) const;
    std::pair<CoeffPair, CoeffPair> minmax_decode_index(double agg_min, double agg_max, size_t index, size_t original_level, int thread_id) const;

    void print_details() const;
    static void print_encoded_result(const std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr> &result);
    void print_decoded_levels() const;
    void write_decoded_data(const std::string &path_string, size_t original_data_size) const;
    void print_query_path(size_t index, size_t original_level) const;
};

class MemoryManager
{
private:
    std::vector<VariableManager> variables;
    std::string data_name;
    size_t data_size;
    size_t original_level;

public:
    MemoryManager(const std::string &data_name) : data_name(data_name)
    {
        std::cout << "MemoryManager initialized for data: " << data_name << std::endl;
    };
    ~MemoryManager() {};

    std::pair<CoeffPair, CoeffPair> minmax_decode_index(double agg_min, double agg_max, size_t index, size_t variable_id, int thread_id);
    bool load_minmax_data(int thread_num);
    void print_variables_status() const;
    void write_decoded_data(const std::string &path_string, size_t variable_id) const;
    void print_query_path(size_t index, size_t variable_id) const;
    void write_decoded_data_all(const std::string &path_string) const;
    size_t get_data_size() const { return data_size; }
};


#endif // MEMORYMANAGER_H