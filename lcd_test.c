/*
 * SF3000 bare-metal LCD init code.
 */
#include <stdint.h>
#include <stdbool.h>
#include "mmio.h"
#include "de_regs.h"
#include "system_regs.h"
#include "gpio_regs.h"
#include "mipi_regs.h"

#define __maybe_unused __attribute__((unused))

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

static const char h[]="0123456789ABCDEF";
static void __maybe_unused UART0_puthex32(uint32_t v)
{
        for (int i = 28; i >= 0; i -= 4)
                UART0_putc(h[(v >> i) & 0xf]);
}

static void __maybe_unused UART0_puthex8(uint8_t v)
{
        for (int i = 4; i >= 0; i -= 4)
                UART0_putc(h[(v >> i) & 0xf]);
}

static void usb_disable(void)
{
        uint8_t tmp;

        /* USB0 & 1 clocks gating */
        REG32(REG_CLKGATE2) &= ~(BIT(24) | BIT(25));
        /* Hold reset for USB0 & USB1 */
        REG32(REG_RESET1) |= BIT(28); /* USB0 */
        REG32(REG_RESET2) |= BIT(29); /* USB1 */

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






static void backlight_init(void)
{
#define PWM_BASE 0xb8818410
#define PWM_CTRL_REG (PWM_BASE + 0x00)
#define  PWM_GLOBAL_ENABLE BIT(31)
#define  PWM_CLOCK_ENABLE  BIT(30)
#define  PWM_CLKDIV(x)     ((x) << 16)
#define PWM_CHAN_CTRL_REG(ch) (PWM_BASE + 0x04 + (ch))
#define  PWM_CHAN_POLARITY  BIT(4)
#define  PWM_CHAN_ENABLE    BIT(7)
#define PWM_CHAN_COUNTER_REG(ch) (PWM_BASE + 0x0c + 4 * (ch))
#define  PWM_LOW_COUNTER(x)  ((x) & 0xffff)
#define  PWM_HIGH_COUNTER(x) ((x) << 16)

        const int pwm_channel = 0;
        const int input_clock = 27000000;

        REG8(REG_PINMUX_L(0)) = 0x04; /* PWM0 on L00 */

        /* PWM normal enabled, period 1/40000s duty cycle 80% */
        //UART0_puts("PWM: normal, enabled\r\n");
        int clkdiv = 1;
        uint32_t period_ticks = (input_clock / clkdiv) / 40000;
        uint32_t duty_ticks = period_ticks * 80 / 100;
        // if both counters are 0 - output remains in 0 (TODO check by oscilloscope)
        // if only high counter is 0 - output remains in 0 (TODO check by oscilloscope)
        // if only low counter is 0 - output remains in 1 (TODO check by oscilloscope)
        // can total counters value be > 65535 ? - yes
        // on disabling channel output goes low (if normal polarity: PWM_CHAN_POLARITY=0)
        // on re-enabling channel counting starts from beginning
        // with normal polarity duty period goes last (aka offset_ns = period_ns - duty_ns)
        // that might be fixed with changing polarity for enabled PWM and switching it back for disabled.
        REG32(PWM_CHAN_COUNTER_REG(pwm_channel)) = PWM_LOW_COUNTER(period_ticks - duty_ticks) | PWM_HIGH_COUNTER(duty_ticks);
        REG32(PWM_CTRL_REG) = PWM_CLKDIV(clkdiv - 1);
        REG8(PWM_CHAN_CTRL_REG(pwm_channel)) = 0;
        REG32(PWM_CTRL_REG) |= PWM_CLOCK_ENABLE | PWM_GLOBAL_ENABLE;
        if (pwm_channel != 0) {
                /* PWM0 must be enabled for any other PWM channel, looks like a HW bug/pecularity */
                REG32(PWM_CHAN_COUNTER_REG(0)) = 0;
                REG8(PWM_CHAN_CTRL_REG(0)) = PWM_CHAN_ENABLE;
        }
        REG8(PWM_CHAN_CTRL_REG(pwm_channel)) |= PWM_CHAN_ENABLE;
}






const uint32_t hactive = 480;
const uint32_t hfp = 88;
const uint32_t hsync = 4;
const uint32_t hbp = 40;
const uint32_t htotal = 480 + 88 + 4 + 40;
const uint32_t cpp = 2;

const uint32_t vactive = 854;
const uint32_t vfp = 20;
const uint32_t vsync = 4;
const uint32_t vbp = 20;
const uint32_t vtotal  = 854 + 20 + 4 + 20;

const uint32_t n_dsi_lanes = 2;
const uint32_t bpp = 24;
#define HCOMPONENT_TO_LBCC(hcmp) (bpp * (hcmp) / (n_dsi_lanes * 8))

static void mipi_dsi_host_init(void)
{
        //hc_clk_enable(MIPI_CLK==>0x35)
        REG32(REG_CLKGATE2) &= ~BIT(28);
        //hc_mipi_lcd_init.constprop.8()
        REG8(REG_PINMUX_L(6)) = 0x00;
        //reset_deassert(GPIO_L06)
        REG32(GPIOL_BASE + REG_GPIO_DIR) |= BIT(6);
        REG32(GPIOL_BASE + REG_GPIO_OUT) &= ~BIT(6);
        busy_wait(10000);
        REG32(GPIOL_BASE + REG_GPIO_OUT) |= BIT(6);
        busy_wait(10000);
        //hc_board_mipi_init.constprop.11()
        //hc_mipi_bus_ctrl_init()
        //hc_mipi_phy_reset()
        REG32(REG_RESET2) |= BIT(7); busy_wait(30000); REG32(REG_RESET2) &= ~BIT(7);
        //hc_mipi_phy_bist_test()
        REG32(REG_MIPI_DSI_REG_200) |= BIT(0x18);
        //hc_mipi_phy_reset()
        REG32(REG_RESET2) |= BIT(7); busy_wait(30000); REG32(REG_RESET2) &= ~BIT(7);
        REG32(REG_MIPI_DSI_REG_200) |= BIT(0x18) | BIT(0x19);
        int timeout = 100;
        while ((REG32(REG_MIPI_DSI_REG_200) & BIT(0x1d)) == 0) {
                busy_wait(30000);
                timeout--;
                if (timeout == 0) {
                        UART0_puts("BIST failed\r\n");
                        return;
                }
        }
        if ((REG32(REG_MIPI_DSI_REG_200) & BIT(0x1c)) == 0) {
                UART0_puts("BIST failed #2\r\n");
                return;
        }
        //hc_mipi_phy_cancel_bist_mode()
        REG32(REG_MIPI_DSI_REG_200) &= ~BIT(0x18);
        //hc_mipi_phy_reset()
        REG32(REG_RESET2) |= BIT(7); busy_wait(30000); REG32(REG_RESET2) &= ~BIT(7);
        //hc_mipi_phy_poweron()
        REG32(REG_MIPI_DSI_PHY_104) = (REG32(REG_MIPI_DSI_PHY_104) & ~0xff) | 0x21; // pll multiplier
        REG32(REG_MIPI_DSI_PHY_100) &= ~BIT(0x1d); // pll multiplier MSB=0
        REG32(REG_MIPI_DSI_PHY_100) = (REG32(REG_MIPI_DSI_PHY_100) & ~0x1f000000) | (1 << 24); // pll divisor = 1
        busy_wait(30000);

        REG32(REG_MIPI_DSI_PHY_100) = (REG32(REG_MIPI_DSI_PHY_100) & ~0x0000ff00) | (0xe4 << 8);
        REG32(REG_MIPI_DSI_PHY_100) = (REG32(REG_MIPI_DSI_PHY_100) & ~0xff) | (0x4c | 0x01); // 0x4c clock lane and two data lanes
        REG32(REG_MIPI_DSI_PHY_108) = (REG32(REG_MIPI_DSI_PHY_108) & ~0x0f) | 0x0e;               // drive_strength = 2 (high)
        REG32(REG_MIPI_DSI_PHY_108) = (REG32(REG_MIPI_DSI_PHY_108) & ~0x0f000000) | (0x0e << 24); // drive_strength = 2 (high)
        REG32(REG_MIPI_DSI_PHY_100) = (REG32(REG_MIPI_DSI_PHY_100) & ~0x0000ff00) | (0xe0 << 8);
        busy_wait(150000);

        REG32(REG_MIPI_DSI_PHY_120) = (REG32(REG_MIPI_DSI_PHY_120) & ~0xff) | 0x1e;
        REG32(REG_MIPI_DSI_PHY_120) = (REG32(REG_MIPI_DSI_PHY_120) & ~0xff) | 0x1f;
        //hc_mipi_clk_calibration()
        REG32(REG_MIPI_DSI_REG_200) = (REG32(REG_MIPI_DSI_REG_200) & ~0x00ff0000) | (0xf1 << 16);
        busy_wait(30000);
        REG32(REG_MIPI_DSI_REG_200) &= ~0x00ff0000;
        busy_wait(30000);

        //hc_mipi_host_reset() /* NB: inexact behavior */
        REG32(REG_RESET1) |= BIT(30); busy_wait(90000);
        REG32(REG_RESET1) &= ~BIT(30); busy_wait(90000);
        //hc_mipi_phy_init(2 lanes)
        //hc_inno_phy_set_gotp(0x80)
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT1,MIPI_DSI_LANE_REG_00)) = 0x4000054A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT1,MIPI_DSI_LANE_REG_04)) = 0x077EC3B0;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT1,MIPI_DSI_LANE_REG_08)) = 0x0F0F4604;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT1,MIPI_DSI_LANE_REG_0C)) = 0x1086FF8A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT1,MIPI_DSI_LANE_REG_10)) = 0x800A0308;
        //hc_inno_phy_set_gotp(0x60)
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT0,MIPI_DSI_LANE_REG_00)) = 0x4000054A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT0,MIPI_DSI_LANE_REG_04)) = 0x077EC3B0;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT0,MIPI_DSI_LANE_REG_08)) = 0x0F0F4604;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT0,MIPI_DSI_LANE_REG_0C)) = 0x1086FF8A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_DAT0,MIPI_DSI_LANE_REG_10)) = 0x800A0308;
        //hc_inno_phy_set_gotp(0x40)
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_CLK,MIPI_DSI_LANE_REG_00)) = 0x4000054A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_CLK,MIPI_DSI_LANE_REG_04)) = 0x197EC3B0;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_CLK,MIPI_DSI_LANE_REG_08)) = 0x0F0A4604;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_CLK,MIPI_DSI_LANE_REG_0C)) = 0x1082FF8A;
        REG32(REG_MIPI_LANE(MIPI_DSI_LANE_CLK,MIPI_DSI_LANE_REG_10)) = 0x800A0308;
        //hc_mipi_bus_dcs_cmd_param()
        //hc_mipi_bus_config_dpi()
        REG32(REG_MIPI_DSI_REG_0C) = (5 << 2); // DPI format 24-bit, DSI format is the same.
        REG32(REG_MIPI_DSI_REG_08) = (REG32(REG_MIPI_DSI_REG_08) & 0xffff0000)
                                   | (0x01 << 8) | 0x03; // 0x03 taken from SF3000 dump DCS read became stable. Initiall value 0x07.
        //hc_mipi_bus_config_dbi()
        REG32(REG_MIPI_DSI_REG_10) = BIT(6) | BIT(0); // ??? - reconstruct sets at least 2 bits BIT(0) | BIT(6)
        //hc_mipi_bus_config_cmd_size()
        REG32(REG_MIPI_DSI_REG_14) = (10 << 16) | 0x1a; // ??? - reconstruct sets (10 << 16) | 0x1a;
        //hc_mipi_bus_config_PCKHDL()
        REG32(REG_MIPI_DSI_PCKHDL) = 0x0000001C;
        REG32(REG_MIPI_DSI_VIDPKTSIZE) = 0; // actually there's 3 fields
        //hc_mipi_bus_enable_cmd_mode()
        REG32(REG_MIPI_DSI_REG_24) = 0x1fff;
        //hc_mipi_bus_config_package(0, {0})
        REG32(REG_MIPI_DSI_REG_28) = 0;
        REG32(REG_MIPI_DSI_REG_2C) = 0;
        REG32(REG_MIPI_DSI_VIDPKTSIZE) = 0;
        REG32(REG_MIPI_DSI_REG_30) = (0x28 << 20) | (0x28 << 12) | 0 << 0; // inexact
        REG32(REG_MIPI_DSI_PHY_IF_CFG) = (REG32(REG_MIPI_DSI_PHY_IF_CFG) & ~0x3) | (n_dsi_lanes - 1);
        REG32(REG_MIPI_DSI_REG_54) |= 0x07;
        uint32_t tmp = REG32(REG_MIPI_DSI_REG_60);
        (void)tmp;
        REG32(REG_MIPI_DSI_REG_5C) |= 0x01;
        //hc_mipi_dsi_host_power_on()
        //hc_mipi_dsi_host_pwrctrl()
        REG32(REG_MIPI_DSI_REG_PWRUP) |= 1;
        timeout = 10;
        while ((REG32(REG_MIPI_DSI_REG_60) & BIT(0)) == 0) {
                busy_wait(30000);
                timeout--;
                if (timeout == 0) {
                        UART0_puts("Poweron failed\r\n");
                        return;
                }
        }
        timeout = 10;
        while ((REG32(REG_MIPI_DSI_REG_60) & BIT(2)) == 0) {
                busy_wait(3000);
                timeout--;
                if (timeout == 0) {
                        //UART0_puts("Poweron failed #2\r\n");
                        //return;
                        break;
                }
        }
}

