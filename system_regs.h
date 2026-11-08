#ifndef SYSTEM_REGS_H
#define SYSTEM_REGS_H

#include <stdint.h>
#include <stddef.h>

#define SYSTEM_BASE          0xb8800000

/* Chip ID register:
 * [9:8]   means something like data or package num (HC15xx only)
 * [11]    means something like package num or data (HC15xx only)
 * [31:16] SoC family 0x1600 means HC16xx, otherwise HC1512 assumed.
 */
#define REG_CHIP_ID        (SYSTEM_BASE + 0x0000)

/* Scratch register:
 * used by vendor to point device tree blob location.
 */
#define REG_SCRATCH        (SYSTEM_BASE + 0x0004)

/* 0x20-0x43 Interrupt controller */

/* 0x44-0x5c GPIO-L controller */

/* Clock and reset controller */

/* REG_CLKGATE1: (1 - disable, 0 - enable)
 * [0]    - Serial flash controller clock gate.
 * [1]    - SD/MMC controller clock gate.
 * [2]    - NAND flash controller clock gate.
 * [3]    - Descramble clock gate (related to NAND?).
 * [7]    - TSG clock gate (meaning unknown).
 * [8]    - DEMUX1 clock gate (meaning unknown).
 * [9]    - DEMUX2 clock gate (meaning unknown).
 * [10]   - TS_SWITCH clock gate (meaning unknown).
 * [11]   - Ethernet MAC clock gate.
 * [12]   - SAR ADC clock gate.
 * [13]   - VDEC (Video Decoder) clock gate.
 * [14]   - DDR-PHY APB clock gate.
 * [15]   - DFI clock gate (meaning unknown).
 * [16]   - UART1 clock gate (baud clock).
 * [17]   - UART2 clock gate (baud clock).
 * [18]   - I2C1 clock gate (baud clock).
 * [19]   - HDMI_TX_PHY_REFP clock gate (meaning unknown but group is HDMI Tx).
 * [22]   - Watchdog clock gate.
 * [23]   - Timer(s) clock gate.
 * [25]   - I2C2 clock gate (baud clock).
 * [26]   - I2C3 clock gate (baud clock).
 * [27]   - I2C4 clock gate (baud clock).
 * [28]   - SB APB clock gate (South Bridge APB - many peripherals bus clock).
 * [29]   - SB 108M clock gate.
 * [30]   - SB 1843M clock gate (Likely 1.843MHz - parent for UART baud clocks).
 * [31]   - SB 27M clock gate.
 */
#define REG_CLKGATE1       (SYSTEM_BASE + 0x0060)

/* REG_CLKGATE2: (1 - disable, 0 - enable)
 * [0]    - VOU HD clock gate (Video Output Unit HD).
 * [1]    - VE clock gate (Video Engine?).
 * [2]    - DE clock gate (Display Engine, also known as FXDE, DE2K).
 * [3]    - TVENC SD clock gate (TV encoder SD, maybe known as CVBS).
 * [4]    - GE clock gate (Graphic Engine, also known as HCGE).
 * [5]    - TVENC HD clock gate (TV encoder HD).
 * [6]    - VIN clock gate (meaning unknown, likely similar to DE4K VIN but for DE).
 * [7]    - DE HDMI clock gate.
 * [8]    - VDAC1 clock gate (video DAC 1).
 * [9]    - VDAC2 clock gate (video DAC 2).
 * [10]   - VDAC3 clock gate (video DAC 3).
 * [11]   - UART3 and UART4 clock gate (baud clock).
 * [12]   - SGDMA1 clock gate (Scatter-Gather DMA).
 * [13]   - SGDMA2 clock gate.
 * [14]   - HDMI TX clock gate (likely controller core clock).
 * [15]   - AUDIO clock gate (meaning unknown, is it common for all I2S/PCM/PDM/TDM).
 * [16]   - SPI Master clock gate (baud clock).
 * [17]   - DVI clock gate.
 * [18]   - VOU HD EXT clock gate (Video Output Unit HD, EXT purpose unknown).
 * [20]   - SPO clock gate (meaning unknown, sound related).
 * [21]   - DDP SPO clock gate (meaning unknown, sound related).
 * [22]   - TSDMA clock gate (purpose of this DMA is unknown).
 * [23]   - JENC clock gate (JPEG encoder).
 * [24]   - USB0 controller clock gate.
 * [25]   - USB1 controller clock gate.
 * [27]   - HDMI TX PHY SYS clock gate (meaning unknown but group is HDMI Tx).
 * [28]   - MIPI clock gate (MIPI-DSI).
 * [29]   - BootROM clock gate.
 * [30]   - TVDEC clock gate (TV decoder).
 * [31]   - CPU1 clock gate (likely Secondary CPU).
 */
