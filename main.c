// Doom-PSP File Selector By Cpasjuste @ psp.neuf.fr

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_dirent.h>
#include <stdio.h>
#include <pspctrl.h>
#define printf	pspDebugScreenPrintf

PSP_MODULE_INFO("SDKTEST", 0x1000, 1, 1);   // 0x1000 = Kernel MODE
PSP_MAIN_THREAD_ATTR(0); // 0 for kernel mode too

#include "pg.h" 
#include "font.c"
#include "dbasefixed/appmain.c"
#include "dbasefixed/app_confirm.c"
#include "dbasefixed/app_confirm2.c"

int backgroundWidth = 480;
int backgroundHeight = 272;

// EXIT //

/* Exit callback */
int exit_callback(void)
{
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
void CallbackThread(void *arg)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();
}


/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}
////END///

unsigned short rgb2col(unsigned char r,unsigned char g,unsigned char b) {
	return ((((b>>3) & 0x1F)<<10)+(((g>>3) & 0x1F)<<5)+(((r>>3) & 0x1F)<<0)+0x8000);
}


char *mh_strncpy(char *s1,char *s2,int n) {
	char *rt = s1;

	while((*s2!=0) && ((n-1)>0)) {
		*s1 = *s2;
		s1++;
		s2++;
		n--;
	}
	*s1 = 0;

	return rt;
}

char *mh_strncat(char *s1,char *s2,int n) {
	char *rt = s1;

	while((*s1!=0) && ((n-1)>0)) {
		s1++;
		n--;
	}

	while((*s2!=0) && ((n-1)>0)) {
		*s1 = *s2;
		s1++;
		s2++;
		n--;
	}
	*s1 = 0;

	return rt;
}

void wait_button(void) {
	ctrl_data_t ctl;
	int btn;

	btn=1;
	while(btn!=0){
		sceCtrlReadBufferPositive(&ctl,1);
		btn = ((ctl.buttons & 0xF000) != 0);
	}
	btn=0;
	while(btn==0){
		sceCtrlReadBufferPositive(&ctl,1);
		btn = ((ctl.buttons & 0xF000) != 0);
	}
}

// ---------------------------------------------------------------------------------

unsigned short num2elisa(unsigned short c) {
	if (c > 4374) {
		return 0x6b;
	} else if (c >= 1410) {
		return c + (0x20c - 1410);
	} else if (c >= 690) {
		return 0x6b;
	} else if (c >= 658) {
		return c + (0x1ec - 658);
	} else if (c >= 612) {
		return c + (0x1cb - 612);
	} else if (c >= 564) {
		return c + (0x1aa - 564);
	} else if (c >= 502) {
		return c + (0x192 - 502);
	} else if (c >= 470) {
		return c + (0x17a - 470);
	} else if (c >= 376) {
		return c + (0x124 - 376);
	} else if (c >= 282) {
		return c + (0xd1 - 282);
	} else if (c >= 252) {
		return c + (0xb7 - 252);
	} else if (c >= 220) {
		return c + (0x9d - 220);
	} else if (c >= 203) {
		return c + (0x93 - 203);
	} else if (c >= 187) {
		return 0x92;
	} else if (c >= 175) {
		return c + (0x8a - 203);
	} else if (c >= 153) {
		return c + (0x7b - 153);
	} else if (c >= 135) {
		return c + (0x74 - 135);
	} else if (c >= 119) {
		return c + (0x6c - 119);
	} else {
		return c;
	}
}

void Draw_Char_Hankaku(int x,int y,unsigned char ch,int col,int backcol,int fill) {
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	int x1,y1;

	// mapping
	if (ch<0x20) {
		ch = 0;
	} else if (ch<0x80) {
		ch -= 0x20;
	} else if (ch<0xa0) {
		ch = 0;
	} else {
		ch -= 0x40;
	}
	fnt = (unsigned char *)&hankaku_font10[ch*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<5;x1++) {
			if (pt & 1) {
				*vr = col;
			} else {
				if (fill) *vr = backcol;
			}
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-5;
	}
}