static int mipi_dsi_host_dcs_write_header(uint32_t hdr)
{
        int timeout = 100;
        while (timeout > 0) {
                uint32_t stat = REG32(REG_MIPI_DSI_CMD_PKT_STATUS);
                if ((stat & BIT(1)) == 0)
                        break;
                timeout--;
                busy_wait(3000);
        }
        if (timeout <= 0) {
                UART0_puts("CMD FIFO full\r\n");
                return -1;
        }
        //UART0_puts("CMD="); UART0_puthex32(hdr); UART0_puts("\r\n");
        REG32(REG_MIPI_DSI_GEN_HDR) = hdr;
        return 0;
}

static int mipi_dsi_host_dcs_write_short(uint8_t cmd)
{
        return mipi_dsi_host_dcs_write_header(0x05 | (cmd << 8));
}

static int mipi_dsi_host_dcs_write_param(uint8_t cmd, uint8_t param)
{
        return mipi_dsi_host_dcs_write_header(0x15 | (cmd << 8) | (param << 16));
}

static int mipi_dsi_host_dcs_write_long(const uint8_t *data, uint8_t len)
{
        if (!data || !len)
                return -1;

        for (int i = 0; i < len;) {
                int timeout = 100;
                uint32_t word;

                if (len - i >= 4) {
                        word = ((uint32_t)data[i]) |
                               ((uint32_t)data[i + 1] << 8) |
                               ((uint32_t)data[i + 2] << 16) |
                               ((uint32_t)data[i + 3] << 24);
                        i += 4;
                } else {
                        word = 0;
                        for (int j = len - 1; j >= i; j--) {
                                word = (word << 8) | data[j];
                        }
                        i = len;
                }

                while (timeout > 0) {
                        uint32_t stat = REG32(REG_MIPI_DSI_CMD_PKT_STATUS);
                        if ((stat & BIT(3)) == 0)
                                break;
                        timeout--;
                        busy_wait(3000);
                }
                if (timeout <= 0) {
                        UART0_puts("PLD Tx FIFO full\r\n");
                        return -1;
                }
                //UART0_puts("PLD="); UART0_puthex32(word); UART0_puts("\r\n");
                REG32(REG_MIPI_DSI_GEN_PLD_DATA) = word;
        }

        return mipi_dsi_host_dcs_write_header(0x39 | (len << 8));
}

