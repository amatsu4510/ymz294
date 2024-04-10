/* midi.h           */
/* midi制御ファイル  */

#ifndef _MIDI_H_
#define _MIDI_H_

#define MIDI_FILE_OK      0
#define MIDI_FILE_NG      1

#define MIDI_EVENT_NO     0
#define MIDI_EVENT_FULL   1
#define MIDI_DELTA_TIM_OK 2

typedef struct
{
    uint8_t midi_ch;
    uint8_t note_no;
    uint8_t velocity;
    uint32_t delta_time;
}play_info;

extern void midi_init( void );
extern void midi_input(uint8_t midi_byte);
extern void midi_task( void );
extern uint8_t get_midi_file_check_result( void );
extern uint8_t get_midi_event_data_result( play_info* info );

#endif /* _MIDI_H_ */