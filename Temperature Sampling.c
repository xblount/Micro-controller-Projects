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
unsigned int samples=0;
int m=0;
volatile float sum=0;
volatile float avg=0;
volatile float a=0;
volatile float b=0;
unsigned int DC=40;
float twomin=0;
float ambient=0;
float ambientfinal;
float temp=0;
float prev_temp[2];
float bitvalue=0;
volatile float buffer[1000];
volatile float avgtemp[1000];
int step=0;
volatile float steady=23;
volatile float steadyf=23;
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

    ///pin 5.6 as pwm
    P5SEL0 |= 0x40;
    P5SEL1 &= ~0x40;
    P5DIR |= 0x40;

    TA2CCR0 = 46875;
    TA2CCR1 = 18750;
    TA2CCTL1 = OUTMOD_7;
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;

    while(1){
    		if (sec==1){
    			temp = .00244*bitvalue;
    			temp = temp+.15;
    			temp = temp/.01;
    			buffer[i]=temp;
    			avgtemp[k]=temp;
    			i++;
    			j++;
    			k++;
    			sec=0;
    			printf("%.1f\n\r",temp);
    			if (j<=300){
    				steady = steady;
    			} else{
    				a= buffer[i-2];
    				b=buffer[i-100];
    				steady = (a-b);
    			}
    		}
    		if (steady<0.2){
    			steadyf=0;
    			steady=23;
    			j=0;
    		}
    		if(DC==80){
    			while(1){};
    		}
}
}

void TA0_0_IRQHandler(){
	sec=1;
	GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);
	ADC14_toggleConversionTrigger();
	while(ADC14_isBusy()){
	}
	bitvalue = ADC14_getResult(ADC_MEM0);
	if (steadyf==0){
		m=k-99;
		samples=k-m;
		for(m;m<=k;m++){
			sum += avgtemp[m];
			avgtemp[m]=0;
		}
		avg = sum/samples;
		printf("DC average %.1f for DC%d\n\r",avg,DC);
		samples=0;
		avg=0;
		sum=0;
		k=0;
		m=0;
		DC = DC+10;
		TA2CCR1 += 4688; //10% duty cycle increment
		steadyf = 23;
	}
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_clearTimer(TIMER_A0_BASE);
}
