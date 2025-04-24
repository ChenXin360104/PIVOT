#include "MemoryManager.h"
#include <unistd.h>
#include <sys/stat.h> // for stat
bool fileExists(const std::string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
  }

std::pair<CoeffPair, CoeffPair> MemoryManager::minmax_decode_index(double agg_min, double agg_max, size_t index, size_t variable_id, int thread_id)
{
    return variables[variable_id].minmax_decode_index(agg_min, agg_max, index, this->original_level, thread_id);
}


void MemoryManager::write_decoded_data(const std::string &path_string, size_t variable_id) const
{
    return variables[variable_id].write_decoded_data(path_string, this->data_size);
}

void MemoryManager::print_query_path(size_t index, size_t variable_id) const
{
    return variables[variable_id].print_query_path(index, this->original_level);
}

void MemoryManager::write_decoded_data_all(const std::string &path_string) const
{
    std::vector<std::vector<double>> all_decoded_data;
    for (size_t i = 0; i < variables.size(); ++i)
    {
        all_decoded_data.push_back(variables[i].get_all_decoded_data(this->data_size));
    }
    std::ofstream outputFile(path_string + ".csv");
    outputFile << "t";
    for (size_t i = 0; i < all_decoded_data.size(); ++i)
    {
        outputFile << ",v" << i+1;
    }
    outputFile << std::endl;
    outputFile << std::fixed << std::setprecision(12);
    for (size_t i = 0; i < all_decoded_data[0].size(); ++i)
    {
        outputFile << i;
        for (size_t j = 0; j < variables.size(); ++j)
        {
            outputFile << "," << all_decoded_data[j][i];
        }
        outputFile << std::endl;
    }
    outputFile.close();
}

/**
 * @brief Loads MinMax encoded data from binary files
 * 
 * This function:
 * 1. Reads metadata file to get column count and data size
 * 2. Loads each variable's encoded data
 * 3. Stores loaded variables in member vector
 *
 * File Structure:
 * - Metadata (.bin): 
 *   - Column count (size_t)
 *   - Data size (size_t)
 * - Variable data: Loaded through VariableManager::load_minmax_data
 *
 * @return true if all data loaded successfully, false on any error
 * 
 * @note Sets data_size to 0 on read failure
 * @note Reserves space in variables vector before loading
 * @note Performs integrity checks on loaded data
 */
bool MemoryManager::load_minmax_data(int thread_num)
{
    // char cwd[500];
    // if (getcwd(cwd, sizeof(cwd)) != NULL) {
    //   std::cout << "当前路径: " << cwd << std::endl;
    // } else {
    //   std::cerr << "获取当前路径失败" << std::endl;
    //   return 1;
    // }
    std::string path_string = "../om3_data/" + this->data_name + "_om3.bin";
    // 也可以直接使用相对路径，前提是程序运行的目录是期望的目录
    // if (fileExists(path_string)) {
    //     std::cout << "文件存在 (相对路径): " << path_string << std::endl;
    //   } else {
    //     std::cout << "文件不存在 (相对路径): " << path_string << std::endl;
    //   }
    std::ifstream infile(path_string, std::ios::binary);
    if (!infile)
    {
        std::cerr << "Error: Cannot open file for reading: " << path_string << std::endl;
        this->data_size = 0; return false;
    }
    
    size_t col_count;
    // 1. Read original variable num & data size
    infile.read(reinterpret_cast<char*>(&col_count), sizeof(col_count));
    variables.reserve(col_count);
    if (!infile) { this->data_size = 0; return false; } // Basic check
    infile.read(reinterpret_cast<char*>(&this->data_size), sizeof(this->data_size));
    if (!infile) { this->data_size = 0; return false; } // Basic check
    this->original_level = BitScanReverse(this->data_size);

    // 2. Read each variable's data
    for (size_t i = 0; i < col_count; ++i)
    {
        variables.emplace_back(i);
        auto &vm=variables.back();
        // VariableManager vm(i);
        if (!vm.load_minmax_data("../om3_data/"+this->data_name + "_om3_v" + std::to_string(i+1), this->data_size))
        {
            std::cerr << "Error: Failed to load variable data for variable ID: " << i+1 << std::endl;
            this->data_size = 0; return false;
        }
        // vm.print_decoded_levels();
        // variables.emplace_back(std::move(vm));
    }

#ifdef COMPRESS_ALP
    if(thread_num<0)
    {
        std::cerr << "Error: thread_num should be greater than 0" << std::endl;
        this->data_size = 0; return false;
    }
    MAX_THREADS = thread_num;
#endif

    // Basic check if we reached EOF unexpectedly or read failed
    if (infile.peek() != EOF && !infile.good())
    {
        std::cerr << "Warning: File read might be incomplete or corrupted: " << path_string << std::endl;
        this->data_size = 0; return false;
    }

    return true;
}

