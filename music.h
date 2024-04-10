/* music.h          */
/* 演奏設定ファイル  */

#include "midi.h"

#ifndef _MUSIC_H_
#define _MUSIC_H_

#define EVENT_NONE       0
#define EVENT_NEXT_TRACK 1
#define EVENT_BACK_TRACK 2
#define EVENT_LOOP_TRACK 3

extern void music_init ( void );
extern void music_info_input(play_info* info);
extern void music_task( void );
extern uint8_t get_music_event(void);

#endif /* _MUSIC_H_ */