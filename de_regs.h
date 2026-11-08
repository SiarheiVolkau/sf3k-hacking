#ifndef DE_REGS_H
#define DE_REGS_H

#include <stdint.h>
#include <stddef.h>

#define DE_BASE          0xB8808000
#define DE4K_BASE        0xB883A000

/* CTL0 Register:
 * [0]     - called play, likely starts/stops (1/0) the DE
 * [2]     - no name, likely keeps DE counters in reset when 0
 * [4]     - called hd, likely HDMI path enable (enabled when mode Parallel RGB or HDMI-out enabled)
 */
#define REG_DE_CTL0      (0x0000)

/* DE_INT Register:
 * [7:0]   - interrupt enable bits
 *           bit 0 - unknown, unused by vendor
 *           bit 1 - unknown, unused by vendor
 *           bit 2 - likely issued on every frame, might be VSync
 *           bit 3 - unknown, unused by vendor
 *           bit 4,5 - likely top/bottom interrupt flags for interleaved output.
 *           bit 6 - unknown, unused by vendor
 *           bit 7 - unknown, unused by vendor
 * [15:8]  - interrupt pending bits W1C, meaning same as for enable bits.
 * [20:16] - some frame flags, used by interrupt handler.
 */
#define REG_DE_INT       (0x0004)

/* DE_008 Register:
 * [27:16] - some field, see unused eIqtXdPfEHkfmDpiNZVXlTTtdLPhvUSJ.
 */
#define REG_DE_008       (0x0008)

/* DE_TIM0 Register:
 * [15:0]  - htotal * cpp, cpp - clocks-per-pixel, for Parallel RGB cpp=2 for serial cpp=3
 * [31:16] - vtotal
 */
#define REG_DE_TIM0      (0x000C)

/* DE_TIM1 Register:
 * [15:0]  - htotal * cpp
 * [31:16] - hsync * cpp
 */
#define REG_DE_TIM1      (0x0010)

/* DE_TIM2 Register:
 * [15:0]  - (hsync + hbp) * cpp - 4
 * [31:16] - (hactive + hsync + hbp) * cpp
 */
#define REG_DE_TIM2      (0x0014)

/* DE_TIM3 Register:
 * [15:0]  - vtotal
 * [31:16] - vsync
 */
#define REG_DE_TIM3      (0x0018)

/* DE_TIM4 Register: defaults to 0x0fff0fff meaning unknown */
#define REG_DE_TIM4      (0x001C)

/* DE_TIM5 Register: defaults to 0x00001fff meaning unknown */
#define REG_DE_TIM5      (0x0020)

/* DE_TIM6 Register:
 * [15:0]  - vsync + vbp
 * [31:16] - vactive + vsync + vbp
 */
#define REG_DE_TIM6      (0x0024)

/* DE_TIM7 Register: defaults to 0x0fff0fff meaning unknown */
#define REG_DE_TIM7      (0x0028)

/* DE_TIM8 Register:
 * [15:0]  - defaults to 0x0fff meaning unknown
 * [31:16] - vtotal
 */
#define REG_DE_TIM8      (0x002C)

/* DE_ISRLINE Register:
 * [15:0]  - top line position, likely chooses the moment for emitting IRQ. Not VSync irq.
 * [31:16] - bottom line position, likely chooses the moment for emitting IRQ, might be active only for interlaced modes. Not VSync irq.
 */
#define REG_DE_ISRLINE    (0x0030)

/* DE_TIM1x Register:
 * ?must? be written at same time and same value as DE_TIM1.
 * Seems absent on E3100.
 */
#define REG_DE_TIM1x     (0x0040)

/* DE_TIM2x Register:
 * ?must? be written at same time and same value as DE_TIM2.
 * Seems absent on E3100.
 */
#define REG_DE_TIM2x     (0x0044)

/* DE_TIM3x Register:
 * ?must? be written at same time and same value as DE_TIM3.
 * Seems absent on E3100.
 */
#define REG_DE_TIM3x     (0x0048)

/* DE_TIM4x Register:
 * ?must? be written at same time and same value as DE_TIM4.
 * Seems absent on E3100.
 */
#define REG_DE_TIM4x     (0x004C)

/* DE_TIM5x Register:
 * ?must? be written at same time and same value as DE_TIM5.
 * Seems absent on E3100.
 */
#define REG_DE_TIM5x     (0x0050)