#define REG_CLKGATE2       (SYSTEM_BASE + 0x0064)

/* REG_SCRATCH2: (8-bit width)
 * Used by vendor software to bootstrap Secondary CPU.
 * Likely some flags or status byte but might be clock or reset bits.
 */
#define REG_SCRATCH2       (SYSTEM_BASE + 0x006B)

/* REG_SYSIO_06C: (Likely HDMI Rx related)
 * [0]     - audio_i2sin_source_select
 * [1]     - i2sin0_source_select
 * [2]     - i2sin1_source_select
 * [3]     - i2sin2_source_select
 * [4]     - i2so_mclk_source_select
 * [5]     - i2so_bclk_source_select
 * [6]     - i2so_lrclk_source_select
 * [7]     - i2so_data0_source_select
 * [8]     - i2so_data1_source_select
 * [9]     - i2so_data2_source_select
 * [10]    - i2so_data3_source_select
 * [13:11] - spo_data_source_select
 * [16:14] - ddpulse_spo_data_source_select
 * [20]    - looks like clock gate or reset bit for something HDMI related.
 *           Set to 0 when DE2K clocks setup.
 *           Set to 1 when DE4K clocks setup.
 */
#define REG_SYSIO_06C      (SYSTEM_BASE + 0x006C)

/* REG_SYSIO_070:
 * [6:4]  - currently active DDR frequency selector, likely readonly.
 *          0b000 - 800MHz
 *          0b001 - 1066MHz
 *          0b010 - 1333MHz
 *          0b011 - 1600MHz
 *          0b100 - 600MHz (not confirmed for HC16xx)
 *          NB: frequency superseded by DDRPLL enable bit REG_DDRPLL[15].
 * [10:8] - currently active Main CPU frequency selector, likely readonly.
 *          0b000 - 594MHz
 *          0b001 - 396MHz
 *          0b010 - 297MHz
 *          0b011 - 198MHz
 *          0b100 - 900MHz
 *          0b101 - 1188MHz
 *          0b110 - 24MHz
 *          0b111 - PLL clock selected, PLL register TBD.
 */
#define REG_SYSIO_070      (SYSTEM_BASE + 0x0070)

/* REG_SYSIO_074: (8-bit width)
 * [7:0] - DDR frequency selector setter.
 *         0x00 - 800MHz
 *         0x10 - 1066MHz
 *         0x20 - 1333MHz
 *         0x30 - 1600MHz
 *         0x40 - 600MHz (not confirmed for HC16xx)
 *         NB: frequency selector superseded by DDRPLL enable bit REG_SYSIO_48a[15].
 *         NB: to apply changes set REG_SYSIO_076 to 0x20.
 *
 * REG_SYSIO_074: (32-bit width)
 * [7:0]  - seems same as for 8-bit register.
 * [10:8] - Main CPU frequency selector setter.
 *          0b000 - 594MHz
 *          0b001 - 396MHz
 *          0b010 - 297MHz
 *          0b011 - 198MHz
 *          0b100 - 900MHz
 *          0b101 - 1188MHz
 *          0b110 - 24MHz
 *          0b111 - PLL clock selected, PLL register REG_MCPUPLL.
 *          NB: if PLL is selected bit REG_SYSIO_07C[7] must be set to 1.
 *          NB: to apply changes set REG_SYSIO_074[22] to 1.
 * [21]   - DDR frequency selector applier (using 32-bit access not verified).
 *          seems same as for 8-bit REG_SYSIO_076 register.
 * [22]   - Main CPU and Secondary CPU frequency selector applier.
 */
#define REG_SYSIO_074      (SYSTEM_BASE + 0x0074)

/* REG_SYSIO_076: (8-bit width)
 * [7:0] - DDR frequency selector applier.
 *         to apply changes in REG_SYSIO_074 write to this register value 0x20.
 */
#define REG_SYSIO_076      (SYSTEM_BASE + 0x0076)

