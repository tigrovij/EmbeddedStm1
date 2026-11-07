#ifndef RELAY_TIMING_H
#define RELAY_TIMING_H

/* Run one measurement series after GPIO, TIM2 and USART2 are initialized. */
void RelayTiming_Run(void);

/* Call from EXTI when the relay contact changes state. */
void RelayTiming_OnContactEdge(void);

#endif
