#include "driverlib.h"
#include <stdio.h>
#define MAIN_1_SECTOR_31 0x0003F000


int buffmem = MAIN_1_SECTOR_31;

volatile int sw1 = 1;
volatile int sw2 = 1;
volatile int i = 0;
volatile int j =0;
volatile float buff[30];
volatile int success = 1;
volatile float bitvalue = 0;
volatile float v_out=0;
volatile float temp=0;
float *to_data = MAIN_1_SECTOR_31;
char toputty[10];


//SMCLK operates at 3MHz/64 = 46875 Hz; then 1 sec equals 46875 cycles,
//so set period = 46875 cycles

volatile Timer_A_UpModeConfig upConfig_0=
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    46875,                                 //interrupt at 1 sec intervals
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR,
};

volatile Timer_A_CompareModeConfig compareU=
{
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_OUTPUTMODE_OUTBITVALUE,
    46875,                                //compare set to check when it's time to interrupt
};

//uART configuration
const eUSCI_UART_Config uartConfig=
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,
    19,
    8,
    85,
    EUSCI_A_UART_NO_PARITY,
    EUSCI_A_UART_LSB_FIRST,
    EUSCI_A_UART_ONE_STOP_BIT,
    EUSCI_A_UART_MODE,
	EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};


int main(void) {
	 WDT_A_holdTimer();
	 FPU_enableModule();
	 Interrupt_disableMaster();
	 compareU.compareValue = 20000;

	 //LED used for debugging
	 GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
	 //set input switches; on is 0 and off is 1
     //negative logic is utilized
     MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN1); //S1
     MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN4); //S2

     //set LED1 and LED 2 as outputs and initializes to off
     //LED1
 	 MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
 	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
 	 //LED2
 	 MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
 	 MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
 	 MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);

 	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
 	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

	 //ADC configured in single channel single conversion mode
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

	 //initializing UART module
	 MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
	 MAP_UART_enableModule(EUSCI_A0_BASE);
	 //set inputs of UART pins
	 MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
	 MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

     while(1){
     //checks the state of switches 1 and 2
    	 sw1 = MAP_GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN1 );
    	 sw2 = MAP_GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN4 );
    	 if (sw1==0){
    		 unsigned int dcoFrequency = 3E+6;
    		 CS_setDCOFrequency(dcoFrequency);
    		 CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_64);
    		 //Data receive mode
    		 Interrupt_enableMaster();
    		 Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    		 //step 1 enable write by unlocking sector
    		 success = FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,FLASH_SECTOR31);
    		 if (success==0){
    			 while(1){}
    		 }
    		 success = FlashCtl_eraseSector(MAIN_1_SECTOR_31);
    		 if (success==0){
     			 while(1){}
     		 }
    		 //step 2 erase entire sector
    		 while(i<30){

    		 }
    		 //step 3 program sector
    		 success = FlashCtl_programMemory(buff,MAIN_1_SECTOR_31, 120);
    		 if (success==0){
    			 while(1){}
       		 }
    		 //step 4 disable further writes by locking sector
    		 success = FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,FLASH_SECTOR31);
    		 if (success==0){
     			 while(1){}
     		 }
			 Timer_A_clearTimer(TIMER_A0_BASE);
			 Timer_A_stopTimer(TIMER_A0_BASE);
			 Interrupt_disableMaster();
			 i=0;
			 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
			 while(1){} //infinite while loop
    	 } else if (sw2==0){
    		 //Data transmit mode
    		 unsigned int dcoFrequency = 3E+6;
   		     MAP_CS_setDCOFrequency(dcoFrequency);
   		     MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

   		     for (i;i<30;i++){
   		    	 sprintf(toputty,"%.1f\n\r", *(to_data+i));

   		    	 for (j;j<7;j++){
				 UART_transmitData(EUSCI_A0_BASE,toputty[j]);
   		    	 }
   		    	 j=0;
   		     }
   		     i = 0;
   		     MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
   		     while(sw2==0){
   		    	sw2 = MAP_GPIO_getInputPinValue( GPIO_PORT_P1, GPIO_PIN4 );
   		     }

   		     while(1){} //infinite while loop
    	 }

     }
}

void TA0_0_IRQHandler()
{
	Timer_A_stopTimer(TIMER_A0_BASE);
	ADC14_toggleConversionTrigger();
	while(ADC14_isBusy()){
	}
	bitvalue = ADC14_getResult(ADC_MEM0);

	//conversion equations
	//voltage = bitvalue*conversion rate
	//conversion rate 2.5V/1024bits = .00244 V/bit
	v_out = bitvalue*.00244;
	v_out = v_out + .28;
	temp = v_out/.0166;
	buff[i] = temp;
	i++;
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
	//LED used for debugging, toggles on and off every second
	GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
	Timer_A_clearTimer(TIMER_A0_BASE);
	Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
	printf("%.1f %d\n",temp,i);
}



