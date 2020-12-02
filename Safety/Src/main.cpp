#include "main.hpp"

#include "Debug.hpp"
#include "GPIO.hpp"
#include "Clock.hpp"
#include "UART.hpp"
#include "PWM.hpp"
#include "PPM.hpp"
#include "Watchdog.hpp"
#include "Profiler.h"

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_iwdg.h"
#include "safety_controller.hpp"

#include "system_config.hpp"


char buffer[200]; //buffer for printing
StatusCode setupPWM(PWMManager &manager);
StatusCode setupPPM(PPMChannel &ppm);
void print_ppm_state(char *buffer, PPMChannel &ppm);

IWDG_HandleTypeDef hiwdg2; //HAL Watchdog Decleration
UART_HandleTypeDef huart1;


void callMXfunctions();

int main() {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	//HAL_IWDG_Init(&hiwdg2); //Start the watchdog
	callMXfunctions();
	
	
	StatusCode status;

	PWMManager &manager = PWMManager::getInstance();
	status = setupPWM(manager);
	info("PWMSetup", status);


	PPMChannel ppm;
	status = setupPPM(ppm);

	info("PPM Setup", status);
	
	/*
	GPIOPin led1 = GPIOPin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_OUTPUT, GPIO_STATE_LOW, GPIO_RES_NONE);
	GPIOPin led2 = GPIOPin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_OUTPUT, GPIO_STATE_LOW, GPIO_RES_NONE);
	GPIOPin led3 = GPIOPin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_OUTPUT, GPIO_STATE_LOW, GPIO_RES_NONE);
	GPIOPin buzzer = GPIOPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_OUTPUT, GPIO_STATE_LOW, GPIO_RES_NONE);
	led1.set_state(GPIO_STATE_HIGH);
	led2.set_state(GPIO_STATE_HIGH);
	*/
	safety_controller_init();

	while (true) 
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_11);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_12);
		//HAL_Delay(50);
		//uint8_t Test[] = "Hello World !!!\r\n"; //Data to send
	  	//HAL_UART_Transmit(&huart1,Test,sizeof(Test),10);
		/*
		led1.set_state(GPIO_STATE_HIGH);
		led2.set_state(GPIO_STATE_HIGH);
		*/
		//safety_run(ppm);
		for(int i = 0; i < 100; i++)
		{
			int ppmCh3 = ppm.get(3);
			manager.channel(5).set(ppmCh3);
			manager.channel(6).set(i);
			HAL_Delay(100);
		}

	}
}

void print_ppm_state(char *buffer, PPMChannel &ppm) {
	int len = sprintf(buffer,
					  "CH1 (p, us): %d %lu\r\nCH2 (p, us): %d %lu\r\n"
					  "CH3 (p, us): %d %lu\r\nCH4 (p, us): %d %lu\r\n"
					  "CH5 (p, us): %d %lu\r\nCH6 (p, us): %d %lu\r\n"
					  "CH7 (p, us): %d %lu\r\nCH8 (p, us): %d %lu\r\n",
					  ppm.get(1), ppm.get_us(1),
					  ppm.get(2), ppm.get_us(2),
					  ppm.get(3), ppm.get_us(3),
					  ppm.get(4), ppm.get_us(4),
					  ppm.get(5), ppm.get_us(5),
					  ppm.get(6), ppm.get_us(6),
					  ppm.get(7), ppm.get_us(7),
					  ppm.get(8), ppm.get_us(8));

	sprintf(&buffer[len], "PPM Disconnected? : %d\r\n", ppm.is_disconnected(get_system_time()));
}


StatusCode setupPWM(PWMManager &manager)
{
	StatusCode status = manager.setup();
	manager.channel(1).set(50);
	manager.channel(2).set(25);
	manager.channel(3).set(75);
	manager.channel(4).set(100);
	manager.channel(5).set(25);
	manager.channel(6).set(75);
	manager.channel(7).set(100);
	manager.channel(8).set(25);
	manager.channel(9).set(50);
	manager.channel(10).set(75);
	manager.channel(11).set(100);
	manager.channel(12).set(50);

	return status;
}

StatusCode setupPPM(PPMChannel &ppm)
{
	ppm.setNumChannels(8);
	ppm.setLimits(1, 1000, 2000, 50);
	StatusCode status = ppm.setup();
	ppm.setTimeout(200);
	return status;
}

void callMXfunctions()
{
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_USART1_UART_Init();
	/*
  	MX_I2C1_Init();
  	MX_IWDG_Init();
  	MX_SPI1_Init();
  	MX_TIM1_Init();
  	MX_TIM3_Init();
  	MX_TIM14_Init();
  	MX_TIM15_Init();
  	MX_TIM16_Init();
  	MX_TIM17_Init();
  	MX_USART1_UART_Init();
  	
	  */
}
