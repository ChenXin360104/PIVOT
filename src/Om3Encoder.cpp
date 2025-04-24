#include "Om3Encoder.h"
// Helper function to create directory
bool create_directory(const std::string& path) {
    // Using system command to create directory
    std::string cmd = "mkdir -p " + path;
    return system(cmd.c_str()) == 0;
}

// Helper function to check if directory exists
bool directory_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

// Helper function to get filename without extension
std::string get_filename_without_extension(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");
    size_t last_dot = path.find_last_of(".");
    
    if (last_dot == std::string::npos || 
        (last_slash != std::string::npos && last_dot < last_slash)) {
        return path.substr(last_slash + 1);
    }
    
    if (last_slash == std::string::npos) {
        return path.substr(0, last_dot);
    }
    
    return path.substr(last_slash + 1, last_dot - last_slash - 1);
}

std::vector<double> get_column(const std::vector<std::vector<double>> &data, size_t col)
{
    // std::cout << data.size() << " col: " << col << std::endl;
    std::vector<double> res(data.size());
    for (int i = 0; i < data.size(); ++i)
    {
        // std::cout << i << "  " << data[i].size() << std::endl;
        res[i] = data[i][col];
    }
    // std::cout << res.size() << std::endl;
    return std::move(res);
}

std::vector<double> get_column_c(const std::vector<double *> &data, size_t col)
{
    std::vector<double> res(data.size());
    for (int i = 0; i < data.size(); ++i)
        res[i] = data[i][col];
    return std::move(res);
}

std::vector<std::vector<double>> load_csv(std::string csv_file_path)
{
    std::vector<std::vector<double>> res;
    res.reserve(1009845000 + 1000000);
    std::ifstream ifs(csv_file_path);
    if (!ifs.is_open())
    {
        std::cerr << "Error: Cannot open file " << csv_file_path << std::endl;
        return res;
    }

    std::string line;
    std::string item;
    std::vector<std::string> headers;

    // 读取第一行，获取列名
    std::getline(ifs, line);
    std::stringstream ss(line);
    while (std::getline(ss, item, ','))
        headers.push_back(item);
    // 读取数据行
    int row_id = 0;
    std::cout << headers.size() << "\n";
    while (std::getline(ifs, line))
    {
        std::vector<double> row(headers.size());
        std::stringstream ss(line);
        std::string item;
        int index = 0;
        while (std::getline(ss, item, ','))
            row[index++] = std::stod(item);
        res.push_back(row);
        if (row_id++ % 1000000 == 0)
            std::cout << "Read " << row_id << " rows" << std::endl;
    }
    return std::move(res);
}

/// 读取下一个值,返回字符长度
static int next_val(char *s, char item[])
{
    int i = 0;
    while (s[i] != '\0' && s[i] != ',')
    {
        item[i] = s[i];
        i++;
    }
    item[i] = '\0';
    if (s[i] == ',')
        return i + 1;
    else
        return i;
}

std::vector<double *> load_csv_c(std::string csv_file_path, int &header_size)
{
    std::vector<double *> res;
    res.reserve(1009845000 + 1000000);
    // 预分配内存，避免频繁的内存分配
    auto f = fopen(csv_file_path.c_str(), "r");
    if (!f)
    {
        std::cerr << "Error: Cannot open file " << csv_file_path << std::endl;
        return res;
    }
    char line[1024];
    char item[256];
    std::vector<std::string> headers;
    // 读取第一行，获取列名
    fgets(line, sizeof(line), f);
    std::stringstream ss(line);
    std::string col_name;
    while (std::getline(ss, col_name, ','))
        headers.push_back(col_name);
    header_size = headers.size();
    // 读取数据行
    int row_id = 0; 
    while (fgets(line, sizeof(line), f))
    {
        // std::vector<double> row(headers.size());
        double *row = new double[header_size];
        int pos = 0;
        int index = 0;
        bool flag = false;
        while (line[pos] != '\0' && line[pos] != '\n')
        {
            pos += next_val(line + pos, item);
            // printf("%s\n", item);
            try
            {
                row[index++] = std::stod(item);
            }
            catch (const std::exception &e)
            {
                flag = true;
                break;
            }
        }
        if (flag)
            continue;
        if (row_id++ % 1000000 == 0)
            std::cout << "Read " << row_id << " rows" << std::endl;
        res.push_back(row);
    }
    fclose(f);
    return res;
}