void MemoryManager::print_variables_status() const {
    std::cout << "\n=== Memory Manager Status ===" << std::endl;
    std::cout << "Data Name: " << data_name << std::endl;
    std::cout << "Data Size: " << data_size << std::endl;
    std::cout << "Number of Variables: " << variables.size() << std::endl;
    
    // Print status for each variable
    for (const auto& var : variables) {
        var.print_details();
    }
}

CoeffPair VariableManager::minmax_decode_pair(double agg_parent, double detail, bool is_min) const
{
    double child_left, child_right;

    if(is_min) {
        // --- Decode the 'min' components (first elements) ---
        if (detail < 0) { // D < 0
            child_left = agg_parent;             // A^j_{4i} = A
            child_right = agg_parent - detail; // A^j_{4i+2} = A - D
        } else { // D >= 0
            child_right = agg_parent;            // A^j_{4i+2} = A
            child_left = agg_parent + detail; // A^j_{4i} = A + D
        }
    } else {
        // --- Decode the 'max' components (second elements) ---
        if (detail < 0) { // D < 0
            child_right = agg_parent;             // A^j_{4i+2} = A
            child_left = agg_parent + detail; // A^j_{4i} = A + D
        } else { // D >= 0
            child_left = agg_parent;            // A^j_{4i} = A
            child_right = agg_parent - detail; // A^j_{4i+2} = A - D
        }
    }

    return {child_left, child_right};
}

std::pair<CoeffPair, CoeffPair>
VariableManager::minmax_decode_index(double agg_min, double agg_max, size_t index, size_t original_level, int thread_id) const
{
    // if(index == 0) throw std::runtime_error("Error: index 0 is not valid for decoding");
    if(index == 0) return {{L1_min[1], L1_max[1]}, {L1_min[1], L1_max[1]}};

    size_t level = BitScanReverse(index);
    size_t level_index = index - (1 << level);
    if(level > original_level) {
        throw std::runtime_error("index out of range ("+std::to_string(index)+")");
    } else if(level == original_level) {
        // is the last level
        if(this->bottom_order[level_index]) {
            // the first element of the last level
            return {{agg_min, agg_min}, {agg_max, agg_max}};
        } else {
            return {{agg_max, agg_max}, {agg_min, agg_min}};
        }
    } else {
        if(index < L1_SIZE) {
            // L1 cache
            return {{L1_min[2*index], L1_max[2*index]}, {L1_min[2*index+1], L1_max[2*index+1]}};
        } else if(index < L2_SIZE) {
            // L2 cache
            CoeffPair min_pair = minmax_decode_pair(agg_min, L2_min[index-L1_SIZE], true);
            CoeffPair max_pair = minmax_decode_pair(agg_max, L2_max[index-L1_SIZE], false);
            return {{min_pair.first, max_pair.first}, {min_pair.second, max_pair.second}};
        } else {
            // L3 cache
            CoeffPair min_pair, max_pair;
            #ifdef COMPRESS_ALP
            if(level_index == L3_min[level-L2_LEVEL]->size()) {
                min_pair = minmax_decode_pair(agg_min, 0.0, true);
                max_pair = minmax_decode_pair(agg_max, 0.0, false);
            } else {
                auto compressed_data = L3_min[level-L2_LEVEL];
                min_pair = minmax_decode_pair(agg_min, get_decoded_value_p(compressed_data, level_index, thread_id), true);
                compressed_data = L3_max[level-L2_LEVEL];
                max_pair = minmax_decode_pair(agg_max, get_decoded_value_p(compressed_data, level_index, thread_id), false);
            }
            #else
            if(level_index == L3_min[level-L2_LEVEL].size()) {
                min_pair = minmax_decode_pair(agg_min, 0.0, true);
                max_pair = minmax_decode_pair(agg_max, 0.0, false);
            } else {
                min_pair = minmax_decode_pair(agg_min, L3_min[level-L2_LEVEL][level_index], true);
                max_pair = minmax_decode_pair(agg_max, L3_max[level-L2_LEVEL][level_index], false);
            }
            #endif
            return {{min_pair.first, max_pair.first}, {min_pair.second, max_pair.second}};
        }
    }
}



