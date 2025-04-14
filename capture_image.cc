#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include "bmp_utility.h"

#define HW_REGS_BASE (0xff200000)
#define HW_REGS_SPAN (0x00200000)
#define HW_REGS_MASK (HW_REGS_SPAN - 1)
#define LED_BASE 0x1000
#define PUSH_BASE 0x3010
#define VIDEO_BASE 0x0000

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240

#define FPGA_ONCHIP_BASE     (0xC8000000)
#define IMAGE_SPAN (IMAGE_WIDTH * IMAGE_HEIGHT * 4)
#define IMAGE_MASK (IMAGE_SPAN - 1)

int main(void){
    void *virtual_base;
    void *virtual_base2;
    volatile unsigned int *video_in_dma = NULL;
    volatile unsigned short *video_mem = NULL;
    int fd;
    unsigned short pixels[IMAGE_WIDTH * IMAGE_HEIGHT];
    unsigned char pixels_bw[IMAGE_WIDTH * IMAGE_HEIGHT];
    
    // Open /dev/mem
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        return 1;
    }

    // Map HW registers into virtual address space
    virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() for HW registers failed...\n");
        close(fd);
        return 1;
    }

    // Map FPGA on-chip memory into virtual address space
    virtual_base2 = mmap(NULL, IMAGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE); // base addr for mapped mem
    if (virtual_base2 == MAP_FAILED) {
        printf("ERROR: mmap() for FPGA on-chip memory failed...\n");
        munmap(virtual_base, HW_REGS_SPAN);
        close(fd);
        return 1;
    }

    // Initialize pointers
    video_in_dma = (unsigned int *)(virtual_base + VIDEO_BASE);
    video_mem = (volatile unsigned short *)(virtual_base2 +(FPGA_ONCHIP_BASE & IMAGE_MASK));// chip

    // Access the video input DMA register
    int value = *(video_in_dma + 3);
    printf("Video In DMA register updated at: 0x%x\n", (unsigned int)video_in_dma);

    // Modify the PIO register
    *(video_in_dma + 3) = 0x4;
    printf("DB1 \n");

    // Capture image
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            int index = (y * IMAGE_WIDTH) + x;
            pixels[index] = video_mem[(y<<9)+x];  // Uncommented this line
            pixels_bw[index] = video_mem[(y<<9) +x];
        }
    }

    printf("DB2 \n");
    const char* filename1 = "final_image_bw.bmp";
    // Save image
    const char* filename = "final_image_color.bmp";
    saveImageShort(filename, pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    saveImageGrayscale(filename1, pixels_bw, IMAGE_WIDTH, IMAGE_HEIGHT);


    // Cleanup - uncommented and fixed
    if (munmap(virtual_base2, IMAGE_SPAN) != 0) {
        printf("ERROR: munmap() for FPGA memory failed...\n");
        //munmap(virtual_base, HW_REGS_SPAN);
        close(fd);
        return 1;
    }

    if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
        printf("ERROR: munmap() for HW registers failed...\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
