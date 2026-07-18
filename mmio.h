#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>
#include <stddef.h>

#define REG8(reg_addr)  (*((volatile uint8_t *)(reg_addr)))
#define REG16(reg_addr) (*((volatile uint16_t *)(reg_addr)))
#define REG32(reg_addr) (*((volatile uint32_t *)(reg_addr)))
#define BIT(x)          (1U << (x))

#endif
