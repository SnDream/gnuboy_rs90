#include "scaler.h"

/* alekmaul's scaler taken from mame4all */
void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* restrict src, uint16_t* restrict dst)
{
    uint32_t W,H,ix,iy,x,y;
    x=startx<<16;
    y=starty<<16;
    W=newwidth;
    H=newheight;
    ix=(viswidth<<16)/W;
    iy=(visheight<<16)/H;

    do 
    {
        uint16_t* restrict buffer_mem=&src[(y>>16)*pitchsrc];
        W=newwidth; x=startx<<16;
        do 
        {
            *dst++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        dst+=pitchdest;
        y+=iy;
    } while (--H);
}

inline uint16_t mix_rgb565(uint16_t p1, uint16_t p2) {
	return ((p1 & 0xF7DE) >> 1) + ((p2 & 0xF7DE) >> 1) + (p1 & p2 & 0x0821);
}

#define RMASK 0b1111100000000000
#define GMASK 0b0000011111100000
#define BMASK 0b0000000000011111
#define RSHIFT(X) (((X) & 0xF7DE) >>1) 
//3:2 Alt stretch (sub-pixel scaling)
void upscale_160x144_to_212x144(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + 240 * 8;
    const uint16_t ix=3, iy=1;
    
    for (int y = 0; y < 144; y+=iy)
    {
        d+=14;
        int x =0;
        buffer_mem = &src[y * 160];
        for(int w =0; w < 160/3; w++)
        {
            uint16_t r0,r1,g1,b1,r2,g2,b2;
            r0 = buffer_mem[x] & RMASK;
            g1 = buffer_mem[x + 1] & GMASK;
            b1 = buffer_mem[x + 1] & BMASK;
            r1 = buffer_mem[x + 1] & RMASK;
            r2 = buffer_mem[x + 2] & RMASK;
            g2 = buffer_mem[x + 2] & GMASK;
            b2 = buffer_mem[x + 2] & BMASK;
            
            *d++ = buffer_mem[x];
            *d++ = (((r0 + r1) >> 1) & RMASK) | g1 | b1;
            *d++ = r1 | (((g1 + g2) >>1 ) & GMASK) | 
                (((b2 + b1) >> 1 ) & BMASK);
            *d++ = buffer_mem[x + 2];
            x += ix;
        }
        *d = buffer_mem[x];
        d+=14;
    }
}

//upscale 4:3
void upscale_160x144_to_212x160(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst;
    const uint16_t ix = 3, iy = 9;
    
    for (int y = 0; y < 144; y+=iy)
    {
        d += 14;
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160 / 3; w++)
        {
            uint16_t c[4][9];
            for(int i = 0; i < 9; i++){
                uint16_t r0,r1,g1,b1,r2,g2,b2;
                r0 = buffer_mem[x + i * 160] & RMASK;
                g1 = buffer_mem[x + i * 160 + 1] & GMASK;
                b1 = buffer_mem[x + i * 160 + 1] & BMASK;
                r1 = buffer_mem[x + i * 160 + 1] & RMASK;
                r2 = buffer_mem[x + i * 160 + 2] & RMASK;
                g2 = buffer_mem[x + i * 160 + 2] & GMASK;
                b2 = buffer_mem[x + i * 160 + 2] & BMASK;
            
                c[0][i] = buffer_mem[x + i * 160];
                c[1][i] = (((r0 + r1) >> 1) & RMASK) | g1 | b1;
                c[2][i] = r1 | (((g1 + g2) >>1 ) & GMASK) | 
                (((b2 + b1) >> 1 ) & BMASK);
                c[3][i]= buffer_mem[x + i * 160 + 2];
            }
            for(int i = 0; i < 4 ; i++){
                *d             = c[i][0];
                *(d + 240)     = c[i][1];
                *(d + 240 * 2) = c[i][2];
                *(d + 240 * 3) = c[i][3];
                *(d + 240 * 4) = c[i][4];
                *(d + 240 * 5) = c[i][5];
                *(d + 240 * 6) = c[i][6];
                *(d + 240 * 7) = c[i][7];
                uint16_t r0, g0, b0, r1, g1, b1;
                r0 = c[i][7] & RMASK;
                g0 = c[i][7] & GMASK;
                b0 = c[i][7] & BMASK;
                r1 = c[i][8] & RMASK;
                g1 = c[i][8] & GMASK;
                b1 = c[i][8] & BMASK;
                *(d + 240 * 8) = (((r0>>1) + (r1>>1)) & RMASK) |
                                (((g0 + g1)>>1) & GMASK) |
                                (((b0 + b1)>>1) & BMASK);
                *(d + 240 * 9) = c[i][8];
                d++;
            }
            x += ix;
        }
        for(int i =0 ; i < 10 ; i++){
            *(d + 240 * i) = buffer_mem[x + i * 160];
        }
        
        d += 14;
        d += 240 * 9;
    }
}

