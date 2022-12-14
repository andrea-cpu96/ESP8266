/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <string.h>

#include "projectconf.h"
#include "systemfunctions.h"
#include "ports.h"

#include "uart.h"

#include "main.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

uartHandler_t huart1;
uartHandler_t huart2;

uint8_t esp01CurrentState = USER_SEND_TO_ESP01;
uint8_t esp01NextState = USER_SEND_TO_ESP01;

uint8_t c;
uint8_t CharacterReceived; // Flag to receive next character
uint8_t userCommand[100];
uint8_t userCommIndex = 0;

uint8_t esp01DataBuff[1000] = {0};

uint32_t endOfCommunicationTimeStamp;
uint32_t delayTimeStamp;


int main(void)
{

	huart1.uart_Istance = UART_1;
	huart2.uart_Istance = UART_2;


	// System configurations
	System_Config();

	// Uart configuration
	uart_Init_It(&huart1);
	uart_Init_It(&huart2);


	uart_Start_Tx_It(&huart2, (uint8_t *)"Enter an AT command:\r\n", 22);
	uart_Start_Rx_It(&huart2, &c, 1);


	while (1)
	{

		esp01CurrentState = esp01NextState;

		switch(esp01CurrentState)
		{

			default:
			case USER_SEND_TO_ESP01:

				// Loop here until the user enter a new command

				if(CharacterReceived)
				{

					CharacterReceived = 0; // Reset of the flag


					if(c == '\r')
					{

						delayTimeStamp = Get_SystemTimeMs();

						while(Compare_SystemTimeMs(delayTimeStamp) < 100);

						uart_Start_Tx_It(&huart2, (uint8_t *)"\r\n", 2);

						userCommand[userCommIndex] = '\n';

						// Prepare for receiving data from esp01
						uart_Start_Rx_It(&huart1, esp01DataBuff, 1000);

						// Little delay
						delayTimeStamp = Get_SystemTimeMs();
						while(Compare_SystemTimeMs(delayTimeStamp) < 100);

						// Send to ESP01 the command the user entered
						uart_Start_Tx_It(&huart1, (uint8_t *)userCommand, userCommIndex + 1);
						userCommIndex = 0;

						endOfCommunicationTimeStamp = Get_SystemTimeMs();

						esp01NextState = ESP01_SEND_TO_USER;

					}
					else
					{

						// Wait a new character

						uart_Start_Rx_It(&huart2, &c, 1);

						esp01NextState = USER_SEND_TO_ESP01;

					}

				}
				else
				{

					esp01NextState = USER_SEND_TO_ESP01;

				}


				break;

			case ESP01_SEND_TO_USER:

				// Loop here until esp01 gives a response



				if(Compare_SystemTimeMs(endOfCommunicationTimeStamp) > 10000)
				{

					// Send data received from esp01 to serial monitor

					uart_Start_Tx_It(&huart2, esp01DataBuff, huart1.uart_IndexRx + 1);

					delayTimeStamp = Get_SystemTimeMs();

					while(Compare_SystemTimeMs(delayTimeStamp) < 1000);

					// End uart1 communication

					uart_End_Rx_It(&huart1);
					memset(esp01DataBuff, 0, 1000);

					uart_Start_Tx_It(&huart2, (uint8_t *)"Enter an AT command:\r\n", 22);
					uart_Start_Rx_It(&huart2, &c, 1);

					esp01NextState = USER_SEND_TO_ESP01;

				}
				else
				{

					esp01NextState = ESP01_SEND_TO_USER;

				}


				break;


		}




	}

}



void SysTick_CallBack(void)
{

	SysTickCountMs++;

}

void USART_ApplicationEventCallback(uartHandler_t *huart, uint8_t event)
{


	if(event == USART_EVENT_RX_CMPLT)
	{

		// Check which uart issued the interrupt
		if(huart->uart_Istance == huart2.uart_Istance)
		{

			// Communication from serial monitor

			if(esp01CurrentState == USER_SEND_TO_ESP01)
			{

				// Print out the character the user entered

				uart_Start_Tx_It(&huart2, &c, 1);

				userCommand[userCommIndex] = c;
				userCommIndex++;


				CharacterReceived = 1; // Flag for next character

			}

		}
		else if(huart->uart_Istance == huart1.uart_Istance)
		{


			// Communication from esp01
/*
			if(esp01CurrentState == ESP01_SEND_TO_USER)
			{

				// Print out the character from esp01

				uart_Start_Tx_It(&huart2, &c, 1);


			}
*/

		}




	}

}

