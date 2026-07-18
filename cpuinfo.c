#include <stdint.h>
#include "mmio.h"

struct cpu_info {
    uint32_t prid;
    uint32_t config0;
    uint32_t config1;
    uint32_t config2;
    uint32_t config3;
    uint32_t config7;
};

volatile struct cpu_info* vpcpuinfo = (volatile struct cpu_info*)0xbfe03f00;

void probe_cpu(void)
{
    __asm__ volatile(
        ".set push\n"
        ".set mips32r2\n"

        "mfc0 $8, $15, 0\n"   // PRId
        "sw   $8,  0(%0)\n"

        "mfc0 $8, $16, 0\n"   // Config0
        "sw   $8,  4(%0)\n"

        "mfc0 $8, $16, 1\n"   // Config1
        "sw   $8,  8(%0)\n"

        "mfc0 $8, $16, 2\n"   // Config2
        "sw   $8, 12(%0)\n"

        "mfc0 $8, $16, 3\n"   // Config3
        "sw   $8, 16(%0)\n"

        "mfc0 $8, $16, 7\n"   // Config7
        "sw   $8, 20(%0)\n"

        ".set pop\n"
        :
        : "r"(vpcpuinfo)
        : "t0", "memory"
    );
}

int main()
{
        probe_cpu();
        return 0;
}