//3:2 Full Screen
void upscale_160x144_to_240x160(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst;
    const uint16_t ix = 2, iy = 9;
    
    for (int y = 0; y < 144; y+=iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160 / 2; w++)
        {
            uint16_t c[3][9];
            for(int i = 0; i < 9; i++){
                uint16_t r0, g0, b0, r1, g1, b1;
                r0 = buffer_mem[x + i * 160]     & RMASK;
                g0 = buffer_mem[x + i * 160]     & GMASK;
                b0 = buffer_mem[x + i * 160]     & BMASK;
                r1 = buffer_mem[x + i * 160 + 1] & RMASK;
                g1 = buffer_mem[x + i * 160 + 1] & GMASK;
                b1 = buffer_mem[x + i * 160 + 1] & BMASK;

                c[0][i] = buffer_mem[x + i * 160];
                c[1][i] = (((r0>>1) + (r0>>2) + (r1>>2)) & RMASK) |
                    (((g0 + g1)>>1) & GMASK) |
                    (((((b0 + b1)>>1) + b1)>>1) & BMASK);
                c[2][i] = buffer_mem[x + i * 160 + 1];
            }
            for(int i = 0; i < 3 ; i++){
                *d             = c[i][0];
                *(d + 240)     = c[i][1];
                *(d + 240 * 2) = c[i][2];
                *(d + 240 * 3) = c[i][3];
                *(d + 240 * 4) = c[i][4];
                *(d + 240 * 5) = c[i][5];
                *(d + 240 * 6) = c[i][6];
                *(d + 240 * 7) = c[i][7];
                uint16_t r0, g0, b0, r1, g1, b1;
                r0 = c[i][7] & RMASK;
                g0 = c[i][7] & GMASK;
                b0 = c[i][7] & BMASK;
                r1 = c[i][8] & RMASK;
                g1 = c[i][8] & GMASK;
                b1 = c[i][8] & BMASK;
                *(d + 240 * 8) = (((r0>>1) + (r1>>1)) & RMASK) |
                                (((g0 + g1)>>1) & GMASK) |
                                (((b0 + b1)>>1) & BMASK);
                *(d + 240 * 9) = c[i][8];
                d++;
            }
            x += ix;
        }
        d += 240 * 9;
    }
}

//upscale 5:3
void upscale_160x144_to_240x144(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + 240 * 8;
    const uint16_t ix = 2, iy = 1;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160 / 2; w++)
        {
            uint16_t rgb1, rgb2;
            rgb1 = buffer_mem[x + 0 * 160 + 0];
            rgb2 = buffer_mem[x + 0 * 160 + 1];

            *(d + 0 * 320 + 0) = rgb1;
            *(d + 0 * 320 + 1) = mix_rgb565(rgb1, rgb2);
            *(d + 0 * 320 + 2) = rgb2;

            d += 3;
            x += ix;
        }
    }
}

