#define TIMER1_PRESCALER (uint8_t) 1

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

const char notes[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};
const uint16_t tones[] = {262, 294, 329, 349, 392, 440, 494, 523};
const uint16_t tempo = 400;

static const char melody[] PROGMEM = "ccggaagffeeddcggffeeddggffeeddccggaagffeeddcccggaagffeeddcggffeeddggffeeddccggaagffeeddc";
static const uint8_t beats[] PROGMEM = {
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
};

void pause(){
  OCR1A = 0;
  _delay_ms(5);
}

void setTone(char note) {
  for(int i = 0; i < 8; i++) {
    if(note == notes[i]) {
      OCR1A = (F_CPU / (tones[i] * TIMER1_PRESCALER * 2) - 1);
      break;
    }
  }
}

int main(void) {
  // Set OC1A as output pin
  DDRB = _BV(PINB1);

  // Set Timer1, mode CTC, toggle on compare, prescale 1
  TCCR1A = _BV(COM1A0);
  TCCR1B = _BV(WGM12) | _BV(CS10);

  for (uint8_t i = 0; i < sizeof(melody); i++){
    setTone(pgm_read_byte(&melody[i]));
    for(uint8_t j = 0; j < pgm_read_byte(&beats[i]); j++) 
      _delay_ms(tempo);
    pause();
  }
}