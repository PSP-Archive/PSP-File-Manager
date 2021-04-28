
// primitive graphics for Hello World PSP

void pgInit();
void pgWaitV();
void pgWaitVn(unsigned long count);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgFillvram(unsigned long color);
void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);
//void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);

void pgPrintCursor(int* xRef, int* yRef, unsigned long color, const char *str);

//typedef struct _ctrl_data 
//{ 
//	unsigned long frame; 			// ŽžŠÔ(tick)
//   unsigned long buttons;
//   unsigned char  analog[4]; 		// [0]:X [1]:Y
//   unsigned long unused; 
//} ctrl_data_t; 

enum { 
    TYPE_DIR=0x10, 
    TYPE_FILE=0x20 
}; 


typedef struct dirent { 
    unsigned long unk0; 
    unsigned long type; 
    unsigned long size; 
    unsigned long unk[19]; 
    char name[0x108]; 
	unsigned char dmy[128];
} dirent_t;

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272

#define		PIXELSIZE	1				//in short
#define		LINESIZE	512				//in short
#define		FRAMESIZE	0x44000			//in byte

/* Index for the two analog directions */ 
#define CTRL_ANALOG_X   0 
#define CTRL_ANALOG_Y   1 


//
//int sceIoOpen(const char* file, int mode, int unknown); 
//int  sceIoRead(int fd, void *data, int size);  
//int  sceIoRemove(const char *file); 
//int  sceIoMkdir(const char *dir, int mode); 
//int  sceIoRmdir(const char *dir); 
//int  sceIoRename(const char *oldname, const char *newname); 
//int sceIoDevctl(const char *name int cmd, void *arg, size_t arglen, void *buf, size_t *buflen); 
//int  sceIoDopen(const char *fn); 

//int sceIoIoctl(int fd, int cmd, int argR6, int argR7, int argR8, int argR9);

//int sceKernelLoadExec(const char *file, int zero);

