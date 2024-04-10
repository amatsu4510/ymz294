#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>
#include "ymz294.h"
#include "music.h"
#include "midi.h"

#define FILE_NAME_LEN 13
#define FILE_MAX      256

/*@@@*/
static File root;
static uint8_t filename[FILE_MAX][FILE_NAME_LEN];
static uint8_t file_cnt;
static play_info midi_play_info;
static uint8_t music_no;
static uint8_t file_error_cnt;

void setup() 
{
    file_cnt = 0;
    file_error_cnt = 0;
    ymz294_init(); /* YMZ294初期化処理 　*/
    midi_init();   /* MIDI制御情報初期化 */
    music_init();  /* 演奏情報初期化     */
    memset(filename,0,sizeof(filename));
    memset(&midi_play_info,0,sizeof(midi_play_info));
    if (!SD.begin(4)) { /* SDカード接続処理 */
      while (1);        /* SDカード接続待ち */
    }
    root = SD.open("/"); /* midiファイルオープン */
    printDirectory(root, 0); /* midiファイル確認処理 */
}
void loop()
{
    uint8_t music_event;
    uint8_t midi_event_flg;
    for( music_no = 0; music_no < file_cnt; music_no++ ) /* ファイル数分ループ */
    {    
        ymz294_init(); /* YMZ294初期化処理 　*/
        midi_init();   /* MIDI制御情報初期化 */
        music_init();  /* 演奏情報初期化     */
        File dataFile = SD.open(&filename[music_no][0]);
        if (dataFile) 
        {
            while (dataFile.available()) 
            {           
                midi_input(dataFile.read()); 
                midi_task();
                midi_event_flg = get_midi_event_data_result(&midi_play_info);
                if ( midi_event_flg == MIDI_EVENT_FULL )
                {
                    music_info_input(&midi_play_info);
                    music_task();
                }
                else if( midi_event_flg == MIDI_DELTA_TIM_OK )
                {
                    delay(midi_play_info.delta_time);
                }
                else
                {
                    /* 処理なし */         
                }
                
                music_event = get_music_event();
                if( music_event == EVENT_NEXT_TRACK )
                {
                    break;
                }
                else if( music_event == EVENT_BACK_TRACK )
                {
                    if( file_cnt == 1 )
                    {
                        /* 処理なし */
                    }
                    else
                    {
                        if( music_no == 0 )
                        {
                           music_no  = file_cnt - 2;
                        }
                        else
                        {
                           music_no = music_no - 2;
                        }        
                    }
                    break;
                }
                else if( music_event == EVENT_LOOP_TRACK )
                {
                    if( file_cnt == 1 )
                    {
                        /* 処理なし */
                    }
                    else
                    {
                        music_no--;
                    }
                }
                else{}
            }
        }
        dataFile.close();                           
    }
} 

/* カレントディレクトリ全ファイル名取得 **
 * 引数 : File    ファイルポインタ     **
          int     ???                 **       
 * 戻り値: なし                        */
void printDirectory( File dir, int numTabs ) 
{
    while (true)
    {

        uint8_t len;
        File entry =  dir.openNextFile(); /* 次のファイルポインタを取得 */
        if (! entry) 
        {
            /* ファイルなし */
            break;
        }
        if( isMidFile(entry.name()) == 1 )
        {
            strcpy(&filename[file_cnt][0],entry.name()); /* ファイル名を配列にコピー */
            Serial.println(entry.name());
            file_cnt++;                                  /* ファイル数カウントアップ */
        }      
        if (entry.isDirectory()) 
        {
            printDirectory(entry, numTabs + 1);
        }
        else 
        {
            /* 処理なし */
        }
        entry.close();
    }
}

bool isMidFile(const char *fileName) 
{
    const char *ext = strrchr(fileName, '.');
    return strcmp(".MID", ext) == 0;
}
