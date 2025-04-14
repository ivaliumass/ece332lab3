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

#define FPGA_ONCHIP_BASE (0xC8000000)
#define IMAGE_SPAN (IMAGE_WIDTH * IMAGE_HEIGHT * 4)
#define IMAGE_MASK (IMAGE_SPAN - 1)

int main(void) {
    void *virtual_base;
    void *virtual_base2;
    volatile unsigned int *video_in_dma = NULL;
    volatile unsigned short *video_mem = NULL;
    int fd;
    unsigned short pixels[IMAGE_HEIGHT * IMAGE_WIDTH];

    // open /dev/mem
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        return 1;
    }

    // memory mapping
    virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        return 1;
    }

    // Map camera buffer memory (ADDED)
    virtual_base2 = mmap(NULL, IMAGE_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_ONCHIP_BASE);
    if (video_buffer_base == MAP_FAILED) {
        printf("ERROR: camera buffer mmap() failed...\n");
        close(fd);
        return 1;
    }

    // initialize pointers
    video_in_dma = (unsigned int *)(virtual_base + VIDEO_BASE);
    video_mem = (volatile unsigned short *)(virtual_base2 + (FPGA_ONCHIP_BASE & IMAGE_MASK));

    // Access the video input DMA register
    int value = (videvideo_mem+ 3);
    printf "Video In DMA register updated at: 0x%x\n", (unsigned int)video_in_dma);
    
    // Modify the PIO register
    *(video_in_dma + 3) = 0x4;
    printf ("DB1 \n");
   
    // Capture image
    for (int y = 0; y < IMAGE_HEIGHT; y++) (
        for (int x = 0; x < IMAGE_ WIDTH; x+) (
            int index = (y * IMAGE_WIDTH) + x;
            pixels[index] = video_mem[(y<<9)+x]; // Uncommented this line
        }
    }

    printf("DB2 \n");
    
    // save image
    const char* filename = "final_image_color.bmp";
    saveImageShort(filename, pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    
    // Cleanup - uncommented and fixed
    if (munmap(virtual_base2, IMAGE_SPAN) ! = 0) (
        printf ("ERROR: munmap() for FPGA memory failed... In*");
        //munmap(virtual_base, HW_REGS_SPAN);
        close(fd);
        return 1; 

    if (munmap(virtual_base, Hi_REGS_SPAN) != 0) {
        printf("ERROR: munmap() for HW registers failed... \n*");
        close(fd);
        return 1;
    }
        close(fd);
        return 0;
}






    // Add button handling (ADDED)
    key_ptr = (volatile unsigned int *)(virtual_base + PUSH_BASE);
    int prev_key_state = 0;
    
    printf("Press any key (1-3) to capture image...\n");
    while(1) {
        int key_state = *key_ptr & 0xF;
        
        if(key_state != prev_key_state && key_state != 0) {
            // Wait for frame ready (ADDED)
            while((*(video_in_dma + 3) & 0x1) == 0);
            
            // Read pixel data (MODIFIED)
            for(int y = 0; y < IMAGE_HEIGHT; y++) {
                for(int x = 0; x < IMAGE_WIDTH; x++) {
                    unsigned short pixel = video_mem[y * IMAGE_WIDTH + x];
                    pixels[y][x] = pixel;  // RGB565 format
                    pixels_bw[y][x] = (pixel >> 8) & 0xFF;  // Grayscale conversion
                }
            }
            
            // Keep original save functions
            const char* filename = "final_image_color.bmp";
            saveImageShort(filename,&pixels[0][0],IMAGE_WIDTH,IMAGE_HEIGHT);

            const char* filename1 = "final_image_bw.bmp";
            saveImageGrayscale(filename1,&pixels_bw[0][0],IMAGE_WIDTH,IMAGE_HEIGHT);
            
            printf("Image captured!\n");
            break;
        }
        prev_key_state = key_state;
    }

    // Fix cleanup (MODIFIED)
    if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
        printf("ERROR: register munmap() failed...\n");
    }
    if (munmap(video_buffer_base, IMAGE_SPAN) != 0) {
        printf("ERROR: buffer munmap() failed...\n");
    }
    close(fd);
    return 0;
}
