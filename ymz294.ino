/* ymz294.ino     */
/* YMZ制御ファイル */
#include "ymz294.h"

#define ICNO_0       0
#define ICNO_1       1
#define ICNO_2       2

#define IC_MAX       3
#define IC_CHORD_MAX 3
#define ALL_CHORD_MAX (IC_CHORD_MAX * IC_MAX)

#define CHANNEL_A_LO           0x00   /* 楽音周波数(チャンネルA下位8bit)アドレス */
#define CHANNEL_A_HI           0x01   /* 楽音周波数(チャンネルA上位8bit)アドレス */
#define CHANNEL_B_LO           0x02   /* 楽音周波数(チャンネルB下位8bit)アドレス */
#define CHANNEL_B_HI           0x03   /* 楽音周波数(チャンネルB上位8bit)アドレス */
#define CHANNEL_C_LO           0x04   /* 楽音周波数(チャンネルC下位8bit)アドレス */
#define CHANNEL_C_HI           0x05   /* 楽音周波数(チャンネルC上位8bit)アドレス */
#define CHANNEL_NOISE          0x06   /* ノイズ周波数アドレス                   */
#define CHANNEL_MIX            0x07   /* ミキサーアドレス                       */
#define CHANNEL_DAC_A          0x08   /* 音量・DAC(チャンネルA)アドレス         */
#define CHANNEL_DAC_B          0x09   /* 音量・DAC(チャンネルB)アドレス         */
#define CHANNEL_DAC_C          0x0A   /* 音量・DAC(チャンネルC)アドレス         */
#define CHANNEL_ENV_FREQ_LO    0x0B   /* エンベロープ周波数(下位8bit)アドレス    */
#define CHANNEL_ENV_FREQ_HI    0x0C   /* エンベロープ周波数(上位8bit)アドレス    */
#define CHANNEL_EMV_SHAPE      0x0D   /* エンベロープ形状アドレス               */

#define SYSTEM_CLK           2000000   /* YMZシステムクロック                   */

#define CHANNELL_OUT_A         0x3D
#define CHANNELL_OUT_B         0x3C
#define CHANNELL_OUT_C         0x3B
#define CHANNELL_OUT_NOISE_A   0x37
#define CHANNELL_OUT_NOISE_B   0x2F
#define CHANNELL_OUT_NOISE_C   0x1F
#define CHANNELL_ALL_OUT       0x38

#define YMZ294_CH0             0x00
#define YMZ294_CH1             0x01
#define YMZ294_CH2             0x02  


/* YMZ294PINコンフィグ */
const uint8_t WR_PIN     = 16;
const uint8_t A0_PIN     = 18;
const uint8_t RESET_PIN  = 17;
const uint8_t DATA_PIN[] = { 5, 6, 7, 9, 10, 19, 20, 21 };

/* @@@ */
const uint8_t CS0        = 0;
const uint8_t CS1        = 1;
const uint8_t CS2        = 2;

static const uint8_t cs_pin[IC_MAX] = 
{
    CS0,
    CS1,
    CS2
};

static const uint8_t ic_select[ALL_CHORD_MAX] = 
{
    ICNO_0,
    ICNO_0,
    ICNO_0,
    ICNO_1,
    ICNO_1,
    ICNO_1,
    ICNO_2,
    ICNO_2,
    ICNO_2
};

static const uint8_t ic_ch_select[ALL_CHORD_MAX] = 
{
    YMZ294_CH0,
    YMZ294_CH1,
    YMZ294_CH2,
    YMZ294_CH0,
    YMZ294_CH1,
    YMZ294_CH2,
    YMZ294_CH0,
    YMZ294_CH1,
    YMZ294_CH2
};

static void dram_kick(void);
static void dram_snare(void);
static void dram_hiht(void);

const void (*dram_set[DRAM_MAX])(void) = 
{
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_kick,
    &dram_kick,
    &dram_snare,
    &dram_snare,
    &dram_hiht,
    &dram_snare,
    &dram_kick,
    &dram_hiht,
    &dram_kick,
    &dram_hiht,
    &dram_kick,
    &dram_hiht,
    &dram_kick,
    &dram_kick,
    &dram_hiht,
    &dram_kick,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_kick,
    &dram_kick,
    &dram_kick,
    &dram_kick,
    &dram_kick,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_hiht,
    &dram_hiht,
    &dram_snare,
    &dram_snare,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_kick,
    &dram_kick,
    &dram_kick,
    &dram_hiht,
    &dram_hiht,
    &dram_hiht,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare,
    &dram_snare
};

/* YMZレジスタ設定                     **
 * 引数 : byte    アドレス             **
          word    設定値              **       
 * 戻り値: なし                        */
