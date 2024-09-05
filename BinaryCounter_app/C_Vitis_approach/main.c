#include "xparameters.h" // info about hardware

#include "xgpio.h" //gpio basic funcionality
#include "xscutimer.h" //timers
#include "xttcps.h"

#include "xil_printf.h" 
#include "xil_types.h" // u16 types etc..
#include "sleep.h"
#include <stdio.h>
#include <stdlib.h>

#include "xscugic.h" //interrupts
#include "xil_exception.h" //exceptions

#define BTN_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define LED_ID			XPAR_AXI_GPIO_0_DEVICE_ID

#define BTN_CHANNEL 	1
#define LED_CHANNEL 	2

#define TIMER_LOAD_VALUE 0xFFFF

XGpio LEDInst, BTNInst;
XGpio_Config *cfg_ptr;
XTtcPs Ttc_Timer,Ttc_Timer1;
XTtcPs_Config *Ttc_ConfigPtr;
XScuGic IntcInstance; 

u8 button_flag = 0;
u8 Ttc_prescaler;
u16 Ttc_interval;
u32 Ttc_freqHz = 50;
u32 data;

int btn_value;
int pressCounter = 1;
int counter = 0;
volatile int global_flag = 0;

volatile int btn_value_global = 0;

void *CallBackRef_mine;

int Timer_Intr_Setup(XScuGic *IntcInstancePtr, XTtcPs *TimerInstancePtr, u16 TimerIntrId);
int Timer1_Intr_Setup(XScuGic *IntcInstancePtr, XTtcPs *TimerInstancePtr, u16 TimerIntrId);
void Timer_ISR(void *CallBackRef);
void Timer1_ISR(void *CallBackRef);

static int my_IntcInitFunction(XScuGic *IntcInstancePtr, XGpio* BTNInst, u16 BtnIntrId);
void Button_ISR(void *CallBackRef);