void Draw_Char_Zenkaku(int x,int y,unsigned char u,unsigned char d,int col,int backcol,int fill) {
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	// mapping
	if (d > 0x7F) d--;
	d -= 0x40; u -= 0x81;
	fnt = (unsigned short *)&zenkaku_font10[num2elisa(u*0xbc+d)*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<10;x1++) {
			if (pt & 1) {
				*vr = col;
			} else {
				if (fill) *vr = backcol;
			}
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-10;
	}
}

void mh_print(int x,int y,unsigned char *str,int col,int backcol,int fill) {
	unsigned char ch = 0,bef = 0;

	while(*str != 0) {
		ch = *str++;
		if (bef!=0) {
			Draw_Char_Zenkaku(x,y,bef,ch,col,backcol,fill);
			x+=10;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {
				Draw_Char_Hankaku(x,y,ch,col,backcol,fill);
				x+=5;
			}
		}
	}
}

// ---------------------------------------------------------------------------------
#define MAXDEPTH  16
#define MAXDIRNUM 1024
#define MAXPATH   0x108
#define PATHLIST_H 20
#define REPEAT_TIME 0x40000
#define BUFSIZE		65536

dirent_t		dlist[MAXDIRNUM];
int				dlist_num;
char			now_path[MAXPATH];
char			target[MAXPATH];
char			path_tmp[MAXPATH];
int				dlist_start;
int				dlist_curpos;
int				cbuf_start[MAXDEPTH];
int				cbuf_curpos[MAXDEPTH];
int				now_depth;
char			buf[BUFSIZE];

static unsigned long control_bef_ctl  = 0;
static unsigned long control_bef_tick = 0;

unsigned long Read_Key(void) {
	ctrl_data_t ctl;

	sceCtrlReadBufferPositive(&ctl,1);
	if (ctl.buttons == control_bef_ctl) {
		if ((ctl.frame - control_bef_tick) > REPEAT_TIME) {
			return control_bef_ctl;
		}
		return 0;
	}
	control_bef_ctl  = ctl.buttons;
	control_bef_tick = ctl.frame;
	return control_bef_ctl;
}

void Get_DirList(char *path) {
	int ret,fd;

	// Directory read
	fd = sceIoDopen(path);
	dlist_num = 0;
	ret = 1;
	while((ret>0) && (dlist_num<MAXDIRNUM)) {
		ret = sceIoDread(fd, &dlist[dlist_num]);
		if (dlist[dlist_num].name[0] == '.') continue;
		if (ret>0) dlist_num++;
	}
	sceIoDclose(fd);

	// ディレクトリリストエラーチェック
	if (dlist_start  >= dlist_num) { dlist_start  = dlist_num-1; }
	if (dlist_start  <  0)         { dlist_start  = 0;           }
	if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
	if (dlist_curpos <  0)         { dlist_curpos = 0;           }
}

void backg(void)
{
	pgFlipDrawFrame();
	unsigned short* vram = (unsigned short*) pgGetVramAddr(0, 0);
	int xo, yo;
	unsigned short* image = backgroundData;
	for (yo = 0; yo < backgroundHeight; yo++) {
		for (xo = 0; xo < backgroundWidth; xo++) {
			if (*image != 0x8000) vram[xo + yo * 512] = *image;
			image++;
		}
	}
}

void backgConfirm(void)
{
	pgFlipDrawFrame();
	unsigned short* vram = (unsigned short*) pgGetVramAddr(0, 0);
	int xo, yo;
	unsigned short* image = backgroundData2;
	for (yo = 0; yo < backgroundHeight; yo++) {
		for (xo = 0; xo < backgroundWidth; xo++) {
			if (*image != 0x8000) vram[xo + yo * 512] = *image;
			image++;
		}
	}
}

void backgConfirm2(void)
{
	pgFlipDrawFrame();
	unsigned short* vram = (unsigned short*) pgGetVramAddr(0, 0);
	int xo, yo;
	unsigned short* image = backgroundData3;
	for (yo = 0; yo < backgroundHeight; yo++) {
		for (xo = 0; xo < backgroundWidth; xo++) {
			if (*image != 0x8000) vram[xo + yo * 512] = *image;
			image++;
		}
	}
}

