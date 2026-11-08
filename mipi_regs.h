#ifndef MIPI_REGS_H
#define MIPI_REGS_H

#include <stdint.h>
#include <stddef.h>

#include "system_regs.h"

#define MIPI_CTRL_BASE      0xb884a000

#define MIPI_CTRL_BASE       0xb884a000
#define REG_MIPI_DSI_REG_04  (MIPI_CTRL_BASE + 0x0004)
#define REG_MIPI_DSI_REG_08  (MIPI_CTRL_BASE + 0x0008)
#define REG_MIPI_DSI_REG_0C  (MIPI_CTRL_BASE + 0x000c)
#define REG_MIPI_DSI_REG_10  (MIPI_CTRL_BASE + 0x0010)
#define REG_MIPI_DSI_REG_14  (MIPI_CTRL_BASE + 0x0014)
#define REG_MIPI_DSI_PCKHDL  (MIPI_CTRL_BASE + 0x0018)
#define REG_MIPI_DSI_FLAGS   (MIPI_CTRL_BASE + 0x001C)
#define REG_MIPI_DSI_REG_20  (MIPI_CTRL_BASE + 0x0020)
#define REG_MIPI_DSI_REG_24  (MIPI_CTRL_BASE + 0x0024)
#define REG_MIPI_DSI_REG_28  (MIPI_CTRL_BASE + 0x0028)
#define REG_MIPI_DSI_REG_2C  (MIPI_CTRL_BASE + 0x002c)
#define REG_MIPI_DSI_REG_30  (MIPI_CTRL_BASE + 0x0030)
#define REG_MIPI_DSI_REG_34  (MIPI_CTRL_BASE + 0x0034)
#define REG_MIPI_DSI_REG_38  (MIPI_CTRL_BASE + 0x0038)
#define REG_MIPI_DSI_REG_3C  (MIPI_CTRL_BASE + 0x003c)
#define REG_MIPI_DSI_REG_48  (MIPI_CTRL_BASE + 0x0048)
#define REG_MIPI_DSI_REG_54  (MIPI_CTRL_BASE + 0x0054)
#define REG_MIPI_DSI_REG_58  (MIPI_CTRL_BASE + 0x0058)
#define REG_MIPI_DSI_REG_5C  (MIPI_CTRL_BASE + 0x005c)
#define REG_MIPI_DSI_REG_60  (MIPI_CTRL_BASE + 0x0060)

#define REG_MIPI_DSI_PHY_100 (MIPI_CTRL_BASE + 0x0100)
#define REG_MIPI_DSI_PHY_104 (MIPI_CTRL_BASE + 0x0104)
#define REG_MIPI_DSI_PHY_108 (MIPI_CTRL_BASE + 0x0108)
#define REG_MIPI_DSI_PHY_120 (MIPI_CTRL_BASE + 0x0120)

#define REG_MIPI_DSI_LANE_BASE (MIPI_CTRL_BASE + 0x0140)
#define MIPI_DSI_LANE_CLK  (0)
#define MIPI_DSI_LANE_DAT0 (1)
#define MIPI_DSI_LANE_DAT1 (2)
#define MIPI_DSI_LANE_DAT2 (3)
#define MIPI_DSI_LANE_DAT3 (4)
#define MIPI_DSI_LANE_REG_00 (0x00)
#define MIPI_DSI_LANE_REG_04 (0x04)
#define MIPI_DSI_LANE_REG_08 (0x08)
#define MIPI_DSI_LANE_REG_0C (0x0c)
#define MIPI_DSI_LANE_REG_10 (0x10)
#define REG_MIPI_LANE(lane,reg) (REG_MIPI_DSI_LANE_BASE + (lane) * 0x20 + (reg))

#define REG_MIPI_DSI_REG_200 (MIPI_CTRL_BASE + 0x0200)
#define REG_MIPI_DSI_REG_218 (MIPI_CTRL_BASE + 0x0218)