static int mipi_dsi_host_generic_read(uint8_t type, uint8_t arg1, uint8_t arg2)
{
        int err = mipi_dsi_host_dcs_write_header(type | (arg1 << 8) | (arg2 << 16));
        int timeout = 1000;
        uint32_t pld;

        if (err < 0)
                return err;

        while ((REG32(REG_MIPI_DSI_CMD_PKT_STATUS) & BIT(6)) != 0) {
                timeout--;
                if (timeout <= 0) {
                        UART0_puts("CMD Execution timeout\r\n");
                        return -1;
                }
                busy_wait(3000);
        }

        timeout = 100;
        while ((REG32(REG_MIPI_DSI_CMD_PKT_STATUS) & BIT(4)) != 0) {
                timeout--;
                if (timeout <= 0) {
                        UART0_puts("PLD Rx FIFO empty\r\n");
                        return -1;
                }
                busy_wait(3000);
        }
        pld = REG32(REG_MIPI_DSI_GEN_PLD_DATA);

        /* clear the rest in FIFO if any */
        while ((REG32(REG_MIPI_DSI_CMD_PKT_STATUS) & BIT(4)) == 0) {
                uint32_t tmp = REG32(REG_MIPI_DSI_GEN_PLD_DATA);
                (void) tmp;
        }

        //UART0_puts("read: 0x"); UART0_puthex8(arg1); UART0_puts(" = 0x"); UART0_puthex32(pld); UART0_puts("\r\n");
        return pld & 0xffffff;
}

