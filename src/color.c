#include "vector.h"
#include "color.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Vec3 unpackRGB(unsigned int packedRGB)
{
    // Extract the RGB components
    float r = (float)((packedRGB >> 16) & 0xFF) / 255.0f;
    float g = (float)((packedRGB >> 8) & 0xFF) / 255.0f;
    float b = (float)(packedRGB & 0xFF) / 255.0f;

    // Return the color as a Vec3
    return (Vec3){r, g, b};
}

void writeColour(FILE *ppmFile, Vec3 color) {
    // Scale the floating-point values (range [0.0, 1.0]) to the range [0, 255]
    // 255.999 to avoid rounding errors
    int r = (int)(color.x * 255.999);
    int g = (int)(color.y * 255.999);
    int b = (int)(color.z * 255.999);

    // Write the RGB values to the file
    fprintf(ppmFile, "%d %d %d ", r, g, b);
}

int compareColor(const void *a, const void *b)
{
    int a1 = 0, b1 = 0;
    for (size_t i = 0; i < sizeof(int); i++)
    {
        a1 |= (*((unsigned char*)a + i) & 0x0F) << (i * 8);
        b1 |= (*((unsigned char*)b + i) & 0x0F) << (i * 8);
    }
    
    return (a1 < b1) ? -1 : (b1 < a1) ? 1 : (*((int*)a) < *((int*)b)) ? -1 : (*((int*)a) > *((int*)b)) ? 1 : 0;
}