/**
 * @brief Calculates the size of each level reducing from n down to 1.
 *
 * Non-powers of 2 are conceptually padded to the next power of 2
 * to determine the number of levels, but the size only counts the
 * original 'n' elements spread across those levels using floor division.
 * The reduction continues until the padded size reaches 1.
 *
 * @param n A positive integer representing the initial size
 * @return A vector of integers representing the effective size of each level
 * @throws std::invalid_argument If n is not a positive integer
 *
 * @example
 * calculate_level_sizes(9)  // returns {4, 2, 1, 1}
 * calculate_level_sizes(10) // returns {5, 2, 1, 1}
 * calculate_level_sizes(16) // returns {8, 4, 2, 1}
 * calculate_level_sizes(18) // returns {9, 4, 2, 1, 1}
 * calculate_level_sizes(28) // returns {14, 7, 3, 2, 1}
 */
std::vector<size_t> calculate_level_sizes(size_t n) {
    if (n < 1) {
        throw std::invalid_argument("Input 'n' must be a positive integer.");
    }
    if (n == 1) {
        return {1};
    }
    
    std::vector<size_t> level_sizes;
    level_sizes.reserve(32);  // Pre-allocate space for reasonable depth

    size_t current_size = n;
    while (current_size > 1) {
        size_t num_pairs = current_size / 2;  // 实际合并的对数
        level_sizes.push_back(num_pairs);     // details 的数量
        current_size = (current_size + 1) / 2;  // 下一层的元素数量
    }

    return level_sizes;
}

/**
 * @brief Loads MinMax encoded data from a binary file.
 *
 * This function reads the MinMax encoded data previously saved by save_minmax_data().
 * The data is read in the following order:
 * 1. Top level aggregate pair (global min, global max)
 * 2. Min detail coefficients for each level
 * 3. Max detail coefficients for each level
 * 4. Bottom level comparison order flags
 *
 * File Format:
 * 1. top_aggregate.first (double)
 * 2. top_aggregate.second (double)
 * 3. For each level (determined by calculate_level_sizes):
 *    a. min detail coefficients (num_coeffs_in_level * double)
 *    b. max detail coefficients (num_coeffs_in_level * double)
 * 4. Bottom order flags (in separate .flagz file)
 *
 * @param path_string Base path for the data files (without extension)
 * @param original_data_size Original size of the data before encoding
 * @return true if the data was successfully loaded, false otherwise
 *
 * @note The function expects both a .bin file for coefficient data and 
 *       a .flagz file for order flags at the specified path
 * @note The function verifies data integrity through EOF checks and read operations
 */
bool VariableManager::load_minmax_data(const std::string& path_string, const size_t original_data_size) {
    std::ifstream infile(path_string + ".bin", std::ios::binary);
    if (!infile) {
        std::cerr << "Error: Cannot open file for reading: " << path_string + ".bin" << std::endl;
        return false;
    }

    CoeffPair top_aggregate;
    CoeffArr min_details;
    CoeffArr max_details;

    // 1. Read top aggregate
    infile.read(reinterpret_cast<char*>(&top_aggregate.first), sizeof(double));
    if (!infile) { return false; }
    infile.read(reinterpret_cast<char*>(&top_aggregate.second), sizeof(double));
    if (!infile) { return false; }

    // 2. Calculate size of each level
    size_t bottom_details_num = original_data_size % 2 == 0 ? original_data_size / 2 : original_data_size / 2 + 1;
    std::vector<size_t> level_sizes = calculate_level_sizes(bottom_details_num);
    std::reverse(level_sizes.begin(), level_sizes.end()); // Reverse to match top-down order
    
    // 3. Read Min Details
    min_details.resize(level_sizes.size()); // Pre-allocate levels
    for (size_t i = 0; i < level_sizes.size(); ++i) {
        size_t num_coeffs_in_level = level_sizes[i];
        min_details[i].resize(num_coeffs_in_level);
        infile.read(reinterpret_cast<char*>(min_details[i].data()), num_coeffs_in_level * sizeof(double));
        if (!infile) { return false; }
    }

    // 4. Read Max Details
    max_details.resize(level_sizes.size()); // Pre-allocate levels
    for (size_t i = 0; i < level_sizes.size(); ++i) {
        size_t num_coeffs_in_level = level_sizes[i];
        max_details[i].resize(num_coeffs_in_level);
        infile.read(reinterpret_cast<char*>(max_details[i].data()), num_coeffs_in_level * sizeof(double));
        if (!infile) { return false; }
    }

    // 5. Bottom order boolean vector
    this->load_order_data(path_string, this->bottom_order);

    // Basic check if we reached EOF unexpectedly or read failed
    if (infile.peek() != EOF && !infile.good()) {
        std::cerr << "Warning: File read might be incomplete or corrupted: " << path_string << std::endl;
        return false;
    }

    build_multilevel_cache(top_aggregate, min_details, true);
    build_multilevel_cache(top_aggregate, max_details, false);
    
    return true;
}