/* REG_SYSIO_078:
 * [3]    - MIPI clock (TODO figure out gate or selector?)
 * [11:8] - DE HDMI clock selector
 *          0x00 - 27MHz
 *          0x01 - 54MHz
 *          0x02 - 74.25MHz
 *          0x03 - 108MHz
 *          0x04 - 132MHz
 *          0x05 - 148.5MHz
 *          0x06 - ???
 *          0x07 - PLL clock from REG_DEPLL
 *          0x08 - 6.6MHz
 *          0x09 - 9MHz
 *          0x0a - 33MHz
 *          0x0b - 39.6MHz
 *          0x0c - 49MHz
 *          0x0d - 66MHz
 *          0x0e - 85MHz
 *          0x0f - ???
 *          NB: 297MHz seems not supported, present in enum however.
 * [16]   - VIN clock (TODO figure out gate or selector?)
 * [17]   - VIN clock polarity? (vin_clk_inv)
 * [19:18]- DE clock selector
 *          0x00 - 170MHz
 *          0x01 - 148MHz
 *          0x02 - 198MHz
 *          0x03 - 225MHz
 *          enum has 297,300 and PLL in addition, but these value are outside the mask.
 * [22:20]- DE to HD TVE clock selector
 *          0x00 - 27MHz
 *          0x01 - 54MHz
 *          0x02 - 108MHz
 *          0x03 - 132MHz
 *          0x04 - 148MHz
 *          0x05 - 170MHz
 *          0x06 - 216MHz
 *          0x07 - 297MHz
 * [27]   - DE HDMI clock polarity? (de_hdmi_clk_inv)
 * [29:28]- MIPI Rx clock selector (MIPI CSI or DSI rx path?)
 *          0x00 - ???
 *          0x01 - ???
 *          0x02 - ???
 *          0x03 - ???
 * [31:30]- NAND flash clock selector
 *          0x00 - ???
 *          0x01 - ???
 *          0x02 - ???
 *          0x03 - ???
 */
#define REG_SYSIO_078      (SYSTEM_BASE + 0x0078)

/* REG_SYSIO_07C:
 * [7]     - Must be set to 1 when PLL selected by REG_SYSIO_074[10:8]
 *           likely it enables the Main CPU PLL.
 * [21:20] - SD/MMC source clock selector:
 *           0x00 - 49MHz (likely actual is 49.5)
 *           0x01 - 99MHz
 *           0x02 - 150MHz (likely actual is 148.5)
 *           0x03 - 198MHz
 */
#define REG_SYSIO_07C      (SYSTEM_BASE + 0x007C)

/* REG_RESET1: (1 - reset, 0 - release)
 * [1]    - VDEC or VE reset (Video Decoder/Video Engine).
 * [4]    - GE reset (Graphic Engine, known as HCGE)
 * [5]    - audio reset (what exactly it resets?)
 * [9]    - UART3 reset
 * [10]   - UART4 reset
 * [13]   - HDMI Tx reset #1 (4 reset bits belong to HDMI Tx, which is which?)
 * [14]   - SPI Master reset
 * [15]   - audio clock reset (what? PLL and divisors reset?)
 * [16]   - UART1 reset
 * [17]   - UART2 reset
 * [23]   - Watchdog reset
 * [28]   - USB0 controller reset
 * [30]   - MIPI-DSI host controller reset
 * NB: rest is not identified yet
 */
#define REG_RESET1         (SYSTEM_BASE + 0x0080)

/* REG_RESET2: (1 - reset, 0 - release)
 * [5]    - HDMI Tx reset #2 (4 reset bits belong to HDMI Tx, which is which?)
 * [6]    - HDMI Tx reset #3 (4 reset bits belong to HDMI Tx, which is which?)
 * [7]    - MIPI-DSI PHY reset
 * [8]    - LVDS controller reset
 * [12]   - HDMI Rx I2C reset
 * [13]   - HDMI Tx reset #4 (4 reset bits belong to HDMI Tx, which is which?)
 * [29]   - USB1 controller reset
 * [31]   - VE or VDEC reset (Video Engine/Video Decoder).
 * NB: rest is not identified yet
 */
#define REG_RESET2         (SYSTEM_BASE + 0x0084)

