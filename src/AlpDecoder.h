#ifndef ALPDECODER_H
#define ALPDECODER_H

#include "CompressedData.h"

class AlpDecoder
{
    PT* decoded_arr;
    UT* unffor_right_arr;
    u16* unffor_left_arr;

public:
    AlpDecoder();
    ~AlpDecoder();
    void decode(const CompressedBlock* compressed_block_bc) const;
    double get(u32 index) const;
};

double get_decoded_value(const CompressedData* compressed_data, u32 index);
double get_decoded_value_p(const CompressedData* compressed_data, u32 index, int thread_index); //parallel version

#endif //ALPDECODER_H