void VariableManager::build_multilevel_cache(const CoeffPair& top_aggregate, CoeffArr &input_data, bool is_min)
{
    auto& target_L1 = is_min ? this->L1_min : this->L1_max;
    auto& target_L2 = is_min ? this->L2_min : this->L2_max;
    auto& target_L3 = is_min ? this->L3_min : this->L3_max;
    target_L1[1] = is_min ? top_aggregate.first : top_aggregate.second;

    size_t offset = 0, i=0, current_aggregates_num=1;
    for(; (i < L1_LEVEL) && (i<input_data.size()); offset+=(1<<i), ++i) {
        auto& details = input_data[i];
        size_t next_aggregates_num=0;
        for(size_t j = 0; j < current_aggregates_num; ++j) {
            size_t idx = offset+j+1;
            if(j < details.size()) {
                CoeffPair children = minmax_decode_pair(target_L1[idx], details[j], is_min);
                target_L1[2*idx] = children.first;
                target_L1[2*idx+1] = children.second;
                next_aggregates_num += 2;
            } else {
                // std::cout << "Carrying over unpaired element:" << target_L1[idx] << std::endl;
                next_aggregates_num++;
                target_L1[2*idx] = target_L1[idx];
            }
        }
        current_aggregates_num = next_aggregates_num;
    }
    size_t L2_offest = offset; // L1_SIZE-1
    for(; (i < L2_LEVEL) && (i<input_data.size()); offset+=(1<<i), ++i) {
        for(size_t j = 0; j < input_data[i].size(); ++j) {
            target_L2[offset+j-L2_offest] = input_data[i][j];
        }
    }
    for(; (i<input_data.size()); ++i) {
        #ifdef COMPRESS_ALP
        // compressed detail coefficients
        target_L3.push_back(tree_level_encode(input_data[i]));
        std::cout << "compression ratio at Level (" << std::to_string(i) << ", " + std::string(is_min?"min":"max") + "):"
                    << (1.0-target_L3.back()->calc_compression_ratio()/64.0)*100 << '%' << std::endl;
        #else
        target_L3.emplace_back(input_data[i]);
        #endif
    }
}

/**
 * @brief Loads boolean order flags from a binary file
 *
 * This function reads an array of boolean flags from a .flagz file,
 * where each bool was stored as two identical bytes.
 *
 * File Format (.flagz):
 * - File size must be even (2 bytes per bool)
 * - Each bool is stored as two consecutive bytes:
 *   - {1,1} -> true
 *   - {0,0} -> false
 *
 * @param path_string Base path for the input file (without extension)
 * @param[out] data Vector to store the loaded boolean flags
 * @return true if data was successfully loaded, false on any error
 *
 * @note Reads from {path_string}.flagz in binary mode
 * @note Resizes output vector based on file size
 * @note Only first byte of each pair is used for bool value
 */
bool VariableManager::load_order_data(const std::string &path_string, OrderArr &data) {
    std::ifstream infile(path_string + ".flagz", std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << path_string << ".flagz" << std::endl;
        return false;
    }

    // Get file size
    infile.seekg(0, std::ios::end);
    std::streamsize file_size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    // Each bool is stored as two bytes in the file
    size_t num_bools = file_size / 2;
    data.resize(num_bools);

    // Read bool values
    for (size_t i = 0; i < num_bools; ++i) {
        char byte1, byte2;
        infile.read(&byte1, 1);
        infile.read(&byte2, 1);
        data[i] = (byte1 != 0);
    }

    infile.close();
    return infile.good();
}

