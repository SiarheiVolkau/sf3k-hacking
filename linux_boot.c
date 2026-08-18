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
#define DRIVER_CAP 0xb8800184

#define SCTRL_DEVCLKRST1_REG 0xb8800064
#define SCTRL_DEVRST0_REG    0xb8800080
#define CLK_SEL_REG1         0xb880007c
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

__attribute__((aligned(4096)))
static uint8_t exception_page[4096];

void exception_asm(void)
{
    __asm__ volatile(
        ".set noreorder\r\n"
        "lui     $26, %hi(exception_c)\r\n "
        "ori     $26, $26, %lo(exception_c)\r\n "
        "jr      $26\r\n "
        "move    $27, $sp\r\n "
    );
}

static const char *exc_name(unsigned code)
{
    switch (code) {
    case 0: return "Interrupt";
    case 1: return "TLB Mod";
    case 2: return "TLB Load";
    case 3: return "TLB Store";
    case 4: return "AddrErr Load";
    case 5: return "AddrErr Store";
    case 6: return "IBE";
    case 7: return "DBE";
    case 8: return "Syscall";
    case 9: return "Breakpoint";
    case 10: return "RI";
    case 11: return "Cop Unusable";
    case 12: return "Overflow";
    case 13: return "Trap";
    default: return "Unknown";
    }
}

static inline uint32_t read_c0_cause(void)
{
    uint32_t v;
    __asm__("mfc0 %0,$13" : "=r"(v));
    return v;
}

static inline uint32_t read_c0_epc(void)
{
    uint32_t v;
    __asm__("mfc0 %0,$14" : "=r"(v));
    return v;
}

static inline uint32_t read_c0_badvaddr(void)
{
    uint32_t v;
    __asm__("mfc0 %0,$8" : "=r"(v));
    return v;
}

static inline uint32_t read_c0_status(void)
{
    uint32_t v;
    __asm__("mfc0 %0,$12" : "=r"(v));
    return v;
}

void exception_c(void)
{
    uint32_t cause = read_c0_cause();

    UART0_puts("\r\n===== EXCEPTION =====\r\n");

    UART0_puts("Cause    : ");
    UART0_puthex32(cause);
    UART0_puts("\r\n");

    UART0_puts("ExcCode  : ");
    UART0_puthex32((cause >> 2) & 31);
    UART0_puts(" (");
    UART0_puts(exc_name((cause >> 2) & 31));
    UART0_puts(")\r\n");

    UART0_puts("EPC      : ");
    UART0_puthex32(read_c0_epc());
    UART0_puts("\r\n");

    UART0_puts("BadVAddr : ");
    UART0_puthex32(read_c0_badvaddr());
    UART0_puts("\r\n");

    UART0_puts("Status   : ");
    UART0_puthex32(read_c0_status());
    UART0_puts("\r\n");

    while (1)
        ;
}

/* some obscure bits flipped in TPL */
static void tune_mips74k(void)
{
        uint32_t config;
        uint32_t reg22;

        __asm__ volatile(
        "mfc0 %0, $16, 0\n\t"
        "ehb"
        : "=r"(config));

        config |= BIT(17);
        //config = (config & 0xFFFFFFF8) | 0x00000003;

        __asm__ volatile(
        "mtc0 %0, $16, 0\n\t"
        "ehb"
        :
        : "r"(config)
        : "memory");

        __asm__ volatile(
        "mfc0 %0, $22, 0\n\t"
        "ehb"
        : "=r"(reg22));

        reg22 |= BIT(0);

        __asm__ volatile(
        "mtc0 %0, $22, 0\n\t"
        "ehb"
        :
        : "r"(reg22)
        : "memory");
}

static void cache_invalidate(void)
{
        __asm__ volatile(
        "mtc0 $0, $28, 0\n\t"
        "ehb\n\t"
        "mtc0 $0, $29, 0\n\t"
        "ehb\n\t"
        "mtc0 $0, $28, 2\n\t"
        "ehb\n\t"
        "mtc0 $0, $29, 2\n\t"
        "ehb\n\t"
        : : : "memory");

        volatile uint8_t *p = (uint8_t *)0x80000000;
        for (int i = 0; i < 32768; i+=32) {
                __asm__ volatile(
                "cache 8, 0(%0)\n\t"
                "cache 9, 0(%0)\n\t"
                : : "r"(p) : "memory");
                p += 32;
        }
}

static void memcpy32_16(void *_dst, void *_src)
{
        uint32_t *dst = _dst;
        uint32_t *src = _src;

        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
}

static void install_exception_handlers(void)
{
        uint32_t status;
        long ebase = (long)exception_page;

        memcpy32_16(exception_page + 0x180, exception_asm); /* general exception */
        memcpy32_16(exception_page + 0x000, exception_asm); /* TLB refill */
        memcpy32_16(exception_page + 0x080, exception_asm); /* XTLB */
        memcpy32_16(exception_page + 0x100, exception_asm); /* Cache error */

        __asm__ volatile(
                "mtc0 %0,$15,1\n\t"
                "ehb"
                :
                : "r"(ebase)
                : "memory");

        __asm__("mfc0 %0,$12" : "=r"(status));

        status &= ~BIT(22);      /* clear BEV */

        __asm__ volatile(
                "mtc0 %0,$12\n\t"
                "ehb"
                :
                : "r"(status)
                : "memory");
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

static void sdio_setup(void)
{
        /* set regulator to 3v3 */
        uint32_t val = REG32(DRIVER_CAP);
        val &= ~(0x03 << 24);
        val |= 0x01 << 24;
        REG32(DRIVER_CAP) = val;

        /* set CIU clock to minimal clock */
        val = REG32(CLK_SEL_REG1);
        val &= ~(BIT(20) | BIT(21));
        val |= BIT(1); /* apply changes bit */
        REG32(CLK_SEL_REG1) = val;
}

typedef void (*kernel_entry)(int a0, int a1, int a2, int a3);
const kernel_entry entry = (kernel_entry)0x81000000;

int main()
{
        /* enables UART0_TX on L12 (Volume down key) */
        REG8(PINMUXL + 12) = 0x01;

        UART0_puts("Installing exception handlers ...\r\n");
        install_exception_handlers();

        UART0_puts("Clearing cache ...\r\n");
        tune_mips74k();
        cache_invalidate();
        asm volatile("sync");

        UART0_puts("Disabling USB ...\r\n");
        usb_disable();

        sdio_setup();
        UART0_puts("Running kernel ...\r\n");
        entry(0,0,0,0);

        return 0;
}