/* REG_CLKGATE3: (1 - disable, 0 - enable)
 * [0]    - DE4K HDMI clock gate (DE4K to HDMI related).
 * [1]    - DE4K EXTO clock gate (purpose unknown).
 * [2]    - DE4K clock gate (likely core clock).
 * [3]    - DE4K DVI clock gate (DE4K to DVI related).
 * [4]    - DE4K VIN clock gate (Video input to DE4K likely).
 * [5]    - DE4K VOU HD clock gate (DE4K to Video output HD related).
 * [7]    - LVDS CH1 PIXEL clock gate.
 * [8]    - LVDS clock gate (likely core clock).
 * [9]    - VID HDMI clock gate (purpose unclear).
 * [10]   - VID CVBS clock gate (purpose unclear).
 * [11]   - HRX clock gate (HDMI Rx).
 * [12]   - HRX IAUD clock gate (HDMI Rx audio related).
 * [13]   - RGB clock gate (RGB parallel output interface).
 * [14]   - PQ clock gate (picture quality).
 * [16]   - AUD EXTI 1 BCLK P1 clock gate (purpose unknown, audio related).
 * [17]   - AUD EXTI SP CLK P1 clock gate (purpose unknown, audio related).
 * [18]   - AUD EXTO 1 MCLK P0 clock gate (purpose unknown, audio related).
 * [19]   - AUD EXTO SP CLK P0 clock gate (purpose unknown, audio related).
 * [20]   - AUD EXTO HDMI ADAC 1 BCLK P0 clock gate (purpose unknown, sound related).
 * [21]   - AUD EXTO HDMI ADAC 1 BCLK P1 clock gate (purpose unknown, sound related).
 * [22]   - AUD EXTO HDMI ADAC 1 BCLK P2 clock gate (purpose unknown, sound related).
 * [23]   - AUD HDMI SP CLK P0 clock gate (purpose unknown, audio related).
 * [24]   - AUD HDMI SP CLK P1 clock gate (purpose unknown, audio related).
 * [25]   - HDMI F128 LR clock gate (purpose unknown, audio related).
 * [26]   - AUD PCMO BCLK clock gate (purpose unknown, audio related).
 */
#define REG_CLKGATE3       (SYSTEM_BASE + 0x008C)

/* REG_STRAP_PIN:
 * [16]    - enables/disables (0/1) eJTAG, supersedes any pinctrl/pinmux.
 * [17]    - enables/disables (0/1) SFLASH CS/CLK/MOSI/MISO pins, supersedes any pinctrl/pinmux.
 * [18]    - enables/disables (0/1) UART0 strap pins (D3100:T22-23) (A3x00:?) (C3100:?) (A5100:?) (C3000/D3000:L17-18) (C5200:B08-09)
 * [19]    - enables/disables (0/1) SFLASH CSJ strap pin (which of?)
 * [20]    - enables/disables (0/1) SFLASH HOLDJ (D3100: T14 or T19)
 * [24]    - enables/disables (0/1) SFLASH WJ (A5100:one of T15-18) (D3100:T14 or T19)
 * [31:28] - controls LVDS pins function, applied only when TTL mode selected in LVDS PHY (LVDS_MODE_EN register).
 *           0x00 - Parallel RGB 888
 *           0x00 - Parallel RGB 666
 *           0x02 - Parallel RGB 565
 *           0x03 - I2SO
 *           0x05 - GPIO (push-pull output only, controller register REG_SYSIO_174).
 */
#define REG_STRAP_PIN       (SYSTEM_BASE + 0x0094)

/* REG_SYSIO_09C:
 * [3:0]  - DE4K HDMI clock selector:
 *          0x00 - 27MHz
 *          0x01 - 54MHz
 *          0x02 - 74.25MHz
 *          0x03 - 148.5MHz
 *          0x04 - 297MHz
 *          0x05 - 132MHz
 *          0x06 - PLL clock from REG_DEPLL
 *          0x07 - ???
 *          0x08 - 6.6MHz
 *          0x09 - 9MHz
 *          0x0a - 33MHz
 *          0x0b - 49MHz
 *          0x0c - 66MHz
 *          0x0d - 108MHz
 *          0x0e - 85MHz
 *          0x0f - 39.6MHz
 *
 * [10:8] - Secondary CPU frequency selector setter (maybe getter too).
 *          0b000 - 594MHz
 *          0b001 - 450MHz
 *          0b010 - 396MHz
 *          0b011 - 297MHz
 *          0b100 - 198MHz
 *          0b101 - 24MHz
 *          0b110 - 800MHz
 *          0b111 - PLL clock selected, PLL register REG_SCPUPLL.
 *          NB: if PLL is selected bit REG_SYSIO_09C[19] must be set to 1.
 *          NB: to apply changes set REG_SYSIO_074[22] to 1.
 *
 * [18:16]- DE4K clock selector:
 *          0x00 - 170MHz
 *          0x01 - 148MHz
 *          0x02 - 198MHz
 *          0x03 - 225MHz
 *          0x04 - 297MHz
 *          0x05 - 300MHz
 *          0x06 - PLL clock from REG_DEPLL
 *          0x07 - ???
 * [19]   - Must be set to 1 when PLL selected by REG_SYSIO_09C[10:8]
 *          likely it enables the Secondary CPU PLL.
 * [31:23]- HDMI Rx In Audio clock (selector? divisor? complex field?)
 */