/* DE_TIM6x Register:
 * ?must? be written at same time and same value as DE_TIM6.
 * Seems absent on E3100.
 */
#define REG_DE_TIM6x     (0x0054)

/* DE_TIM7x Register:
 * ?must? be written at same time and same value as DE_TIM7.
 * Seems absent on E3100.
 */
#define REG_DE_TIM7x     (0x0058)

/* DE_TIM8x Register:
 * ?must? be written at same time and same value as DE_TIM8.
 * Seems absent on E3100.
 */
#define REG_DE_TIM8x     (0x005C)

/* DE_064 Register:
 * Fields not decoded, set to 0x64012a or 0x37012a (0/1 respectively) depending on
 * argument called hd_output, which is 0 only for PAL and NTSC modes.
 */
#define REG_DE_064       (0x0064)

/* DE_068 Register:
 * Fields not decoded, set to 0x20500d0 or 0x21d0089 (0/1 respectively) depending on
 * argument called hd_output, which is 0 only for PAL and NTSC modes.
 */
#define REG_DE_068       (0x0068)

/* DE_06C Register:
 * Fields not decoded, set to 0x198 or 0x1cb (0/1 respectively) depending on
 * argument called hd_output, which is 0 only for PAL and NTSC modes.
 */
#define REG_DE_06C       (0x006C)

/* DE_070 Register: seems unused by vendor's code. SF3000 dump shows 0x00B07030 */
#define REG_DE_070       (0x0070)

/* DE_074 Register: seems unused by vendor's code. SF3000 dump shows 0x01B07030 */
#define REG_DE_074       (0x0074)

/* DE_07C/080/084 Register:
 * [1:0]   - name not recovered. see pZjAICHEBtKUqbagUglHNcWTWvqsboNi.
 *           1 - if bt1220
 *           2 - if PRGB || HDMI enabled
 *           0 - otherwise
 *           in ioctl it depends on de2hdmi.format:
 *           0 - for YCBCR420
 *           1 - for YCBCR422
 *           2 - otherwise
 * [4]     - name not recovered. clearing during dis.open call for all 3 regs, see rFGHHMTrKSCPKYgnCnBufZxWuOfOAOVx.
 * [5]     - field called "bit_mode" by vendor code. enabled for bt1220 only. see GQLKBgbUfsknBtOvZMAMoNtbSEkcfbUk. set to 0 for all. set to 1 for 084 only.
 * [8]     - field called "level" by vendor code (tracked to active-polarity in dt). see klSLPGHMpiSeURLeIpCTpKtseFnastVz.
 * [9]     - field called "h_level1" by vendor code. see EyZEsBYegLPheCznGbQWYbmtRHKPmCDX and HuqoKXSXHJGUbLSdjrLIVGPsVbzDOeLE. 080 and 084.
 *           0 for PAL and NTSC, 1 for others. For RGB configurable via dt "h-sync-level"
 * [10]    - field called "v_level1" by vendor code. see EyZEsBYegLPheCznGbQWYbmtRHKPmCDX and HuqoKXSXHJGUbLSdjrLIVGPsVbzDOeLE. 080 and 084.
 *           0 for PAL and NTSC, 1 for others. For RGB configurable via dt "v-sync-level"
 * [12]    - field called "enable" by vendor code. see unused vihpYceKUuiRLjmwicWDKnCdxMgJeFFY. disabled for bt1220 in ycWVNIgBAFoaWMgEKvZiyQKtCNzFVGVG. 084 only.
 * [13]    - field called "enable" by vendor code. see unused ZgzUkGlKLwcfWNrJFPgKjcpUPGzqqUWa. disabled for bt1220 in ycWVNIgBAFoaWMgEKvZiyQKtCNzFVGVG. 084 only.
 * [14]    - field called "enable" by vendor code. see unused bSeWZAXOJngDtGUcmUDtGtcptPavrCbj. enabled in rFGHHMTrKSCPKYgnCnBufZxWuOfOAOVx. enabled for bt1220 in ycWVNIgBAFoaWMgEKvZiyQKtCNzFVGVG. 084 only.
 * [16]    - field called "enable" by vendor code. see iFDixwpcoDYtRFfNQLGLvhwyHfWExGMa.
 *           in init: bt1220 || (!rgb_enable && hdmi_out_yuv).
 *           in ioctl it depends on de2hdmi.format:
 *           0 - for RGB_MODE1 and RGB_MODE2
 *           1 - otherwise.
 * [17]    - field called "enable" by vendor code. see GUbMzjhBcASkKznCsJnoLZAYtNGQrAls. 0 for bt1220, 1 otherwise.
 * [20]    - field called "rgb" by vendor code. see NuVQBEPNaCpsmPSCNVJEOFRDOfJqLYxB and AYIiNIVQHRUsVvfadSsUDKetVvrfmQVC
 *           in init: (rgb_enable || !hdmi_out_yuv) && !bt1220
 *           in ioctl it depends on de2hdmi.format:
 *           1 - for RGB_MODE1 and RGB_MODE2
 *           0 - otherwise.
 * [24]    - field called "format_sel" by vendor code. see qjoqNHWLrJkXEGDMssHNdpCiGImggVzQ. 084 only.
 *           0 - RGB mode parallel (PRGB)
 *           1 - RGB mode serial (SRGB)
 * [25]    - field called "format_sel" of type dis_prgb_mode by vendor code. see GikBqrhjOpKLnTGyuNCJFjnMvatrMonK. 084 only.
 *           PRGB format dis_prgb_mode, SRGB not affected:
 *           0 - RGB888, 1 - RGB666
 * [26]    - field called "srgb_order". see EgxGCiMlOfKRObtBFJdFdDKjATtLEBIT. 084 only. dt("srgb-order")
 *           according to dis_srgb_order: 0 - RGB, 1 - BGR. PRGB not affected.
 * [28:27] - field called "srgb_ch_sel" by vendor code. see KbpFDwcdlqqqsUiHvrEUzoADwqAECWmr. 084 only. dt("srgb-ch-sel").
 *           Meaning not recovered. PRGB not affected.
 *
 * Note for E3100 - PRGB is used to feed MIPI-DSI.
 *
 * SF3000 dump:
 * 07C: 0x00000000
 * 080: 0x00120102
 * 084: 0x00127102
 */
