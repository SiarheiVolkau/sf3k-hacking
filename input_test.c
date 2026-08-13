/*
 * SF3000 bare-metal keyboard diagnostic.
 *
 * Reads 12 keys through the onboard serial keyboard controller and
 * four additional keys connected directly to GPIO, then reports
 * key state changes over UART0.
 *
 * The serial controller is accessed using the reverse-engineered
 * latch/clock/data protocol via GPIO.
 *
 * NOTE: GPIO L12 is shared with UART0 TX, so the Volume Down key
 * cannot be detected while UART output is enabled.
 */
#include <stdint.h>
#include "mmio.h"

#define PINMUXL   0xb88004a0
#define PINMUXB   0xb88004e0
#define PINMUXR   0xb8800520
#define PINMUXT   0xb8800560
#define GPIOLCTRL 0xb8800044
#define GPIOBCTRL 0xb88000c4
#define GPIORCTRL 0xb88000e4
#define GPIOTCTRL 0xb8800344

/* Per-bank register offsets */
#define HC16XX_GPIO_INT_EN		0x00
#define HC16XX_GPIO_EDGE_RISING		0x04
#define HC16XX_GPIO_EDGE_FALLING	0x08
#define HC16XX_GPIO_INPUT		0x0c
#define HC16XX_GPIO_OUTPUT		0x10
#define HC16XX_GPIO_DIR			0x14
#define HC16XX_GPIO_INT_ST		0x18

#define SCTRL_DEVCLKRST1_REG 0xb8800064
#define SCTRL_DEVRST0_REG    0xb8800080
#define SCTRL_DEVRST1_REG    0xb8800084

#define REG_IN_STATE  0x0c
#define REG_OUT_VALUE 0x10
#define REG_DIRECTION 0x14

/* UARTs are 8250 compatible, maybe even 16550 compatible */
#define UART0_BASE    0xb8818300
#define UART0_THR     REG8(UART0_BASE + 0)
#define UART0_RBR     REG8(UART0_BASE + 0)
#define UART0_FCR     REG8(UART0_BASE + 2)
#define UART0_LCR     REG8(UART0_BASE + 3)
#define UART0_MCR     REG8(UART0_BASE + 4)
#define UART0_LSR     REG8(UART0_BASE + 5)
#define UART0_MSR     REG8(UART0_BASE + 6)

#define LSR_THR_EMPTY BIT(6)

static void UART0_putc(char c)
{
        while (!(UART0_LSR & LSR_THR_EMPTY))
                ;
        UART0_THR = c;
}

static void UART0_puts(const char *s)
{
        while (*s) {
                UART0_putc(*s);
                s++;
        }
}

static void UART0_puthex32(uint32_t v)
{
    static const char h[]="0123456789ABCDEF";

    for (int i = 28; i >= 0; i -= 4)
        UART0_putc(h[(v >> i) & 0xf]);
}

static void usb_disable(void)
{
        uint8_t tmp;

        /* USB0 & 1 clocks gating */
        REG32(SCTRL_DEVCLKRST1_REG) &= ~(BIT(24) | BIT(25));
        /* Hold reset for USB0 & USB1 */
        REG32(SCTRL_DEVRST0_REG) |= BIT(28); /* USB0 */
        REG32(SCTRL_DEVRST1_REG) |= BIT(29); /* USB1 */

        /* set disconnect threshold voltage USB0 */
        tmp = REG8(0xB8845004);
        tmp &= ~(0xf << 4);
        tmp |= (0xd << 4);  /* 675 mV */
        REG8(0xB8845004) = tmp;

        /* set disconnect threshold voltage USB1 */
        tmp = REG8(0xB8845104);
        tmp &= ~(0xf << 4);
        tmp |= (0xd << 4);  /* 675 mV */
        REG8(0xB8845104) = tmp;
}

static void busy_wait(uint32_t wait)
{
        while (wait) {
                asm volatile ("nop\r\n");
                wait--;
        }
}

static const char *serializer_key_map[12][2] = {
        { "Lsh1 ", "     " },
        { "X ", "  " },
        { "Y ", "  " },
        { "Rsh1 ", "     " },
        { "B ", "  " },
        { "A ", "  " },
        { "Select ", "       " },
        { "Start ", "      " },
        { "Up ", "   " },
        { "Down ", "     " },
        { "Left ", "     " },
        { "Right ", "      " },
};

