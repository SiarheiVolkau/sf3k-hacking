// DDR IC: Nanya NT5CB128M16FP-EK DDR3 (1.5V), 256MB 128M/16, 1866-13-13-13, 1600-11-11-11
// PCB ID: SF3000-E-V1.0(2026.01.06)
#include <stdint.h>
#include "mmio.h"

static void busy_wait(uint32_t counter)
{
        while (counter != 0) {
                counter--;
        }
}

void ddr_init_nanya_nt5cb128m16fp_ek () {
        REG8(0xB8800074)  = 0x10;
        REG8(0xB8800076)  = 0x20;
        busy_wait(10);
        REG8(0xB880108C)  = 0x01;
        REG8(0xB8801033)  = 0x60;
        REG32(0xB883E044) = 0x0000040B;
        REG8(0xB883E03C)  = 0x80;
        REG32(0xB883E028) = 0x08C30D40;
        REG32(0xB883E02C) = 0x08013880;
        REG8(0xB883E034)  = 0x1F;
        REG32(0xB883E05C) = 0x00000008;
        REG32(0xB883E060) = 0x00000000;
        REG32(0xB883E058) = 0x00000040;
        REG32(0xB883E054) = 0x00001930;
        REG32(0xB883E048) = 0x71947744;
        REG32(0xB883E04C) = 0x1562B360;
        while((REG32(0xB883E010) & 1U) != 1U)
                ;

        REG32(0xB883E180) = 0x10018D8C;
        REG32(0xB883E190) = 0x10018D8C;
        REG32(0xB883E068) = 0x910075C7;
        REG8(0xB883E040)  = 0x1E;
        REG8(0xB883E047)  = 0x10;
        REG32(0xB883E004) = 0x0000FF81;
        while((REG32(0xB883E010) & 1U) != 1U)
                ;

        REG32(0xB883E1E4) -= 3U; /* maybe better ((x) & ~3U) */
        REG32(0xB883E224) -= 3U; /* maybe better ((x) & ~3U) */
        REG32(0xB8801000) = 0xD894084;
        REG32(0xB8801004) = 0x0939100;
        REG16(0xB8801030) = 0x5000;
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
        busy_wait(200);
}

int main()
{
        ddr_init_nanya_nt5cb128m16fp_ek();
        return 0;
}