#define REG_DE_07C       (0x007C)
#define REG_DE_080       (0x0084)
#define REG_DE_084       (0x0084)

/* DE_088 Register: seems unused by vendor's code. SF3000 dump shows 0x00108080
 * which corresponds to black color in YCbCr 8-bit format */
#define REG_DE_088       (0x0088)

/* DE_08C Register: seems unused by vendor's code. SF3000 dump shows 0x00000004 */
#define REG_DE_08C       (0x008C)

/* DE_TIM9 Register:
 * [31:0]  - (hactive * 1) for PRGB or HDMI enabled; otherwise (hactive * 2)
 */
#define REG_DE_TIM9      (0x0094)

/* REG_DE_KEYSTONE_CTL Register:
 * [15:0]  - width_down
 * [30:16] - width_up
 * [31]    - Keystone enable
 * DE4K does not support keystone according to vendor's code.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_KEYSTONE_CTL      (0x00F0)

/* REG_DE_KEYSTONE_BG Register:
 * [9:0]   - Cr of the keystone background.
 * [19:10] - Cb of the keystone background.
 * [29:20] - Y of the keystone background.
 * DE4K does not support keystone according to vendor's code.
 * SF3000 dump: 0x04080200 which corresponds to Y=0x40, Cb=0x200, Cr=0x200
 * this is black color in YCbCr 10-bit */
#define REG_DE_KEYSTONE_BG       (0x00F4)

/* DE_100 Register:
 * [4:0]   - part of DIS_SCALE_INFO.y_h_init field: y_h_init[20:16].
 * [18:8]  - part of DIS_SCALE_INFO.y_h_init field: y_h_init[10:0].
 * [28:24] - part of DIS_SCALE_INFO.y_h_init field: y_h_init[15:11].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_100       (0x0100)

/* DE_104 Register:
 * [4:0]   - part of DIS_SCALE_INFO.y_o_init field: y_o_init[16:12].
 * [5]     - part of DIS_SCALE_INFO.y_o_init field: y_o_init[30].
 * [14:8]  - part of DIS_SCALE_INFO.y_o_init field: y_o_init[6:0].
 * [28:24] - part of DIS_SCALE_INFO.y_o_init field: y_o_init[11:7].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_104       (0x0104)

/* DE_108 Register:
 * [4:0]   - part of DIS_SCALE_INFO.c_h_init field: c_h_init[20:16].
 * [18:8]  - part of DIS_SCALE_INFO.c_h_init field: c_h_init[10:0].
 * [28:24] - part of DIS_SCALE_INFO.c_h_init field: c_h_init[15:11].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_108       (0x0108)

/* DE_10C Register:
 * [4:0]   - part of DIS_SCALE_INFO.c_o_init field: c_o_init[16:12].
 * [5]     - part of DIS_SCALE_INFO.c_o_init field: c_o_init[30].
 * [14:8]  - part of DIS_SCALE_INFO.c_o_init field: c_o_init[6:0].
 * [28:24] - part of DIS_SCALE_INFO.c_o_init field: c_o_init[11:7].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x1800003F
 */
