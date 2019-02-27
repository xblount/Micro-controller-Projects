#include "driverlib.h"
#include "math.h"


volatile Timer_A_PWMConfig pwmConfig0=
{
		TIMER_A_CLOCKSOURCE_SMCLK,
		TIMER_A_CLOCKSOURCE_DIVIDER_64,
		46875,
		TIMER_A_CAPTURECOMPARE_REGISTER_1,
		TIMER_A_OUTPUTMODE_RESET_SET,
		23430
};

volatile Timer_A_PWMConfig pwmConfig2=
{
		TIMER_A_CLOCKSOURCE_SMCLK,
		TIMER_A_CLOCKSOURCE_DIVIDER_64,
		46875,
		TIMER_A_CAPTURECOMPARE_REGISTER_1,
		TIMER_A_OUTPUTMODE_RESET_SET,
		23430
};

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


int sw1=1;
int sw2=1;
int a=0;
int pin1=0;
int pin2=0;
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
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN4);

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN4,GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5,GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN4);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN4,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN1,GPIO_HIGH_TO_LOW_TRANSITION);

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN7);




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

    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    Interrupt_enableInterrupt(INT_PORT1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN4);
    MAP_Interrupt_setPriority(INT_TA1_0,1);
    MAP_Interrupt_setPriority(INT_PORT1,0);
    Interrupt_enableMaster();

    while(1){
    	if(on1==1){
    		ADC14_toggleConversionTrigger();
    		while(ADC14_isBusy()){
    		}
    		bitvalue = ADC14_getResult(ADC_MEM0);
    		pwm = (bitvalue/760)*46875;
    		pwm = floor(pwm);
    		pwmConfig0.dutyCycle = pwm;

    	}
    	if (on2==1){
    		ADC14_toggleConversionTrigger();
    		while(ADC14_isBusy()){
    		}
    		bitvalue = ADC14_getResult(ADC_MEM0);
    		pwm = (bitvalue/760)*46875;
    		pwm = floor(pwm);
    		pwmConfig2.dutyCycle = pwm;
    	}
    	if (on1==2){
    		pwmConfig0.dutyCycle = 0;
			on1=0;
    	}
    	if (on2==2){
    		pwmConfig2.dutyCycle = 0;
			on2=0;
    	}
    	if (on1==0){
    		pwmConfig0.dutyCycle = 0;
    	}
    	if (on2==0){
    		pwmConfig2.dutyCycle = 0;
    	}
    	//Timer_A_generatePWM();
		//Timer_A_stopTimer();
}
}

void PORT1_IRQHandler(){
	Timer_A_clearTimer(TIMER_A1_BASE);
	sw1 = GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN1);
	sw2 = GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN4);
	if (sw1==0){
		on1++;
		if (on1==1){
			Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig0);
		} else if (on1==2){
			pwmConfig0.dutyCycle = 0;
			Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig0);
			on1=0;
		}
	} else if(sw2==0){
		on2++;
		if (on2==1){
			Timer_A_generatePWM(TIMER_A2_BASE,&pwmConfig2);
		} else if (on2==2){
			pwmConfig2.dutyCycle = 0;
			Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig2);
			on2=0;
		}
	}
	pin1=GPIO_getInterruptStatus(GPIO_PORT_P1,GPIO_PIN1);
	pin2=GPIO_getInterruptStatus(GPIO_PORT_P1,GPIO_PIN4);
	GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1);
	GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN4);
}

void TA1_0_IRQHandler()
{
	a++;
	if (a==3){
		if (on1==1){
			a=0;
			Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig0);
		} else if (on2==1){
			a=0;
			Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig2);
		}
		else {
			pwmConfig0.dutyCycle = 0;
			Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig0);
			pwmConfig2.dutyCycle = 0;
			Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig2);
			a=0;
		}
	}

	Timer_A_clearTimer(TIMER_A1_BASE);
	Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

