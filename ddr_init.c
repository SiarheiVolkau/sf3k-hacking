// These two shipped with 256-1066 preset.
// 1. DDR IC: Nanya NT5CB128M16FP-EK DDR3 (1.5V), 256MB 128M/16, 1866-13-13-13, 1600-11-11-11
//    PCB ID: SF3000-E-V1.0(2026.01.06)
// 2. DDR IC: Nanya NT5CB128M16HP-EK DDR3 (1.5V), 256MB 128M/16, 1866-13-13-13, 1600-11-11-11
//    PCB ID: RS36S-V2.7(2026.01.09)
// Can confirm that 256-1600 preset also should work.

#include <stdint.h>
#include "mmio.h"

enum ddr3_size {
        DDR3_SIZE_256,
        DDR3_SIZE_256_FOR_512,
        DDR3_SIZE_COUNT
};

/* values are data rates in MT/s */
enum ddr3_freq {
        DDR3_600,
        DDR3_800,
        DDR3_1066,
        DDR3_1200,
        DDR3_1333,
        DDR3_1600,
        DDR3_FREQ_COUNT
};

/* Registers that differ depending on frequency, indexed by enum ddr3_freq */
/*                                        600         800         1066        1200        1333        1600 */
static const uint16_t r_B880048A[]  = { 0x8019,     0,          0,          0x8032,     0,          0          }; /* 0 = not used, 74/76 are written instead */
static const uint8_t  r_B8800074[]  = { 0,          0x00,       0x10,       0,          0x20,       0x30       }; /* unused for 600 and 1200 */
static const uint8_t  r_B880108C[]  = { 0x00,       0x00,       0x01,       0x12,       0x12,       0x13       };
static const uint32_t r_B883E05C[]  = { 0x00,       0x00,       0x08,       0x10,       0x10,       0x18       };
static const uint32_t r_B883E054[]  = { 0x1520,     0x1520,     0x1930,     0x1B50,     0x1B50,     0x1D70     };
static const uint32_t r_B883E048[]  = { 0x510F6644, 0x510F6644, 0x71947744, 0x85589955, 0x85589955, 0x9D9DBB66 };
static const uint32_t r_B883E04C[]  = { 0x11020280, 0x11020280, 0x1562B360, 0x19B35BC0, 0x19B35BC0, 0x1A040400 };
static const uint32_t r_B883E180[]  = { 0x10018D8C, 0x10018D8C, 0x10018D8C, 0x100318A5, 0x10018D8C, 0x10018D8C }; /* also written to B883E190 */
static const uint32_t r_B883E004[]  = { 0xFF81,     0xFF81,     0xFF81,     0xFF81,     0xF781,     0xF781     };
static const uint32_t r_B8801004[]  = { 0x00938100, 0x00938100, 0x00939100, 0x0093A100, 0x0093A100, 0x0093B100 };
static const uint16_t r_B8801030[]  = { 0x4000,     0x4000,     0x5000,     0x5800,     0x5800,     0x6000     };
/* depends on size too; 0 = no such profile (512 has no 600 and 1200) */
static const uint32_t r_B8801000[DDR3_SIZE_COUNT][DDR3_FREQ_COUNT] = {
        [DDR3_SIZE_256]             = { 0x058240C4, 0x058240C4, 0x0D894084, 0x54904044, 0x54904044, 0x58994004 },
        [DDR3_SIZE_256_FOR_512]     = { 0,          0x198240C4, 0x25894084, 0,          0x6C904044, 0x70994004 },
};

static void busy_wait(uint32_t counter)
{
        while (counter != 0) {
                counter--;
        }
}

