/* midi.ino         */
/* midi制御ファイル  */
#include "midi.h"

#define MIDI_EVENT_MAX      11  /* ステータスバイト数 */ 
#define META_EVENT_MAX      18  /* メタイベント数    */
#define HEDER_TRUCK_MAX      2  /* ヘッダドラック数  */
#define HEDER_TRUCK_LEN      4  /* ヘッダトラック長  */

#define HEDER_CHECK_POS      0  /* ヘッダチェック位置   */
#define TRUCK_CHECK_POS      1  /* トランクチェック位置 */

#define MIDI_FILE_CHECK         0 /* MIDI処理状態:ファイルチェック        */
#define MIDI_HEADER_CHECK       1 /* MIDI処理状態:ヘッダチェック          */
#define MIDI_TRUCK_CHECK        2 /* MIDI処理状態:トラックチャンクチェック */
#define MIDI_STATEBYTE_CHECK    3 /* MIDI処理状態:ステータスバイトチェック */
#define MIDI_METABYTE_CHECK     4 /* MIDI処理状態:メタイベント種類チェック */
#define MIDI_SYSEX_CHECK        5 /* MIDI処理状態:SysExチェック          */
#define MIDI_DATA_LEN_CHECK     6 /* MIDI処理状態:データ長チェック        */
#define MIDI_EVENTDETAILS_CHECK 7 /* MIDI処理状態:イベント内容チェック    */
#define MIDI_DELTA_TIME_CHECK   8 /* MIDI処理状態:デルタタイムチェック    */
#define MIDI_FILE_ERROR         9 /* MIDI処理状態:ファイルエラー         */

#define MIDI_EVENT_4D   0 /* MIDIイベント:ヘッダ,トラック               */
#define MIDI_EVENT_90   1 /* MIDIイベント:ノートオン                    */
#define MIDI_EVENT_80   2 /* MIDIイベント:ノートオフ                    */
#define MIDI_EVENT_A0   3 /* MIDIイベント:ポリフォニックキープレッシャー  */
#define MIDI_EVENT_B0   4 /* MIDIイベント:コントロールチェンジ           */
#define MIDI_EVENT_C0   5 /* MIDIイベント:プログラムチェンジ             */
#define MIDI_EVENT_D0   6 /* MIDIイベント:チャンネルプレッシャー         */
#define MIDI_EVENT_E0   7 /* MIDIイベント:ピッチベンド(リトルエンディアン)*/
#define MIDI_EVENT_F0   8 /* MIDIイベント:SysExイベント                 */
#define MIDI_EVENT_F7   9 /* MIDIイベント:SysExイベント                 */
#define MIDI_EVENT_FF  10 /* MIDIイベント:メタイベント                  */

#define META_EVENT_00  0  /* メタイベント:シーケンス番号             */
#define META_EVENT_01  1  /* メタイベント:テキスト                  */
#define META_EVENT_02  2  /* メタイベント:著作権表示　　             */
#define META_EVENT_03  3  /* メタイベント:シーケンス名               */
#define META_EVENT_04  4  /* メタイベント:楽器名                     */
#define META_EVENT_05  5  /* メタイベント:歌詞                       */
#define META_EVENT_06  6  /* メタイベント:マーカー                   */
#define META_EVENT_07  7  /* メタイベント:キューポイント              */
#define META_EVENT_08  8  /* メタイベント:プログラム名(音色)          */
#define META_EVENT_09  9  /* メタイベント:デバイス名(音源)            */
#define META_EVENT_20 10  /* メタイベント:MIDIチャンネルプリフィックス */
#define META_EVENT_21 11  /* メタイベント:ポート指定                  */ 
#define META_EVENT_2F 12  /* メタイベント:トラック終端                */
#define META_EVENT_51 13  /* メタイベント:テンポ設定                 */
#define META_EVENT_54 14  /* メタイベント:SMPTEオフセット            */
#define META_EVENT_58 15  /* メタイベント:拍子の設定                 */
#define META_EVENT_59 16  /* メタイベント:調の設定                  */
#define META_EVENT_7F 17  /* メタイベント:シーケンサ特定メタイベント  */