static int __maybe_unused
mipi_dsi_host_dcs_read(uint8_t arg)
{
        return mipi_dsi_host_generic_read(0x06, arg, 0);
}

static int __maybe_unused
mipi_dsi_host_generic_short_read_0args(void)
{
        return mipi_dsi_host_generic_read(0x04, 0, 0);
}

static int __maybe_unused
mipi_dsi_host_generic_short_read_1arg(uint8_t arg)
{
        return mipi_dsi_host_generic_read(0x14, arg, 0);
}

static int __maybe_unused
mipi_dsi_host_generic_short_read_2arg(uint8_t arg1, uint8_t arg2)
{
        return mipi_dsi_host_generic_read(0x24, arg1, arg2);
}

static void mipi_dsi_host_dcs_reset(void)
{
        mipi_dsi_host_dcs_write_short(0x01);
        busy_wait(300000);
}

static int __maybe_unused
mipi_dsi_host_dcs_set_max_ret_pkt_size(uint8_t maxsz)
{
        return mipi_dsi_host_dcs_write_header(0x37 | (maxsz << 8));
}

static void mipi_dsi_host_set_command_mode(void)
{
        //mipi_ioctl_vedio_to_cmd()
        //hc_mipi_dsi_enable_hs_clk(1)
        REG32(REG_MIPI_DSI_REG_5C) |= 0x01;
        //hc_mipi_bus_enable_vedio_mode(0)
        REG32(REG_MIPI_DSI_MODECFG) = 0x03fc; // "dsi,flags" minus BIT(0)
        //hc_mipi_bus_enable_cmd_mode(1)
        REG32(REG_MIPI_DSI_REG_24) = (REG32(REG_MIPI_DSI_REG_24) & 0xffff0000) | 0x1fff;
}

static void mipi_dsi_host_set_video_mode(void)
{
        //hc_mipi_bus_config_package(0, {0})
        REG32(REG_MIPI_DSI_REG_28) = (HCOMPONENT_TO_LBCC(hsync) << 0)
                                | (HCOMPONENT_TO_LBCC(hbp) << 9)
                                | (HCOMPONENT_TO_LBCC(htotal) << 18); // expected: 0x0E587806
        _Static_assert(HCOMPONENT_TO_LBCC(hsync) < 512, "");
        _Static_assert(HCOMPONENT_TO_LBCC(hbp) < 512, "");

        REG32(REG_MIPI_DSI_REG_2C) = (vsync << 0)
                                | (vbp << 4)
                                | (vfp << 10)
                                | (vactive << 16); // expected: 0x03565144
        _Static_assert(vsync < 16, "");
        _Static_assert(vbp < 64, "");
        _Static_assert(vfp < 64, "");

        REG32(REG_MIPI_DSI_VIDPKTSIZE) = hactive; // hactive & 2 zero fields

        //hc_mipi_dsi_enable_hs_clk(1)
        REG32(REG_MIPI_DSI_REG_5C) |= 0x01;

        //hc_mipi_bus_enable_cmd_mode(0)
        REG32(REG_MIPI_DSI_REG_24) = (REG32(REG_MIPI_DSI_REG_24) & 0xffff0000) | 0x4000;

        //hc_mipi_bus_enable_vedio_mode(1)
        REG32(REG_MIPI_DSI_MODECFG) = 0x03fd; // "dsi,flags" as is
        //set_mipi_enabled
        REG32(REG_MIPI_DSI_REG_200) |= BIT(3);
}

