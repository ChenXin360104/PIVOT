#include "AlpDecoder.h"

#include <stdexcept>

AlpDecoder::AlpDecoder()
{
    decoded_arr = new PT[BLOCK_SIZE];
    unffor_right_arr = new UT[BLOCK_SIZE];
    unffor_left_arr = new u16[BLOCK_SIZE];
}

AlpDecoder::~AlpDecoder()
{
    delete[] decoded_arr;
    delete[] unffor_right_arr;
    delete[] unffor_left_arr;
}

void AlpDecoder::decode(const CompressedBlock* compressed_block_bc) const
{
    if (compressed_block_bc->scheme == alp::Scheme::ALP)
    {
        const auto compressed_block = dynamic_cast<const CompressedBlock_Alp*>(compressed_block_bc);
        const auto& [ffor_base, ffor_arr, exc_arr, exc_pos_arr,
                exc_cnt, bit_width, fac, exp]
            = compressed_block->as_tuple();
        generated::falp::fallback::scalar::falp(ffor_arr, decoded_arr, bit_width, &ffor_base, fac, exp);
        alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, exc_pos_arr, &exc_cnt);
    }
    else if (compressed_block_bc->scheme == alp::Scheme::ALP_RD)
    {
        const auto compressed_block = dynamic_cast<const CompressedBlock_AlpRd*>(compressed_block_bc);
        auto [stt_c, right_ffor_arr, left_ffor_arr,
                exc_arr, exc_pos_arr, exc_cnt_c]
            = compressed_block->as_tuple();
        auto exc_cnt = exc_cnt_c;
        auto stt = stt_c;
        unffor::unffor(right_ffor_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
        unffor::unffor(left_ffor_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
        alp::rd_encoder<PT>::decode(
            decoded_arr, unffor_right_arr, unffor_left_arr,
            exc_arr, exc_pos_arr, &exc_cnt, stt);
    }
    else
        throw std::runtime_error("Unsupported scheme");
}

double AlpDecoder::get(const u32 index) const
{
    return decoded_arr[index];
}

double get_decoded_value(const CompressedData* compressed_data, const u32 index)
{
    static AlpDecoder decoder;
    const CompressedBlock* block = compressed_data->blocks[index / BLOCK_SIZE];
    decoder.decode(block);
    return decoder.get(index % BLOCK_SIZE);
}
double get_decoded_value_p(const CompressedData* compressed_data, const u32 index, const int thread_index)
{
    static bool initialized = false;
    static std::vector<AlpDecoder> decoders;
    if (!initialized)
    {
        decoders.resize(MAX_THREADS);
        initialized = true;
    }
    const auto& decoder = decoders[thread_index];
    const CompressedBlock* block = compressed_data->blocks[index / BLOCK_SIZE];
    decoder.decode(block);
    return decoder.get(index % BLOCK_SIZE);
}