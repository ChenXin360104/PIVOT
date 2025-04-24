#include "CompressedData.h"

#include <string.h>
#include <alp/constants.hpp>
#include <iostream>
#include "utils.h"

CompressedBlock::CompressedBlock(const alp::Scheme alp)
    : scheme(alp)
{
}

CompressedBlock_Alp::CompressedBlock_Alp(const ST ffor_base, const ST* ffor_arr, const PT* exc_arr,
                                         const u16* exc_pos_arr, const u16 exc_cnt,
                                         const u8 bit_width, const u8 fac, const u8 exp)
    : CompressedBlock(alp::Scheme::ALP), ffor_base(ffor_base),
      exc_cnt(exc_cnt), bit_width(bit_width), fac(fac), exp(exp)
{
    const u64 ffor_byte_size = ceil(bit_width * BLOCK_SIZE, 8);
    const u64 ffor_size = ceil(ffor_byte_size, sizeof(ST));
    this->ffor_arr = new ST[ffor_size];
    this->exc_arr = new PT[exc_cnt];
    this->exc_pos_arr = new u16[exc_cnt];
    memcpy(this->ffor_arr, ffor_arr, ffor_byte_size);
    memcpy(this->exc_arr, exc_arr, exc_cnt * sizeof(ST));
    memcpy(this->exc_pos_arr, exc_pos_arr, exc_cnt * sizeof(u16));
}

CompressedBlock_Alp::~CompressedBlock_Alp()
{
    delete[] ffor_arr;
    delete[] exc_arr;
    delete[] exc_pos_arr;
}

double CompressedBlock_Alp::calc_compression_ration() const
{
    double size = sizeof(CompressedBlock_Alp);
    size += sizeof(ST) * ceil(bit_width * BLOCK_SIZE, 8);
    size += (sizeof(PT) + sizeof(u16)) * exc_cnt;
    return size / BLOCK_SIZE;
}


CompressedBlock_AlpRd::CompressedBlock_AlpRd(const UT* right_ffor_arr, const u16* left_ffor_arr,
                                             const u16* exc_arr, const u16* exc_pos_arr, const u16 exc_cnt,
                                             const alp::state<PT>& stt)
    : CompressedBlock(alp::Scheme::ALP_RD), stt(stt), exc_cnt(exc_cnt)
{
    const u64 r_ffor_byte_size = ceil(stt.right_bit_width * BLOCK_SIZE, 8);
    const u64 r_ffor_size = ceil(r_ffor_byte_size, sizeof(UT));
    const u64 l_ffor_byte_size = ceil(stt.left_bit_width * BLOCK_SIZE, 8);
    const u64 l_ffor_size = ceil(l_ffor_byte_size, sizeof(u16));
    this->right_ffor_arr = new UT[r_ffor_size];
    this->left_ffor_arr = new u16[l_ffor_size];
    this->exc_arr = new u16[exc_cnt];
    this->exc_pos_arr = new u16[exc_cnt];
    memcpy(this->right_ffor_arr, right_ffor_arr, r_ffor_byte_size);
    memcpy(this->left_ffor_arr, left_ffor_arr, l_ffor_byte_size);
    memcpy(this->exc_arr, exc_arr, exc_cnt * sizeof(u16));
    memcpy(this->exc_pos_arr, exc_pos_arr, exc_cnt * sizeof(u16));
}

CompressedBlock_AlpRd::~CompressedBlock_AlpRd()
{
    delete[] right_ffor_arr;
    delete[] left_ffor_arr;
    delete[] exc_arr;
    delete[] exc_pos_arr;
}

double CompressedBlock_AlpRd::calc_compression_ration() const
{
    double size = sizeof(CompressedBlock_AlpRd);
    size += sizeof(UT) * ceil(stt.right_bit_width * BLOCK_SIZE, 8);
    size += sizeof(u16) * ceil(stt.left_bit_width * BLOCK_SIZE, 8);
    size += (sizeof(u16) + sizeof(u16)) * exc_cnt;
    return size / BLOCK_SIZE;
}


CompressedData::CompressedData(const u32 num_blocks, const u64 data_size)
    : cur_index(0), blocks(new const CompressedBlock*[num_blocks]), data_size(data_size)
{
    // std::cout<<"<<CompressedData constructor called."<<num_blocks<<std::endl;
}

CompressedData::~CompressedData()
{
    // std::cout<<">>CompressedData destructor called."<<cur_index<<std::endl;
    for(int i=0;i<cur_index;++i)
        delete blocks[i];
    delete [] blocks;
}

void CompressedData::push(const CompressedBlock* block)
{
    blocks[cur_index++] = block;
}

double CompressedData::calc_compression_ratio() const
{
    double ave_cr = 0;
    for (int i = 0; i < cur_index; ++i)
        ave_cr += blocks[i]->calc_compression_ration();
    return ave_cr / cur_index;
}