#define REG_SYSIO_09C      (SYSTEM_BASE + 0x009C)

/* 0xC4-0xFF GPIO-B and R controllers */

/* REG_SYSIO_148:
 * [9:0]   - aud_exto_hdmi_adac_1_bclk_div
 * [15:10] - aud_dac_clk_div
 * [22:16] - aud_adac_d2a_clk_div
 * [31:23] - aud_exto_1_mclk_div
 */
#define REG_SYSIO_148      (SYSTEM_BASE + 0x0148)

/* REG_SYSIO_14C:
 * [15:8]  - hdmi_f128_lc_clk_div
 * [23:16] - aud_exto_sp_clk_div
 * [31:24] - aud_hdmi_sp_clk_div
 */
#define REG_SYSIO_14C      (SYSTEM_BASE + 0x014C)

/* REG_SYSIO_154:
 * [14]    - aud_exto_adac
 * [17]    - aud_exti_adac
 * [21:20] - aud_exti_1_bclk_sel
 * [31]    - clk_set_31
 */
#define REG_SYSIO_154      (SYSTEM_BASE + 0x0154)

/* 0x160-0x173 SAR ADC wait registers, see `adc_wait_reg_t` */


/* REG_SYSIO_174:
 * LVDS-as-GPIO output level register.
 * Pins to bits mapping isn't known and depends on chip revision.
 * E3100 doesn't support LVDS.
 * See REG_STRAP_PIN on how to switch LVDS pins into GPIO mode.
 */
#define REG_SYSIO_174      (SYSTEM_BASE + 0x0174)

/* REG_SYSIO_180:
 * [15:8]  - CVBS-in training value (0..255), purpose unclear, libviddrv supports training ioctl.
 * [24]    - CVBS related, set to 1 for standby
 * [25]    - CVBS related, set to 1 for standby
 * [26]    - CVBS related, set to 0 for standby
 * [27]    - CVBS related, set to 1 for standby
 * [28]    - CVBS related, set to 1 for standby
 * [29]    - CVBS related, set to 1 for standby
 */
#define REG_SYSIO_180      (SYSTEM_BASE + 0x0180)

/* REG_SYSIO_184:
 * [23]    - SD/MMC related, set to 0 for standby
 * [25:24] - SD/MMC IO voltage
 *           0b00 - ?
 *           0b01 - 3V3
 *           0b10 - 1V8
 * [29:28] - SD/MMC IO voltage #2
 *           set to 0b01 when entering 1V8, not modified when returning to 3V3
 */
#define REG_SYSIO_184      (SYSTEM_BASE + 0x0184)

/* REG_SYSIO_200:
 * [16]    - set to 1 for standby, purpose unknown
 */
#define REG_SYSIO_200      (SYSTEM_BASE + 0x0200)

/* REG_SYSIO_220:
 * [1]     - Set to 0 and 1 by SCPU during bootup, purpose unknown.
 * [24]    - Configures DRAM mapping for 256Mb.
 *           If set then SFLASH XIP window placed at 0xbf000000.
 *           If cleared (default) then SFLASH XIP window placed at 0xafc00000.
 *           Effective DRAM amount limited to 192Mb when cleared and to ? when set.
 */
#define REG_SYSIO_220      (SYSTEM_BASE + 0x0220)

/* REG_SYSIO_28x:
 * Configured by MCPU to bootstrap SCPU.
 * Likely related to entry point setup and reset.
 */
