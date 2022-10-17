// Modified by Cervi
// cardRead and ndsFileFD not static, access from Engine.cpp

#define ARM9
#include <nds/memory.h>
#include <nds/system.h>
#include <nds/card.h>

#include <fat.h>

#include <sys/dir.h>
#include <sys/iosupport.h>
#include <sys/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static DIR_ITER* nitroFSDirOpen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int nitroDirReset(struct _reent *r, DIR_ITER *dirState);
static int nitroFSDirNext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st);
static int nitroFSDirClose(struct _reent *r, DIR_ITER *dirState);
static int nitroFSOpen(struct _reent *r, void *fileStruct, const char *path,int flags,int mode);
static int nitroFSClose(struct _reent *r,void *fd);
static ssize_t nitroFSRead(struct _reent *r,void *fd,char *ptr,size_t len);
static off_t nitroFSSeek(struct _reent *r,void *fd,off_t pos,int dir);
static int nitroFSFstat(struct _reent *r,void *fd,struct stat *st);
static int nitroFSstat(struct _reent *r,const char *file,struct stat *st);
static int nitroFSChdir(struct _reent *r,const char *name);

static tNDSHeader *__gba_cart_header = (tNDSHeader *)0x08000000;

#define NITRONAMELENMAX 0x80	//max file name is 127 +1 for zero byte
#define NITROMAXPATHLEN	0x100

#define NITROROOT 		0xf000	//root entry_file_id
#define NITRODIRMASK	0x0fff	//remove leading 0xf

#define NITROISDIR		0x80	//mask to indicate this name entry is a dir, other 7 bits = name length

//Directory filename subtable entry structure
struct ROM_FNTDir {
	u32 	entry_start;
	u16	entry_file_id;
	u16	parent_id;
};


struct ROM_FAT {
	u32	top;	//start of file in rom image
	u32	bottom;	//end of file in rom image
};

struct nitroFSStruct {
	unsigned int	pos;	//where in the file am I?
	unsigned int 	start;	//where in the rom this file starts
	unsigned int 	end;	//where in the rom this file ends
};

struct nitroDIRStruct {
	unsigned int	pos;		//where in the file am I?
	unsigned int 	namepos;	//ptr to next name to lookup in list
	struct ROM_FAT  romfat;
	u16		entry_id;			//which entry this is (for files only) incremented with each new file in dir?
	u16		dir_id;				//which directory entry this is
	u16		cur_dir_id;			//which directory entry we are using
	u16		parent_id;			//who is the parent of the current directory
	u8		spc;				//system path count .. used by dirnext, when 0=./ 1=../ >=2 actual dirs
};

//Globals!
static u32	fntOffset;	//offset to start of filename table
static u32	fatOffset;	//offset to start of file alloc table
static u16	chdirpathid;	//default dir path id...
static int	ndsFileFD = -1;
static unsigned int ndsFileLastpos;	//Used to determine need to fseek or not
static bool cardRead = false;

devoptab_t nitroFSdevoptab={
	"nitro",
	sizeof(struct nitroFSStruct),	//	int	structSize;
	&nitroFSOpen,		//	int (*open_r)(struct _reent *r, void *fileStruct, const char *path,int flags,int mode);
	&nitroFSClose,		//	int (*close_r)(struct _reent *r,int fd);
	NULL,				//	int (*write_r)(struct _reent *r,int fd,const char *ptr,int len);
	&nitroFSRead,		//	int (*read_r)(struct _reent *r,int fd,char *ptr,int len);
	&nitroFSSeek,		//	int (*seek_r)(struct _reent *r,int fd,int pos,int dir);
	&nitroFSFstat,		//	int (*fstat_r)(struct _reent *r,int fd,struct stat *st);
	&nitroFSstat,		//	int (*stat_r)(struct _reent *r,const char *file,struct stat *st);
	NULL,				//	int (*link_r)(struct _reent *r,const char *existing, const char  *newLink);
	NULL,				//	int (*unlink_r)(struct _reent *r,const char *name);
	&nitroFSChdir,		//	int (*chdir_r)(struct _reent *r,const char *name);
	NULL,				//	int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
	NULL,				//	int (*mkdir_r) (struct _reent *r, const char *path, int mode);
	sizeof(struct nitroDIRStruct),	//	int dirStateSize;
	&nitroFSDirOpen,	//	DIR_ITER* (*diropen_r)(struct _reent *r, DIR_ITER *dirState, const char *path);
	&nitroDirReset,		//	int (*dirreset_r)(struct _reent *r, DIR_ITER *dirState);
	&nitroFSDirNext,	//	int (*dirnext_r)(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
	&nitroFSDirClose,	//	int (*dirclose_r)(struct _reent *r, DIR_ITER *dirState);
	NULL,				//	int (*statvfs_r)(struct _reent *r, const char *path, struct statvfs *buf);
	NULL,				//	int (*ftruncate_r)(struct _reent *r, int fd, off_t len);
	NULL,				//	int (*fsync_r)(struct _reent *r, int fd);
	NULL,				//	void *deviceData;
	NULL,				//	int (*chmod_r)(struct _reent *r, const char *path, mode_t mode);
	NULL				//	int (*fchmod_r)(struct _reent *r, int fd, mode_t mode);
};