#define MIDI_VARIABLE_DATA_MAX  4 /* 最大可変長データ */

#define VARIABLE_DATA_COMPLETE    0 /* 可変長データ処理完了   */
#define VARIABLE_DATA_INCOMPLETE  1 /* 可変長データ処理未完了 */

#define EVENT_TYPE_MIDI  0          /* MIDIイベントタイプ:MIDI  */
#define EVENT_TYPE_META  1          /* MIDIイベントタイプ:META  */
#define EVENT_TYPE_SYSEX 2          /* MIDIイベントタイプ:SYSEX */

#define HEDER_CHANK_SIZE 14         /* ヘッダチャンクサイズ     */

const uint8_t heder_truck_check[HEDER_TRUCK_MAX][HEDER_TRUCK_LEN] = 
{
    { 0x4D, 0x54, 0x68, 0x64  }, /* ヘッダデータ   */
    { 0x4D, 0x54, 0x72, 0x6B  }, /* トラックデータ */
};

const uint8_t midi_event[MIDI_EVENT_MAX] = /* MIDIイベント */
{
    0x4D,
    0x90,
    0x80,
    0xA0,
    0xB0,
    0xC0,
    0xD0,
    0xE0,
    0xF0,
    0xF7,
    0xFF
};

const uint8_t meta_event[META_EVENT_MAX] = /* メタイベント */
{
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x20,
    0x21,
    0x2F,
    0x51,
    0x54,
    0x58,
    0x59,
    0x7F
};

static uint8_t  midi_byte_data;           /* midiデータ1byte処理用        */
static uint8_t  data_com_cnt;             /* データ共通カウンタ            */
static uint8_t  data_match_cnt;           /* データマッチカウンタ          */
static uint32_t free_data_cnt;            /* フリーデータカウンタ          */
static uint8_t  midi_file_check_result;   /* midiファイルチェック結果      */
static uint8_t  midi_state;               /* midi処理状態                 */
static uint8_t  midi_state_byte;          /* ステータスバイト判定用        */
static uint8_t  midi_meta_byte;           /* メタイベント判定用            */
static uint8_t  midi_event_type;          /* イベントタイプ                */
static uint32_t midi_data_len;            /* データ長                      */                  
static uint32_t midi_variable_data[MIDI_VARIABLE_DATA_MAX]; /* 可変長データ処理用 */
static uint32_t tempo;                    /* テンポデータ                  */
static float    delta_base;               /* デルタ基準タイム              */
static uint16_t resolution;               /* 分解能取得変数                */
static uint8_t  midi_note_no;             /* ノートナンバー                */
static uint8_t  midi_velocity;            /* ベロシティ                   */
static uint32_t midi_delta_time;          /* デルタタイム元データ          */
static float    delta_time_ms;            /* ms変換後デルタタイム          */
static uint8_t  midi_event_full;          /* midiイベント情報処理完了フラグ */
static uint8_t  midi_ch_no;               /* midiイベントチャンネルナンバー */

/* MIDI初期化処理                      **
 * 引数  : なし                        **    
 * 戻り値: なし                        **
 * 注意:この関数はmidiファイルを        **
   読み込む前に必ずコールすること        */ 
void midi_init( void )
{
    midi_byte_data  = 0;
    data_com_cnt    = 0;
    data_match_cnt  = 0;
    free_data_cnt   = 0;
    midi_state_byte = 0;
    midi_meta_byte  = 0;
    midi_event_type = 0;
    midi_note_no    = 0;  
    midi_velocity   = 0;
    midi_delta_time = 0;
    resolution      = 0;
    midi_event_full = MIDI_EVENT_NO;
    tempo           = 0;
    delta_base      = 0.0;
    delta_time_ms   = 0.0;
    midi_state      = MIDI_FILE_CHECK;
    midi_file_check_result = MIDI_FILE_NG;
    for(uint8_t i = 0; i < MIDI_VARIABLE_DATA_MAX; i++)
    {
        midi_variable_data[i] = 0;
    }
}

