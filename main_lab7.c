#include "driverlib.h"
#include <stdio.h>


int i = 0;
int a = 0;
int b = 0;
volatile int j = 0;
volatile int k = 0;
volatile unsigned int delay = 0;
int position = 0;
volatile unsigned int move=0;
char prompt1[];
int p=0;
char prompt2[];
volatile float buffer[20];
volatile char rdata;
volatile int start = 0;
int phase = 0;
volatile float countdown = 0;
volatile float angle = 0;
volatile char cd[];
volatile int end = 0;
volatile unsigned short a1=0;
volatile unsigned short second = 0;
volatile int ticks=0;
volatile float clicks =0;
volatile float l=0;
volatile int rpm = 1000;


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

const Timer_A_UpModeConfig upConfig_0=
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_64,
    460,                                 //interrupt at 1 sec intervals
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR,
};

const Timer_A_CompareModeConfig compareU=
{
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_OUTPUTMODE_OUTBITVALUE,
    460,                                //compare set to check when it's time to interrupt 46875 full second
};
int main(void) 
{
    WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN5);
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN6);
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN7);

    //set starting position
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);

    unsigned int dcoFrequency = 3E+6;
    MAP_CS_setDCOFrequency(dcoFrequency);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //initializing UART module and interrupts
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    //set inputs of UART pins
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION); //receive
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION); //transfer
    //interrupt for receive
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    //clear flags
    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_setPriority(INT_EUSCIA0,0);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);

    //enabling Timer A
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    //Interrupt_enableInterrupt(INT_TA0_0);
    //Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    MAP_Interrupt_setPriority(INT_TA0_0,1);
    Interrupt_enableMaster();

    while(1){
    switch (i){
    	//Prompt for number between 0-60 seconds multiple of 5
    case 0:
sprintf(prompt1,"\n\rEnter a number between 0-60 multiple of 5\n\r");
    	for (p;p<46;p++){
    	UART_transmitData(EUSCI_A0_BASE,prompt1[p]);
    	}
    	p = 0;
    	while(!(position)){

    	}
    		position = 0;
    	i++;
    	//countdown = 10*buffer[0]+buffer[1];
    	a = buffer[0]-'0';
    	b = buffer[1]-'0';
    	countdown = (10*a)+b;
    	angle = 6*countdown;
    	clicks = angle/15;
    	for(j;j<4;j++){
    		UART_transmitData(EUSCI_A0_BASE,buffer[j]);
    		buffer[j] = '\0'; //clears values in character buffer
    	}
    	j=0;
    	for (delay;delay<100000;delay++){
    	}
    	delay = 0;
    	break;
    case 1:
    	//rotate to desired position
    	for (l;l<clicks;){

    		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
    		GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
    		for (a;a<rpm;a++){
   	    		}
    		l++;
    		a1=3;
			if (l>=clicks){
				break;
			}
    		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
    		GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
    		for (i;i<rpm;i++){
    			}
    		l++;
    		a1=2;
    		if (l>=clicks){
    			break;
    		}
    		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
    		GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
    		for (j;j<rpm;j++){
   	    		}
    		l++;
    		a1=1;
    		if (l>=clicks){
    			break;
    		}
    		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    		GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
    		for (k;k<rpm;k++){
    			}
    		l++;
    		a1=0;
    		if (l>=clicks){
    			break;
    		}
    		a=0;
    		i=0;
    		j=0;
    		k=0;
    	}
    	phase = 1;
    	sprintf(prompt2,"\n\rstart\n\r");
    	for (p;p<9;p++){
    		UART_transmitData(EUSCI_A0_BASE,prompt2[p]);
    	}
    	p=0;
    	while(!(start)){
    	}
    	start=0;
    	Interrupt_enableInterrupt(INT_TA0_0);
    	Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    	while(!(end)){
    	}
    	GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
   	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
   	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
   	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    	Timer_A_stopTimer(TIMER_A0_BASE);
    	Interrupt_disableInterrupt(INT_TA0_0);
    	i=0;
    	phase = 0;
    	k=0;
    	end = 0;
    	l=0;
    	a=0;
    	j=0;
    	a1=0;
    	clicks=0;
    	angle=0;
    	countdown=0;
    	ticks = 0;
    	second = 0;
    	break;
    }
    }


    //Rotate to necessary degrees counter-clockwise
    //Prompt "Start" *wait for enter to be pressed
    //countdown based on input number --> restart
}

void EUSCIA0_IRQHandler(void){
	//used for debugging
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);


	switch (phase){
	case 0:
	rdata = UART_receiveData(EUSCI_A0_BASE);
	if (rdata==0x0000000D){ //checks for carriage return
		position = 1;
		buffer[j] = '\n'; //appends new line
		j++;
		buffer[j] = '\r'; //appends carriage return
		j=0;
	} else { //adds data received to character array
		buffer[j]=rdata;
		j++;  //increment counter
	}
	break;
	case 1:
	rdata = UART_receiveData(EUSCI_A0_BASE);
	if (rdata ==0x0000000D){
		phase = 0;
		start = 1;
	}
	break;



}
	MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT); //clears interrupt flags
}
void TA0_0_IRQHandler()
{
	k=0;
	second++;
	ticks++;

	if (second==100){
		if (countdown==0){
			end = 1;
			second = 0;
		} else {
			sprintf(cd,"%.1f\n\r",countdown);
			GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);
			countdown = countdown-1;
			for (k;k<6;k++){
				UART_transmitData(EUSCI_A0_BASE,cd[k]);
			}
			second=0;
		}
		}
	if (ticks==250){
		ticks = 0;
		switch (a1){
		case 0:
			GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
		    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
			a1++;
			break;
		case 1:
			GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
		    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
			a1++;
			break;
		case 2:
			GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
		    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
			a1++;
			break;
		case 3:
			GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
		    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
			a1 = 0;
	}
	}
		else{
		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
	    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
	}
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_clearTimer(TIMER_A0_BASE);

	//LED used for debugging, toggles on and off every second
}
