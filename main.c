/*
HWE Automatic Flyback converter adc voltage dedetct and pwm gennerator

Used Pins
ADC0 PF0
PC7 OC4A
 */

#include <avr/io.h>
#include <avr/interrupt.h>

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
    //Timer 0 Stop
    TCCR0B = 0x00;
//0x3F
    DDRB=0xFF;
    PORTB=0x00;
//0x40
    DDRD=0xFF;
    PORTD=0x00;
    //Port c
    DDRC = 0x80;
    PORTC = 0x80;

	//Timer 4 Setup Hardware PWM
	TCCR4D = 0x01;
	OCR4C = 23;
	TCCR4A = 0xC2;
	TCCR4B = 0x05;
	OCR4A = 17;

	//Interupts Aktivieren
    sei();

    //uint8_t ISR_Count=0x00;

    adc_measure_init(0); // Initialize measurement on ADC0 pin

    while(1)
    {
        //On the fly interupt handling
        if(ISR_Occured==0xFF)
        {
            ISR_Occured=0x00;
            //ISR_Count=ISR_Count+1;
        }

        //Software Timer
        /*
        if(ISR_Count>=240)
        {
            ISR_Count=0x00;
        }
        */

        //ADC Measure Compleated interupt handling
        if(ADC_Mesure_Compleated==0xFF)
        {
            ADC_Mesure_Compleated=0x00;
            adc_val_update();
            adc_measure_init(0); // Initialize measurement on ADC0 pin
        }


        //Handling of different voltage levels detected on ADC0 pin with automatic PWM controll
        PORTB = 0x00;
        if(ADC_Val>=586)
        {
           PORTB = 0x04;
           unsigned char tmp = ADC_Val - 585;
           if (tmp >= 5)
           {
               OCR4A = 17 - tmp;
           }
           else{OCR4A = 12;}
           //if (tmp >= 12){OCR4A = 23;}
        }
        else if(ADC_Val>=575&&ADC_Val<=585)
        {
            PORTB = 0x02;
            OCR4A = 17;
        }
        else if(ADC_Val<=574)
        {
            PORTB = 0x01;
            /*
           unsigned char tmp = 575 - ADC_Val;
           if (tmp >= 5)
           {
               OCR4A = 17 + tmp;
           }
           else{OCR4A = 22;}
           if (tmp >= 12){OCR4A = 23;}
            */
            OCR4A = 18;
        }
        // Ad something in the future

    }

    return 0;
}
