#ifndef GPIO_REGS_H
#define GPIO_REGS_H

#include <stdint.h>
#include <stddef.h>

#include "system_regs.h"

/* GPIO registers, 32-bit wide each.
 * There are 4 banks for Left,Botton,Right,Top GPIOs/pins.
 * Let's call these banks L,B,R,T respectively.
 * Each GPIO has its own bit in the bank registers.
 * e.g. pin L09 corresponds to BIT(9) in registers of bank L.
 */
#define GPIOL_BASE (SYSTEM_BASE + 0x044)
#define GPIOB_BASE (SYSTEM_BASE + 0x0C4)
#define GPIOR_BASE (SYSTEM_BASE + 0x0E4)
#define GPIOT_BASE (SYSTEM_BASE + 0x344)

/* Enables/disables 1/0 interrupt functionality
 * for specific GPIO/pin.
 */
#define REG_GPIO_INTEN (0x00)

/* Enables/disables 1/0 interrupt on rising edge
 * for specific GPIO/pin. Global interrupt enable
 * should also be enabled.
 */
#define REG_GPIO_EDGER (0x04)

/* Enables/disables 1/0 interrupt on falling edge
 * for specific GPIO/pin. Global interrupt enable
 * should also be enabled.
 */
#define REG_GPIO_EDGEF (0x08)

/* Readonly, reflects logic level on the specific
 * GPIO/pin. How it works with output direction
 * isn't known.
 */
#define REG_GPIO_IN    (0x0C)

/* Defines output logic level for output GPIOs/pins.
 * GPIO/pin can be set as output via REG_GPIO_DIR.
 */
#define REG_GPIO_OUT   (0x10)

/* Defines GPIOs/pins as output (1) or input (0).
 */
#define REG_GPIO_DIR   (0x14)

/* Interrupt status register: 0 - no interrupt, 1 - has interrupt.
 * Write 1 to clear.
 */
#define REG_GPIO_INTST (0x18)

#endif