int main() {

	int status;
	int DutyCycle = 50;

	static u16 Ttc_options;
	static u16 Ttc_matchval;

	enum StateMachine {s0, s1, s2, s3, s4, s5};
	static enum StateMachine state = s1;

	// Initialize LEDs
	status = XGpio_Initialize(&LEDInst, LED_ID);
	if (status != XST_SUCCESS)
		return XST_FAILURE;
	// End of LED Initialization

	// Initialize Push Buttons
	status = XGpio_Initialize(&BTNInst, BTN_ID);
	if (status != XST_SUCCESS)
		return XST_FAILURE;
	// End of PushButtons Initialization

	//Initialize TTC timer 0
	Ttc_ConfigPtr = XTtcPs_LookupConfig(XPAR_PS7_TTC_0_DEVICE_ID);
	if(Ttc_ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	u32 base = Ttc_ConfigPtr->BaseAddress;
	status = XTtcPs_CfgInitialize(&Ttc_Timer, Ttc_ConfigPtr, Ttc_ConfigPtr->BaseAddress);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Ttc_options |= (XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_MATCH_MODE | XTTCPS_OPTION_WAVE_POLARITY);

	XTtcPs_SetOptions(&Ttc_Timer, Ttc_options);

	XTtcPs_CalcIntervalFromFreq(&Ttc_Timer, Ttc_freqHz, &Ttc_interval, &Ttc_prescaler);
	XTtcPs_SetInterval(&Ttc_Timer, Ttc_interval);
	XTtcPs_SetPrescaler(&Ttc_Timer, Ttc_prescaler);

	Ttc_matchval = (Ttc_interval * DutyCycle) / 100;
	XTtcPs_SetMatchValue(&Ttc_Timer, 0, Ttc_matchval);

	status = Timer_Intr_Setup(&IntcInstance, &Ttc_Timer, XPS_TTC0_0_INT_ID);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//End of TTC timer 0 Initialization

	//Initialize TTC timer 1

	Ttc_ConfigPtr = XTtcPs_LookupConfig(XPAR_XTTCPS_1_DEVICE_ID);
	if(Ttc_ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	status = XTtcPs_CfgInitialize(&Ttc_Timer1, Ttc_ConfigPtr, Ttc_ConfigPtr->BaseAddress); // Base Address, visible in xparameters as baseaddress
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Ttc_options = 0;
	Ttc_options |= (XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_POLARITY);

	XTtcPs_SetOptions(&Ttc_Timer1, Ttc_options);
	XTtcPs_SetInterval(&Ttc_Timer1, 450); //interval = 27000 for 16ms period
	XTtcPs_SetPrescaler(&Ttc_Timer1, 9); //prescaler 2^(7+1) = 256

	status = Timer1_Intr_Setup(&IntcInstance, &Ttc_Timer1, XPS_TTC0_1_INT_ID);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//end of TTC timer 1 initialization

	// Set LEDs direction to outputs
	XGpio_SetDataDirection(&LEDInst, LED_CHANNEL, 0x00);

	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&BTNInst, BTN_CHANNEL, 0xFF);

	//Timer's start
	XTtcPs_Start(&Ttc_Timer);
	XTtcPs_Start(&Ttc_Timer1);


	while (1) {

		if(button_flag) {


			if(btn_value == 4) { // right button
				xil_printf("przycisk");
				switch (state) {
					case s0:

						//for 50 hz

						Ttc_freqHz = 50;
						XTtcPs_CalcIntervalFromFreq(&Ttc_Timer, Ttc_freqHz, &Ttc_interval, &Ttc_prescaler);
						XTtcPs_SetInterval(&Ttc_Timer, Ttc_interval);
						XTtcPs_SetPrescaler(&Ttc_Timer, Ttc_prescaler);

						state = s1;

					break;
					case s1:

						//25

						Ttc_freqHz = 25;
						XTtcPs_CalcIntervalFromFreq(&Ttc_Timer, Ttc_freqHz, &Ttc_interval, &Ttc_prescaler);
						XTtcPs_SetInterval(&Ttc_Timer, Ttc_interval);
						XTtcPs_SetPrescaler(&Ttc_Timer, Ttc_prescaler);

						state = s2;


					break;
					case s2:

						//12.5

						XTtcPs_SetInterval(&Ttc_Timer, 34722); //interval = 34 722
						XTtcPs_SetPrescaler(&Ttc_Timer, 7); //prescaler 2^7+1 = 256

						state = s3;


					break;
					case s3:

						//6.25

						XTtcPs_SetInterval(&Ttc_Timer, 34722); //interval = 34 722
						XTtcPs_SetPrescaler(&Ttc_Timer, 8); //prescaler 2^8+1 = 512

						state = s4;


					break;
					case s4:
						//dla 3.125

						XTtcPs_SetInterval(&Ttc_Timer, 34722); //interval = 34 722
						XTtcPs_SetPrescaler(&Ttc_Timer, 9); //prescaler 2^9+1 = 1024

						state = s5;


					break;
					case s5:
						//1.56

						XTtcPs_SetInterval(&Ttc_Timer, 34778); //interval = 34 722
						XTtcPs_SetPrescaler(&Ttc_Timer, 10); //prescaler 2^10+1 = 2048

						state = s0;

					break;
					default:

					break;
				}

				counter++;
				counter &= 31; // curb for values larger than 0b011111

				XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, counter); //actualisation of binary counter

			} else if(btn_value == 2){ //Reset if lower button is pressed

				counter = 0;
				XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, counter);
				Ttc_freqHz = 50;
				XTtcPs_CalcIntervalFromFreq(&Ttc_Timer, Ttc_freqHz, &Ttc_interval, &Ttc_prescaler);
				XTtcPs_SetInterval(&Ttc_Timer, Ttc_interval);
				XTtcPs_SetPrescaler(&Ttc_Timer, Ttc_prescaler);
				state = s1;
			}

			button_flag = 0;


		}
		if((XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL) == 0) && !XTtcPs_IsStarted(&Ttc_Timer1)) {
			XGpio_InterruptEnable(&BTNInst, 1);

		}


	}

	return 0;
}


