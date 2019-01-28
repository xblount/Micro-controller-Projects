#include "driverlib.h"
#include <stdio.h>

const Timer_A_UpModeConfig upConfig_0=
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    4687,                                 //interrupt at 1 sec intervals
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR,
};

const Timer_A_CompareModeConfig compareU=
{
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_OUTPUTMODE_OUTBITVALUE,
    4687,                                //compare set to check when it's time to interrupt 46875 full second
};

volatile int set_temp=100;
volatile float bitvalue=0;
volatile float temp=0;
float step=0.1; //time step
//PID gains
float kp=1.9581;
float ki=.025;
float kd=11.0933;
volatile float error=0;
volatile float i_error=0;
volatile float d_error=0;
volatile float last_error=0;
volatile float gain=0;
volatile int counter=0;
volatile float DC=1; //Duty Cycle
volatile int on=0;

int main(void)
{
    WDT_A_holdTimer();

    Interrupt_disableMaster();

    unsigned int dcoFrequency = 3E+6;
    MAP_CS_setDCOFrequency(dcoFrequency);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_64);

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

    	if (DC>46875){
    		DC=46875;
    		TA3CCR1=46875;
    	} else if (DC<0){
    		DC=1;
    		TA3CCR1=1;
    	}

    }
}

void TA0_0_IRQHandler(){
	counter++;
	ADC14_toggleConversionTrigger();
//temperature conversion
	while(ADC14_isBusy()){
	}
	bitvalue = ADC14_getResult(ADC_MEM0);
	temp = .00244*bitvalue;
	temp = temp+.15;
	temp = temp/.01;
	//error calculations
	error = set_temp-temp;
	//reset integral error
	if (on==140){
	i_error= 0;
	}
	i_error= i_error+step*(set_temp-temp);
	d_error=(error-last_error)/step;
	last_error=error;
	gain= (kp*error)+(ki*i_error)+(kd*d_error);
	DC=DC+DC*gain;
	TA3CCR1 = DC; //adjust DC
	if (counter==10){
		counter=0;
		on++;
		printf("%.1f\n\r",temp);
	}
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_clearTimer(TIMER_A0_BASE);
}