void VariableManager::print_details() const {
    std::cout << "\n=== Variable " << variable_id << " Details ===" << std::endl;
    
    // Print L1 cache info
    std::cout << "L1 Cache Status:" << std::endl;
    std::cout << "MinMax Pairs: " << L1_min.size()-1 << "/" << L1_SIZE_TIMES2 << " elements" << std::endl;
    
    // Print L2 cache info
    std::cout << "L2 Cache Status:" << std::endl;
    std::cout << "  OM3 coefficients: " << L2_min.size() << "/" << (L2_SIZE-L1_SIZE) << " elements" << std::endl;
    
    // Print detail coefficients info
    std::cout << "L3 Cache Status:" << std::endl;
    std::cout << "  Compressed Levels: " << L3_min.size()  << " Levels" << std::endl;
    
    // Print bottom order info
    std::cout << "Bottom Order Flags: " << bottom_order.size() << " flags" << std::endl;
}

void VariableManager::print_encoded_result(
    const std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr>& result) {
    
    const auto& top_aggregate = std::get<0>(result);
    const auto& min_details = std::get<1>(result);
    const auto& max_details = std::get<2>(result);
    const auto& order = std::get<3>(result);

    std::cout << "\n=== Encoding Result ===" << std::endl;
    std::cout << "Top Aggregate: {" << top_aggregate.first << ", " 
              << top_aggregate.second << "}" << std::endl;

    // Print detail coefficients by level
    for (size_t i = 0; i < min_details.size(); ++i) {
        std::cout << "Level " << i << " Details:" << std::endl;
        std::cout << "  Min: ";
        for (double d : min_details[i]) std::cout << d << " ";
        std::cout << "\n  Max: ";
        for (double d : max_details[i]) std::cout << d << " ";
        std::cout << std::endl;
    }

    // Print bottom order flags
    std::cout << "Bottom Order: ";
    for (bool b : order) std::cout << (b ? "1" : "0") << " ";
    std::cout << std::endl;
}

void VariableManager::print_decoded_levels() const {
    std::cout << "\n=== Decoding Comparison ===" << std::endl;
    
    // Print L1 cache
    std::cout << "L1 Cache:" << std::endl;
    std::cout << "  Min: ";
    for (size_t i = 0; i < std::min(L1_min.size(), (size_t)20); ++i)
        std::cout << L1_min[i] << " ";
    std::cout << "\n  Max: ";
    for (size_t i = 0; i < std::min(L1_max.size(), (size_t)20); ++i)
        std::cout << L1_max[i] << " ";
    std::cout << std::endl;

    // Print L2 cache
    std::cout << "L2 Cache:" << std::endl;
    std::cout << "  Min: ";
    for (size_t i = 0; i < std::min(L2_min.size(), (size_t)20); ++i)
        std::cout << L2_min[i] << " ";
    std::cout << "\n  Max: ";
    for (size_t i = 0; i < std::min(L2_min.size(), (size_t)20); ++i)
        std::cout << L2_max[i] << " ";
    std::cout << std::endl;
}