/**
 * @brief Finds the smallest power of 2 greater than or equal to n using bit manipulation.
 *
 * @param n The input number.
 * @return The smallest power of 2 >= n. Returns 1 for n = 0.
 *         Returns 0 if n is too large such that the next power of 2 would exceed
 *         the maximum value representable by size_t.
 */
size_t nextPowerOfTwo(size_t n)
{
    if (n == 0)
    {
        return 1;
    }

    if (n > SIZE_MAX / 2)
    {
        return SIZE_MAX; // Handle potential overflow
    }

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;                       // Only if size_t is at least 64 bits
#if defined(__LP64__) || defined(_LP64) // Check for 64-bit systems
    n |= n >> 32;                       // Guaranteed to be safe on 64-bit systems.
#endif

    return n + 1;
}

/**
 * @brief Encodes CSV data using MinMax hierarchical compression
 * 
 * This function:
 * 1. Reads CSV file using rapidcsv
 * 2. Processes each column (except first 't' column)
 * 3. Applies MinMax encoding to each column
 * 4. Saves encoded data to binary files
 * 5. Creates metadata file with column/row counts
 *
 * File Structure:
 * - ../om3_data/                    (directory)
 *   |- {data_name}_om3.bin         (metadata file)
 *   |- {data_name}_om3_v{id}.bin   (encoded data)
 *   |- {data_name}_om3_v{id}.flagz (order flags)
 *
 * @param path_string Path to input CSV file
 * @return true if encoding and saving succeeds, false on any error
 * @throws std::runtime_error If first column is not named 't'
 */
bool DatasetEncoder::minmax_encode(const std::string &csv_file_path)
{
    std::cout << "Loading CSV: " << csv_file_path << std::endl;
    int header_size;
    auto data = load_csv_c(csv_file_path, header_size); // Print CSV info

    std::string data_name = get_filename_without_extension(csv_file_path);

    // Check if the path exists, create if it doesn't
    try {
        if (!directory_exists("../om3_data/")) {
            if (!create_directory("../om3_data/")) {
                std::cerr << "Error: Failed to create data directory." << std::endl;
                return false;
            }
            std::cout << "Created directory: ../om3_data/" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }

    std::string path_string = "../om3_data/"+data_name + "_om3.bin";
    std::ofstream outfile(path_string, std::ios::binary | std::ios::trunc);
    if (!outfile) {
        std::cerr << "Error: Cannot open file for writing: " << data_name << std::endl;
        return false;
    }

    for (size_t i = 1; i < header_size; ++i) {
        std::vector<double> values = get_column_c(data, i); // 直接获取整列数据

        auto result = VariableEncoder::minmax_encode(values);
        VariableEncoder::save_minmax_data(data_name, i, result);
    }

    size_t data_size = data.size();
    size_t col_count = header_size-1;
    outfile.write(reinterpret_cast<const char*>(&col_count), sizeof(col_count));
    outfile.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));

    outfile.close();
    std::cout << "Successfully wrote col/row to " << path_string << std::endl;
    return outfile.good(); // Check if any errors occurred during writing/closing
}

/**
 * @brief Performs MinMax hierarchical encoding without padding.
 *
 * This function implements a hierarchical compression scheme that:
 * 1. Creates base level pairs from input data
 * 2. Iteratively merges pairs upward, storing detail coefficients
 * 3. Records comparison order at bottom level
 * 4. Produces a final top-level aggregate
 *
 * Algorithm Steps:
 * 1. Initialize: Convert input values to (min,max) pairs
 * 2. Iterate until single pair remains:
 *    - Merge adjacent pairs
 *    - Store detail coefficients (D^{j-1}_{2i}, D^{j-1}_{2i+1})
 *    - Record comparison order at bottom level
 * 3. Reverse detail coefficients for top-down order
 *
 * @param input_data Vector of input data points to encode
 *
 * @return Tuple containing:
 *         - first: Top-level aggregate pair (global min, max)
 *         - second: Min detail coefficients by level (top-down)
 *         - third: Max detail coefficients by level (top-down)
 *         - fourth: Bottom level comparison order flags
 *
 * @throws std::runtime_error If final aggregation fails
 *
 * @note Detail coefficients are stored in top-down order
 * @note Empty input returns ({0,0}, {}, {}, {})
 * @note Single value input returns ({val,val}, {}, {}, {})
 *
 * @example
 *   std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
 *   auto [top_agg, min_details, max_details, order] = minmax_encode(data);
 */
