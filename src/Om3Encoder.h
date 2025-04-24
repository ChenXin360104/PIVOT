#ifndef OM3ENCODER_H
#define OM3ENCODER_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <fstream>
#include <sstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <sys/stat.h>  // For stat() and mkdir()
#include <cstdlib>     // For system()

using CoeffPair = std::pair<double, double>;
using CoeffArr = std::vector<std::vector<double>>;
using OrderArr = std::vector<bool>;

class VariableEncoder
{
public:
    static std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr> minmax_encode(const std::vector<double> &input_data);
    static bool save_minmax_data(const std::string &data_name, const size_t id, const std::tuple<CoeffPair, CoeffArr, CoeffArr, OrderArr> &data);
    static bool save_order_data(const std::string &path_string, const OrderArr &data);
};

class DatasetEncoder
{
public:
    static bool minmax_encode(const std::string &path_string);
};

std::vector<std::vector<double>> load_csv(std::string csv_file_path);
std::vector<double *> load_csv_c(std::string csv_file_path,int &header_size);
std::vector<double> get_column(const std::vector<std::vector<double>> &data, size_t col);
std::vector<double> get_column_c(const std::vector<double *> &data, size_t col);
#endif // OM3ENCODER_H