#define REG_DE_10C       (0x010C)

/* DE_110 Register:
 * [4:0]   - part of width scaling coefficient #1 field: [20:16].
 * [18:8]  - part of width scaling coefficient #1 field: [10:0].
 * [28:24] - part of width scaling coefficient #1 field: [15:11].
 * the coefficient is likely Q16.16.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000001
 */
#define REG_DE_110       (0x0110)

/* DE_114 Register:
 * [5:0]   - part of height scaling coefficient #1 field: [17:12].
 * [14:8]  - part of height scaling coefficient #1 field: [6:0].
 * [28:24] - part of height scaling coefficient #1 field: [11:7].
 * the coefficient is likely Q20.12.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000001
 */
#define REG_DE_114       (0x0114)

/* DE_118 Register:
 * [4:0]   - part of width scaling coefficient #2 field: [20:16].
 * [18:8]  - part of width scaling coefficient #2 field: [10:0].
 * [28:24] - part of width scaling coefficient #2 field: [15:11].
 * the coefficient is likely Q16.16.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x10000000
 */
#define REG_DE_118       (0x0118)

/* DE_11C Register:
 * [5:0]   - part of height scaling coefficient #2 field: [17:12].
 * [14:8]  - part of height scaling coefficient #2 field: [6:0].
 * [28:24] - part of height scaling coefficient #2 field: [11:7].
 * the coefficient is likely Q20.12.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x10000000
 */
#define REG_DE_11C       (0x011C)

/* DE_134 Register:
 * [27:16] - display_left + display_width - 1. Likely never called.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x02D00000 -> 720
 */
#define REG_DE_134       (0x0134)

/* DE_138 Register:
 * [27:16] - display_left + display_width - 1. Likely never called. Maybe bug and should be top+height?
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x01680000 -> 360
 */
#define REG_DE_138       (0x0138)

/* DE_13C Register:
 * Set to 0 in vendor code. Likely never called.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_13C       (0x013C)

/* DE_140 Register:
 * Set to 0 in vendor code. Likely never called.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_140       (0x0140)

/* DE_144 Register:
 * Set to 1 in vendor code. Likely never called.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_144       (0x0144)

/* DE_148 Register:
 * Set to 1 in vendor code. Likely never called.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_148       (0x0148)


/* DE_14C Register:
 * [3:0]   - register offset in the CSC matrix.
 * [4]     - read/write bit (0/1).
 * [31:16] - value of a 16-bit CSC matrix register.
 * There's a plenty of 16-bit registers behind this one. Purpose: a colorspace conversion matrix.
 * To write a register behind it:
 * 1. write at once a new value, a register offset and a BIT(4) like that: REG_DE_14C = (value << 16) | BIT(4) | (offset & 0xf).
 * To read a register behind it:
 * 1. write an offset first: REG_DE_14C = offset & 0xf;
 * 2. read a register and extract value: value = REG_DE_14C >> 16;
 *
 * CSC registers:
 * 0 - default 0x0000, purpose unknown
 * 1 - default 0x0100, purpose unknown
 * 2 - default 0x0000, purpose unknown
 * 3 - default 0x0000, somehow related to sharpness enhancement, range -5 .. +5.
 * 4 - default 0x0000, purpose unknown
 * 5 - default 0x0100, purpose unknown
 * 6 - default 0x0000, purpose unknown
 * 7 - default 0x0100, somehow related to saturation enhancement, range 0 .. 511, might be Q8.8.
 * 8 - default 0x0000, somehow related to hue enhancement, Q8.8.
 * 9 - default 0x0100, somehow related to contrast enhancement, Q8.8.
 * hue and contrast correlation is:
 * - hue=sin(x)
 * - contrast=cos(x)
 */