static int mipi_dsi_host_dcs_panel_init(const uint8_t *seq)
{
        uint8_t len;
        uint8_t cmd;
        int err;

        cmd = seq[0];

        while (cmd) {
                switch (cmd) {
                case 1:
                        err = mipi_dsi_host_dcs_write_short(seq[1]);
                        seq += 2;
                        break;
                case 2:
                        err = mipi_dsi_host_dcs_write_param(seq[1], seq[2]);
                        seq += 3;
                        break;
                case 3:
                {
                        len = seq[1];
                        err = mipi_dsi_host_dcs_write_long(&seq[2], len);
                        seq += len + 2;
                        break;
                }
                case 4:
                        busy_wait(seq[0] * 10000);
                        seq += 2;
                        err = 0;
                        break;
                default:
                        UART0_puts("Unrecognized DCS command:");
                        UART0_puthex8(cmd);
                        UART0_puts("\r\n");
                        return -1;
                }
                if (err < 0)
                        return err;
                cmd = seq[0];
        }
        return 0;
}


static const uint8_t __maybe_unused sf3000_0x00_sequence[] = {
       3,  6,  0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00, // Novatek page 0 unlock
       3,  3,  0xf6, 0x5a, 0x87,
       2,      0xc1, 0x3f,
       2,      0xcd, 0x25,
       2,      0xc9, 0x10,
       2,      0xf8, 0x8a,
       2,      0xac, 0x45,
       2,      0xa7, 0x47,
       2,      0xa0, 0xdd,
       3,  5,  0x86, 0x99, 0xa3, 0xa3, 0x51,
       3,  5,  0xfa, 0x08, 0x08, 0x00, 0x04,
       2,      0xa3, 0x22,
       3,  4,  0xfd, 0x28, 0x38, 0x00,
       2,      0x9a, 0x5a,
       2,      0x9b, 0x5a,
       3,  3,  0x82, 0x2f, 0x2f,
       2,      0xb1, 0x10,
       3,  3,  0x7a, 0x0f, 0x13,
       3,  3,  0x7b, 0x0f, 0x13,
       3, 33,  0x6d, 0x1e, 0x00, 0x09, 0x0b, 0x01, 0x1f, 0x1e,
               0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
               0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
               0x1e, 0x1e, 0x1e, 0x1f, 0x02, 0x0c, 0x0a, 0x00,
               0x1e,
       3,  9,  0x60, 0x18, 0x07, 0x08, 0x7a, 0x18, 0x06, 0x08,
               0x7a,
       3, 17,  0x64, 0x18, 0x06, 0x03, 0x56, 0x03, 0x03, 0x18,
               0x05, 0x03, 0x57, 0x03, 0x03, 0x08, 0x7a, 0x08,
               0x7a,
       3, 17,  0x65, 0x18, 0x04, 0x03, 0x58, 0x03, 0x03, 0x18,
               0x03, 0x03, 0x59, 0x03, 0x03, 0x08, 0x7a, 0x08,
               0x7a,
       3,  8,  0x69, 0x14, 0x22, 0x14, 0x22, 0x44, 0x22, 0x08,
       2,      0x6b, 0x03,
       3, 53,  0xd1, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xf8, 0x01, 0xfe, 0x02, 0x4f, 0x02,
               0x9c, 0x02, 0xd4, 0x03, 0x1a, 0x03, 0x32, 0x03,
               0x67, 0x03, 0x88, 0x03, 0xb9, 0x03, 0xc4, 0x03,
               0xc5, 0x03, 0xed, 0x03, 0xff,
       3, 53,  0xd2, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xf8, 0x01, 0xfe, 0x02, 0x4f, 0x02,
               0x9c, 0x02, 0xd4, 0x03, 0x1a, 0x03, 0x32, 0x03,
               0x67, 0x03, 0x88, 0x03, 0xb9, 0x03, 0xc4, 0x03,
               0xc5, 0x03, 0xed, 0x03, 0xff,
       3, 53,  0xd3, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xf8, 0x01, 0xfe, 0x02, 0x4f, 0x02,
               0x9c, 0x02, 0xd4, 0x03, 0x1a, 0x03, 0x32, 0x03,
               0x67, 0x03, 0x88, 0x03, 0xb9, 0x03, 0xc4, 0x03,
               0xc5, 0x03, 0xed, 0x03, 0xff,
       3, 53,  0xd4, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xd8, 0x01, 0xde, 0x02, 0x25, 0x02,
               0x66, 0x02, 0x90, 0x02, 0xd6, 0x02, 0xee, 0x03,
               0x26, 0x03, 0x38, 0x03, 0x63, 0x03, 0x74, 0x03,
               0x8c, 0x03, 0xd3, 0x03, 0xff,
       3, 53,  0xd5, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xd8, 0x01, 0xde, 0x02, 0x25, 0x02,
               0x66, 0x02, 0x90, 0x02, 0xd6, 0x02, 0xee, 0x03,
               0x26, 0x03, 0x38, 0x03, 0x63, 0x03, 0x74, 0x03,
               0x8c, 0x03, 0xd3, 0x03, 0xff,
       3, 53,  0xd6, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x20, 0x00,
               0x30, 0x00, 0x42, 0x00, 0x60, 0x00, 0x7d, 0x00,
               0xa7, 0x00, 0xbf, 0x01, 0x02, 0x01, 0x3c, 0x01,
               0x9d, 0x01, 0xd8, 0x01, 0xde, 0x02, 0x25, 0x02,
               0x66, 0x02, 0x90, 0x02, 0xd6, 0x02, 0xee, 0x03,
               0x26, 0x03, 0x38, 0x03, 0x63, 0x03, 0x74, 0x03,
               0x8c, 0x03, 0xd3, 0x03, 0xff,
       1,      0x11,
       4, 120,
       1,      0x29,
       4, 120,
       0, /* sentinel */
};