//---------------------------------------------------------------------------------
bool nitroFSInit(char **base_path) {
//---------------------------------------------------------------------------------

	bool nitroInit = false;

	if (__NDSHeader->fatSize == 0 ) return false;

	sysSetCartOwner(BUS_OWNER_ARM9);
	sysSetCardOwner(BUS_OWNER_ARM9);

	char *nitropath = NULL;

	// test for argv & open nds file
	if ( __system_argv->argvMagic == ARGV_MAGIC && __system_argv->argc >= 1 ) {
		if ( strncmp(__system_argv->argv[0],"fat",3) == 0 || strncmp(__system_argv->argv[0],"sd",2) == 0) {
			if (fatInitDefault()) {
				ndsFileFD = open(__system_argv->argv[0], O_RDONLY);
				if (ndsFileFD != -1) {
					nitroInit = true;
					nitropath = malloc(PATH_MAX);
					if(nitropath != NULL) {
						nitropath=getcwd(nitropath,PATH_MAX);
					}
				}
			}
		}
	}

	// test for valid nitrofs on gba cart
    // Cervi: disable gba card, use cardRead instead

    // fallback to direct card reads for desmume
	// TODO: validate nitrofs
	if (!nitroInit) {
		cardRead = true; nitroInit = true;
		nitropath = strdup("nitro:/");
	}

    fntOffset = __NDSHeader->filenameOffset;
    fatOffset = __NDSHeader->fatOffset;
    AddDevice(&nitroFSdevoptab);
    chdir(nitropath);

    if (base_path != NULL) {
		*base_path = nitropath;
	} else {
		free(nitropath);
	}
	return nitroInit;
}


// cannot read across block boundaries (multiples of 0x200 bytes)
//---------------------------------------------------------------------------------
static void nitroSubReadBlock(u32 pos, u8 *ptr, u32 len) {
//---------------------------------------------------------------------------------
	if ( (len & 3) == 0 && (((u32)ptr) & 3) == 0 && (pos & 3) == 0) {
		// if everything is word-aligned, read directly
		cardParamCommand(0xB7, pos, CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F) | CARD_CLK_SLOW | CARD_WR | CARD_nRESET | CARD_SEC_CMD | CARD_SEC_DAT | CARD_ACTIVATE | CARD_BLK_SIZE(1), (u32*)ptr, len >> 2);
 	} else {
		// otherwise, use a temporary buffer
		static u32 temp[128];
		cardParamCommand(0xB7, (pos & ~0x1ff), CARD_DELAY1(0x1FFF) | CARD_DELAY2(0x3F) | CARD_CLK_SLOW | CARD_WR | CARD_nRESET | CARD_SEC_CMD | CARD_SEC_DAT | CARD_ACTIVATE | CARD_BLK_SIZE(1), temp, 0x200 >> 2);
		memcpy(ptr, ((u8*)temp) + (pos & 0x1FF), len);
	}
}

