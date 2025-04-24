#ifndef COMPRESSEDDATA_H
#define COMPRESSEDDATA_H

#include <alp.hpp>

#include "utils.h"

struct CompressedBlock
{
    alp::Scheme scheme;
    explicit CompressedBlock(alp::Scheme alp);

    virtual ~CompressedBlock()
    {
    };
    virtual double calc_compression_ration() const =0;
};

struct CompressedBlock_Alp final : CompressedBlock
{
    ST ffor_base;
    ST* ffor_arr;
    PT* exc_arr;
    u16* exc_pos_arr;
    u16 exc_cnt;
    u8 bit_width;
    u8 fac, exp;
    CompressedBlock_Alp(ST ffor_base, const ST* ffor_arr, const PT* exc_arr,
                        const u16* exc_pos_arr, u16 exc_cnt, u8 bit_width, u8 fac, u8 exp);
    ~CompressedBlock_Alp() override;
    double calc_compression_ration() const override;

    auto as_tuple() const
    {
        return std::tie(ffor_base, ffor_arr, exc_arr, exc_pos_arr, exc_cnt, bit_width, fac, exp);
    }
};

struct CompressedBlock_AlpRd final : CompressedBlock
{
    alp::state<PT> stt;
    UT* right_ffor_arr;
    u16* left_ffor_arr;
    u16* exc_arr;
    u16* exc_pos_arr;
    u16 exc_cnt;

    CompressedBlock_AlpRd(const UT* right_ffor_arr, const u16* left_ffor_arr,
                          const u16* exc_arr, const u16* exc_pos_arr, u16 exc_cnt,
                          const alp::state<PT>& stt);
    ~CompressedBlock_AlpRd() override;
    double calc_compression_ration() const override;

    auto as_tuple() const
    {
        return std::tie(stt, right_ffor_arr, left_ffor_arr, exc_arr, exc_pos_arr, exc_cnt);
    }
};

struct CompressedData
{
    u32 cur_index;
    u64 data_size;
    const CompressedBlock* * blocks;
    explicit CompressedData(u32 num_blocks, u64 data_size);
    ~CompressedData();
    void push(const CompressedBlock* block);
    double calc_compression_ratio() const;
    u64 size() const { return data_size; };
};


#endif //COMPRESSEDDATA_H