static const uint8_t __maybe_unused sf3000_0x80_sequence[] = {
       3,  4,  0x99, 0x71, 0x02, 0xa2,
       3,  4,  0x99, 0x71, 0x02, 0xa3,
       3,  4,  0x99, 0x71, 0x02, 0xa4,
       3,  8,  0xb0, 0x22, 0x6b, 0x1e, 0x75, 0x2f, 0x39, 0x43,
       3,  3,  0xb7, 0x7d, 0x7d,
       3,  3,  0xbf, 0xa0, 0xa0,
       3,  9,  0xba, 0x72, 0x76, 0x52, 0xd6, 0xc0, 0x20, 0x00,
               0x00,
       3,  7,  0xd7, 0x00, 0x0b, 0xc6, 0x19, 0xab, 0xab,
       3, 33,  0xa3, 0x51, 0x03, 0x88, 0x00, 0x44, 0x00, 0x00,
               0x00, 0x00, 0x04, 0x5c, 0xcf, 0x00, 0x1a, 0x00,
               0x45, 0x05, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00,
               0x00, 0x02, 0x20, 0x52, 0x00, 0x00, 0x00, 0x00,
               0xff,
       3, 45,  0xa6, 0x30, 0x00, 0x24, 0x77, 0x36, 0x00, 0x37,
               0x00, 0x5c, 0xcf, 0x02, 0xe4, 0x99, 0x00, 0x0b,
               0x00, 0x0b, 0xcf, 0x5c, 0x02, 0xa4, 0x11, 0x00,
               0x00, 0x00, 0x00, 0x5c, 0xcf, 0x00, 0xac, 0x11,
               0x00, 0x00, 0x00, 0x00, 0x5c, 0xcf, 0x00, 0x00,
               0x06, 0x00, 0x00, 0x00, 0x00,
       3, 49,  0xa7, 0x19, 0x19, 0x00, 0x64, 0x40, 0x05, 0x14,
               0x40, 0x00, 0x04, 0x03, 0x5c, 0xcf, 0x00, 0x64,
               0x40, 0x23, 0x67, 0x01, 0x00, 0x02, 0x00, 0x5c,
               0xcf, 0x00, 0x64, 0x40, 0x4b, 0x5a, 0x00, 0x00,
               0x02, 0x01, 0x5c, 0xcf, 0x00, 0x24, 0x40, 0x69,
               0x78, 0x00, 0x00, 0x00, 0x00, 0x5c, 0xcf, 0x00,
               0x44,
       3, 38,  0xac, 0x13, 0x10, 0x01, 0x00, 0x1c, 0x12, 0x02,
               0x1b, 0x18, 0x09, 0x19, 0x1a, 0x1b, 0x1b, 0x18,
               0x1b, 0x0b, 0x11, 0x00, 0x02, 0x1c, 0x0a, 0x02,
               0x18, 0x18, 0x08, 0x19, 0x1a, 0x1b, 0x1b, 0x18,
               0x1b, 0xff, 0xff, 0xff, 0xff, 0x00,
       3,  8,  0xad, 0xcc, 0x40, 0x46, 0x11, 0x04, 0x5c, 0xcf,
       3, 15,  0xe8, 0x30, 0x07, 0x00, 0xeb, 0xeb, 0x9c, 0x00,
               0xe2, 0x04, 0x00, 0x00, 0x00, 0x00, 0xef,
       3,  3,  0x75, 0x03, 0x04,
       3, 34,  0xe7, 0x8b, 0x3c, 0x00, 0x0c, 0xf0, 0x5d, 0x00,
               0x5d, 0x05, 0x5d, 0x00, 0x5d, 0x00, 0xff, 0x00,
               0x08, 0x7b, 0x00, 0x00, 0xc8, 0x6a, 0x5a, 0x08,
               0x1a, 0x3c, 0x00, 0x91, 0x01, 0xcc, 0x01, 0x7f,
               0xf0, 0x22,
       3, 38,  0xc8, 0x00, 0x00, 0x15, 0x26, 0x44, 0x00, 0x78,
               0x03, 0xbe, 0x06, 0x11, 0x1c, 0x09, 0x8a, 0x03,
               0x21, 0xd4, 0x01, 0x11, 0x0f, 0x22, 0x4a, 0x0f,
               0x8f, 0x0a, 0x32, 0xf0, 0x0a, 0x41, 0x0d, 0xf3,
               0x80, 0x0d, 0xae, 0xc5, 0x03, 0xc4,
       3, 38,  0xc9, 0x00, 0x00, 0x15, 0x26, 0x44, 0x00, 0x78,
               0x03, 0xbe, 0x06, 0x11, 0x1c, 0x09, 0x8a, 0x03,
               0x21, 0xd4, 0x01, 0x11, 0x0f, 0x22, 0x4a, 0x0f,
               0x8f, 0x0a, 0x32, 0xf0, 0x0a, 0x41, 0x0d, 0xf3,
               0x80, 0x0d, 0xae, 0xc5, 0x03, 0xc4,
       1,      0x11,
       4, 255,
       1,      0x29,
       4, 150,
       3,  3,  0x11, 0x35, 0x00,
       4, 150,
};