//---------------------------------------------------------------------------------
static ssize_t nitroSubReadCard(unsigned int *npos, void *ptr, ssize_t len) {
//---------------------------------------------------------------------------------
	u8 *ptr_u8 = (u8*)ptr;
	ssize_t remaining = len;

	if((*npos) & 0x1FF) {

		u32 amt = 0x200 - (*npos & 0x1FF);

		if(amt > remaining) amt = remaining;

		nitroSubReadBlock(*npos, ptr_u8, amt);
		remaining -= amt;
		ptr_u8 += amt;
		*npos += amt;
	}

	while(remaining >= 0x200)	{
		nitroSubReadBlock(*npos, ptr_u8, 0x200);
		remaining -= 0x200;
		ptr_u8 += 0x200;
		*npos += 0x200;
	}

	if(remaining > 0) {
		nitroSubReadBlock(*npos, ptr_u8, remaining);
		*npos += remaining;
	}
	return len;
}

//---------------------------------------------------------------------------------
static inline ssize_t nitroSubRead(unsigned int *npos, void *ptr, ssize_t len) {
//---------------------------------------------------------------------------------
	if(cardRead) {
		unsigned int tmpPos = *npos;
		len = nitroSubReadCard(&tmpPos, ptr, len);

	} else if(ndsFileFD!=-1) { //read from ndsfile

		if(ndsFileLastpos!=*npos)
			lseek(ndsFileFD, *npos, SEEK_SET);

		len=read(ndsFileFD, ptr, len);

	} else {	//reading from gbarom
		if (len > 0) memcpy(ptr, *npos+(void*)GBAROM,len);
	}

	if(len > 0)
		*npos+=len;

	ndsFileLastpos=*npos;	//save the current file position

	return(len);
}

//---------------------------------------------------------------------------------
static inline void nitroSubSeek(unsigned int *npos, int pos, int dir) {
//---------------------------------------------------------------------------------
	if((dir==SEEK_SET)||(dir==SEEK_END)) 	//otherwise just set the pos :)
		*npos=pos;
	else if(dir==SEEK_CUR)
		*npos+=pos;
}

//---------------------------------------------------------------------------------
// Directory functions

//---------------------------------------------------------------------------------
static DIR_ITER* nitroFSDirOpen(struct _reent *r, DIR_ITER *dirState, const char *path) {
//---------------------------------------------------------------------------------

	struct nitroDIRStruct *dirStruct=(struct nitroDIRStruct*)dirState->dirStruct;
	struct stat st;
	char dirname[NITRONAMELENMAX];
	char *cptr;
	char mydirpath[NITROMAXPATHLEN];	//to hold copy of path string
	char *dirpath=mydirpath;
	bool pathfound;

	if((cptr=strchr(path,':')))
		path=cptr+1;	//move path past any device names

	strncpy(dirpath,path,sizeof(mydirpath)-1);

	dirStruct->pos=0;

	if(*dirpath=='/')				//if first character is '/' use absolute root path
		dirStruct->cur_dir_id=NITROROOT;	//first root dir
	else
		dirStruct->cur_dir_id=chdirpathid;	//else use chdirpath
	nitroDirReset(r,dirState);			//set dir to current path

	do {

		while( dirpath[0] == '/') dirpath++; // move past leading /
		cptr=strchr(dirpath,'/');

		if(cptr)
			*cptr=0;		// erase /


		if(*dirpath==0) {	// are we at the end of the path string?? if so there is nothing to search for we're already here !
			pathfound=true; // mostly this handles searches for root or /  or no path specified cases
			break;
		}
		pathfound=false;

		while(nitroFSDirNext(r,dirState,dirname,&st)==0) {

			if(S_ISDIR(st.st_mode) && !(strcasecmp(dirname,dirpath))) { //if it's a directory and name matches dirpath
				dirStruct->cur_dir_id=dirStruct->dir_id;  //move us to the next dir in tree
				nitroDirReset(r,dirState);		//set dir to current path we just found...
				pathfound=true;
				break;
			}
		}
		if(!pathfound)
			break;
		dirpath=cptr+1;	//move to right after last / we found
	} while(cptr); // go till after the last /

	if(pathfound) {
		return(dirState);
	} else {
		return(NULL);
	}
}


//---------------------------------------------------------------------------------
static int nitroFSDirClose(struct _reent *r, DIR_ITER *dirState) {
//---------------------------------------------------------------------------------
	return(0);
}

/*Consts containing relative system path strings*/
const char *syspaths[2]={
".",
".."
};