//upscale 4:3 in rg99
void upscale_160x144_to_320x240(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst;
    const uint16_t ix = 1, iy = 3;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160; w++)
        {
            uint16_t r0, g0, b0, r1, b1, g1, r2, b2, g2, rgb01, rgb12;
            r0 = buffer_mem[x + 0 * 160] & RMASK;
            g0 = buffer_mem[x + 0 * 160] & GMASK;
            b0 = buffer_mem[x + 0 * 160] & BMASK;
            r1 = buffer_mem[x + 1 * 160] & RMASK;
            g1 = buffer_mem[x + 1 * 160] & GMASK;
            b1 = buffer_mem[x + 1 * 160] & BMASK;
            r2 = buffer_mem[x + 2 * 160] & RMASK;
            g2 = buffer_mem[x + 2 * 160] & GMASK;
            b2 = buffer_mem[x + 2 * 160] & BMASK;
            rgb01 =   ((((uint32_t)r0 + r0 + r1) / 3) & (RMASK))
                    | ((((uint32_t)g0 + g0 + g1) / 3) & (GMASK))
                    | ((((uint32_t)b0 + b0 + b1) / 3) & (BMASK));
            rgb12 =   ((((uint32_t)r2 + r2 + r1) / 3) & (RMASK))
                    | ((((uint32_t)g2 + g2 + g1) / 3) & (GMASK))
                    | ((((uint32_t)b2 + b2 + b1) / 3) & (BMASK));

            *(d + 0 * 320 + 0) = buffer_mem[x + 0 * 160];
            *(d + 0 * 320 + 1) = buffer_mem[x + 0 * 160];
            *(d + 1 * 320 + 0) = rgb01;
            *(d + 1 * 320 + 1) = rgb01;
            *(d + 2 * 320 + 0) = buffer_mem[x + 1 * 160];
            *(d + 2 * 320 + 1) = buffer_mem[x + 1 * 160];
            *(d + 3 * 320 + 0) = rgb12;
            *(d + 3 * 320 + 1) = rgb12;
            *(d + 4 * 320 + 0) = buffer_mem[x + 2 * 160];
            *(d + 4 * 320 + 1) = buffer_mem[x + 2 * 160];

            d += 2;
            x += ix;
        }
        d += 320 * 4;
    }
}

//upscale 40:27 in rg99
void upscale_160x144_to_320x216(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + (320 * 12);
    const uint16_t ix = 1, iy = 2;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160; w++)
        {
            uint16_t rgb01;
            rgb01 = mix_rgb565(buffer_mem[x + 0 * 160], buffer_mem[x + 1 * 160]);

            *(d + 0 * 320 + 0) = buffer_mem[x + 0 * 160];
            *(d + 0 * 320 + 1) = buffer_mem[x + 0 * 160];
            *(d + 1 * 320 + 0) = rgb01;
            *(d + 1 * 320 + 1) = rgb01;
            *(d + 2 * 320 + 0) = buffer_mem[x + 1 * 160];
            *(d + 2 * 320 + 1) = buffer_mem[x + 1 * 160];

            d += 2;
            x += ix;
        }
        d += 320 * 2;
    }
}

//upscale 1:1(1.5x) in rg99
void upscale_160x144_to_240x216(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + (320 * 12 + 40);
    const uint16_t ix = 2, iy = 2;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160 / 2; w++)
        {
            uint16_t rgb1, rgb2, rgb3, rgb4, rgb5, rgb6;
            rgb1 = buffer_mem[x + 0 * 160 + 0];
            rgb2 = buffer_mem[x + 0 * 160 + 1];
            rgb3 = buffer_mem[x + 1 * 160 + 0];
            rgb4 = buffer_mem[x + 1 * 160 + 1];
            rgb5 = mix_rgb565(rgb1, rgb2);
            rgb6 = mix_rgb565(rgb3, rgb4);

            *(d + 0 * 320 + 0) = rgb1;
            *(d + 0 * 320 + 1) = rgb5;
            *(d + 0 * 320 + 2) = rgb2;
            *(d + 1 * 320 + 0) = mix_rgb565(rgb1, rgb3);
            *(d + 1 * 320 + 1) = mix_rgb565(rgb5, rgb6);
            *(d + 1 * 320 + 2) = mix_rgb565(rgb2, rgb4);
            *(d + 2 * 320 + 0) = rgb3;
            *(d + 2 * 320 + 1) = rgb6;
            *(d + 2 * 320 + 2) = rgb4;

            d += 3;
            x += ix;
        }
        d += 320 * 2 + 80;
    }
}