static void video_clocks_enable(void)
{
        /* DE4K clocks */
        REG32(REG_CLKGATE3) &= ~0x3f;

        /* VideoDAC Clocks */
        REG32(REG_CLKGATE2) &= ~0x700;

        /* HDMI */
        REG32(REG_CLKGATE1) &= ~BIT(19); /* HDMI_TX_PHY_REFP_CLK */
        REG32(REG_CLKGATE2) &= ~(BIT(27) | BIT(14)); /* HDMI_TX_PHY_SYS_CLK | HDMI_TX_CLK */
        REG32(REG_CLKGATE3) &= ~0x3f00000; /* HDMI Audio clocks */

        /* MISC */
        REG32(REG_CLKGATE2) &= ~BIT(0);  /* VOU_HD_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(2);  /* DE_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(3);  /* TVENC_SD_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(5);  /* TVENC_HD_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(7);  /* DE_HDMI_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(12); /* SGDMA1_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(13); /* SGDMA2_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(17); /* DVI_CLK */
        REG32(REG_CLKGATE2) &= ~BIT(0);  /* VOU_HD_EXT_CLK */

        /* VDEC and VE */
        REG32(REG_CLKGATE2) &= ~BIT(1);  /* VE_CLK */
        REG32(REG_CLKGATE1) &= ~BIT(13); /* VDEC_CLK */
}

