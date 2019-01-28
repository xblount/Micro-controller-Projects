#include "driverlib.h"
#include "math.h"

int s1=1;
int s2=1;
int a=0;
int on1=0;
int on2=0;
volatile float bitvalue=0;
volatile float pwm=0;


int main(void) 
{
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    unsigned int dcoFrequency = 3E+6;
    MAP_CS_setDCOFrequency(dcoFrequency);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_64);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN4);

    //pin 2.4 as pwm
        P2SEL0 |= 0x10;
        P2SEL1 &= ~0x10;
        P2DIR |= 0x10;

        TA0CCR0 = 4680;
        TA0CCR1 = 0;
        TA0CCTL1 = OUTMOD_7;
        TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;

    ///pin 5.6 as pwm
        P5SEL0 |= 0x40;
        P5SEL1 &= ~0x40;
        P5DIR |= 0x40;

        TA2CCR0 = 4680;
        TA2CCR1 = 0;
        TA2CCTL1 = OUTMOD_7;
        TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;

    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN4);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN4,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN1,GPIO_HIGH_TO_LOW_TRANSITION);

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

    Interrupt_enableInterrupt(INT_PORT1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN4);
    MAP_Interrupt_setPriority(INT_PORT1,0);
    Interrupt_enableMaster();

    while(1){
    	if(on1==1){
    		ADC14_toggleConversionTrigger();
    		while(ADC14_isBusy()){
    		}
    		bitvalue = ADC14_getResult(ADC_MEM0);
    		pwm = (bitvalue/850)*4680;
    		pwm = floor(pwm);
    		TA0CCR1 = pwm;

    	}
    	if (on2==1){
    		ADC14_toggleConversionTrigger();
    		while(ADC14_isBusy()){
    		}
    		bitvalue = ADC14_getResult(ADC_MEM0);
    		pwm = (bitvalue/850)*4680;
    		pwm = floor(pwm);
    		TA2CCR1 = pwm;
    	}
}
}

void PORT1_IRQHandler(){
	Timer_A_clearTimer(TIMER_A1_BASE);
	s1 = GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN1);
	s2 = GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN4);
	if (s1==0){
		on1++;
		if (on1==1){
			TA0CCR1 = 0;
		} else if (on1==2){
			TA0CCR1 = 0;
			on1=0;
		}
	}
	if(s2==0){
		on2++;
		if (on2==1){
			TA2CCR1 = 0;
		} else if (on2==2){
			TA2CCR1 = 0;
			on2=0;
		}
	}
	GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1);
	GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN4);
}

