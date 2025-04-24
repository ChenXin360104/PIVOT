//
// Created by DELL on 2025/4/14.
//

#ifndef ALPENCODER_H
#define ALPENCODER_H

#include <vector>

#include "CompressedData.h"

class AlpEncoder
{
    PT* sample_arr;
    PT* exc_arr;
    ST* encoded_arr;
    ST* ffor_base_pt;
    ST* ffor_arr;
    UT* right_arr;
    UT* ffor_right_arr;
    u16* left_arr;
    u16* ffor_left_arr;
    u16* rd_exc_arr;
    u16* exc_pos_arr;
    u16* exc_cnt_pt;
    u8 bit_width;

public:
    AlpEncoder();
    ~AlpEncoder();
    const CompressedBlock* compress(const PT* input_arr);
};

CompressedData* tree_level_encode(const std::vector<PT>& data);
CompressedData* tree_level_encode_p(const std::vector<PT>& data); //parallel version

#endif //ALPENCODER_H