/* MIDI1byte入力処理                  **
 * 引数 : uint8_t midiデータ(1byte)   **    
 * 戻り値: なし                       **
 * 注意:MIDIメイン処理の前             **
 *      にコールすること               */
void midi_input(uint8_t midi_byte)
{
    midi_byte_data = midi_byte;
    free_data_cnt++;
}

/* MIDIメイン処理                     **
 * 引数  : なし                       **    
 * 戻り値: なし                       **
 * 注意:MIDI1byte入力処理コール後に    **
 *      にコールすること               */ 
void midi_task( void )
{
    switch(midi_state)
    {      
        case MIDI_FILE_CHECK:
            midi_file_check();
            break;
        case MIDI_HEADER_CHECK:
            midi_header_check();
            break;
        case MIDI_TRUCK_CHECK:
            midi_truck_check();
            break;
        case MIDI_STATEBYTE_CHECK:
            midi_state_byte_check();
            break;  
        case MIDI_METABYTE_CHECK:
            midi_metabyte_check();
            break;
        case MIDI_SYSEX_CHECK:
            midi_sysex_check();
            break;
        case MIDI_DATA_LEN_CHECK:
            midi_data_len_check();
            break;
        case MIDI_EVENTDETAILS_CHECK:
            midi_eventdetails_check();
            break;
        case MIDI_DELTA_TIME_CHECK:
            midi_delta_tim_check();
            break;
        case MIDI_FILE_ERROR:
            midi_file_error();
            break;
        default:
            midi_file_error();
            break;
    }
}

/* MIDIファイルチェック処理            **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_file_check( void )
{
    midi_heder_truck_check( HEDER_CHECK_POS );
}

/* MIDIヘッダチェック処理              **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_header_check( void )
{
    if( free_data_cnt == 13 )
    {
        resolution |= ((uint16_t)midi_byte_data << 8);
    }
    if ( free_data_cnt == HEDER_CHANK_SIZE )  /* ヘッダチャンク読み込み完了               */
    {
        resolution |= ((uint16_t)midi_byte_data);
        midi_state = MIDI_STATEBYTE_CHECK;
    }
}

/* MIDIトラックチャンクチェック処理     **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_truck_check( void )
{
    /*@@@応急処置(メタイベントが来るまで待機)*/
    // midi_heder_truck_check( TRUCK_CHECK_POS );
    if ( midi_byte_data == 0xFF)
    {
        midi_state = MIDI_STATEBYTE_CHECK;
        midi_task();
    }
}

/* MIDIステータスバイトチェック処理     **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_state_byte_check( void )
{
    /* @@@処理検討中 */
    if( midi_byte_data == 0xF0)
    {
         midi_state_byte = MIDI_EVENT_F0;
         midi_event_type = EVENT_TYPE_SYSEX;
         midi_state = MIDI_TRUCK_CHECK;
    }
    else if(midi_byte_data == 0x4D)
    {
        midi_state = MIDI_TRUCK_CHECK;
        midi_heder_truck_check( HEDER_CHECK_POS );
    }
    else if(midi_byte_data == 0xFF)
    {
        midi_state_byte = MIDI_EVENT_FF;
        midi_state = MIDI_METABYTE_CHECK;
        midi_event_type = EVENT_TYPE_META;
    }
    else
    {
        for(uint8_t i = 0; i < MIDI_EVENT_MAX; i++)
        {
            if( (midi_byte_data & 0xF0) == midi_event[i] )
            {
                midi_state_byte = i;
                midi_ch_no = midi_byte_data & 0x0F;
                midi_event_type = EVENT_TYPE_MIDI;
                midi_state = MIDI_EVENTDETAILS_CHECK;
                break;
            }
        }
    }
}

