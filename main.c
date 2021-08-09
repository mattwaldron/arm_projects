// Adapted from Blinky.c from the ARM FreeRTOS example

#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h"

// adapted from the pseudocode at https://en.wikipedia.org/wiki/Mersenne_Twister

#define N 624
#define M 397
#define W 32

const unsigned f = 1812433253;
const unsigned a = 0x9908B0DF;
const unsigned u = 11, d = 0xFFFFFFFF;
const unsigned s = 7, b = 0x9D2C5680;
const unsigned t = 15, c = 0xEFC60000;

unsigned state [N];
unsigned next_idx = 0;


osThreadId_t processorThread;
osThreadId_t streamerThread;
osThreadId_t twisterThread;

osMessageQueueId_t prng_queue;
osSemaphoreId_t twist_sem;
osMutexId_t mersenne_state_lock;

osMessageQueueId_t transmitQueue;
osMessageQueueId_t receiveQueue;


void seed(unsigned s) {
	osMutexAcquire(mersenne_state_lock, osWaitForever);
	state[0] = s;
	for (int i = 1; i < N; i++) {
		state[i] = ((f * (state[i-1] ^ (state[i-1] >> (W-2)))) + i);
	}
	next_idx = 0;
	osMutexRelease(mersenne_state_lock);
}

void twister(void * argument) {
	for (;;) {	
		osSemaphoreAcquire(twist_sem, osWaitForever);
		osMutexAcquire(mersenne_state_lock, osWaitForever);
		for (int i = 0; i < N; i++) {
			int x = state[(i+1) % N];
			int xa = x >> 1;
			if (x & 0x1) {
				xa ^= a;
			}
			state[i] = state[(i + M) % N] ^ xa;
		}
		osMutexRelease(mersenne_state_lock);
	}
}

void prng_streamer(void *argument) {
		for (;;) {
				osMutexAcquire(mersenne_state_lock, osWaitForever);
				unsigned y = state[next_idx];
				y ^= ((y >> u) & d);
				y ^= ((y >> s) & b);
				y ^= ((y >> t) & c);
				y ^= (y >> 1);
				next_idx += 1;
				if (next_idx == N) {
					osSemaphoreRelease(twist_sem);
					next_idx = 0;
				}
				osMessageQueuePut(prng_queue, &y, 255, osWaitForever);
				osMutexRelease(mersenne_state_lock);
		}
}

void processor (void *argument) {
	unsigned runningXor = 0;
	unsigned v;
	uint8_t prio;
  for (;;) {
		osMessageQueueGet (prng_queue, &v, &prio, osWaitForever);
		runningXor ^= v;
  }
}

int main (void) {
  // System Initialization
  SystemCoreClockUpdate();
	
	seed(0);

  osKernelInitialize();                 // Initialize CMSIS-RTOS
	prng_queue = osMessageQueueNew (64, 4, NULL);
  twist_sem = osSemaphoreNew(1, 0, NULL);
	mersenne_state_lock = osMutexNew(NULL);
	
  streamerThread = osThreadNew(prng_streamer, NULL, NULL);
  processorThread = osThreadNew(processor, NULL, NULL);
  twisterThread = osThreadNew(twister, NULL, NULL);
	
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  for(;;);
}