//upscale 4:3 in rg99
void upscale_160x144_to_320x480(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst;
    const uint16_t ix = 1, iy = 3;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160; w++)
        {
            uint16_t rgb0, rgb1, rgb2;
            uint16_t r0, g0, b0, r1, b1, g1, r2, b2, g2, rgb01, rgb12;
            rgb0 =buffer_mem[x + 0 * 160];
            rgb1 =buffer_mem[x + 1 * 160];
            rgb2 =buffer_mem[x + 2 * 160];
            r0 = rgb0 & RMASK;
            g0 = rgb0 & GMASK;
            b0 = rgb0 & BMASK;
            r1 = rgb1 & RMASK;
            g1 = rgb1 & GMASK;
            b1 = rgb1 & BMASK;
            r2 = rgb2 & RMASK;
            g2 = rgb2 & GMASK;
            b2 = rgb2 & BMASK;
            rgb01 =   ((((uint32_t)r0 + r1 + r1) / 3) & (RMASK))
                    | ((((uint32_t)g0 + g1 + g1) / 3) & (GMASK))
                    | ((((uint32_t)b0 + b1 + b1) / 3) & (BMASK));
            rgb12 =   ((((uint32_t)r2 + r1 + r1) / 3) & (RMASK))
                    | ((((uint32_t)g2 + g1 + g1) / 3) & (GMASK))
                    | ((((uint32_t)b2 + b1 + b1) / 3) & (BMASK));

            *(d + 0 * 320 + 0) = rgb0;
            *(d + 0 * 320 + 1) = rgb0;
            *(d + 1 * 320 + 0) = rgb0;
            *(d + 1 * 320 + 1) = rgb0;
            *(d + 2 * 320 + 0) = rgb0;
            *(d + 2 * 320 + 1) = rgb0;
            *(d + 3 * 320 + 0) = rgb01;
            *(d + 3 * 320 + 1) = rgb01;
            *(d + 4 * 320 + 0) = rgb1;
            *(d + 4 * 320 + 1) = rgb1;
            *(d + 5 * 320 + 0) = rgb1;
            *(d + 5 * 320 + 1) = rgb1;
            *(d + 6 * 320 + 0) = rgb12;
            *(d + 6 * 320 + 1) = rgb12;
            *(d + 7 * 320 + 0) = rgb2;
            *(d + 7 * 320 + 1) = rgb2;
            *(d + 8 * 320 + 0) = rgb2;
            *(d + 8 * 320 + 1) = rgb2;
            *(d + 9 * 320 + 0) = rgb2;
            *(d + 9 * 320 + 1) = rgb2;

            d += 2;
            x += ix;
        }
        d += 320 * 9;
    }
}

//upscale 40:27 in rg99
void upscale_160x144_to_320x432(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + (320 * 12 * 2);
    const uint16_t ix = 1, iy = 1;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160; w++)
        {
            uint16_t rgb = buffer_mem[x + 0 * 160];
            *(d + 0 * 320 + 0) = rgb;
            *(d + 0 * 320 + 1) = rgb;
            *(d + 1 * 320 + 0) = rgb;
            *(d + 1 * 320 + 1) = rgb;
            *(d + 2 * 320 + 0) = rgb;
            *(d + 2 * 320 + 1) = rgb;

            d += 2;
            x += ix;
        }
        d += 320 * 2;
    }
}

//upscale 1:1(1.5x) in rg99
void upscale_160x144_to_240x432(uint16_t* restrict src, uint16_t* restrict dst){    
    uint16_t* __restrict__ buffer_mem;
    uint16_t* d = dst + (320 * 12 * 2 + 40);
    const uint16_t ix = 2, iy = 1;
    
    for (int y = 0; y < 144; y += iy)
    {
        int x = 0;
        buffer_mem = &src[y * 160];
        for(int w = 0; w < 160 / 2; w++)
        {
            uint16_t rgb0, rgb1, rgb01;
            rgb0 = buffer_mem[x + 0 * 160 + 0];
            rgb1 = buffer_mem[x + 0 * 160 + 1];
            rgb01 = mix_rgb565(rgb0, rgb1);

            *(d + 0 * 320 + 0) = rgb0;
            *(d + 1 * 320 + 0) = rgb0;
            *(d + 2 * 320 + 0) = rgb0;
            *(d + 0 * 320 + 1) = mix_rgb565(rgb0, rgb1);
            *(d + 1 * 320 + 1) = mix_rgb565(rgb0, rgb1);
            *(d + 2 * 320 + 1) = mix_rgb565(rgb0, rgb1);
            *(d + 0 * 320 + 2) = rgb1;
            *(d + 1 * 320 + 2) = rgb1;
            *(d + 2 * 320 + 2) = rgb1;

            d += 3;
            x += ix;
        }
        d += 320 * 2 + 80;
    }
}