std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr>
VariableEncoder::minmax_encode(const std::vector<double>& input_data) {
    if (input_data.empty()) {
        // Return default/empty values
        return {{0.0, 0.0}, {}, {}, {}};
    }

    if (input_data.size() == 1) {
         // Special case: single data point is the global min/max
         // No detail coefficients are generated.
        return {{input_data[0], input_data[0]}, {}, {}, {}};
    }

    // 1. Initialization: Create base level aggregate pairs
    std::vector<CoeffPair> current_aggregates;
    current_aggregates.reserve(input_data.size());
    for (double val : input_data) {
        current_aggregates.push_back({val, val});
    }

    // Store all detail coefficients level by level, bottom-up initially.
    CoeffArr all_min_details_bottom_up;
    CoeffArr all_max_details_bottom_up;
    OrderArr bottom_order;

    // 2. Iterative Computation: From level j down to level 1 (calculating j-1)
    while (current_aggregates.size() > 1) {
        size_t current_level_pair_count = current_aggregates.size();
        size_t num_pairs_to_process = current_level_pair_count / 2;
        size_t next_level_pair_count = (current_level_pair_count + 1) / 2;
        bool is_bottom_level = (current_level_pair_count == input_data.size());
        size_t order_num = nextPowerOfTwo(current_level_pair_count)/2;

        std::vector<CoeffPair> next_aggregates(next_level_pair_count);
        std::vector<double> current_level_min_details; // For D^{j-1}_{2i}
        std::vector<double> current_level_max_details; // For D^{j-1}_{2i+1}
        current_level_min_details.reserve(num_pairs_to_process);
        current_level_max_details.reserve(num_pairs_to_process);
        OrderArr current_level_min_order(order_num, false); // For min_order

        for (size_t i = 0; i < num_pairs_to_process; ++i)
        {
            size_t left_pair_idx = 2 * i;
            size_t right_pair_idx = 2 * i + 1;

            // Use const& to avoid unnecessary copies if CoeffPair is complex,
            // but since it's std::pair<double, double>, value might be fine too.
            // Stick with const& for consistency with the original intent.
            const auto &left_pair = current_aggregates[left_pair_idx];
            const auto &right_pair = current_aggregates[right_pair_idx];

            // Calculate next level aggregate pair
            double next_agg_min = std::min(left_pair.first, right_pair.first);
            double next_agg_max = std::max(left_pair.second, right_pair.second);
            next_aggregates[i] = {next_agg_min, next_agg_max};

            if (is_bottom_level)
            {
                current_level_min_order[i] = left_pair.first < right_pair.first;
            }
            else
            {
                // Calculate detail coefficients (using subtraction as in the target code)
                double detail_min = left_pair.first - right_pair.first;   // D^{j-1}_{2i}
                double detail_max = left_pair.second - right_pair.second; // D^{j-1}_{2i+1}
                current_level_min_details.push_back(detail_min);
                current_level_max_details.push_back(detail_max);
            }
        }

        // Handle odd number of pairs in the current level
        if (current_level_pair_count % 2 != 0) {
            next_aggregates[next_level_pair_count - 1] = current_aggregates.back();
            // Note: The original code didn't explicitly handle order for the odd element.
            // This version maintains that behavior for bottom_order.
        }

        // Store the details calculated for this level (which corresponds to level j-1)
        // We store them bottom-up first.
        // Discard the details generated from the initial base level (level l)
        if (!current_level_min_details.empty()) // If num_pairs_to_process > 0
        {
            // std::cout << "current level detail size: " << current_level_min_details.size() << std::endl;
            // This is not the bottom level, store the details
            all_min_details_bottom_up.push_back(std::move(current_level_min_details));
            all_max_details_bottom_up.push_back(std::move(current_level_max_details));
        }
        else if(is_bottom_level)
        {
            bottom_order = std::move(current_level_min_order);
        }

        // Move to the next level
        current_aggregates = std::move(next_aggregates);
    }

    // 3. Final Result Preparation
    if (current_aggregates.size() != 1) {
         throw std::runtime_error("Internal error: Final aggregates size is not 1 pair.");
    }

    CoeffPair top_level_aggregate = current_aggregates[0];

    // Reverse the order of levels in details to be top-down
    std::reverse(all_min_details_bottom_up.begin(), all_min_details_bottom_up.end());
    std::reverse(all_max_details_bottom_up.begin(), all_max_details_bottom_up.end());

    return {top_level_aggregate, std::move(all_min_details_bottom_up), std::move(all_max_details_bottom_up), std::move(bottom_order)};
}