static void fxde_init(void)
{
#if 0 /* seems like it doesn't affect the result (DSI at least) */
        REG32(DE_BASE + 0x1f04) = 0x1204860;
        REG32(DE_BASE + 0x1f0c) = 0x1204860;
        REG32(DE_BASE + 0x1f14) = 0x1204860;
        REG32(DE_BASE + 0x1f1c) = 0x1204860;
#endif
        REG32(DE_BASE + REG_DE_190) |= BIT(0);
        REG32(DE_BASE + REG_DE_19C) = 0x00;    // mp_global_k field
        REG32(DE_BASE + REG_DE_190) = (REG32(DE_BASE + REG_DE_190) & ~(BIT(24)|BIT(22)|BIT(21)|BIT(16)|BIT(4))) | BIT(23) | BIT(8);

#if 0 /* seems like it doesn't affect the result (DSI at least) */
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (0 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (0x100 << 16) | (1 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (2 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (3 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (4 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (0x100 << 16) | (5 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (6 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (0x100 << 16) | (7 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (    0 << 16) | (8 & 0x0f) | 0x10;
        REG32(DE_BASE + REG_DE_14C) = (0x100 << 16) | (9 & 0x0f) | 0x10;
#endif

        REG32(DE_BASE + REG_DE_1B8) = hactive;
        REG32(DE_BASE + REG_DE_188) = 0x000000ff; // in YCbCr black background color is 0x00108080, red 0x00525af0 but now it is in GBR888 0x00ggbbrr colors.
        REG32(DE_BASE + REG_DE_18C) = 0;

#if 0
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0x800;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0x900;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xa00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xb00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xc00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xd00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xe00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) = (REG32(DE_BASE + REG_DE_194) & ~0xf00) | 0xf00;
        REG32(DE_BASE + REG_DE_198) = (REG32(DE_BASE + REG_DE_198) & ~0x0ff) | 0x00f;
        REG32(DE_BASE + REG_DE_194) &= ~0xffe;
        REG32(DE_BASE + REG_DE_194) |= BIT(16);
        REG32(DE_BASE + REG_DE_1DC) = 0;
#endif
        REG32(DE_BASE + REG_DE_190) &= ~BIT(0);

        // irq settled here

        REG32(DE_BASE + REG_DE_CTL0) &= ~(BIT(4)|BIT(2));
        REG32(DE_BASE + REG_DE_INT) &= ~0xff;
        REG32(DE_BASE + REG_DE_ISRLINE) = (10 << 16) | (10 << 0); // (bot_start_line + 10) << 16 | (top_start_line + 10)
        REG32(DE_BASE + REG_DE_07C) = 0x00000000;
        REG32(DE_BASE + REG_DE_080) = 0x00120102;
        REG32(DE_BASE + REG_DE_084) = 0x00127102;
        REG32(DE_BASE + REG_DE_CTL0) |= BIT(4);
        REG32(DE_BASE + REG_DE_064) = 0x0037012a;
        REG32(DE_BASE + REG_DE_068) = 0x021d0089;
        REG32(DE_BASE + REG_DE_06C) = 0x000001cb;
        REG32(DE_BASE + REG_DE_TIM9) = hactive;

        // de clock setup:
        REG32(REG_SYSIO_078) = (REG32(REG_SYSIO_078) & 0xff82f0ff) | (0 << 20) | (2 << 18) | (10 << 8);

        // timings:
        REG32(DE_BASE + REG_DE_TIM0)  = (vtotal << 16) | (htotal * cpp); // 0x038204c8
        REG32(DE_BASE + REG_DE_TIM1)  = ((hsync * cpp) << 16) | (htotal * cpp); // 0x000804c8
        REG32(DE_BASE + REG_DE_TIM2)  = ((hactive + hsync + hbp) * cpp << 16) | ((hsync + hbp) * cpp - 4); // 0x04180054
        REG32(DE_BASE + REG_DE_TIM3)  = (vsync << 0x10) | vtotal; // 0x00040382
        REG32(DE_BASE + REG_DE_TIM4)  = 0x0fff0fff;
        REG32(DE_BASE + REG_DE_TIM5)  = 0x00001fff;
        REG32(DE_BASE + REG_DE_TIM6)  = ((vactive + vsync + vbp) << 16) | (vsync + vbp); //0x036e0018
        REG32(DE_BASE + REG_DE_TIM7)  = 0x0fff0fff;
        REG32(DE_BASE + REG_DE_TIM8)  = (vtotal << 16) | 0xfff; // 0x03820fff

        REG32(DE_BASE + REG_DE_CTL0) |= BIT(2);
}

static void fxde_start(void)
{
        REG32(DE_BASE + REG_DE_CTL0) |= 1;
}

static void __maybe_unused fxde_stop(void)
{
        REG32(DE_BASE + REG_DE_CTL0) &= ~1;
}

static void crossbar_connect_fxde_to_mipi_dsi_host(void)
{
        REG32(REG_VIDEO_XBAR_CTRL1) = 0x00000000;
        REG32(REG_VIDEO_XBAR_CTRL2) = 0x00500000;
        REG32(REG_VIDEO_XBAR_CTRL3) = 0x00000000;
}

static void hw_init(void)
{
        /* A collection of assorted vendor hacks with no understanding */

        // hardware_init_hook
        REG32(0xb8800038) = 0; // disable all interrupts
        REG32(0xb880003c) = 0; // disable all interrupts
        REG32(0xb8800040) &= BIT(2); // MBOX interrupt disable
        REG32(0xb8870000) &= BIT(25); // address belongs to DSC in DT, but then reassigned to another address. DSC - descrambler.

        // ddr_access_priority_init , DE only
	REG32(0xb880100c) &= ~((0x3 << 28) | (0xf << 20));
	REG32(0xb880100c) |= (0x1 << 28) | (0xA << 20);
	REG32(0xb8801010) &= ~(BIT(22) | (0x3 << 18));
	REG32(0xb8801020) = (REG32(0xb8801020) & 0xffff) | (0x4040 << 16);
	REG32(0xb8801024) = (REG32(0xb8801024) & 0xff00ffff) | (0x40 << 16);
}

int main()
{
        int err;
        uint32_t panel_id;

        /* enables UART0_TX on L12 (Volume down key) */
        REG8(REG_PINMUX_L(12)) = 0x01;

        UART0_puts("---\r\n");
        hw_init();
        usb_disable();

        UART0_puts("usb\r\n");
        video_clocks_enable();
        UART0_puts("fxde\r\n");
        fxde_init();
        UART0_puts("bl\r\n");
        backlight_init();
        UART0_puts("xbar\r\n");
        crossbar_connect_fxde_to_mipi_dsi_host();

retry:
        UART0_puts("dsi\r\n");
        mipi_dsi_host_init();
        UART0_puts("cmd\r\n");
        mipi_dsi_host_set_command_mode();

        UART0_puts("dcs reset\r\n");
        mipi_dsi_host_dcs_reset();

        err = mipi_dsi_host_dcs_set_max_ret_pkt_size(4);
        if (err < 0)
                goto retry;
        err = mipi_dsi_host_generic_short_read_1arg(0x04);
        if (err < 0)
                goto retry;

        panel_id = err;
        UART0_puts("Panel ID: 0x"); UART0_puthex32(panel_id); UART0_puts("\r\n");
        err = mipi_dsi_host_dcs_read(0x0A);
        if (err < 0)
                goto retry;
        if ((err & 0x80) != 0)
                UART0_puts("WARN: Panel is already enabled\r\n");

        UART0_puts("panel\r\n");
        if (panel_id == 0x00049500) {
                err = mipi_dsi_host_dcs_panel_init(sf3000_0x00_sequence);
        } else {
                //err = mipi_dsi_host_dcs_panel_init(sf3000_0x80_sequence);
                UART0_puts("panel id: 0x");
                UART0_puthex32(panel_id);
                UART0_puts(" not supported\r\n");
                err = -1; // not supported panel
        }

        if (err < 0)
                goto retry;

        err = mipi_dsi_host_dcs_read(0x0A);
        if (err < 0)
                goto retry;
        if ((err & 0x80) == 0)
                UART0_puts("WARN: Panel is not enabled\r\n");

        UART0_puts("video\r\n");
        mipi_dsi_host_set_video_mode();
        fxde_start();

        UART0_puts("done\r\n");

#if 1
        static const uint32_t colors[8] = {
                0x00000000,
                0x000000ff,
                0x0000ff00,
                0x0000ffff,
                0x00ff0000,
                0x00ff00ff,
                0x00ffff00,
                0x00ffffff,
        };
        uint32_t frame = 0;
        uint32_t color = 0;
        REG32(DE_BASE + REG_DE_188) = colors[0];

        while (1) {
                uint32_t isr_reg = REG32(DE_BASE + REG_DE_INT);
                if ((isr_reg >> 8) & 0x03) { // matches VSYNC IRQ rate
                        REG32(DE_BASE + REG_DE_INT) = isr_reg;
                        frame++;
                        if (frame >= 60) { // 60 fps so every second color is changing
                                color++;
                                frame = 0;
                                REG32(DE_BASE + REG_DE_188) = colors[color & 7];
                        }
                }
        }
#else
        while (1)
                ;
#endif
        return 0;
}