static const char *gpio_key_map[4][2] = {
        { "Lsh2 ", "     " }, // L07
        { "Rsh2 ", "     " }, // L11
        { "Vol- ", "     " }, // L12
        { "Vol+ ", "     " }, // L13
};

static void print_pressed_keys(uint32_t skeys, uint32_t gkeys)
{
        UART0_puts("Keys: ");
        for (int i = 0; i < 12; i++) {
                if (skeys & BIT(i)) {
                        UART0_puts(serializer_key_map[i][0]);
                } else {
                        UART0_puts(serializer_key_map[i][1]);
                }
        }
        for (int i = 0; i < 4; i++) {
                if (gkeys & BIT(i)) {
                        UART0_puts(gpio_key_map[i][0]);
                } else {
                        UART0_puts(gpio_key_map[i][1]);
                }
        }
        UART0_putc('\r');
}

static void init_key_serializer(void)
{
        REG8(PINMUXL + 8) = 0x00; // L08 as GPIO
        REG8(PINMUXL + 9) = 0x00; // L09 as GPIO

        /* set L09 as input, L08 as output high */
        REG32(GPIOLCTRL + HC16XX_GPIO_DIR) &= ~BIT(9);
        REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) &= ~BIT(9);
        REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) |= BIT(8);
        REG32(GPIOLCTRL + HC16XX_GPIO_DIR) |= BIT(8);
}

static uint32_t read_key_serializer(void)
{
        /* for every button - one pulse on L08, falling edge triggered except first */
        uint32_t buttons = 0;

        /* reset sequence - short pulse on L09 towards U16 */
        REG32(GPIOLCTRL + HC16XX_GPIO_DIR) |= BIT(9);
        busy_wait(1000);
        REG32(GPIOLCTRL + HC16XX_GPIO_DIR) &= ~BIT(9);

        busy_wait(500); // wait a bit bc L09 is is open drain pull-up

        if (REG32(GPIOLCTRL + HC16XX_GPIO_INPUT) & BIT(9)) {
                buttons |= 0 << 0;
        } else {
                buttons |= 1 << 0;
        }

        for (int i = 1; i < 12; i++) {

                REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) &= ~BIT(8);
                busy_wait(500); // wait a bit bc L09 is is open drain pull-up

                if (REG32(GPIOLCTRL + HC16XX_GPIO_INPUT) & BIT(9)) {
                        buttons |= 0 << i;
                } else {
                        buttons |= 1 << i;
                }

                REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) |= BIT(8);
                busy_wait(10);
        }

        /* Last clock, unsampled */
        REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) &= ~BIT(8);
        busy_wait(10);
        REG32(GPIOLCTRL + HC16XX_GPIO_OUTPUT) |= BIT(8);

        return buttons;
}

static uint32_t read_key_gpio(void)
{
        uint32_t l = REG32(GPIOLCTRL + HC16XX_GPIO_INPUT);
        uint32_t ret = 0;

        ret |= (l &  BIT(7)) ? 0 :  BIT(0);
        ret |= (l & BIT(11)) ? 0 : BIT(1);
        // ret |= (l & BIT(12)) ? 0 : BIT(2);
        ret |= (l & BIT(13)) ? 0 : BIT(3);
        return ret;
}

static void poll_keys(void)
{
        init_key_serializer();
        uint32_t sprev = ~0, gprev = ~0;
        while (1) {
                uint32_t scur = read_key_serializer();
                uint32_t gcur = read_key_gpio();
                if (scur != sprev || gcur != gprev) {
                        print_pressed_keys(scur, gcur);
                }
                sprev = scur;
                gprev = gcur;
                busy_wait(10000);
        }
}

int main()
{
        /* enables UART0_TX on L12 (Volume down key) */
        REG8(PINMUXL + 12) = 0x01;

        UART0_puts("Disabling USB ...\r\n");
        usb_disable();

        UART0_puts("NB: Volume Down key can't be catched\r\n");
        UART0_puts("NB: Because it's on same wire with UART\r\n");
        poll_keys();

        return 0;
}