void Draw_All(void) {
	int			i,col,fd3,fd4;
	char tpath[256];
	char tfile[256];
	
	fd3 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd3, tpath, 255);
	sceIoClose(fd3);

	fd4 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM2.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd4, tfile, 255);	
	sceIoClose(fd4);

	backg();

	// 現在地
	mh_strncpy(path_tmp,now_path,40);
	mh_print(10,9,path_tmp,rgb2col(255,255,255),0,0);
	mh_print(10,241,"CURRENT TARGET FOLDER :",rgb2col(255,255,255),0,0);
	mh_print(130,241,tfile,rgb2col(0,255,0),0,0);
	mh_print(10,257,"CURRENT TARGET FILE :",rgb2col(255,255,255),0,0);
	mh_print(125,257,tpath,rgb2col(148,0,211),0,0);
	mh_print(365,0,"PFM v02b1 @ Cpasjuste",rgb2col(255,255,0),0,0);

	// ディレクトリリスト
	i = dlist_start;
	while (i<(dlist_start+PATHLIST_H)) {
		if (i<dlist_num) {
			col = rgb2col(0,0,0);
			if (dlist[i].type & TYPE_DIR) {
				col = rgb2col(112,128,144);
			}
			if (i==dlist_curpos) {
				col = rgb2col(255,0,0);
			}
			mh_strncpy(path_tmp,dlist[i].name,40);
			mh_print(20,((i-dlist_start)+4)*9,path_tmp,col,0,0);
		}
		i++;
	}
	pgFlipShowFrameV();
}


void CopyToFolder(void) {
	int fd1,fd2,fd3,fd4,len;
	char tfolder[256];
	char tfile[BUFSIZE];

	backgConfirm();
	mh_print(180,150,"COPYING...",rgb2col(255,255,255),0,0);
	pgFlipShowFrameV();
	
	fd3 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM2.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd3, tfolder, 255);

	fd4 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM3.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd4, tfile, BUFSIZE);

	strcat(tfolder, tfile);

	fd1 = sceIoOpen(target,PSP_O_RDONLY, 0777);
	fd2 = sceIoOpen(tfolder,PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	while(1) {
		len = sceIoRead(fd1, buf, BUFSIZE);
		if (len == 0) break;
		sceIoWrite(fd2,buf,len);
	}
	sceIoClose(fd1);
	sceIoClose(fd2);
	sceIoClose(fd3);
	sceIoClose(fd4);
}


void DelExecute(void) {
	int fd1;

	fd1 = sceIoRemove(target);
	sceIoClose(fd1);

}

void GetTfile(void) {
	int fd;
	char tfile[256];	

	fd = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd, tfile, 255);
	sceIoClose(fd);
	mh_print(10,257,"CURRENT TARGET FILE :",rgb2col(255,255,255),0,0);
	mh_print(125,257,tfile,rgb2col(148,0,211),0,0);
	}

void GetTfolder(void) {
	int fd;
	char tfolder[256];	

	fd = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM2.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd, tfolder, 255);
	sceIoClose(fd);
	mh_print(10,241,"CURRENT TARGET FOLDER :",rgb2col(255,255,255),0,0);
	mh_print(130,241,tfolder,rgb2col(0,255,0),0,0);
	}


void GetTargetPath(void) {
	int fd;
	
	fd = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM2.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	sceIoWrite(fd,now_path, 255);
	sceIoClose(fd);
	}

void GetTargetFile(void) {
	int fd;

	fd = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	sceIoWrite(fd,target, 255);
	sceIoClose(fd);	
	
}

void Draw_Confirm_Modules(void) {

	backgConfirm2();

	mh_print(10,8,"MODULES MENU",rgb2col(255,255,255),0,0);

	mh_print(60,106,"TO COPY UMD TO MEMORY STICK",rgb2col(0,0,0),0,0);
	mh_print(60,140,"TO LAUNCH FTPD SERVER",rgb2col(0,0,0),0,0);

	mh_print(60,172,"TO CANCEL",rgb2col(0,0,0),0,0);
	GetTfile();
	GetTfolder();
	pgFlipShowFrameV();
}


