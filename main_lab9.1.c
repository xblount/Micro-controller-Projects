#include "driverlib.h"
#include <stdio.h>

const Timer_A_UpModeConfig upConfig_0=
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    46875,                                 //interrupt at 1 sec intervals
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR,
};

const Timer_A_CompareModeConfig compareU=
{
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_OUTPUTMODE_OUTBITVALUE,
    46875,                                //compare set to check when it's time to interrupt 46875 full second
};

unsigned int i=0;
unsigned int j=0;
unsigned int k=0;
unsigned int a=0;
unsigned int b=0;
unsigned int c=0;
unsigned int d=0;
float twomin=0;
float ambient=0;
float ambientfinal;
float temp=0;
float prev_temp[2];
float bitvalue=0;
volatile float buffer[1000];
int step=0;
unsigned int steady=23;
unsigned int sec=0;
unsigned int stop=0;

int main(void) 
{
    WDT_A_holdTimer();

    Interrupt_disableMaster();

    unsigned int dcoFrequency = 3E+6;
    MAP_CS_setDCOFrequency(dcoFrequency);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_64);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //ADC configured in single channel single conversion mode pin 5.5
    ADC14_enableModule();
    //Step 1 set ADC peripheral pin function
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);
    //Step 2 set resolution of conversion
    ADC14_setResolution(ADC_10BIT);
    //Step 3 initialize ADC module
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK,ADC_PREDIVIDER_1,ADC_DIVIDER_1,0);
    //Step 4 configure mode, repeat=true
    ADC14_configureSingleSampleMode(ADC_MEM0, 1);
    //Step 5 allocate memory for conversion value
    ADC14_configureConversionMemory(ADC_MEM0,ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A0, 0);
    //sample timer manually determined when to take samples
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();

    //enabling Timer A
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    Interrupt_enableMaster();
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //port 10.5
               P10SEL0 |= 0x20;
               P10SEL1 &= ~0x20;
               P10DIR |= 0x20;

               TA3CCR0 = 4687;
               TA3CCR1 = 0;
               TA3CCTL1 = OUTMOD_7;
               TA3CTL = TASSEL__SMCLK | MC__UP | TACLR;
    while(1){
    	switch (step){
    	case 0:
    		if (sec==1){
    			//conversion 250F/1024bits=.24414 F/bit
    			temp = .00244*bitvalue;
    			temp = temp+.15;
    			temp = temp/.01;
    			ambient = (ambient + (temp));
    			sec=0;
    			k++;
    			printf("%.1f\n\r",temp);
    		}
    		break;
    	case 1:
    		if (sec==1){
    			temp = .00244*bitvalue;
    			temp = temp+.15;
    			temp = temp/.01;
    			buffer[i]=temp;
    			i++;
    			j++;
    			sec=0;
    			printf("%.1f\n\r",temp);
    			if (j<450){
    				steady = steady;
    			} else{
    				a= buffer[i-2];
    				b=buffer[i-100];
    				steady = (a-b);
    			}
    		}
    		if (steady<0.5){
    			j=0;
    			printf("%.1f ambient final\n\r",ambientfinal);
    			step=0;
    			twomin=0;
    			while(1){};//infinite while
    		}
    		break;
    	}
    }
}

void TA0_0_IRQHandler(){
	sec=1;
	twomin++;
	GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);


	ADC14_toggleConversionTrigger();
	while(ADC14_isBusy()){
	}
	bitvalue = ADC14_getResult(ADC_MEM0);
	if (twomin==120){
		step = 1;
		ambient = ambient/120;
		ambientfinal=ambient;
		TA2CCR1 = 23437; //50% duty cycle
	}
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_clearTimer(TIMER_A0_BASE);
}