/* MIDIメタイベント種類チェック処理     **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_metabyte_check( void )
{
    for(uint8_t i = 0; i < META_EVENT_MAX; i++)
    {
        if( midi_byte_data == meta_event[i] )
        {
            midi_meta_byte = i;            
            if(  midi_meta_byte == META_EVENT_2F  )
            {
                midi_state = MIDI_DELTA_TIME_CHECK;
            }
            else
            {
                midi_state = MIDI_DATA_LEN_CHECK;
            }
            break;
        }
    } 
}

/* MIDI SysExチェック処理             **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_sysex_check( void )
{
    if(midi_byte_data == 0xF7)
    {
        midi_state = MIDI_DELTA_TIME_CHECK;  
    }
}

/* メタイベントデータ長チェック処理     **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_data_len_check( void )
{
     if( midi_variable_len_data_check(&midi_data_len) == VARIABLE_DATA_COMPLETE )
     {
       if(midi_data_len == 0)
       {   
           midi_state = MIDI_DELTA_TIME_CHECK;
       }
       else if( midi_event_type == EVENT_TYPE_SYSEX )
       {
           midi_state = MIDI_SYSEX_CHECK;
       }
       else
       {                  
           midi_state = MIDI_EVENTDETAILS_CHECK;
       }
     }

}

/* イベント内容チェック処理            **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_eventdetails_check( void )
{
    if( midi_event_type == EVENT_TYPE_MIDI)
    {
        midi_midi_eventdetails();
    }
    else
    {
        midi_meta_eventdetails();
    }
        
}

/* MIDIイベント内容チェック処理        **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_midi_eventdetails( void )
{
    data_com_cnt++;
    switch(midi_state_byte)
    {
        case MIDI_EVENT_90:
            if(data_com_cnt == 1)
            {
                midi_note_no = midi_byte_data;
            }
            else
            {
                data_com_cnt = 0;
                midi_velocity = midi_byte_data;
                midi_state = MIDI_DELTA_TIME_CHECK;                
            }
            break;
        case MIDI_EVENT_80:
            if(data_com_cnt == 1)
            {
                midi_note_no = midi_byte_data;
            }
            else
            {
                data_com_cnt = 0;
                midi_velocity = midi_byte_data;
                midi_state = MIDI_DELTA_TIME_CHECK;                
            }
            break;
        case MIDI_EVENT_A0:
            if(data_com_cnt == 2)
            {
                data_com_cnt = 0;
                midi_state = MIDI_DELTA_TIME_CHECK; 
            }        
            break;
        case MIDI_EVENT_B0:
            if(data_com_cnt == 2)
            {
                data_com_cnt = 0;
                midi_state = MIDI_DELTA_TIME_CHECK; 
            }
            break;
        case MIDI_EVENT_C0:
            if(data_com_cnt == 1)
            {
                data_com_cnt = 0;              
                midi_state = MIDI_DELTA_TIME_CHECK; 
            }
            break;
        case MIDI_EVENT_D0:
            if(data_com_cnt == 1)
            {
                data_com_cnt = 0;
                midi_state = MIDI_DELTA_TIME_CHECK; 
            }
            break;
        case MIDI_EVENT_E0:
            if(data_com_cnt == 2)
            {
                data_com_cnt = 0;
                midi_state = MIDI_DELTA_TIME_CHECK; 
            }
            break;
        default:
            break;
    }
}

/* メタイベント内容チェック処理         **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_meta_eventdetails( void )
{
    if( midi_meta_byte == META_EVENT_51)
    {
        if(midi_data_len == 3)
        {
            tempo = 0;
            delta_base = 0.0; 
        }
        tempo |= ( ((uint32_t)midi_byte_data) << (8 * (midi_data_len - 1)));  /* テンポ情報を取得          */                                
        delta_base = ((float)tempo / 1000) / (float)resolution;               /* 単位usをsに変換           */      
    }
    midi_data_len--;
    if(midi_data_len == 0)
    {
        midi_state = MIDI_DELTA_TIME_CHECK;
    }
}

