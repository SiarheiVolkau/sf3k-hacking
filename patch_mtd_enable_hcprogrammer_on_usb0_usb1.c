#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void)
{
    int fd;
    struct mtd_info_user mtd;
    struct erase_info_user erase;
    uint8_t *buf;
    ssize_t ret;
    const int spl_size = 16384;

    fd = open("/dev/mtd0", O_SYNC | O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (ioctl(fd, MEMGETINFO, &mtd) < 0) {
        perror("MEMGETINFO");
        close(fd);
        return 2;
    }

    if (mtd.erasesize != 4096) {
        fprintf(stderr,
                "Unexpected erase size %u (expected 4096)\n",
                mtd.erasesize);
        close(fd);
        return 3;
    }

    buf = malloc(spl_size);
    if (!buf) {
        perror("malloc");
        close(fd);
        return 4;
    }

    /* Read first erase block */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        ret = 5;
        goto err;
    }

    ret = read(fd, buf, spl_size);
    if (ret != (ssize_t)spl_size) {
        perror("read");
        ret = 6;
        goto err;
    }

    /* Patch two bytes to enable hcprogrammer on both USBs */
    buf[0x30] = 0x01;
    buf[0x31] = 0x01;

#if 0 /* disabled bc not used */
    /* here is located some absolute time for hcprogrammer
     * let's make it long enough to keep usb working
     * initially it is 0 which might prevent usb from
     * working properly
     */
    buf[0x34] = 0xff;
    buf[0x35] = 0xff;
    buf[0x36] = 0xff;
    buf[0x37] = 0x7f;

    /* here is located some relative timeout for hcprogrammer
     * let's make it long enough to keep usb working
     * initially it is 0 which might prevent usb from
     * working properly
     */
    buf[0x38] = 0xff;
    buf[0x39] = 0xff;
    buf[0x3a] = 0xff;
    buf[0x3b] = 0x7f;
#if 0 /* restore back to original */
    buf[0x34] = 0x00;
    buf[0x35] = 0x00;
    buf[0x36] = 0x00;
    buf[0x37] = 0x00;
    buf[0x38] = 0x00;
    buf[0x39] = 0x00;
    buf[0x3a] = 0x00;
    buf[0x3b] = 0x00;
#endif
#endif
    /* patching instruction, to make usb enumeration work */
    /* usb stack always sent 32 bytes on configuration request
     * linux expects to get only what it was requested (9 bytes)
     * so we have to patch: "li v0, 32" to "lbu v0, (received_wLength_address)"
     * closest address already located in v0 register, offset from
     * wLength is -58 bytes, so the final instruction is "lbu v0, -58(v0)"
     * Moreover the SPL is obfuscated by xor'ing with 0x7e3f9c2d so
     * below is obfuscated instruction.
     */
    buf[0x1c90] = 0xEB;
    buf[0x1c91] = 0x63;
    buf[0x1c92] = 0x7D;
    buf[0x1c93] = 0xEE;

    /* patching idle timeout from 5 seconds to 30 second
     * was "sltiu v0,v0, 5001"
     * now "sltiu v0,v0, 30001"
     * Obfuscated as well.
     */
    buf[0x24cc] = 0x1C;
    buf[0x24cd] = 0xE9;
    buf[0x24ce] = 0x7D;
    buf[0x24cf] = 0x52;

    /* Erase first block */
    erase.start = 0;
    erase.length = spl_size;

    fprintf(stdout, "ioctl MEMERASE\n");
    if (ioctl(fd, MEMERASE, &erase) < 0) {
        perror("MEMERASE");
        ret = 7;
        goto err;
    }

    /* Write it back */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        ret = 8;
        goto err;
    }

    ret = write(fd, buf, spl_size);
    if (ret != (ssize_t)spl_size) {
        perror("write");
        ret = 9;
        goto err;
    }

    fsync(fd);

    printf("Patched successfully.\n");

    free(buf);
    close(fd);
    return 0;

err:
    free(buf);
    close(fd);
    return ret;
}