static void ddr3_init(enum ddr3_size size, enum ddr3_freq freq)
{
        if ((unsigned int)size >= DDR3_SIZE_COUNT)
                return;
        if ((unsigned int)freq >= DDR3_FREQ_COUNT)
                return;
        if (r_B8801000[size][freq] == 0)
                return;

        if (r_B880048A[freq]) {
                REG16(0xB880048A) = r_B880048A[freq];
        } else {
                REG8(0xB8800074)  = r_B8800074[freq];
                REG8(0xB8800076)  = 0x20;
        }
        busy_wait(0x10);
        REG8(0xB880108C)  = r_B880108C[freq];
        REG8(0xB8801033)  = 0x60;
        REG32(0xB883E044) = 0x0000040B;
        REG8(0xB883E03C)  = 0x80;
        REG32(0xB883E028) = 0x08C30D40;
        REG32(0xB883E02C) = 0x08013880;
        REG8(0xB883E034)  = 0x1F;
        REG32(0xB883E05C) = r_B883E05C[freq];
        REG32(0xB883E060) = 0x00000000;
        REG32(0xB883E058) = 0x00000040;
        REG32(0xB883E054) = r_B883E054[freq];
        REG32(0xB883E048) = r_B883E048[freq];
        REG32(0xB883E04C) = r_B883E04C[freq];
        while((REG32(0xB883E010) & 1U) != 1U)
                ;

        REG32(0xB883E180) = r_B883E180[freq];
        REG32(0xB883E190) = r_B883E180[freq];
        REG32(0xB883E068) = 0x910075C7;
        REG8(0xB883E040)  = 0x1E;
        REG8(0xB883E047)  = 0x10;
        REG32(0xB883E004) = r_B883E004[freq];
        while((REG32(0xB883E010) & 1U) != 1U)
                ;

        REG32(0xB883E1E4) -= 3U; /* maybe better ((x) & ~3U) */
        REG32(0xB883E224) -= 3U; /* maybe better ((x) & ~3U) */
        REG32(0xB8801000) = r_B8801000[size][freq];
        REG32(0xB8801004) = r_B8801004[freq];
        REG16(0xB8801030) = r_B8801030[freq];
        REG32(0xB880100C) = 0xFFFF4055;
        REG32(0xB8800224) = 0xFFFFFFFF;
        REG32(0xB8801010) = 0xFFFFFFFF;
        REG32(0xB8801018) = 0xFFFFFFFF;
        REG32(0xB8801078) = 0xFFFFFFFF;
        REG32(0xB8801020) = 0xFFFFFFFF;
        REG32(0xB8801024) = 0xFFFFFFFF;
        REG32(0xB880107C) = 0xFFFFFFFF;
        REG32(0xB8801080) = 0xFFFFFFFF;
        REG32(0xB8801800) = 0xFFFFFFFF;
        REG8(0xB8801821)  = 0x80;
        REG8(0xB8801822)  = 0xFF;
        REG32(0xB8801824) = 0x88880F00;
        REG32(0xB8801828) = 0xFFFFFFFF;
        REG8(0xB8801831)  = 0x80;
        REG8(0xB8801832)  = 0xFF;
        REG32(0xB8801834) = 0x88880F00;
        REG32(0xB8801838) = 0xFFFFFFFF;
        REG8(0xB8801841)  = 0x80;
        REG8(0xB8801842)  = 0xFF;
        REG32(0xB8801844) = 0x88880700;
        REG32(0xB8801848) = 0xFFFFFFFF;
        REG8(0xB8801851)  = 0x80;
        REG8(0xB8801852)  = 0xFF;
        REG32(0xB8801854) = 0x88880A00;
        REG32(0xB8801858) = 0xFFFFFFFF;
        REG8(0xB8801861)  = 0x80;
        REG8(0xB8801862)  = 0xFF;
        REG32(0xB8801864) = 0x88880F00;
        REG32(0xB8801868) = 0xFFFFFFFF;
        REG8(0xB8801871)  = 0x80;
        REG8(0xB8801872)  = 0xFF;
        REG32(0xB8801874) = 0x88880B00;
        REG32(0xB8801878) = 0xFFFFFFFF;
        REG8(0xB8801881)  = 0x80;
        REG8(0xB8801882)  = 0xFF;
        REG32(0xB8801884) = 0x88880F00;
        REG32(0xB8801888) = 0xFFFFFFFF;
        REG8(0xB8801891)  = 0x80;
        REG8(0xB8801892)  = 0xFF;
        REG32(0xB8801894) = 0x88880F00;
        REG32(0xB8801898) = 0xFFFFFFFF;
        REG8(0xB88018A1)  = 0x80;
        REG8(0xB88018A2)  = 0xFF;
        REG32(0xB88018A4) = 0x88880F00;
        REG32(0xB88018A8) = 0xFFFFFFFF;
        busy_wait(0x200);
}

#ifndef SZ
#define SZ 256
#endif

#ifndef FREQ
#define FREQ 1066
#endif

#if SZ == 256
# define CFG_SZ DDR3_SIZE_256
#elif SZ == 512
# define CFG_SZ DDR3_SIZE_256_FOR_512
#else
# error "Not supported size, supported are 256, 512"
#endif

#if FREQ == 600 && SZ == 256
# define CFG_FREQ DDR3_600
#elif FREQ == 800
# define CFG_FREQ DDR3_800
#elif FREQ == 1066
# define CFG_FREQ DDR3_1066
#elif FREQ == 1200 && SZ == 256
# define CFG_FREQ DDR3_1200
#elif FREQ == 1333
# define CFG_FREQ DDR3_1333
#elif FREQ == 1600
# define CFG_FREQ DDR3_1600
#else
# if SZ == 256
#  error "Not supported frequency, supported are 600,800,1066,1200,1333,1600"
# else
#  error "Not supported frequency, supported are 800,1066,1333,1600"
# endif
#endif

int main()
{
        ddr3_init(CFG_SZ, CFG_FREQ);
        return 0;
}