void Draw_Confirm(void) {

	backgConfirm();

	mh_print(10,8,"OPTION MENU",rgb2col(255,255,255),0,0);
	mh_print(35,25,"SELECTED FILE :",rgb2col(0,0,0),0,0);
	mh_print(36,35,target         ,rgb2col(255,0,0)  ,0,0);


	mh_print(60,106,"TO COPY",rgb2col(0,0,0),0,0);
	mh_print(100,106,"SELECTED FILE",rgb2col(255,0,0),0,0);
	mh_print(170,106,"TO",rgb2col(0,0,0),0,0);
	mh_print(185,106,"TARGET FOLDER",rgb2col(0,255,0),0,0);

	mh_print(60,140,"TO REPLACE",rgb2col(0,0,0),0,0);
	mh_print(115,140,"TARGET FILE",rgb2col(148,0,211),0,0);
	mh_print(175,140,"WITH",rgb2col(0,0,0),0,0);
	mh_print(200,140,"SELECTED FILE",rgb2col(255,0,0),0,0);

	mh_print(60,172,"TO CANCEL",rgb2col(0,0,0),0,0);
	GetTfile();
	GetTfolder();
	pgFlipShowFrameV();
}

void Draw_Confirm_LoadExec(void) {

	backgConfirm2();

	mh_print(10,8,"OPTION MENU",rgb2col(255,255,255),0,0);
	mh_print(35,25,"SELECTED FILE :",rgb2col(0,0,0),0,0);
	mh_print(36,35,target         ,rgb2col(255,0,0)  ,0,0);

	mh_print(60,76,"TO RUN"   ,rgb2col(0,0,0),0,0);
	mh_print(100,76,"SELECTED FILE"   ,rgb2col(255,0,0),0,0);

	mh_print(60,106,"TO COPY",rgb2col(0,0,0),0,0);
	mh_print(100,106,"SELECTED FILE",rgb2col(255,0,0),0,0);
	mh_print(170,106,"TO",rgb2col(0,0,0),0,0);
	mh_print(185,106,"TARGET FOLDER",rgb2col(0,255,0),0,0);

	mh_print(60,140,"TO REPLACE",rgb2col(0,0,0),0,0);
	mh_print(115,140,"TARGET FILE",rgb2col(148,0,211),0,0);
	mh_print(175,140,"WITH",rgb2col(0,0,0),0,0);
	mh_print(200,140,"SELECTED FILE",rgb2col(255,0,0),0,0);

	mh_print(60,172,"TO CANCEL",rgb2col(0,0,0),0,0);
	GetTfile();
	GetTfolder();
	pgFlipShowFrameV();
	
}


