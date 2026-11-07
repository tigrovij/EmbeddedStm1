#ifndef SOFTWARE_PWM_H
#define SOFTWARE_PWM_H

/* Initialize the PWM and button state after GPIO initialization. */
void SoftwarePwm_Init(void);

/* Call continuously from the main loop. */
void SoftwarePwm_Update(void);

#endif