//reset dir to start of entry selected by dirStruct->cur_dir_id
//---------------------------------------------------------------------------------
static int nitroDirReset(struct _reent *r, DIR_ITER *dirState) {
//---------------------------------------------------------------------------------
	struct nitroDIRStruct *dirStruct=(struct nitroDIRStruct*)dirState->dirStruct;
	struct ROM_FNTDir dirsubtable;
	unsigned int *pos=&dirStruct->pos;
	nitroSubSeek(pos,fntOffset+((dirStruct->cur_dir_id&NITRODIRMASK)*sizeof(struct ROM_FNTDir)),SEEK_SET);
	nitroSubRead(pos, &dirsubtable, sizeof(dirsubtable));
	dirStruct->namepos=dirsubtable.entry_start;		//set namepos to first entry in this dir's table
	dirStruct->entry_id=dirsubtable.entry_file_id;	//get number of first file ID in this branch
	dirStruct->parent_id=dirsubtable.parent_id;		//save parent ID
	dirStruct->spc=0;								//system path counter, first two dirnext's deliver . and ..
	return(0);
}

//---------------------------------------------------------------------------------
static int nitroFSDirNext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) {
//---------------------------------------------------------------------------------
	unsigned char next;
	struct nitroDIRStruct *dirStruct=(struct nitroDIRStruct*)dirState->dirStruct;
	unsigned int *pos=&dirStruct->pos;
	if(dirStruct->spc<=1) {
		if(st) st->st_mode=S_IFDIR;
		if((dirStruct->spc==0)||(dirStruct->cur_dir_id==NITROROOT)) {	// "." or its already root (no parent)
			dirStruct->dir_id=dirStruct->cur_dir_id;
		} else {			// ".."
			dirStruct->dir_id=dirStruct->parent_id;
		}
		strcpy(filename,syspaths[dirStruct->spc++]);
		return(0);
	}
	nitroSubSeek(pos,fntOffset+dirStruct->namepos,SEEK_SET);
	nitroSubRead(pos, &next , sizeof(next));
	// next: high bit 0x80 = entry isdir .. other 7 bits are size, the 16 bits following name are dir's entryid (starts with f000)
	//  00 = end of table //
	if(next) {
		if(next&NITROISDIR) {
			if(st) st->st_mode=S_IFDIR;
			next&=NITROISDIR^0xff;	//invert bits and mask off 0x80
			nitroSubRead(pos,filename,next);
			nitroSubRead(&dirStruct->pos,&dirStruct->dir_id,sizeof(dirStruct->dir_id)); //read the dir_id
			dirStruct->namepos+=next+sizeof(u16)+1;		// now we point to next one plus dir_id size
		} else {
			if(st) st->st_mode=0;
			nitroSubRead(pos,filename,next);
			dirStruct->namepos+=next+1;		//now we point to next one
			//read file info to get filesize (and for fileopen)
			nitroSubSeek(pos,fatOffset+(dirStruct->entry_id*sizeof(struct ROM_FAT)),SEEK_SET);
			nitroSubRead(pos, &dirStruct->romfat, sizeof(dirStruct->romfat));	//retrieve romfat entry (contains filestart and end positions)
			dirStruct->entry_id++; //advance ROM_FNTStrFile ptr
			if(st) st->st_size=dirStruct->romfat.bottom-dirStruct->romfat.top; //calculate filesize
		}
		filename[(int)next]=0;	//zero last char
		return(0);
	} else {
		return(-1);
	}
}

//---------------------------------------------------------------------------------
static int nitroFSOpen(struct _reent *r, void *fileStruct, const char *path,int flags,int mode) {
//---------------------------------------------------------------------------------
	struct nitroFSStruct *fatStruct=(struct nitroFSStruct *)fileStruct;
	struct nitroDIRStruct dirStruct;
	DIR_ITER dirState;
	dirState.dirStruct=&dirStruct;	//create a temp dirstruct
	struct _reent dre;
	struct stat st;		//all these are just used for reading the dir ~_~
	char dirfilename[NITROMAXPATHLEN]; // to hold a full path (I tried to avoid using so much stack but blah :/)
	char *filename; // to hold filename
	char *cptr;	//used to string searching and manipulation
	cptr=(char*)path+strlen(path);	//find the end...
	filename=NULL;

	do {
		if((*cptr=='/') || (*cptr==':')) { // split at either / or : (whichever comes first form the end!)
			cptr++;
			strncpy(dirfilename,path,cptr-path);	//copy string up till and including/ or : zero rest
			dirfilename[cptr-path]=0;
			filename=cptr;	//filename = now remainder of string
			break;
		}
	} while(cptr--!=path); //search till start

	if(!filename) {
		filename=(char*)path;	//filename = complete path
		dirfilename[0]=0;	//make directory path ""
	}

	if(nitroFSDirOpen(&dre,&dirState,dirfilename)) {

		fatStruct->start=0;

		while(nitroFSDirNext(&dre,&dirState, dirfilename, &st)==0) {

			if(!(st.st_mode & S_IFDIR) && (strcasecmp(dirfilename,filename)==0)) {
				fatStruct->start=dirStruct.romfat.top;
				fatStruct->end=dirStruct.romfat.bottom;
				break;
			}

		}

		nitroFSDirClose(&dre,&dirState);

		if(fatStruct->start) {
			nitroSubSeek(&fatStruct->pos,fatStruct->start,SEEK_SET);	//seek to start of file
			return(0);	//woot!
		}

	}
	return(-1);
}

