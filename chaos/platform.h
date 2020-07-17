#ifndef platform_h_seen
#define platform_h_seen


#define BIT00 1
#define BIT01 2
#define BIT02 4
#define BIT03 8
#define BIT04 16
#define BIT05 32
#define BIT06 64
#define BIT07 128
#define BIT08 256
#define BIT09 512
#define BIT10 1024
#define BIT11 2048
#define BIT12 4096
#define BIT13 8192
#define BIT14 16384
#define BIT15 32768

/*this converts a color value to 15 bit BGR value used by GBA */
#define RGB16(r,g,b)  ((r)+(g<<5)+(b<<10))
#define TILE_FLIP_VERT 0x800
#define TILE_FLIP_HORZ 0x400
#define GetRed(col)  ((col)&0x1f)
#define GetGreen(col) (((col)>>5)&0x1f)
#define GetBlue(col)  ( ((col)>>10)&0x1f)

#endif