#define REG_SYSIO_280      (SYSTEM_BASE + 0x0280)
#define REG_SYSIO_284      (SYSTEM_BASE + 0x0284)
#define REG_SYSIO_288      (SYSTEM_BASE + 0x0288)
#define REG_SYSIO_28C      (SYSTEM_BASE + 0x028C)

/* 0x344-0x35F GPIO-T controller */

/* REG_MCPUPLL:
 * [5:0]   - Main CPU PLL factor L.
 * [13:8]  - Main CPU PLL factor N.
 * [25:16] - Main CPU PLL factor M.
 * [30]    - Standby bit, exact behavior is obscure.
 * [31]    - Likely Enable the PLL.
 * PLL frequency = 24MHz * (M+1) / (N+1) / (L+1).
 * Typically (N + 1) * (L + 1) == 10, not sure which one is 10.
 * NB: see REG_SYSIO_074 on how to select PLL as source clock.
 */
#define REG_MCPUPLL        (SYSTEM_BASE + 0x0380)

/* REG_DEPLL:
 * [25:16] - DE PLL factor M.
 * [30]    - Standby bit, exact behavior is obscure.
 * [31]    - Likely Enable the PLL.
 * PLL frequency = 24MHz * (M+1) / 10.
 * 10 as divisor might show that there's typical N and L factors in lower bits.
 */
#define REG_DEPLL          (SYSTEM_BASE + 0x0390)

/* REG_PLL_3A0:
 * [30]    - Standby bit, exact behavior is obscure.
 * [31]    - Likely Enable the PLL.
 */
#define REG_PLL_3A0        (SYSTEM_BASE + 0x03A0)

/* REG_SCPUPLL:
 * [5:0]   - Secondary CPU PLL factor L.
 * [13:8]  - Secondary CPU PLL factor N.
 * [25:16] - Secondary CPU PLL factor M.
 * [30]    - Standby bit, exact behavior is obscure.
 * [31]    - Likely Enable the PLL.
 * PLL frequency = 24MHz * (M+1) / (N+1) / (L+1)
 * Typically (N + 1) * (L + 1) == 10, not sure which one is 10.
 * NB: see REG_SYSIO_09C on how to select PLL as source clock.
 */
#define REG_SCPUPLL        (SYSTEM_BASE + 0x03B0)

/* REG_SYSIO_3C8:
 * [2:0]   - VDAC1 output clock selector.
 *           0x00 - 27MHz
 *           0x01 - 54MHz
 *           0x02 - 108MHz
 *           0x03 - 132MHz
 *           0x04 - 148.5MHz
 *           0x05 - 170MHz
 *           0x06 - 216MHz
 *           0x07 - 297MHz
 * [6:4]   - VDAC2 output clock selector.
 * [10:8]  - VDAC3 output clock selector.
 * [12]    - TVE HD SD selector. (purpose undefined, values unknown, not confirmed for HC16xx)
 * [18:16] - PRGB VGA clock selector: (used by HC1512 only, not confirmed for HC16xx)
 *           0x00 - 6.6MHz
 *           0x01 - 9MHz
 *           0x02 - 33MHz
 *           0x03 - 49MHz
 *           0x04 - 66MHz
 *           0x05 - 74MHz
 *           0x06 - 85MHz
 *           0x07 - 39.6MHz
 *           0x08 - 108MHz
 *           0x09 - 54MHz
 *           0x0a - 132MHz
 */
#define REG_SYSIO_3C8        (SYSTEM_BASE + 0x03C8)

/* REG_SYSIO_410:
 * [5:0]   - used by MMC calibration/tuning, delay parameter.
 * [?:6]   - used by MMC calibration/tuning, nineties parameter.
 */
#define REG_SYSIO_410      (SYSTEM_BASE + 0x0410)

/* REG_SYSIO_440:
 * [0] - set to 0 when enter standby. LVDS CH0 related.
 * [1] - set to 0 when enter standby. LVDS CH1 related.
 * [2] - set to 1 when enter standby. LVDS both channels related.
 */
#define REG_SYSIO_440      (SYSTEM_BASE + 0x0440)

