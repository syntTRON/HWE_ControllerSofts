/*
Bot SP level Dedect
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PWM_STOPP_RIGHT	300
#define PWM_STOPP_LEFT	302
#define PWM_MAX			400
#define PWM_MIN			200

// PORTB
#define LED_RV			(1<<0)
#define LED_RH			(1<<1)
#define LED_LV			(1<<2)
#define LED_LH			(1<<3)
#define LED_GRUEN		(1<<4)
#define LED_ROT			(1<<5)
#define MOT_RIGHT		(1<<6)

// PORTD
// PD0 -> I2C SCL
// PD1 -> I2C SDA
#define RECEIVER		(1<<2)
#define TRANSMITTER		(1<<3)
#define WHEEL_RIGHT		(1<<4)
#define DEBUG1			(1<<5)
#define LF_EMITTER		(1<<6)
#define MOT_LEFT		(1<<7)

//PORTF
#define MEASURE_UB		(1<<0)
#define ADC_UB			0
#define LF_LEFT			(1<<1)
#define ADC_LF_LEFT		1
#define LF_RIGHT		(1<<4)
#define ADC_LF_RIGHT	4
#define CHOOSE_LR		(1<<5)
#define CHOOSE_FB		(1<<6)
#define DEBUG2			(1<<7)

//PORTC
#define IO_RESET		(1<<6)
#define WHEEL_LEFT		(1<<7)

//PORTE
#define IO_INT			(1<<6)

//Motorick
#define PWM_R_VOR_MAX	125				//2 ms
#define PWM_R_RET_MAX	62				//0,992 ms
#define PWM_R_STOPP		94				//1,504 ms

#define PWM_L_VOR_MAX	62				//0,992 ms
#define PWM_L_RET_MAX	125				//2 ms
#define PWM_L_STOPP		94				//1,504 ms


unsigned char ADC_Mesure_Compleated = 0x00;
unsigned char ISR_Occured=0x00;
uint16_t ADC_Val=0x0000;

ISR(TIMER0_OVF_vect)
{
    ISR_Occured=0xFF;
}

ISR(ADC_vect)
{
    ADC_Mesure_Compleated=0xFF;
}

void adc_measure_init(unsigned char channel)
{
	//unsigned int result=0;

	ADMUX = 0;
	ADMUX &= ~(1<<REFS1)&~(1<<REFS0);			//ext. AREF = 5V
	ADMUX &= ~(1<<ADLAR);						//reight adjusted 10bit

	ADCSRB &= ~(1<<MUX5);
	ADMUX &= ~(1<<MUX4)&~(1<<MUX3)&~(1<<MUX2)&~(1<<MUX1);
	ADMUX |= channel;

	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADIE)|(1<<ADIF);	//ADC activate, Prescaler on 128 -> 125kHz Samplingfrequenzy

	ADCSRA |= (1<<ADSC);						//start
	//ADCSRA=0xC8;
	//while(ADCSRA&(1<<ADSC));					//wate till compleate -- wuld be like delay and got remouved

}

void adc_val_update(void)
{
        // Updating global var ADC_Val
    	uint8_t adcl_tmp=0x00, adch_tmp=0x00;
        adcl_tmp=ADCL;
        adch_tmp=ADCH;
        ADC_Val=(adch_tmp<<8)|adcl_tmp;
}

int main(void)
{

    // CLK_IO to 16MHz
    CLKPR=0x80;
    CLKPR=0x00;
    //Disable Jtag
    MCUCR |= (1<<JTD);
	MCUCR |= (1<<JTD);

    //Timer 0 Setup
    TCCR0A=0x00;
    TCCR0B=0x04;
    TIMSK0=0x01;
//0x3F
    DDRB=0xFF;
    PORTB=0x00;
//0x40
    DDRD=0xFF;
    PORTD=0x00;

    //Timer4 HardwarePWM Setup
    /*
    TCCR4A = TCCR4A | (1<<PWM4B);
	TCCR4C = TCCR4C | (1<<PWM4D);
	TCCR4D = TCCR4D &~(1<<WGM41);
	TCCR4D = TCCR4D &~(1<<WGM40);		//Fast PWM am OC4B und OC4D

	TCCR4A = TCCR4A &~(1<<COM4B0);
	TCCR4A = TCCR4A | (1<<COM4B1);		//COM4B1:0=2

	TCCR4C = TCCR4C &~(1<<COM4D0);
	TCCR4C = TCCR4C | (1<<COM4D1);		//COM4D1:0=2

	TC4H = 0x03;
	OCR4C = 0xE8;						//f_PWM = f_CLK_T4/(1+OCR4C) = 62,5kHz/1000 = 62,5 Hz
	TC4H = 0x00;
	OCR4B = PWM_R_STOPP;				//Tastverhältnis am OC4B-Pin (PB6), PWM_rechts
	OCR4D = PWM_L_STOPP;				//Tastverhältnis am OC4D-Pin (PD7), PWM_links

	TCCR4B = TCCR4B | (1<<CS43);
	TCCR4B = TCCR4B &~(1<<CS42);
	TCCR4B = TCCR4B &~(1<<CS41);		//f_CLK_T4 = CLK_IO/Prescaler = 16MHz/256 = 62,5kHz
	TCCR4B = TCCR4B | (1<<CS40);		//Timer4 Prescaler = 1, Start PWM

    Will be active in future realeases
	*/

	//Interupts Aktivieren
    sei();

    uint8_t ISR_Count=0x00;


    while(1)
    {
        //On the fly interupt handling
        if(ISR_Occured==0xFF)
        {
            ISR_Occured=0x00;
            ISR_Count=ISR_Count+1;
        }

        //Software Timer
        if(ISR_Count>=240)
        {
            adc_measure_init(0); // Initialize measurement on ADC0 pin
            ISR_Count=0x00;
        }


        //ADC Measure Compleated interupt handling
        if(ADC_Mesure_Compleated==0xFF)
        {
            ADC_Mesure_Compleated=0x00;

            adc_val_update();
        }


        //Handling of different voltage levels detected on ADC0 pin
        PORTB = 0x00;
        if(ADC_Val>=586)
        {
           PORTB = 0x04;
        }
        else if(ADC_Val>=575&&ADC_Val<=585)
        {
            PORTB = 0x02;
        }
        else if(ADC_Val<=574)
        {
            PORTB = 0x01;
        }
        // Ad something in the future

    }

    return 0;
}