#ifdef IS_DEBUG
// Helper function to print a vector of CoeffPairs
void print_coeff_pair_vector(const std::vector<CoeffPair>& pairs) {
    std::cout << "{";
    for (size_t i = 0; i < pairs.size(); ++i) {
        std::cout << pairs[i].first << ", " << pairs[i].second;
        if (i < pairs.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "}";
}
#endif

std::vector<double> VariableManager::get_all_decoded_data(size_t original_data_size) const
{
    // 1.  Start with the top-level aggregate (global min/max)
    std::vector<CoeffPair> current_level_aggregates = {this->minmax_decode_index(0, 0, 0, 0, 1).first};
    size_t bottom_details_num = original_data_size % 2 == 0 ? original_data_size / 2 : original_data_size / 2 + 1;
    std::vector<size_t> level_sizes = calculate_level_sizes(bottom_details_num);
    std::reverse(level_sizes.begin(), level_sizes.end()); // Reverse to match top-down order
    size_t orignial_level = BitScanReverse(original_data_size);
#ifdef IS_DEBUG
    std::cout << "  Level 0 Aggregates: ";
    print_coeff_pair_vector(current_level_aggregates);
    std::cout << std::endl;
#endif

    // 2.  Iterate through the detail levels, decoding and printing at each level
    for (size_t level = 0; level < level_sizes.size(); ++level)
    {
        size_t start = (1 << level);
        std::vector<size_t> current_level_idx(level_sizes[level]);
        std::iota(current_level_idx.begin(), current_level_idx.end(), start);
        std::cout << "Level start:" << start << ", Level size:" << level_sizes[level] << std::endl;

        std::vector<CoeffPair> next_level_aggregates;
        next_level_aggregates.reserve(level < level_sizes.size()-1 ? level_sizes[level+1] : original_data_size);
        for (size_t i = 0; i < current_level_aggregates.size(); ++i)
        {
            CoeffPair parent = current_level_aggregates[i];
            if (i < level_sizes[level])
            {
                std::pair<CoeffPair, CoeffPair> children =
                    this->minmax_decode_index(parent.first, parent.second, current_level_idx[i], orignial_level, 0);
                next_level_aggregates.push_back(children.first);
                next_level_aggregates.push_back(children.second);
            }
            else
            {
#ifdef IS_DEBUG
                std::cout << "    Carrying over unpaired element:" << parent.first << ',' << parent.second << std::endl;
#endif
                next_level_aggregates.push_back(parent);
            }
        }
#ifdef IS_DEBUG
        std::cout << "    Level " << level + 1 << " Aggregates: ";
        print_coeff_pair_vector(next_level_aggregates);
        std::cout << std::endl;
#endif
        current_level_aggregates = std::move(next_level_aggregates);
    }

    std::vector<double> decoded_data;
    for (size_t i = 0; i < current_level_aggregates.size(); ++i)
    {
        CoeffPair &parent = current_level_aggregates[i];
        if (i < original_data_size / 2)
        {
            if (this->bottom_order[i])
            {
                decoded_data.push_back(parent.first);
                decoded_data.push_back(parent.second);
            }
            else
            {
                decoded_data.push_back(parent.second);
                decoded_data.push_back(parent.first);
            }
        }
        else
        {
            decoded_data.push_back(parent.first);
        }
    }
#ifdef IS_DEBUG
    std::cout << "    Level " << level_sizes.size() + 1 << " Aggregates: ";
    print_coeff_pair_vector(current_level_aggregates);
    std::cout << std::endl;
#endif
    return decoded_data;
}

void VariableManager::write_decoded_data(const std::string &path_string, size_t original_data_size) const
{
    std::vector<double> decoded_data = std::move(get_all_decoded_data(original_data_size));
    std::ofstream outputFile(path_string + ".csv");
    outputFile << "t,v1" << std::endl;
    outputFile << std::fixed << std::setprecision(12);
    for (size_t i = 0; i < decoded_data.size(); ++i)
    {
        outputFile << i << "," << decoded_data[i] << std::endl;
    }
    outputFile.close();
}

std::vector<size_t> cal_path_from_root(size_t index)
{
    std::vector<size_t> path;
    while (index > 0)
    {
        index /= 2;
        path.push_back(index);
    }
    std::reverse(path.begin(), path.end());
    return path;
}


void VariableManager::print_query_path(size_t index, size_t original_level) const
{
    std::vector<size_t> path = cal_path_from_root(index);
    std::cout << "Query path for index " << index << std::endl;

    CoeffPair parent = this->minmax_decode_index(0, 0, 0, 0, 0).first;
    for (size_t i = 0; i < path.size(); ++i)
    {
        std::cout << "tree id: " << path[i] << ", min:" << parent.first << ", max:" << parent.second << std::endl;
        
        std::pair<CoeffPair, CoeffPair> children =
            this->minmax_decode_index(parent.first, parent.second, path[i], original_level, 0);
        if(i < path.size() - 1)
        {
            parent = path[i+1]%2==0 ? children.first : children.second;
        }
        else
        {
            CoeffPair result = path[i+1]%2==0 ? children.first : children.second;
            std::cout << "Final decoded values: " << result.first << ", " << result.second << std::endl;
        }
    }
}