// Adapted from Blinky.c from the ARM FreeRTOS example

#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h"


osThreadId_t processorThread;
osThreadId_t transmitterThread;
osThreadId_t receiverThread;

osMessageQueueId_t transmitQueue;
osMessageQueueId_t receiveQueue;

#pragma anon_unions

volatile union {
	unsigned char BYTE;
	struct {
		 unsigned char PROCESSING: 1;
		 unsigned char TRANSMITTING: 1;
		 unsigned char RECEIVING: 1;
		 unsigned char UNUSED: 5;
	};
} status;

void process (void *argument) {
  uint32_t flags;
	char value;
	uint32_t primes [] = {2, 3, 5, 7, 11, 13, 17, 19 };
	for (;;) {
		status.PROCESSING = 0;
		flags = osThreadFlagsWait(0x0000FFFF, osFlagsWaitAny ,osWaitForever);
		status.PROCESSING = 1;
		value = 0;
		char bitSet = 1;
		int i = 0;
		while(flags != 1 && bitSet != 0) {
			while(flags % primes[i] == 0) {
				flags /= primes[i];
				value |= bitSet;
			}
			i++;
			bitSet <<= 1;
		}
		if (flags == 1) {
			osMessageQueuePut(transmitQueue, &value, 1, osWaitForever);
		}
		else {
			value = flags & 0xFF;
			osMessageQueuePut(transmitQueue, &value, 1, osWaitForever);
		}
  }
}

void transmit (void *argument) {
	char value;
	uint8_t prio;
	
  for (;;) {
		status.TRANSMITTING = 0;
		osMessageQueueGet(transmitQueue, &value, &prio, osWaitForever);
		status.TRANSMITTING = 1;
		osDelay(100);	// mimic transmit time
		status.TRANSMITTING = 0;
		osMessageQueuePut(receiveQueue, &value, 1, osWaitForever);
		status.TRANSMITTING = 1;
  }
}

void receive (void *argument) {
	char value;
	uint8_t prio;
	uint32_t valueSquaredPlusOne;
	
  for (;;) {
		status.RECEIVING = 0;
    osMessageQueueGet(receiveQueue, &value, &prio, osWaitForever);
		status.RECEIVING = 1;
    valueSquaredPlusOne = value;
		valueSquaredPlusOne *= valueSquaredPlusOne;
		valueSquaredPlusOne += 1;
		osThreadFlagsSet(processorThread, valueSquaredPlusOne);
  }
}

int main (void) {

	char seed = 0xAA;
  // System Initialization
  SystemCoreClockUpdate();

  osKernelInitialize();                 // Initialize CMSIS-RTOS
  transmitQueue = osMessageQueueNew (8, 8, NULL);
	receiveQueue = osMessageQueueNew (8, 8, NULL);
	osMessageQueuePut(receiveQueue, &seed, 1, osWaitForever);
	
  receiverThread = osThreadNew(receive, NULL, NULL);
  processorThread = osThreadNew(process, NULL, NULL);
  transmitterThread = osThreadNew(transmit, NULL, NULL);
	
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  while(1);
}