/* REG_AUDIOPLL1:
 * [0]     - audio_pll_foutvcopd_core
 * [1]     - audio_pll_fout4phasepd_core
 * [2]     - audio_pll_foutpostdivpd_core
 * [3]     - audio_pll_dsmpd_core, set to 1 when enter standby.
 * [4]     - audio_pll_dacpd_core, set to 1 when enter standby.
 * [5]     - audio_pll_pd_core, set to 1 when enter standby.
 * [19:16] - audio_pll_postdiv1_core
 * [23:20] - audio_pll_postdiv2_core
 */
#define REG_AUDIOPLL1      (SYSTEM_BASE + 0x0470)

/* REG_AUDIOPLL2:
 * [23:0]  - audio_pll_frac_core
 */
#define REG_AUDIOPLL2      (SYSTEM_BASE + 0x0474)

/* REG_AUDIOPLL3:
 * [15:0]  - audio_pll_refdiv_core
 * [31:16] - audio_pll_fbdiv_core
 */
#define REG_AUDIOPLL3      (SYSTEM_BASE + 0x0478)

/* REG_DDRPLL: (16-bit width)
 * [14:0] - DDR PLL factor M. This is the frequency on which DDR will operate.
 *          effective DDR2/3 frequency for HC16xx = M * 24MHz.
 * [15]   - Enable PLL, if enabled supersedes frequency selector in REG_SYSIO_070/REG_SYSIO_074.
 * TODO:    Verify that factors N and L are also present here at address 0x488.
 */
#define REG_DDRPLL         (SYSTEM_BASE + 0x048A)

/* REG_SYSIO_490:
 * [5] - set to enter standby, cleared on exit,
 *       it helps restore clocks as they were at cold boot.
 */
#define REG_SYSIO_490      (SYSTEM_BASE + 0x0490)

/* Pinmux registers, 8-bit wide each.
 * There are 4 banks for Left,Botton,Right,Top pins and a separate LVDS bank.
 * Let's call these banks L,B,R,T,LVDS respectively.
 * Each register corresponds to one pin.
 * e.g. register at offset 9 in the bank L controls mux of pin L09.
 * Gap between banks gives opportunity to place up to 64 pinmuxes
 * in a bank, but basically first 32 registers of the bank are utilized.
 * Also the pin map is sparse.
 * [3:0] - LBRT mux settings, each pin has it's own set of muxes, see vendor's hc16xx_pinmux.h
 * [7:4] - drive strength, confirmed only for SDMMC pins.
 *         Vendor code uses maximum strength (0x0f) when these pins used as GPIOs.
 * LVDS bits aren't recovered yet.
 */
#define REG_PINMUX_L(pin)    (SYSTEM_BASE + 0x04A0 + (pin))
#define REG_PINMUX_B(pin)    (SYSTEM_BASE + 0x04E0 + (pin))
#define REG_PINMUX_R(pin)    (SYSTEM_BASE + 0x0520 + (pin))
#define REG_PINMUX_T(pin)    (SYSTEM_BASE + 0x0560 + (pin))
#define REG_PINMUX_LVDS(pin) (SYSTEM_BASE + 0x0580 + (pin))