void CopyExecute(void) {
	int fd1,fd2,fd3,len;
	char tpath[256];

	backgConfirm();
	mh_print(180,150,"COPYING...",rgb2col(255,255,255),0,0);
	pgFlipShowFrameV();
	
	fd3 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM.DAT",PSP_O_RDONLY, 0777);
	sceIoRead(fd3, tpath, 255);
	
	fd1 = sceIoOpen(target,PSP_O_RDONLY, 0777);
	fd2 = sceIoOpen(tpath,PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	while(1) {
		len = sceIoRead(fd1, buf, BUFSIZE);
		if (len == 0) break;
		sceIoWrite(fd2,buf,len);
	}
	sceIoClose(fd1);
	sceIoClose(fd2);
	sceIoClose(fd3);
}

void Ftpd(void) {

	backgConfirm();
	mh_print(180,150,"Starting...",rgb2col(255,255,255),0,0);
	sceKernelLoadExec("ms0:/PSP/GAME/PFM/DATA/FTPD.PSP",0);
}	

void UmdRip(void) {

	backgConfirm();
	mh_print(180,150,"Starting...",rgb2col(255,255,255),0,0);
	sceKernelLoadExec("ms0:/PSP/GAME/PFM/DATA/UMD.PSP",0);
	
}

void LoadExec(void) {

	backgConfirm();
	mh_print(180,150,"Lunching...",rgb2col(255,255,255),0,0);
	sceKernelLoadExec(target,0);
	
}

int Confirm_Control(void) {
	unsigned long key;

	while(1) {
		while(1) {
			key = Read_Key();
			if (key != 0) break;
			pgWaitV();
		}

		if (key & CTRL_START) {
		DelExecute();
		backgConfirm();
		pgFlipShowFrameV();
		Get_DirList(now_path);	
		return 0;
		}
		
		if (key & CTRL_SQUARE) {
		CopyToFolder();
		Get_DirList(now_path);	
		return 0;
		}

		if (key & CTRL_CIRCLE) {
			return 1;
		}
		if (key & CTRL_CROSS) {
			return 0;
		}
	}
}

int Confirm_Control_LoadExec(void) {
	unsigned long key;

	while(1) {
		while(1) {
			key = Read_Key();
			if (key != 0) break;
			pgWaitV();
		}

		if (key & CTRL_START) {
		DelExecute();
		backgConfirm();
		pgFlipShowFrameV();
		Get_DirList(now_path);
		return 0;
		}
		
		if (key & CTRL_TRIANGLE) {
		LoadExec();
		Get_DirList(now_path);
		return 0;
		}

		if (key & CTRL_SQUARE) {
		CopyToFolder();
		Get_DirList(now_path);	
		return 0;
		}

		if (key & CTRL_CIRCLE) {
		return 1;
		}

		if (key & CTRL_CROSS) {
		return 0;
		}
	}
}

int Confirm_Control_Modules(void) {
	unsigned long key;

	while(1) {
		while(1) {
			key = Read_Key();
			if (key != 0) break;
			pgWaitV();
		}
	
		if (key & CTRL_SQUARE) {
		UmdRip();
		return 1;
		}
		
		if (key & CTRL_CIRCLE) {
		Ftpd();
		return 1;
		}

		if (key & CTRL_CROSS) {
		return 0;
		}
	}
}


int Control(void) {
	unsigned long key;
	int i;

	// wait key
		key = Read_Key();
		if (key == 0) return 0;

	if (key & CTRL_UP) {
		if (dlist_curpos > 0) {
			dlist_curpos--;
			if (dlist_curpos < dlist_start) { dlist_start = dlist_curpos; }
		}
	}
	if (key & CTRL_DOWN) {
		if (dlist_curpos < (dlist_num-1)) {
			dlist_curpos++;
			if (dlist_curpos >= (dlist_start+PATHLIST_H)) { dlist_start++; }
		}
	}

	if (key & CTRL_LTRIGGER) {
		Draw_Confirm_Modules();
		Confirm_Control_Modules();

	}


	if (key & CTRL_LEFT) {
		dlist_curpos -= PATHLIST_H;
		if (dlist_curpos <  0)          { dlist_curpos = 0;           }
		if (dlist_curpos < dlist_start) { dlist_start = dlist_curpos; }
	}
	if (key & CTRL_RIGHT) {
		dlist_curpos += PATHLIST_H;
		if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
		while (dlist_curpos >= (dlist_start+PATHLIST_H)) { dlist_start++; }
	}

	if (key & CTRL_SELECT) {
		GetTargetPath();
	}

	if (key & CTRL_START) {
	if (dlist[dlist_curpos].type & TYPE_DIR) {
			if (now_depth<MAXDEPTH) {
				// path移動
				mh_strncat(now_path,dlist[dlist_curpos].name,MAXPATH);
				mh_strncat(now_path,"/",MAXPATH);
				cbuf_start[now_depth] = dlist_start;
				cbuf_curpos[now_depth] = dlist_curpos;
				dlist_start  = 0;
				dlist_curpos = 0;
				now_depth++;
				return 1;
			}
		} else {
			//
			mh_strncpy(target,now_path,MAXPATH);
			mh_strncat(target,dlist[dlist_curpos].name,MAXPATH);
			GetTargetFile();
		
		
/*			for(i=0;i<MAXPATH;i++) {
				if (dlist[dlist_curpos].name[i]==0) break;
			}
			if (i>2) {
				if (((dlist[dlist_curpos].name[i-3] & 0xDF) == '.') &&
				    ((dlist[dlist_curpos].name[i-2] & 0xDF) == 'G') &&
				    ((dlist[dlist_curpos].name[i-1] & 0xDF) == 'B')) {
					return 2;
				}
				i--;
			}*/
		}
	}


	if (key & CTRL_CIRCLE) {
		int fd;
		if (dlist[dlist_curpos].type & TYPE_DIR) {
			if (now_depth<MAXDEPTH) {
				// path移動
				mh_strncat(now_path,dlist[dlist_curpos].name,MAXPATH);
				mh_strncat(now_path,"/",MAXPATH);
				cbuf_start[now_depth] = dlist_start;
				cbuf_curpos[now_depth] = dlist_curpos;
				dlist_start  = 0;
				dlist_curpos = 0;
				now_depth++;
				return 1;
			}
		} else {
			//HERE
			fd = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM3.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
			sceIoWrite(fd,dlist[dlist_curpos].name, BUFSIZE);
			sceIoClose(fd);	
			
			for(i=0;i<MAXPATH;i++) {
				if (dlist[dlist_curpos].name[i]==0) break;
			}
			if (i>3) {
				if (((dlist[dlist_curpos].name[i-4]) == '.') &&
				    ((dlist[dlist_curpos].name[i-3]) == 'P') &&
				    ((dlist[dlist_curpos].name[i-2]) == 'S') &&
				    ((dlist[dlist_curpos].name[i-1]) == 'P')) {
					return 3;
				}
				if (((dlist[dlist_curpos].name[i-4]) == '.') &&
				    ((dlist[dlist_curpos].name[i-3]) == 'P') &&
				    ((dlist[dlist_curpos].name[i-2]) == 'B') &&
				    ((dlist[dlist_curpos].name[i-1]) == 'P')) {
					return 3;
				}
	
				if (((dlist[dlist_curpos].name[i-4]) == '.') &&
				    ((dlist[dlist_curpos].name[i-3]) == 'B') &&
				    ((dlist[dlist_curpos].name[i-2]) == 'I') &&
				    ((dlist[dlist_curpos].name[i-1]) == 'N')) {
					return 3;
				}

				i--;
			}
			return 2;
		}
	}
	if (key & CTRL_CROSS) {
		if (now_depth > 0) {
			// path移動
			for(i=0;i<MAXPATH;i++) {
				if (now_path[i]==0) break;
			}
			i--;
			while(i>4) {
				if (now_path[i-1]=='/') {
					now_path[i]=0;
					break;
				}
				i--;
			}
			now_depth--;
			dlist_start  = cbuf_start[now_depth];
			dlist_curpos = cbuf_curpos[now_depth];
			return 1;
		}
	}
	
	return 0;
}


int main(unsigned long args, void *argp) {
	int fd6,fd7,fd8;

	// Initialize
	pgInit();
	pgScreenFrame(1, 1);
	SetupCallbacks();

	ctrl_data_t pad;
 	sceCtrlSetSamplingCycle(0);
 	sceCtrlSetSamplingMode(0);
 	
	

	// default value
	mh_strncpy(now_path,"ms0:/",MAXPATH);
	fd6 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	sceIoWrite(fd6,"NONE, PRESS START TO SET", 255);
	sceIoClose(fd6);
	fd7 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM2.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	sceIoWrite(fd7,"NONE, PRESS SELECT TO SET", 255);
	sceIoClose(fd7);
	fd8 = sceIoOpen("ms0:/PSP/GAME/PFM/DATA/PFM3.DAT",PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	sceIoWrite(fd8,"NONE", 255);
	sceIoClose(fd8);


	//
	Get_DirList(now_path);
	dlist_start  = 0;
	dlist_curpos = 0;
	now_depth    = 0;

	while(1) {
		Draw_All();
		switch(Control()) {
		case 1:
			Get_DirList(now_path);
			break;
		case 2:
			// Copy confirm
			mh_strncpy(target,now_path,MAXPATH);
			mh_strncat(target,dlist[dlist_curpos].name,MAXPATH);
			Draw_Confirm();
			if (Confirm_Control()!=0) {
				CopyExecute();
			}
			break;
		case 3:
			// LoadExec confirm
			mh_strncpy(target,now_path,MAXPATH);
			mh_strncat(target,dlist[dlist_curpos].name,MAXPATH);
			Draw_Confirm_LoadExec();
			if (Confirm_Control_LoadExec()!=0) {
				CopyExecute();
			}
			break;
		}
	}

	return 0;
}