int Timer_Intr_Setup(XScuGic *IntcInstancePtr, XTtcPs *TimerInstancePtr, u16 TimerIntrId) {

	XScuGic_Config *IntcConfig;
	int status;

	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if(NULL == IntcConfig) {
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//step 1: Interrupt set up
	Xil_ExceptionInit(); 

	//step 2: Interrupt set up
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, IntcInstancePtr);

	//step 3:
	status = XScuGic_Connect(IntcInstancePtr, TimerIntrId, (Xil_ExceptionHandler)Timer_ISR, (void *)TimerInstancePtr);

	//step 4:
	XScuGic_Enable(IntcInstancePtr, TimerIntrId);

	//step 5:
	XTtcPs_EnableInterrupts(TimerInstancePtr,XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_EnableInterrupts(TimerInstancePtr,XTTCPS_IXR_MATCH_0_MASK);

	//step 6:
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int Timer1_Intr_Setup(XScuGic *IntcInstancePtr, XTtcPs *TimerInstancePtr, u16 TimerIntrId) {

	XScuGic_Config *IntcConfig;
	int status;

	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if(NULL == IntcConfig) {
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//step 1: Interrupt set up
	Xil_ExceptionInit(); 

	//step 2: Interrupt set up
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, IntcInstancePtr);

	//step 3:
	status = XScuGic_Connect(IntcInstancePtr, TimerIntrId, (Xil_ExceptionHandler)Timer1_ISR, (void *)TimerInstancePtr);

	//step 4:
	XScuGic_Enable(IntcInstancePtr, TimerIntrId);

	//step 5:
	XTtcPs_EnableInterrupts(TimerInstancePtr,XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_EnableInterrupts(TimerInstancePtr,XTTCPS_IXR_MATCH_0_MASK);

	//step 6:
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

static int my_IntcInitFunction(XScuGic *IntcInstancePtr, XGpio* BTNInst, u16 BtnIntrId){
	XScuGic_Config *IntcConfig;
	int status;

	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if(NULL == IntcConfig) {
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//step 1: Interrupt set up
	Xil_ExceptionInit(); 

	//step 2: Interrupt set up
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, IntcInstancePtr);

	//step 3:
	status = XScuGic_Connect(IntcInstancePtr, BtnIntrId, (Xil_ExceptionHandler)Button_ISR, (void *)BTNInst);
	//XPAR_FABRIC_AXI_GPIO_1_IP2INTC_IRPT_INTR;
	//step 4:
	XScuGic_Enable(IntcInstancePtr, BtnIntrId);


	//step 5:
	XGpio_InterruptEnable(BTNInst, 1);
	XGpio_InterruptGlobalEnable(BTNInst);

	//step 6: 	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

void Timer_ISR(void *CallBackRef) {
	u32 Intr_status;

	Intr_status = XTtcPs_GetInterruptStatus((XTtcPs*) CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs*) CallBackRef, Intr_status);


	if(0 != (XTTCPS_IXR_INTERVAL_MASK & Intr_status)) {
		data = counter | 0b10000000;

		XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, data);
	}
	if(0 != (XTTCPS_IXR_MATCH_0_MASK & Intr_status)) {
		data = counter | 0b00000000;

		XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, data);
	}


}
void Timer1_ISR(void *CallBackRef) {

	u32 Intr_status;
	u16 btn_value_global_previous;
	static u16 rawState = 0;

	Intr_status = XTtcPs_GetInterruptStatus((XTtcPs*) CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs*) CallBackRef, Intr_status);

	btn_value = XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL);
	rawState = (rawState<<1) | !btn_value | 0xe000;

	if(rawState == 0xf000) {
		button_flag = 1;
	} else {
		button_flag = 0;
	}

}

void Button_ISR(void *CallBackRef) {

	XGpio_InterruptDisable(&BTNInst, BTN_CHANNEL);
	XGpio_InterruptClear(CallBackRef, XGPIO_IR_CH1_MASK);

	btn_value_global = XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL);

	if(!XTtcPs_IsStarted(&Ttc_Timer1)) { 

		XTtcPs_Start(&Ttc_Timer1);

	} else {

		return;
	}


}