#define REG_DE_14C       (0x014C)

/* DE_160 Register:
 * [31:0]  - phys address for special memory region, 4 kB, purpose not recovered.
 * In vendor lib it is called MF / MAF, mf_coef and filled at display init, TODO restore.
 */
#define REG_DE_160       (0x0160)

/* DE_164 Register:
 * [31:0]  - top Y phys memory address, this points to a start of Y plane for progressive mode.
 *           or a first(top) Y plane for interleaved mode.
 * Layer is unknown but likely DIS_LAYER_MAIN.
 */
#define REG_DE_164       (0x0164)

/* DE_168 Register:
 * [31:0]  - bottom Y phys memory address, this points to a start of a second(bottom) Y plane
 * for interleaved mode.
 * Layer is unknown but likely DIS_LAYER_MAIN.
 */
#define REG_DE_168       (0x0168)

/* DE_16C Register:
 * [31:0]  - top CbCr phys memory address, this points to a start of CbCr plane for progressive mode.
 *           or a first(top) CbCr plane for interleaved mode.
 * Layer is unknown but likely DIS_LAYER_MAIN.
 */
#define REG_DE_16C       (0x016C)

/* DE_170 Register:
 * [31:0]  - bottom CbCr phys memory address, this points to a start of a second(bottom) CbCr plane
 * for interleaved mode.
 * Layer is unknown but likely DIS_LAYER_MAIN.
 */
#define REG_DE_170       (0x0170)

/* DE_174 Register:
 * [15:0]  - left/16
 * [31:16] - top/16
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * DE_174,178,17C represent, likely, input frame + position.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_174       (0x0174)

/* DE_178 Register:
 * [8:0]   - ((width + left) - (left / 16 * 16) + 15) / 16; simplified version: (w+(l&0xf)+15)/16. width must be >= 5
 * [24:16] - stride, not in bytes but might be in 16 or 32 byte chunks depending on tile mapping mode.
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * DE_174,178,17C represent, likely, input frame + position.
 * SF3000 dump: DE=0x5A17242D DE4K=0x0017002D (width=720,left=0 -> 720/16 = 0x2D, stride: 0x17*32->736)
 */
#define REG_DE_178       (0x0178)

/* DE_17C Register:
 * [15:0]  - (width + left) - (left / 16 * 16). simplified version: w+(l&0xf). width must be >= 5
 * [31:16] - (height + top) - (top / 16 * 16). simplified version: h+(t&0xf).
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * DE_174,178,17C represent, likely, input frame + position.
 * SF3000 dump: 0x024002D0 (width=720,left=0 -> 720 -> 0x2D0) (height=576,top=0 -> 0x240)
 */
#define REG_DE_17C       (0x017C)

/* DE_180 Register:
 * [15:0]  - end_x
 * [31:16] - start_x
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * DE_180,184 represent, likely, output frame/area.
 * SF3000 dump: 0x000002CF (start_x=0, end_x=(720-1))
 */
#define REG_DE_180       (0x0180)

/* DE_184 Register:
 * [15:0]  - end_y
 * [31:16] - start_y
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * DE_180,184 represent, likely, output frame/area.
 * SF3000 dump: 0x0000023F (start_y=0, end_y=(576-1))
 */
#define REG_DE_184       (0x0184)

/* DE_188 Register:
 * [7:0]   - Cr component of the color
 * [15:8]  - Cb component of the color
 * [23:16] - Y component of the color
 * Highly likely it is background color for DIS_LAYER_MAIN or entire DE.
 * SF3000 dump: 0x00108080 (Y=0x10, Cb=0x80, Cr=0x80 - this is black color)
 */
#define REG_DE_188       (0x0188)

