/* ymz294.h          */
/* YMZ294設定ファイル */

#ifndef _YMZ294_H_
#define _YMZ294_H_

#define FREQ_MAX                128   /* 周波数テーブル最大値                  */
#define DRAM_MAX                128   /* ドラムセットテーブル最大値            */

const uint32_t freq_table[FREQ_MAX] = /* 周波数テーブル                       */
{
       8,     9,     9,    10,    10,    11,   12,     12,    13,    14,    15,    15,    16,    17,    18,    19,
      21,    22,    23,    25,    26,    28,   29,     31,    33,    35,    37,    39,    42,    44,    47,    49,
      52,    55,    59,    62,    66,    70,   74,     78,    83,    88,    93,    98,   104,   110,   117,   124,
     131,   139,   147,   156,   165,   175,   185,   196,   208,   220,   234,   247,   262,   278,   294,   312,
     330,   350,   370,   392,   416,   440,   467,   494,   524,   555,   588,   623,   660,   699,   740,   784,
     831,   880,   933,   988,  1047,  1109,  1175,  1245,  1319,  1397,  1480,  1568,  1662,  1760,  1865,  1976,
    2094,  2218,  2350,  2490,  2638,  2794,  2960,  3136,  3323,  3520,  3730,  3952,  4186,  4435,  4699,  4978,
    5274,  5587,  5920,  6272,  6645,  7040,  7459,  7902,  8372,  8870,  9397,  9956, 10548, 11175, 11840, 12544
};

/* 公開関数 */
extern void ymz294_init( void );
extern void set_freqency(uint8_t ch, uint32_t freq);
extern void set_noisefreqency(uint8_t ch, uint8_t freq);
extern void set_mixer( uint8_t set_mix );
extern void set_volume(uint8_t ch, uint8_t volume);
extern void set_envelope(uint8_t ch, double time, uint8_t shape);
extern void set_dram(uint8_t dram_no);

#endif /*_YMZ294_H_*/