/*
 * Recovered MIPI registers:
 * REG_04:
 *       [0]: power enable DSI host controller, 1 - enable
 *
 * REG_08:
 *     [7:0]: uncovered field, likely control field, set to 0x07 at init.
 *    [15:8]: uncovered field, likely control field, set to 0x01 at init.
 *
 * REG_0C:
 *     [1:0]: uncovered DPI config parameter, vendor sets it to 0 always.
 *     [4:2]: DSI format for DPI bus (likely input for DSI host) same as for VIDEO_XBAR_CTRL2[22:20]
 *       [5]: uncovered DPI config parameter, vendor sets it to 0 always.
 *       [6]: set to 1 if DPI vsync is active high (change polarity?)
 *       [7]: set to 1 if DPI hsync is active high
 *       [8]: uncovered DPI config parameter, vendor sets it to 0 always.
 *       [9]: uncovered DPI config parameter, vendor sets it to 0 always.
 *      [10]: set to 1 for RGB666 DSI formats
 *
 * REG_10:
 *     [1:0]: uncovered DBI config parameter, vendor sets it to 1 always.
 *     [5:2]: uncovered DBI config parameter, vendor sets it to 0 always.
 *     [7:6]: uncovered DBI config parameter, vendor sets it to 1 always.
 *       [8]: uncovered DBI config parameter, vendor sets it to 0 always.
 *    [12:9]: uncovered DBI config parameter, vendor sets it to 0 always.
 *
 * REG_14:
 *    [15:0]: configures command size, vendor sets it to 26
 *   [31:16]: configures command size, vendor sets it to 10
 *
 * PCKHDL:
 *       [0]: Enables EOTp transmission.
 *       [1]: Enables EOTp reception.
 *       [2]: MIPI_Bus_Turn_request.
 *       [3]: Enables ECC reception, error correction and reporting
 *       [4]: Enables CRC reception and error reporting.
 *     [6:5]: Generic interface read-back Virtual Channel identification.
 *   default value: 0x1c vendor provides it via "dsi,cfg" dt prop.
 *
 * FLAGS:
 *       [0]: Enables DPI Video mode transmission
 *     [2:1]: Selects video mode transmission type. 0: Non-burst with Sync pulses; 1: Non-burst with Sync events; 2-3: Burst with Sync pulses.
 *       [3]: Enables return to Low Power inside VSA period when timing allows
 *       [4]: Enables return to Low Power inside VBP period when timing allows
 *       [5]: Enables return to Low Power inside VFP period when timing allows
 *       [6]: Enables return to Low Power inside VACT period when timing allows
 *       [7]: Enables return to Low Power inside HBP period when timing allows
 *       [8]: Enables return to Low Power inside HFP period when timing allows
 *       [9]: Enables the transmission of multi video packets in the HACT period
 *      [10]: Enables the transmission of null packets in the HACT period
 *      [11]: Enables the request for an acknowledge response at the end of a frame
 *      vendor's "dsi,flags" dt property defines defaults to entire register. Actual default is 0x3fd
 *
 * REG_20:
 *    [10:0]: hactive
 *   [20:11]: uncovered field, set to 0 at init
 *   [30:21]: uncovered field, set to 0 at init
 *
 * REG_24:
 *    [15:0]: command or data mode. 0x1fff: command mode 0x4000: data mode
 *
 * REG_28:
 *     [8:0]: hsync in byte clocks: hsync * bit_period / byte_period
 *    [17:9]: hbp in byte clocks
 *   [31:18]: htotal in byte clocks
 *
 * REG_2C:
 *     [3:0]: vsync
 *     [9:4]: vbp
 *   [15:10]: vfp
 *   [26:16]: vactive
 *
 * REG_30:
 *    [11:0]: uncovered field, set to 0x000 at init.
 *   [19:12]: uncovered field, set to 0x28 at init.
 *   [27:20]: uncovered field, set to 0x28 at init.
 *
 * REG_34:
 *    [23:0]: dcs header, write to start command. [7:0] - data identifier, [15:8] - data0, [23:16] - data1
 *   [31:24]: reserved, write zero. ECC is calculated in hardware.
 *
 * REG_38:
 *    [31:0]: dcs FIFO, depth unknown.
 *            for read operation wait for completion first then read FIFO.
 *            for write op write all data first before writing header.
 *
 * REG_3C:
 *       [6]: dcs read operation completion bit.
 * REG_48:
 *    [17:0]: dcs write operation status bits, only [17] checked by vendor, likely completion bit.
 *
 * REG_54:
 *       [0]: uncovered bit, set to 1 at init.
 *       [1]: uncovered bit, set to 1 at init.
 *       [2]: uncovered bit, set to 1 at init.
 *
 * REG_58:
 *     [1:0]: num active data lanes minus one (0:1-lane 1:2-lanes 2:3-lanes 3:4-lanes)
 *
 * REG_5C:
 *       [0]: enable high speed clock, 1 - enable
 *
 * REG_60: - labelled as PHY_STATUS by vendor.
 *    [12:0]: uncovered field, likely read-to-clear, likely error bits.
 *            bit [4] is used to detect no data readed during dcs read command(s).
 *
 * PHY_100:
 *       [0]: set to 1
 *       [2]: lanes pair 1 enable
 *       [3]: lanes pair 2 enable
 *       [4]: lanes pair 3 enable
 *       [5]: lanes pair 4 enable
 *       [6]: clock lanes pair enable
 *    [15:8]: some control word, set to 0xe4 during init by vendor code and then to 0xe0 and wait 5 ms.
 *   [28:24]: PLL divider field, field = (div - 1).
 *      [29]: PLL multiplier MSB bit [8].
 *
 * PHY_104:
 *     [7:0]: PLL multiplier LSB bits [7:0].
 *
 * PHY_108:
 *     [3:0]: Drive strength for some lines. lowest: 0x01 low: 0x03 high: 0x0e max: 0x0f
 *   [27:24]: Drive strength for some other lines. lowest: 0x01 low: 0x03 high: 0x0e max: 0x0f
 *
 * PHY_120:
 *     [7:0]: some other control word, during poweron set to 0x1e and then to 0x1f.
 *            during power down it set to 0x01 and then to 0x13.
 *
 * LANE_REG_00:
 *       [4]: lane swap
 *   [31:24]: set to 0x07 during error recovering (dcs read command) and then to 0x00 for data0 only.
 *
 * LANE_REG_04:
 *    [13:8]: bit clock frequency derived field:
 *            0x02: < 300MHz
 *            0x03: 300-600
 *            0x05: 600-1000
 *            0x06: 1000-1200
 *            0x09: 1200-1400
 *            0x0d: 1400-1600
 *            0x0e: 1600-1800
 *            0x11: 1800-2000
 *            0x13: 2000-2400
 *            0x15: 2400-2500
 *   [22:16]: bit clock frequency derived field:
 *            0x7f: < 300MHz
 *            0x7e: 300-400
 *            0x7c: 400-500
 *            0x70: 500-600
 *            0x40: 600-700
 *            0x02: 700-800
 *            0x08: 800-1000
 *            0x03: 1000-1400
 *            0x42: 1400-1600
 *            0x47: 1600-1800
 *            0x64: 1800-2200
 *            0x33: 2200-2400
 *            0x54: 2400-2500
 *      [23]: set to 1 only for the clock lane when bit clock >= 1600MHz, 0 otherwise
 *   [29:24]: bit clock frequency derived field:
 *         for data lanes it is evaluated as follows:
 *            0x02: < 110MHz
 *            0x03: 110-150
 *            0x04: 150-200
 *            0x05: 200-250
 *            0x06: 250-300
 *            0x07: 300-500
 *            0x08: 500-700
 *            0x09: 700-1000
 *            0x14: 1000-1400
 *            0x0e: 1400-2000
 *            0x15: 2000-2500
 *        for clock lane:
 *            0x16: < 150MHz
 *            0x17: 150-250
 *            0x18: 250-300
 *            0x19: 300-400
 *            0x1b: 400-500
 *            0x1d: 500-600
 *            0x1e: 600-700
 *            0x1f: 700-800
 *            0x20: 800-1000
 *            0x32: 1000-1400
 *            0x36: 1400-1600
 *            0x3a: 1600-2000
 *            0x3e: 2000-2200
 *            0x3f: 2200-2500
 *
 * LANE_REG_08:
 *     [6:0]: bit clock frequency derived field:
 *            0x02: < 200MHz
 *            0x04: 200-400
 *            0x08: 400-500
 *            0x10: 500-600
 *            0x30: 600-1000
 *            0x0f: 1000-1800
 *            0x0b: 1800-2200
 *            0x6a: 2200-2500
 *    [12:8]: lower 5 bits of a 6-bit parameter calculated as: 120000 / byte_period_ps + 1
 *            The most significant bit is stored in LANE_REG_10[14].
 *   [19:16]: clock-lane only timing parameter:
 *                ((bit_period_ps * 52 + 60000) / byte_period_ps + 1) & 0x0f
 *
 * LANE_REG_0C:
 *     [1:0]: upper 2 bits of the 10-bit parameter, rest stored in LANE_REG_0C[15:8]
 *    [15:8]: lower 8 bits of the parameter calculated as:
 *               1010000000 / pll_ref_period_ps + 1
 *              The result is expected to be limited to 10 bits (0x3ff).
 *              However, vendor code uses 0x2ff instead of 0x3ff.
 *   [19:16]: clock-lane only timing parameter:
 *                ((bit_period_ps * 10) / byte_period_ps + 1) & 0x0f
 *
 * LANE_REG_10:
 *     [5:0]: constant 8 stored
 *       [6]: clock-lane only, reflects value in bit 14 LANE_REG_10[14]
 *       [7]: clock-lane only, set to 0, might form a single field with LANE_REG_10[6]
 *    [13:8]: constant 3 stored
 *      [14]: upper bit of value stored in LANE_REG_08[12:8]
 *   [21:16]: constant 10 stored
 *
 * REG_200:
 *     [7:0]: clock calibration, enabled by 0xf1, disabled by 0x00
 *            vendor enables it after phy poweron for 1ms then disables and waits another 1ms
 *            bit 3 used as retval in is_mipi_enabled().
 *            bit 3 set to 1 in set_mipi_enabled().
 *      [12]: set to 1 during error recovering (dcs read command) and then to 0
 *      [27]: "dupl-2lane" feature enable, likely to transmit same data dat0==dat2, dat1==dat3
 *            makes sense for 2 same panels connected to a single MIPI DSI bus.
 *
 * REG_218:
 *      [24]: Enable BIST mode: 0 disabled, 1 enabled
 *      [25]: set right after MIPI reset, before BIST mode set. likely BIST start.
 *      [28]: BIST status: 0 fail, 1 ok
 *      [29]: BIST done: 0 in progress, 1 finished.
 */
#endif