/**
 * Recovered crossbar architecture:
 *      sources                         sinks
 *                  +----+
 *      [FXDE]----->|    |------------->[HDMItx]
 *      [4KDE]----->|    |------------->[MIPI]
 *    [HDMIrx]----->| XX |------------->[LVDS]
 *        [PQ]----->|    |--->[RGBm]--->[RGB]
 *                  +----+
 *                  +----+
 *      [FXDE]----->|    |
 *      [4KDE]----->| M  |--->[PQ]
 *      [HDMIrx]--->|    |
 *                  +----+
 *
 * Where:
 * - |XX| means crossbar (XBAR) logical block function.
 *        The crossbar core represents four 4-to-1 muxes.
 * - |M| means 3-to-1 mux, part of crossbar.
 * - [RGBm] means color components (re-)mapper for RGB interface, part of crossbar.
 * - [PQ] picture quality block, acts as both source and sink for crossbar.
 *        It can adjust gamma of input source on the fly, maybe has more algos.
 * - [FXDE] - one of display engines - present in HC15xx and HC16xx.
 * - [4KDE] - one of display engines - absent in HC15xx.
 *            Might be able to provide 4K resolution while FXDE don't.
 * - [HDMIrx] - HDMI-receiver.
 * - [HDMItx] - HDMI-transmitter.
 * - [MIPI] - MIPI DSI host controller
 * - [LVDS] - LVDS display output
 * - [RGB]  - Parallel display output (DPI)
 *
 *
 * VIDEO_XBAR_CTRL1:
 *  [ 9: 8]  LVDS source #0*    (0:FXDE 1:4KDE 2:HDMI 3:?)
 *  [13:12]  LVDS source #1*    (0:FXDE 1:4KDE 2:HDMI 3:?)
 *  [19:16]  MIPI DSI source #0 (0:FXDE 1:4KDE 2-15:?) *****
 *  [21:20]  PQ clock source    (0:FXDE 1:4KDE 2:?    3:?)
 *
 * VIDEO_XBAR_CTRL2:
 *  [ 1: 0]  HDMI Tx source     (0:?    1:?    2:?    3:PQ)
 *  [ 5: 4]  PQ data source**   (0:FXDE 1:4KDE 2:HDMI 3:?)
 *  [13:12]  LVDS source #2     (0:FXDE 1:4KDE 2:HDMI 3:PQ)
 *  [15:14]  LVDS source #3     (0:FXDE 1:4KDE 2:HDMI 3:PQ)
 *  [17:16]  MIPI DSI source #1 (0:FXDE 1:4KDE 2:HDMI 3:PQ)
 *  [19:18]  RGB DPI source     (0:FXDE 1:4KDE 2:HDMI 3:PQ)
 *  [22:20]  MIPI DSI format    (0:RGB565_0 1:RGB565_1 2:RGB565_2 3:RGB666_0 4:RGB666_1 5:RGB888 6-7:?)
 *
 * VIDEO_XBAR_CTRL3:
 *  [ 2: 0]  RGB color X source*** (0:FXDE 1:4KDE 2:HDMI 3:FXDE 4:FXDE 5:4KDE 6:HDMI 7:PQ)
 *  [ 6: 4]  RGB color Y source*** (0:FXDE 1:4KDE 2:HDMI 3:FXDE 4:FXDE 5:4KDE 6:HDMI 7:PQ)
 *  [10: 8]  RGB color Z source*** (0:FXDE 1:4KDE 2:HDMI 3:FXDE 4:FXDE 5:4KDE 6:HDMI 7:PQ)
 *  [19:16]  G channel selector    (0:Green 1:Blue  2:Red   3-15:?)
 *  [23:20]  B channel selector    (0:Blue  1:Red   2:Green 3-15:?)
 *  [27:24]  R channel selector    (0:Red   1:Green 2:Blue  3-15:?)
 *  [28]     R channel high-to-low****  (0:? 1:?)
 *  [29]     ? channel high-to-low****  (0:? 1:?)
 *  [30]     ? channel high-to-low****  (0:? 1:?)
 * [*]     LVDS fields kept in sync by lvds_hal but it doesn't use PQ as source; whereas pq_tool doesn't touch CTRL1 reg LVDS fields.
 *         So all 4 sources proven for CTRL2 LVDS fields but not for CTRL1.
 * [**]    RGB selector code also sets this field to same value as for RGB source: CTRL2[19:18].
 * [***]   X Y Z maps to R G B colors somehow. Values 4-7 labelled by vendor as low-to-high or high-to-low - meaning unknown but might
 *         be related to 10-to-8 or 8-to-10 bit conversion, although no evidence of 10-bit support by SoC.
 *         Value 3 marked as FXDE in vendor code but it remains unused by vendor, so better to avoid.
 * [****]  These bits are only observed being manipulated by pq_tool when PQ is selected for RGB.
 *         Bit 28 appears to correspond to R. Bits 29/30 are likely related to G/B, but vendor code does not establish which is which.
 * [*****] Mipi driver sets two fields in CTRL2 [19:16] as a single field. Same for CTRL1 [19:16] but in CTRL1 upper bits aren't known.
 *         It selects FXDE or 4KDE only, effectively resetting RGB DPI source CTRL2[19:18] to FXDE path.
 *
 * SF3000 register values from original FW readback:
 * CTRL1: 0x00010000
 * CTRL2: 0x00510001
 * CTRL3: 0x00000000
 */
#define REG_VIDEO_XBAR_CTRL1 (SYSTEM_BASE + 0x0444)
#define REG_VIDEO_XBAR_CTRL2 (SYSTEM_BASE + 0x0448)
#define REG_VIDEO_XBAR_CTRL3 (SYSTEM_BASE + 0x044c)

#endif
