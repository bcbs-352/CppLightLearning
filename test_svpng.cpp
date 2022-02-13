#include "svpng.inc"
#include<stdlib.h>
int main(void) {
    unsigned char rgb[256 * 256 * 3], *p = rgb;
    unsigned x, y;
    FILE *fp = fopen("rgb.png", "wb");
    for (y = 0; y < 256; y++)
        for (x = 0; x < 256; x++) {
            *p++ = (unsigned char)x;    /* R */
            *p++ = (unsigned char)y;    /* G */
            *p++ = (int)(rand()%255);                 /* B */
        }
    svpng(fp, 256, 256, rgb, 0);
    fclose(fp);
    system("pause");
}