//---------------------------------------------------------------------------------
static int nitroFSClose(struct _reent *r,void *fd) {
//---------------------------------------------------------------------------------
	return(0);
}

//---------------------------------------------------------------------------------
static ssize_t nitroFSRead(struct _reent *r,void *fd,char *ptr,size_t len) {
//---------------------------------------------------------------------------------
	struct nitroFSStruct *fatStruct=(struct nitroFSStruct *)fd;
	unsigned int *npos=&fatStruct->pos;
	if(*npos+len > fatStruct->end)
		len=fatStruct->end-*npos;	//don't read past the end
	if(*npos > fatStruct->end)
		return(0);	//hit eof
	return(nitroSubRead(npos,ptr,len));
}

//---------------------------------------------------------------------------------
static off_t nitroFSSeek(struct _reent *r,void *fd,off_t pos,int dir) {
//---------------------------------------------------------------------------------
	//need check for eof here...
	struct nitroFSStruct *fatStruct=(struct nitroFSStruct *)fd;
	unsigned int *npos=&fatStruct->pos;
	if(dir==SEEK_SET)
		pos+=fatStruct->start;	// add start from .nds file offset
	else if(dir==SEEK_END)
		pos+=fatStruct->end;	// set start to end of file
	if(pos > fatStruct->end)
		return(-1);				// don't read past the end
	nitroSubSeek(npos,pos,dir);
	return(*npos-fatStruct->start);
}

//---------------------------------------------------------------------------------
static int nitroFSFstat(struct _reent *r,void *fd,struct stat *st) {
//---------------------------------------------------------------------------------
	struct nitroFSStruct *fatStruct=(struct nitroFSStruct *)fd;
	st->st_size=fatStruct->end-fatStruct->start;
	return(0);
}

//---------------------------------------------------------------------------------
static int nitroFSstat(struct _reent *r,const char *file,struct stat *st) {
//---------------------------------------------------------------------------------
	struct nitroFSStruct fatStruct;
	struct nitroDIRStruct dirStruct;
	DIR_ITER dirState;

	if(nitroFSOpen(NULL, &fatStruct, file, 0, 0)>=0) {
		st->st_mode = S_IFREG;
		st->st_size=fatStruct.end-fatStruct.start;
		return(0);
	}

	dirState.dirStruct=&dirStruct;
	if(nitroFSOpen(NULL, &fatStruct, file, 0, 0)>=0) {
		st->st_mode = S_IFREG;
		st->st_size=fatStruct.end-fatStruct.start;
		return(0);
	}
	if((nitroFSDirOpen(r, &dirState, file)!=NULL)) {

		st->st_mode = S_IFDIR;

		nitroFSDirClose(r, &dirState);
		return(0);
	}

	return(-1);
}

//---------------------------------------------------------------------------------
static int nitroFSChdir(struct _reent *r,const char *name) {
//---------------------------------------------------------------------------------
	struct nitroDIRStruct dirStruct;
	DIR_ITER dirState;
	dirState.dirStruct=&dirStruct;
	if((name!=NULL) && (nitroFSDirOpen(r, &dirState, name)!=NULL)) {
		chdirpathid=dirStruct.cur_dir_id;
		nitroFSDirClose(r, &dirState);
		return(0);
	} else {
		return(-1);
	}
}