/**
 * @brief Saves MinMax encoded data to binary files.
 *
 * This function writes the MinMax encoded data to two files:
 * 1. A .bin file containing coefficient data
 * 2. A .flagz file containing order flags
 *
 * File Format (.bin):
 * 1. top_aggregate.first (double)
 * 2. top_aggregate.second (double)
 * 3. Min detail coefficients for each level (doubles)
 * 4. Max detail coefficients for each level (doubles)
 *
 * @param data_name Base name for the output files
 * @param id Variable identifier (appended to filename)
 * @param data Tuple containing:
 *            - first: Top level aggregate pair (global min, max)
 *            - second: Min detail coefficients by level
 *            - third: Max detail coefficients by level
 *            - fourth: Bottom level order flags
 * @return true if data was successfully written, false on any error
 *
 * @note Files are created in ../om3_data/ directory
 * @note Order flags are written to a separate .flagz file
 * @note All files are written in binary mode
 */
bool VariableEncoder::save_minmax_data(const std::string& data_name, const size_t id, const std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr>& data) {
    std::string path_string = "../om3_data/" + data_name + "_om3_v" + std::to_string(id);
    std::ofstream outfile(path_string + ".bin", std::ios::binary | std::ios::trunc);
    if (!outfile) {
        std::cerr << "Error: Cannot open file for writing: " << data_name << std::endl;
        return false;
    }

    const auto& top_aggregate = std::get<0>(data);
    const auto& min_details = std::get<1>(data);
    const auto& max_details = std::get<2>(data);
    const auto& bottom_order = std::get<3>(data);

    // std::cout << "\n=== Encoding Result ===" << std::endl;
    // std::cout << "Top Aggregate: {" << top_aggregate.first << ", " 
    //           << top_aggregate.second << "}" << std::endl;

    // // Print detail coefficients by level
    // for (size_t i = 0; i < min_details.size(); ++i) {
    //     std::cout << "Level " << i << " Details:" << std::endl;
    //     std::cout << "  Min: ";
    //     for (double d : min_details[i]) std::cout << d << " ";
    //     std::cout << "\n  Max: ";
    //     for (double d : max_details[i]) std::cout << d << " ";
    //     std::cout << std::endl;
    // }

    // // Print bottom order flags
    // std::cout << "Bottom Order: ";
    // for (bool b : bottom_order) std::cout << (b ? "1" : "0") << " ";
    // std::cout << std::endl;

    // 2. Save top aggregate
    outfile.write(reinterpret_cast<const char*>(&top_aggregate.first), sizeof(double));
    outfile.write(reinterpret_cast<const char*>(&top_aggregate.second), sizeof(double));

    // 3. Save Min Details
    size_t num_min_levels = min_details.size();
    for (const auto& level_details : min_details) {
        outfile.write(reinterpret_cast<const char*>(level_details.data()), level_details.size() * sizeof(double));
    }

    // 4. Save Max Details
    size_t num_max_levels = max_details.size();
    for (const auto& level_details : max_details) {
        outfile.write(reinterpret_cast<const char*>(level_details.data()), level_details.size() * sizeof(double));
    }

    // 5. Bottom order boolean vector
    VariableEncoder::save_order_data(path_string, bottom_order);
    
    outfile.close();
    std::cout << "Successfully wrote minmax data to " << path_string << ".bin" << std::endl;
    return outfile.good(); // Check if any errors occurred during writing/closing
}


/**
 * @brief Saves boolean order flags to a binary file
 *
 * This function writes an array of boolean flags to a .flagz file,
 * where each bool is stored as two identical bytes for OM3 compatibility.
 *
 * File Format (.flagz):
 * - Each bool is stored as two consecutive identical bytes:
 *   - true  -> {1,1}
 *   - false -> {0,0}
 *
 * @param path_string Base path for the output file (without extension)
 * @param data Vector of boolean flags to write
 * @return true if data was successfully written, false on any error
 *
 * @note Writes to {path_string}.flagz in binary mode
 * @note Each bool is written twice for OM3 compatibility
 */
bool VariableEncoder::save_order_data(const std::string &path_string, const OrderArr &data)
{
    std::ofstream outfile(path_string+".flagz", std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file: " << path_string << ".flagz" << std::endl;
        return false;
    }

    for (bool b : data) {
        char byte = b ? 1 : 0; // Convert bool to a byte (1 or 0)
        // using two times to compatible with om3
        outfile.write(&byte, 1);  
        outfile.write(&byte, 1);
    }

    outfile.close();
    std::cout << "Successfully wrote bool vector to " << path_string << ".flagz" << std::endl;
    return outfile.good();
}