/* デルタタイムチェック処理            **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_delta_tim_check( void )
{
    if( midi_variable_len_data_check(&midi_delta_time) == VARIABLE_DATA_COMPLETE )
    {     
        delta_time_ms = (float)midi_delta_time * delta_base;  
        midi_state = MIDI_STATEBYTE_CHECK;
        if( ( midi_state_byte == MIDI_EVENT_90 ) || ( midi_state_byte == MIDI_EVENT_80 ) )
        {
            midi_event_full = MIDI_EVENT_FULL;                        
        }
        else
        {
            midi_event_full = MIDI_DELTA_TIM_OK;
        }
    }

}

/* ヘッダ,トラックチャンクチェック処理  **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_heder_truck_check( uint8_t heder_truck_pos )
{
    if( heder_truck_check[heder_truck_pos][data_com_cnt] == midi_byte_data)
    {
        data_match_cnt++;
    }
    data_com_cnt++;
    if( data_com_cnt == HEDER_TRUCK_LEN )
    {
        if( data_match_cnt == 4 )
        {
            midi_file_check_result = MIDI_FILE_OK;
            if( heder_truck_pos == HEDER_CHECK_POS )
            {
                midi_state = MIDI_HEADER_CHECK;
            }
            else
            {
                midi_state = MIDI_STATEBYTE_CHECK;
            }
        }
        else
        {
            midi_file_error();
        }
        data_com_cnt = 0;        
    }
}

/* 可変長データチェック処理            **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static uint32_t midi_variable_len_data_check( uint32_t* variable_data )
{
    uint32_t ret = VARIABLE_DATA_INCOMPLETE;
    if ((midi_byte_data & 0x80) == 0)
    {
         midi_variable_data[data_com_cnt] = ((uint32_t)midi_byte_data & ((uint32_t)0x07F)) << (24 - (data_com_cnt * 8));
         if (data_com_cnt == 1)
         {
             *variable_data = ((midi_variable_data[0] >> 1) | (midi_variable_data[1]) | (midi_variable_data[2]) | (midi_variable_data[3])) >> (24 - (data_com_cnt * 8));
         }
         else if (data_com_cnt == 2)
         {
             *variable_data = ((midi_variable_data[0] >> 1) | (midi_variable_data[1] >> 1) | (midi_variable_data[2]) | (midi_variable_data[3])) >> (24 - (data_com_cnt * 8));
         }
         else if (data_com_cnt == 3)
         {
             *variable_data = ((midi_variable_data[0] >> 1) | (midi_variable_data[1] >> 1) | (midi_variable_data[2] >> 1) | (midi_variable_data[3])) >> (24 - (data_com_cnt * 8));
         }
         else
         {
             *variable_data = ((midi_variable_data[0]) | (midi_variable_data[1]) | (midi_variable_data[2]) | (midi_variable_data[3])) >> (24 - (data_com_cnt * 8));
         }
         data_com_cnt = 0;         
         ret = VARIABLE_DATA_COMPLETE; 
    }
    else
    {
         midi_variable_data[data_com_cnt] = ((uint32_t)midi_byte_data & ((uint32_t)0x7F)) << (24 - (data_com_cnt * 8));
         data_com_cnt++;       
    }
    return ret;
}

/* midiファイルエラー時処理            **
 * 引数  : なし                       **    
 * 戻り値: なし                       */ 
static void midi_file_error( void )
{
    midi_file_check_result = MIDI_FILE_NG;
    midi_state             = MIDI_FILE_ERROR;
}

/* midiファイルエラーチェック結果公開処理 **
 * 引数  : なし                         **    
 * 戻り値: なし                         */ 
uint8_t get_midi_file_check_result( void )
{
    return midi_file_check_result;
}

/* midiイベントデータ公開処理            **
 * 引数  : なし                         **    
 * 戻り値: なし                         */ 
uint8_t get_midi_event_data_result( play_info* info )
{
    uint8_t ret = midi_event_full;
    midi_event_full = MIDI_EVENT_NO;
    
    info->midi_ch  = midi_ch_no;
    info->note_no  = midi_note_no;
    info->velocity = midi_velocity;
    info->delta_time = delta_time_ms;
    
    return ret;
}