/* DE_18C Register:
 * [7:0]    - field name not recovered. Likely a bitmask.
 *            See unused StIhcHQnBPDdCMNaWXWgeUEbeLdxZxYk and QYGEyPsFjcyVkhbFIYqkAIYcfZhMVTmR.
 * [31:16]  - some 8-bit mask, arg called "mask_bit", zero extended to 16 bit then put into register.
 *            Set to all 0s during init. See fGlOqMdUhlBJpafAfwPsAVvnjjzrVcJx.
 *            See unused HiPqpqHHZjxHPlsTiWCpcKJKsAAydGrY.
 * Likely it is interrupt control register, see unused IFWpoiHYTbPRpvXLNIZkvYHJJKjndjiC which acts as typical W1C.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_18C       (0x018C)

/* DE_190 Register:
 * [0]     - likely mask/commit operations on DE.
 *           It typically setted before changes have made and cleared after.
 *           See SDMxSMIptLxvkDWYPklfptdksRkJiNNt and nlOjpNVoTzRLPEQVrcnHOnPnlTymfbAj.
 * [3:1]   - tiling/mapping mode, see T_eTileMode, mode names 0/1/2 enlight nothing.
 * [4]     - enables deinterlacing aka "dit_enable". See
 * [6]     - field named "v_c_2tap" set same as "v_y_2tap"
 * [7]     - field named "v_y_2tap" set same as "v_c_2tap"
 * [8]     - output is progressive when set.
 * [11:9]  - format of the input/source frame
 *           0 - YCBCR_420
 *           1 - YCBCR_411
 *           2 - YCBCR_422
 *           0 - others non-YCBCR (DIS_LAYER_MAIN might not support)
 * [14]    - limited range. of what?
 * [15]    - is_gamma22 aka GAMMA_CORRECTION_SRGB_TO_2P2.
 * [16]    - field named "duplicate" set/clear together with "v_c_2tap" and "v_y_2tap".
 *           Function TKuiaYzlAzYForMxvrqhXSSgCkspLzer set it to 0 at init, keeping "..2tap" as is.
 *           so related somehow. Decision to "duplicate" is made during scaling setup at ZgvBdQYzTQrbUzzwxizoNRYlbkYXzAqf.
 * [18:17] - fetch mode, one of
 *           0 - DIS_FIELDBASED
 *           1 - DIS_FRAMEBASED
 *           2 - DIS_MA
 * [22]    - gate_clock_by_pass. set to 0 during init, not changed after.
 * [23]    - enable something. set to 1 during init, not changed after. see ANRNfKGXxyfRKPEiRkmhCatOxusXYQdD.
 * [24]    - preframe_onoff. set to 0 at init. see UQeezTYRXGsDolpEEjvxswHTMDqDabvw.
 * [26]    - enable something. set to 0 at same time as dit_enable and preframe_onoff. see StFSmqrOZvlacpcUTtVNAdGYgsEEaffs.
 * [28]    - is_jpeg, set then GAMMA_CORRECTION_SRGB_TO_BT709 or GAMMA_CORRECTION_SRGB_TO_2P2 are applied.
 * [29]    - enable colorspace conversion. see gen1_hw_csc_cfg.enabled
 * SF3000 dump: 0x04800100 (progressive and 2 unknowns)
 */
#define REG_DE_190       (0x0190)

/* DE_194 Register:
 * [0]     - Names are "botBased", "mp_based_field". see xeDEnsfkZGksXmFvvLNuQxoXCpAbtFED for conditions it set.
 * [11:8]  - reg windowing, address. this field sets what register is mapped to DE_198[7:0].
 *           See hjNuVpzoDssoQjryLZIXFJjFWgJoTYuR.
 * [16]    - enables something. set to 1 at init. See MBPopnNUMUImQGQtvRzTtlgOIRpEnUHD.
 * SF3000 dump: 0x00010000
 */
#define REG_DE_194       (0x0194)

/* DE_198 Register:
 * [7:0]   - window to a set of 16 8-bit registers of unknown purpose.
 *           at init, registers 0x08..0x0f from this set are set to 0x0f.
 *           Address of register to map here is selected via DE_194[11:8].
 *           See hjNuVpzoDssoQjryLZIXFJjFWgJoTYuR.
 * [16]    - field name "PNConvert". usage is too complex, can't guess the value.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_198       (0x0198)

/* DE_19C Register:
 * [7:0]   - mp_global_k_to_bg. likely always 0.
 * [15]    - mp_global_k_enable. likely always 0. see aQouQgEaZOxwyuSiZsyOMRqZGBxkuESk.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_19C       (0x019C)

/* DE_1B8 Register:
 * [12:0]  - width, of the output in pixels. See dXDcDytGzlbNhUhOioaaQmcXPvSxfHHi.
 * SF3000 dump: 0x000001E0
 */
#define REG_DE_1B8       (0x01B8)

