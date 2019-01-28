#include "driverlib.h"
#include <stdio.h>

volatile float value = 0;
volatile float bitvalue = 0;
volatile float temp = 0;
volatile int i = 0;
volatile int buff[200];

//SMCLK operates at 3MHz/64 = 46875 Hz; then 1 sec equals 46875 cycles,
//so set period = 46875 cycles
const Timer_A_UpModeConfig upConfig_0=
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_64,
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
    46875,                                //compare set to check when it's time to interrupt
};




int main(void) 
{
    WDT_A_holdTimer();
    //setting and tying clock source and signal
    unsigned int dcoFrequency = 3E+6;
    CS_setDCOFrequency(dcoFrequency);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    Interrupt_disableMaster();

    //LED used for debugging
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);

    //ADC should be configured in single channel single conversion mode
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

    //after conversion complete print results to console display

    //enabling Timer A
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Interrupt_enableMaster();
    while(1){

    }
}

void TA0_0_IRQHandler()
{
	ADC14_toggleConversionTrigger();
	while(ADC14_isBusy()){
	}
	bitvalue = ADC14_getResult(ADC_MEM0);
	buff[i] = value;
	i++;
	Timer_A_clearTimer(TIMER_A0_BASE);
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);

	//LED used for debugging, toggles on and off every second
	GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);

	//conversion rate 2.5V/1024bits = .00244 V/bit
	//value = bitvalue*conversion rate
	value = bitvalue*.00244;
	value = value + .28;
	temp = value/.0166;
	printf("%f %f\n",value,temp);
}

