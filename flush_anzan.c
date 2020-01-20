/*
テーマ名:フラッシュ暗算ゲーム
製作者:芦川 拓実
最終更新日:2020/01/20
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#define CTOP 480000

volatile unsigned char wait = 0;
volatile unsigned char sw, sw_flag;
volatile unsigned char bz_conf;
unsigned char led[8];

unsigned char pat[13][8] = {
 {7, 5, 5, 5, 7, 0, 0, 0}, // 0
 {1, 1, 1, 1, 1, 0, 0, 0}, // 1
 {7, 1, 7, 4, 7, 0, 0, 0}, // 2
 {7, 1, 7, 1, 7, 0, 0, 0}, // 3
 {5, 5, 7, 1, 1, 0, 0, 0}, // 4
 {7, 4, 7, 1, 7, 0, 0, 0}, // 5
 {7, 4, 7, 5, 7, 0, 0, 0}, // 6
 {7, 1, 1, 1, 1, 0, 0, 0}, // 7
 {7, 5, 7, 5, 7, 0, 0, 0}, // 8
 {7, 5, 7, 1, 7, 0, 0, 0}, // 9
 {0, 0, 0, 0, 0, 8, 0, 0}, // .
 {0, 0, 0, 0, 0, 0, 7, 0}, // 
 {0, 0, 32, 56, 60, 56, 32, 0},
 };

void update_led()
{
	static unsigned char sc = 0xFE;
	static unsigned char scan = 0;

	PORTB = 0;
	sc = (sc << 1) | (sc >> 7);
	PORTD = (PORTD & 0x0F) | (sc & 0xF0);
	PORTC = (PORTC & 0xF0) | (sc & 0x0F);
	scan = (scan + 1) & 7;
	PORTB = led[scan];
}

void update_sw(){
	if(wait){
		wait--;
		if(wait == 0){
			unsigned prev = sw;
			sw = (~PINC >> 4) & 3;
			sw_flag = sw ^ prev;
		}
	}
}

ISR(PCINT1_vect){
	wait = 10;
}

ISR(TIMER0_COMPA_vect){
	update_led();
	update_sw();

	if(bz_conf){
		PORTD ^= 0x08;
	}
}

ISR(TIMER1_COMPA_vect){
	TCCR1B = 0;
	TCNT1 = 0;

	bz_conf = 0;
	PORTD &= ~0x08;
}

int main(){
	unsigned int i, n;
	unsigned int j = 0;
	unsigned int sum = 0;
	unsigned int result_hand = 0;
	unsigned int result_ltwd = 0;
	unsigned long cnt = 0;
	
	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0xFE;
	PORTB = 0x00;
	PORTC = 0x3F;
	PORTD = 0xF0;	
	
	PCMSK1 = 0x30;
	PCICR = _BV(PCIE1);

	TCNT0 = 0;
	TCCR0A = 2;
	TCCR0B = 3;
	OCR0A = 50;
	TIMSK0 |= _BV(OCIE0A);

	TCNT1 = 0;
	TCCR1A = 2;
	TCCR1B = 3;
	OCR1A = 6218;
	TIMSK1 |= _BV(OCIE1A);
	
	sei();

	// タイトル画面	
	for(i = 0; i < 8; i++){
		led[i] = pat[12][i];						
	}
	

	for(;;){
		wdt_reset();
		cnt++;
		if(sw_flag & 1){
			if(cnt >= CTOP){
				cnt = 0;
				n = rand() % 100;
				if(j <= 4){
					j += 1;
					sum += n;
					for(i = 0; i < 7; i++){
						if(led[i + 1] != 0){
							led[i + 1] = 0;
						}
						led[i + 1] = (pat[n / 10][i] << 4 )| pat[n % 10][i];						
					}
					bz_conf = 1;
					TCCR1B = 0x0b;
				}				
			}			
		}

		if(sw_flag & 2){
			sw_flag = 0;
			result_hand = sum / 100;
			result_ltwd = sum % 100;
			
			for(i = 0; i < 7; i++){
				if(led[i + 1] != 0){
					led[i + 1] = 0;
				}
				if(result_hand == 0){
					led[i + 1] = (pat[sum / 10][i] << 4) | pat[sum % 10][i];
				}
				else{
					led[i + 1] = (pat[result_ltwd / 10][i] << 4) | pat[sum % 10][i];
				}
			}
			
			if(result_hand == 0){
				led[7] = 0;
			}
			else if(result_hand == 1){
				led[7] = 128;
			}
			else if(result_hand == 2){
				led[7] = 192;
			}
			else if(result_hand == 3){
				led[7] = 224;
			}
			else if(result_hand == 4){
				led[7] = 240;
			}
			sw_flag = 0;
			bz_conf = 1;
			TCCR1B = 0x0C;
		}

			
	}	
	return 0;
}