/* DE_1BC Register:
 * [0]     - enable something. never intentionally enabled. disabled when uRequestReturn == 1.
 *           See EnYYeANfAyidWBPpOfNHTpuYgHVnFsQK.
 * [8]     - enable something. see unused TueDWpToRUnQYAnsVKnKDJPZDzhrrxXK.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_1BC       (0x01BC)

/* DE_1C8 Register:
 * [4:0]   - part of DIS_SCALE_INFO.y_e_init field: y_e_init[16:12].
 * [5]     - part of DIS_SCALE_INFO.y_e_init field: y_e_init[30].
 * [14:8]  - part of DIS_SCALE_INFO.y_e_init field: y_e_init[6:0].
 * [28:24] - part of DIS_SCALE_INFO.y_e_init field: y_e_init[11:7].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x10000000
 */
#define REG_DE_1C8       (0x01C8)

/* DE_1CC Register:
 * [4:0]   - part of DIS_SCALE_INFO.c_e_init field: c_e_init[16:12].
 * [5]     - part of DIS_SCALE_INFO.c_e_init field: c_e_init[30].
 * [14:8]  - part of DIS_SCALE_INFO.c_e_init field: c_e_init[6:0].
 * [28:24] - part of DIS_SCALE_INFO.c_e_init field: c_e_init[11:7].
 * Part of YUV layer (likely DIS_LAYER_MAIN) scaling setup.
 * SF3000 dump: 0x08000000
 */
#define REG_DE_1CC       (0x01CC)

/* DE_1D8 Register:
 * [7:0]   - field "nValue". See unused tTnVXfbLwKxRTDPHOiLWTPKZBHfPaDZl.
 * [17:16] - field "nSel", see unused JhRBWYAvdejrkQZgdLlucIdxGkCSpJsd.
 *           valid values are 2 and 3. never touched.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_1D8       (0x01D8)

/* DE_1DC Register:
 * [31:0]  - field "layer". set to 0. might be set to 6. purpose unknown.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_1DC       (0x01DC)

/* DE_1E0 Register:
 * [31:0] - field "vc1_mixer_control". always 0.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_1E0       (0x01E0)

/* DE_1E4 Register:
 * [31:0] - nMPCoefBaseAddr.
 * SF3000 dump: DE: 0x00700000 DE4K: 0x0CDD8800
 */
#define REG_DE_1E4       (0x01E4)

/* DE_1E8 Register:
 * [13:0] - name not known. See unused fiTSQkJAKWhKVNGYcnSUIJGdRTtwbkoP and WGlWPpJtRNYbAQYfSQoUWSDbcewyfWDB.
 * SF3000 dump: DE: 0x00003210 DE4K: 0x00001230
 */
#define REG_DE_1E8       (0x01E8)

/* DE_1EC Register:
 * [12]    - see unused vpCBcClLCwVtQJKOSijAJOwMTPeyzVbs.
 * [14:13] - purpose not recognized. see suOZEwgCUXswttnNoVTeZsCjmGWYOwZX.
 * [16]    - buf_ready, related to VideoCapture (VC). set at same time with VC register 0xb8809000[8]. See NYSuOMyyFqyUztxEtXZlxtmSdwJXpyft.
 * SF3000 dump: DE: 0x00000000 DE4K: 0x00040000
 */
#define REG_DE_1EC       (0x01EC)

/* DE_1F4 Register:
 * [31:0]  - top_y phys address. Likely the Y buffer start address for progressive and top frame in interlaced mode.
 *           set when preframe_onoff != 0 and two_preframe != 0
 * SF3000 dump: 0x00300000
 */
#define REG_DE_1F4       (0x01F4)

/* DE_1F8 Register:
 * [31:0]  - bot_y phys address. Likely the Y buffer for bottom frame in interlaced mode.
 *           set when preframe_onoff != 0 and two_preframe != 0
 * SF3000 dump: 0x00300000
 */
#define REG_DE_1F8       (0x01F8)

/* DE_1FC Register:
 * [0]     - not recognized bit, might be readonly. See dUStMzkoOHNlohbWrqfPqcTBnPiHroxa.
 * SF3000 dump: 0x00000000
 */
#define REG_DE_1FC       (0x01FC)

/*
300 - GMA0 HC_GMA_CTL
304 - GMA0 HC_GMA_DMBA
308 - GMA0 HC_GMA_K
350 - GMA0 HC_GMA_MASK
358 - GMA0 HC_GMA_LINEBUF

380 - GMA1

400
404

9f4 -> same structure as 1f4
9f8 -> same structure as 1f8
*/

#endif
