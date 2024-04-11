/* music.ino        */
/* 演奏制御ファイル  */
#include "ymz294.h"
#include "midi.h"
#include "music.h"

const uint8_t SW1      = A1;
const uint8_t TEST_PIN = 14;

#define PLAY_MODE   0
#define STOP_MODE   1
#define EVENT_CHECK 2

#define MUSIC_INFO_MAX 9
#define MIDI_CH_UNUSED 0xFF

static uint8_t music_state;
static uint8_t event_flg;
static uint32_t push_tim;
static uint32_t push_pre_tim;
static uint32_t cur_tim;
static uint8_t  midi_data_cnt;
static volatile uint8_t  push_cnt;
static uint8_t pre_music_state;

static play_info ch_already_take_no[MUSIC_INFO_MAX];
static play_info music_play_info;

void music_init ( void )
{
    music_state  = PLAY_MODE;
    event_flg    = EVENT_NONE;
    push_tim     = 0;
    cur_tim      = 0;
    push_cnt     = 0;
    push_pre_tim = 0;
    midi_data_cnt = 0;
    memset(&music_play_info,0xFF,sizeof(music_play_info));
    for(uint8_t i = 0; i < MUSIC_INFO_MAX; i++)
    {
        ch_already_take_no[i].midi_ch    = MIDI_CH_UNUSED;
        ch_already_take_no[i].note_no    = MIDI_CH_UNUSED;
        ch_already_take_no[i].velocity   = MIDI_CH_UNUSED;
        ch_already_take_no[i].delta_time = MIDI_CH_UNUSED;
    }
    pinMode(SW1,INPUT_PULLUP);
    pinMode(TEST_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(SW1),sw1_push,FALLING);
}

void music_info_input(play_info* info)
{
    music_play_info.midi_ch    = info->midi_ch;
    music_play_info.note_no    = info->note_no;
    music_play_info.velocity   = info->velocity;
    music_play_info.delta_time = info->delta_time;
}

void music_task( void )
{
    music_play();
    music_state_control();
}

static void music_state_control( void )
{
    cur_tim = millis() + 5;
    switch(music_state)
    {
        case PLAY_MODE:
            break;
        case STOP_MODE:
            sound_stop();
            while(push_cnt == 0){};
            break;
        case EVENT_CHECK:
            while( ( cur_tim - push_tim ) < 1000 )
            {
                cur_tim = millis() + 5;
            }
            if( push_cnt == 1 )
            {
                music_state = ( ~pre_music_state ) & 0x01;
            }
            else if( push_cnt == 2 )
            {
                music_state = PLAY_MODE;
                event_flg = EVENT_NEXT_TRACK;
            }
            else if( push_cnt == 3 )
            {
                music_state = PLAY_MODE;              
                event_flg = EVENT_BACK_TRACK;
            }
            else if( push_cnt == 4 )
            {
                /* @@@処理考え中 */
                music_state = PLAY_MODE;
                event_flg = EVENT_LOOP_TRACK;
            }
            else{}
            push_cnt = 0;
            break;
        default:
            break;
    }
}

static void music_play( void )
{
    if(FREQ_MAX > music_play_info.note_no)   /* ノートナンバーが最大値以下 */
    {
        if(music_play_info.midi_ch == 0x09)
        {
            ch_already_take_no[0].midi_ch    = music_play_info.midi_ch;/* チャンネル使用に設定                */
            ch_already_take_no[0].note_no    = music_play_info.note_no;
            ch_already_take_no[0].velocity   = music_play_info.velocity;
            ch_already_take_no[0].delta_time = music_play_info.delta_time;
            set_dram(music_play_info.note_no);
        }
        else
        {
            for(uint8_t i = 1; i < MUSIC_INFO_MAX; i++)
            {
                if(music_play_info.velocity > 0)                             /* ベロシティが0より大きい   */
                {
                    if(ch_already_take_no[i].midi_ch == MIDI_CH_UNUSED)                 /* チャンネル占有なし       */
                    {
                        ch_already_take_no[i].midi_ch    = music_play_info.midi_ch;/* チャンネル使用に設定                */
                        ch_already_take_no[i].note_no    = music_play_info.note_no;
                        ch_already_take_no[i].velocity   = music_play_info.velocity;
                        ch_already_take_no[i].delta_time = music_play_info.delta_time;
                        set_freqency(i, freq_table[music_play_info.note_no]);  /* ノートナンバーに合わせて周波数を設定 */
                        set_volume(i,13);                                      /* チャンネルの音量を設定              */    
                        break;
                    }
                }
                else                                                                          /* ベロシティが0            */
                {
                    if( (ch_already_take_no[i].midi_ch == music_play_info.midi_ch)
                     && (ch_already_take_no[i].note_no == music_play_info.note_no) ) /* ノートナンバーがチャンネルを占有 */
                    {
                        ch_already_take_no[i].midi_ch    = MIDI_CH_UNUSED;      /* チャンネル未使用に設定          */
                        ch_already_take_no[i].note_no    = MIDI_CH_UNUSED;
                        ch_already_take_no[i].velocity   = MIDI_CH_UNUSED;
                        ch_already_take_no[i].delta_time = MIDI_CH_UNUSED;
                        set_volume(i, 0);                                       /* チャンネルの音量を0に設定       */
                        break;
                    }
                }
            }
        }
    }  
    digitalWrite(TEST_PIN,LOW);
    delay(music_play_info.delta_time);         /* デルタタイム(ms)分遅延  */
    digitalWrite(TEST_PIN,HIGH);
    midi_data_cnt++;
    if(midi_data_cnt > MUSIC_INFO_MAX)
    {
        midi_data_cnt = 0;
    }
}

uint8_t get_music_event(void)
{
    uint8_t event = event_flg;
    event_flg = EVENT_NONE;
    return event;
}

void sound_stop( void )
{
    for(uint8_t i = 0; i < MUSIC_INFO_MAX; i++)
    {
        ch_already_take_no[i].midi_ch = MIDI_CH_UNUSED;              /* チャンネル未使用に設定          */
        ch_already_take_no[i].note_no    = 0;
        ch_already_take_no[i].velocity   = 0;
        ch_already_take_no[i].delta_time = 0;
        set_volume(i, 0);                                       /* チャンネルの音量を0に設定       */
    }
}

void sw1_push(void)
{
    push_tim = millis();
    sound_stop();
    if( push_cnt == 0 )
    {
        pre_music_state = music_state;
        music_state     = EVENT_CHECK;
    }
    if( ( push_tim - push_pre_tim ) > 150 )
    {
        set_dram(0);
        push_cnt++;
        if(push_cnt > 5)
        {
            push_cnt = 1;
        }
    }
    push_pre_tim = push_tim;
}