static void set_register( uint8_t addr, uint32_t value, uint8_t cs_sel )
{
    /* アドレスセット */
    digitalWrite(cs_sel, LOW);
    digitalWrite(WR_PIN, LOW);
    digitalWrite(A0_PIN, LOW);
    for (int i = 0; i < 8; i++) 
    {
        digitalWrite(DATA_PIN[i], bitRead(addr, i));
    }
    digitalWrite(WR_PIN, HIGH);

    /* 設定値セット */
    digitalWrite(WR_PIN, LOW);
    digitalWrite(A0_PIN, HIGH);
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(DATA_PIN[i], bitRead(value, i));
    }
    digitalWrite(WR_PIN, HIGH);
    digitalWrite(cs_sel, HIGH);
}

/* YMZ294初期化                      **
 * 引数  : なし                      **
 * 戻り値: なし                       */
void ymz294_init( void )
{
    pinMode(CS0, OUTPUT);
    pinMode(CS1, OUTPUT);
    pinMode(CS2, OUTPUT);
    /* ピン入出力設定 */
    for (int i = 0; i < 8; i++)
    {
        pinMode(DATA_PIN[i], OUTPUT);
    }
    pinMode(WR_PIN, OUTPUT);
    pinMode(A0_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
  
    /* YMZ294初期化処理 */
    digitalWrite(CS0, LOW);
    digitalWrite(CS1, LOW);
    digitalWrite(CS2, LOW);
    digitalWrite(RESET_PIN, LOW);
    delay(10);
    digitalWrite(RESET_PIN, HIGH);    
    digitalWrite(WR_PIN, HIGH);
    digitalWrite(A0_PIN, LOW);
    digitalWrite(CS0, HIGH);
    digitalWrite(CS1, HIGH);
    digitalWrite(CS2, HIGH);

    set_mixer(0x30,cs_pin[0]);
    set_mixer(CHANNELL_ALL_OUT,cs_pin[1]);
    set_mixer(CHANNELL_ALL_OUT,cs_pin[2]);
    
}

/* 周波数設定                          **
 * 引数 : uint8_t チャンネル           **
          word    周波数              **       
 * 戻り値: なし                        */
void set_freqency(uint8_t ch, uint32_t freq) 
{
    word cal_freqency = 0;

    if (freq != 0) 
    {
      cal_freqency = (word)( 125000 / freq);
    }
    cal_freqency &= 0x0FFF;
    set_register(CHANNEL_A_LO + (ic_ch_select[ch] * 2), cal_freqency & 0xFF,cs_pin[ic_select[ch]]);
    set_register(CHANNEL_A_HI + (ic_ch_select[ch] * 2), (cal_freqency >> 8) & 0xFF,cs_pin[ic_select[ch]]);
}

void set_noisefreqency(uint8_t ch, uint8_t freq)
{
    word cal_freqency = 0;
    if (freq != 0) {
      cal_freqency = (word)(125000 / freq);
    }
    cal_freqency &= 0xFF;
    set_register(CHANNEL_NOISE, cal_freqency,cs_pin[ic_select[ch]]);
}

/* ミキサー設定                        **
 * 引数 : uint8_t 設定値               **     
 * 戻り値: なし                        */
void set_mixer( uint8_t set_mix, uint8_t cs_sel )
{
    set_register(CHANNEL_MIX, set_mix, cs_sel);
}

/* 音量設定                           **
 * 引数 : uint8_t チャンネル           **
          uint8_t 設定値              **          
 * 戻り値: なし                        */
void set_volume(uint8_t ch, uint8_t volume) 
{
    set_register(CHANNEL_DAC_A + ic_ch_select[ch], volume,cs_pin[ic_select[ch]]);   
}

/* エンベロープ設定                    **
 * 引数 : uint8_t チャンネル           **
          double  周期(ms)            **
          uint8_t エンベロープパターン **          
 * 戻り値: なし                        */
void set_envelope(uint8_t ch, double time, uint8_t shape) 
{
    uint16_t set_val; 

    set_val =  SYSTEM_CLK / ( 256 / ( time / 1000 ) );
    
    set_register(CHANNEL_DAC_A + ic_ch_select[ch], 0x10,cs_pin[ic_select[ch]]);  
    set_register(CHANNEL_ENV_FREQ_LO, 0xFF & set_val,cs_pin[ic_select[ch]]);
    set_register(CHANNEL_ENV_FREQ_HI, 0xFF & ( set_val >> 8 ),cs_pin[ic_select[ch]]);
    set_register(CHANNEL_EMV_SHAPE, shape,cs_pin[ic_select[ch]]);
}

void dram_kick( void )
{
    set_noisefreqency(0,freq_table[80]);
    set_envelope(0,10,0);
}

void dram_snare( void )
{
    set_noisefreqency(0,freq_table[40]);
    set_envelope(0,500,0);
}

void dram_hiht( void )
{
    set_noisefreqency(0,freq_table[40]);
    set_envelope(0,100,0);
}

void set_dram(uint8_t dram_no)
{
    (*dram_set[dram_no])();
}