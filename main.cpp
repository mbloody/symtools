/**
 * main.cpp
 * This file is part of the Symtools Project http://code.google.com/p/symtools/
 *
 * Symbian Tools
 * Copyright (C) 2008-2009 Evteev Maxim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <time.h>
#include <stdint.h>
#include <utime.h>

#include <sys/stat.h>
#include <sys/types.h>
	      

#ifdef _MSC_VER
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#	define DELIM '\\'
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#	define DELIM '/'
#else
#	error you must byte-align these structures with the appropriate compiler directives
#endif


#ifndef HIBYTE
#define HIBYTE(x)       ((((u_int)(x)) & 0xff00)>>8)
#endif
      
#ifndef LOBYTE
#define LOBYTE(x)       (((u_int)(x)) & 0x00ff)
#endif

int usc2utf8(unsigned char* pUCS,int lenUCS,unsigned char* pUTF8)
{
    int UCSlen = 0, UTF8len = 0;
    unsigned char* pTempUTF8 = NULL;
    UCSlen = lenUCS*2;
   
    if(pUCS == NULL || pUTF8 == NULL)
	return -1;
       
    pTempUTF8 = pUTF8;
    for(int i = 0; i < UCSlen; i+=2)
    {
	unsigned int tt = pUCS[i] | (pUCS[i+1] << 8);
        if(tt   <=   0x007F)//1   byte   0xxxxxxx   
        {   
    	    *(pTempUTF8++)   =   LOBYTE(tt);   
    	    UTF8len++;   
        }   
        else   if(tt   <=0x07FF)//2   bytes   110xxxxx   10xxxxxx   
        {   
    	    *(pTempUTF8++)   =   HIBYTE(tt   <<   2)   &   0x3F   |   0xC0;   
    	    *(pTempUTF8++)   =   LOBYTE(tt   &   0x3f)   |   0x80;   
    	    UTF8len   +=   2;   
        }   
        else//3   bytes   1110xxxx   10xxxxxx   10xxxxxx   
        {   
    	    *(pTempUTF8++)   =   HIBYTE(pUCS[i]   >>   4)   |   0xe0;   
    	    *(pTempUTF8++)   =   HIBYTE(pUCS[i]   <<   2)   &   0x3F   |   0x80;   
    	    *(pTempUTF8++)   =   LOBYTE(pUCS[i])   &   0x3F   |   0x80;   
    	    UTF8len   +=   3;   
        }   
    }
    *pTempUTF8 ='\0';
    return UTF8len;
}



static char USAGE[]={                                                                                                            
   "\tunback [options]\n"                                            
   "\n\n"                                                                       
   "options:\n"                                                                 
   "\t--help                    -  To display usage information\n"              
   "\t-f <file name>            -  File to use\n"                           
   "\t-t                        -  Trace. Use multiple times to increase \n"    
};

struct rt
{
    char type; //??
    unsigned int uid; //?????
    unsigned int offset;
    unsigned int size;
    unsigned int unkn[4]; //????
} PACK_STRUCT;



/*
For win32. not tested

void r_mkdir(const char *dir) 
{
    char tmp[256];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp),"data\\%s",dir);
    printf("++%s\n",tmp);
    
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
            tmp[len - 1] = '\0';
    for(p = tmp + 1; *p; p++)
        if(*p == '\\') 
	{
	    *p = '\0';
	    printf("++%s\n",tmp);
	    if(_access(tmp, 0))
		_mkdir(tmp);
	    *p = '\\';
	}
//    if(_access(tmp, 0))
//    	_mkdir(tmp);
}


*/



void r_mkdir(const char *dir) 
{
    char tmp[256];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp),"data\\%s",dir);
    printf("++%s\n",tmp);
    
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
            tmp[len - 1] = '\0';
    for(p = tmp + 1; *p; p++)
        if(*p == '\\') 
	{
	    *p = '\0';
	    printf("++%s\n",tmp);
	    if(access(tmp, F_OK))
		mkdir(tmp, S_IRWXU);
	    *p = '/';
	}
//    if(access(tmp, F_OK))
//    	mkdir(tmp, S_IRWXU);
}


char * getfname(char * name)
{
    int len = strlen(name);
//    printf("---%d\n",len);
    for(int i=len-2;i>0;i--)
    {
//	printf("---%c---%d\n",*(name+i),i);
	if(*(name+i)=='\\') return (name+i+1);
    }
    return name;
}


void unfile(char * fname, FILE * f,int offset, int size, int rsize)
{
	fseek(f,offset,SEEK_SET);
	char * tmp = new char[size];
	fread(tmp,size,1,f);
    	char * und = new char[rsize];                                               
	uLongf s = rsize; 
	int ret = uncompress((Bytef*)und,&s,(Bytef*)tmp,size);
	printf("[Unfile] Decompress - %d\n",ret);
	if(ret==Z_OK) 
	{
	    char fn [255];
	    sprintf(fn,"data%c%s", DELIM, getfname(fname));
	    printf("[Unfile] File name for save - %s\n",fn);
	    FILE * fo = fopen(fn,"wb");
	    if(fo!=NULL)
	    {
		fwrite(und,s,1,fo);
		fclose(fo);
	    }
	}
	delete [] tmp;
	delete [] und;                                                                                   

}

void parse5(char * b, int size)
{
    int cur = 0;
    printf("[Parse5] %d\n",size);
    while(cur<size)
    {
	int fnsize;                                                                  
	memcpy(&fnsize,b + cur,4);
	cur += 4;
	printf("[Parse5] File name size - %d\n",fnsize);
	if(fnsize>size)
	{
	    printf("[Parse5] Error name size - %d\n",fnsize);
	    return;
	}
	
	int fsize;
	memcpy(&fsize,b + cur,4);
	cur += 4;
	printf("[Parse5] File size - %d\n",fsize);
	if(fsize>size)
	{
	    printf("[Parse5] Error size - %d\n",fsize);
	    return;
	}
	
	int unkn1;
	memcpy(&unkn1,b + cur,4);
	cur += 4;
	printf("[Parse5] Unknown 1 - %x\n",unkn1);
	
	int unkn2;
	memcpy(&unkn2,b + cur,4);
	cur += 4;
	printf("[Parse5] Unknown 2 - %x\n",unkn2);
	
	uint64_t time = 0;
	
	memcpy(&time,b + cur,8);
	cur += 8; //time
	printf("[Parse5] File time - %llx\n",time);
	
//      uint64_t timestamp = uint64_t(time(NULL))*1000000 + 0xDCDDB3E5D20000LL;
//      timeLo = (timestamp >> 0) & 0xffffffff;
//      timeHi = (timestamp >> 32) & 0xffffffff;

	time_t sec = (time - 0xDCDDB3E5D20000LL)/1000000; //convert Symbian time (0000-01-01 00:00:00) to UNIX time (1970-01-01 00:00:00)
	struct tm ttt;
	char buf[64];

	struct tm* brokenTime = gmtime_r(&sec,&ttt);
	strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",brokenTime);
	printf("[Parse5] Time - %s\n",buf);
	
	
	char * fname = new char[fnsize*2];
	memcpy(fname,b + cur,fnsize*2);
	cur += fnsize*2;
        char * tmp = new char[fnsize*2];
        int j = 0;
	
	usc2utf8((unsigned char*)fname,fnsize,(unsigned char*)tmp);
	printf("%s\n",tmp);

//	r_mkdir(tmp);

        printf("[Parse5] File name  - %s\n",tmp);
	char fn [255];
	sprintf(fn,"data%c%s", DELIM, getfname(tmp));
	printf("[Parse5] File name for save - %s\n",fn);

	FILE * fo = fopen(fn,"wb");
	if(fo!=NULL)
	{
	    fwrite(b+cur,fsize,1,fo);
	    fclose(fo);
	}
	else
	    printf("[Parse5] Error create file %s\n",tmp);
	cur+=fsize;
	
//set file time
	struct utimbuf ftm;
	ftm.actime = ftm.modtime = sec;
	utime(fn,&ftm);

        delete [] fname;
        delete [] tmp;

    }

}

void unfile5(FILE * f,int offset, int size, int rsize)
{
	fseek(f,offset,SEEK_SET);
	char * tmp = new char[size];
	fread(tmp,size,1,f);
    	char * und = new char[rsize];
	uLongf s = rsize;
	int ret = uncompress((Bytef*)und,&s,(Bytef*)tmp,size);
	printf("[Unfile5] Decompress - %d\n",ret);
	if(ret==Z_OK) 
	    parse5(und,s);
	delete [] tmp;
	delete [] und;                                                                                   

}


void parsef5(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    fseek(f,8,SEEK_CUR);
    int s,rs;
    fread(&s,4,1,f);
    fread(&rs,4,1,f);
    fread(&rs,4,1,f);
//    printf("Count - %d\n",count);
//    printf("%x\n",ftell(f));
    unfile5(f,offset+20,s,rs);
}

void parsef(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    fseek(f,8,SEEK_CUR);
    int s,rs;
    fread(&s,4,1,f);
    fread(&rs,4,1,f);
    fread(&rs,4,1,f);
//    printf("Count - %d\n",count);
    printf("%x\n",ftell(f));
    char fname[20];
    sprintf(fname,"chk_%d",offset);
    unfile(fname, f,offset+20,s,rs);
}




void treeparse0(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int count;
    fread(&count,4,1,f);
    printf("Count - %d\n",count);
}

void treeparse1(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int uid;
    fread(&uid,4,1,f);                                                                  
    if(uid != 1) 
    {
        printf("Error[P1]: Error uid - [%d]\n",uid);
        return;
    }
}



void treeparse2(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int count;
    fread(&count,4,1,f);
    printf("Count - %d\n",count);
    unsigned char * data = new unsigned char[size];
    fread(data,size-4,1,f);
    int cur = 0;
    for(int i = 0; i<count ;i++)
    {
	int uid;                                                                  
	memcpy(&uid,data + cur,4);                                        
	if(uid != 1) 
	{
	    printf("[P2] Error Entry [%d] uid - [%d]\n",i,uid);
	    break;
	}
	cur += 16; // uids???                                             
	int nsize = *(data + cur);
	cur++;
//        printf("%d\n",nsize);           
	char * fname = new char[nsize/4+1];
	memcpy(fname,data + cur,nsize/4);
	fname[nsize/4]='\0';
//	printf("%s\n",fname);
	cur += nsize/4;
	cur += 12;
	int of,s,rs;
	memcpy(&rs,data + cur,4);
//	printf("%x\n",rs);
	cur += 12;
	memcpy(&of,data + cur,4);
//	printf("%x\n",of);
	cur += 4;
	memcpy(&s,data + cur,4);
//	printf("%x\n",s);
	unfile(fname, f,of,s,rs);
	delete [] fname;               
	cur += 12;
    }
    delete [] data;
}

void treeparse3(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int count;
    fread(&count,4,1,f);
    printf("Count - %d\n",count);
}

void treeparse4(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int uid;
    fread(&uid,4,1,f);                                                                  
    if(uid != 1) 
    {
        printf("Error[P4]: Error uid - [%d]\n",uid);
        return;
    }
    fseek(f,12,SEEK_CUR);
    int count;
    fread(&count,4,1,f);
    printf("Count - %d\n",count);
    fseek(f,40,SEEK_CUR);
    unsigned char * data = new unsigned char[size];
    fread(data,size-60,1,f);
    int cur = 0;
    for(int i = 0; i<count ;i++)
    {   
	cur++;                                                              
	memcpy(&uid,data + cur,4);                                        
	if(uid != 1) 
	{
	    printf("Error[P4]: Error Entry [%d] uid - [%d]\n",i,uid);
	    break;
	}
	cur += 4;
	int of,s;
	memcpy(&of,data + cur,4);
	printf("==%x\n",of);
	cur += 4;
	memcpy(&s,data + cur,4);
	printf("%x\n",s);
	cur += 4;
	parsef(of,s,f);
	cur += 48;
    }
    delete [] data;
    
}

void treeparse5(int offset, int size,FILE* f)
{
    fseek(f,offset,SEEK_SET);
    int uid;
    fread(&uid,4,1,f);                                                                  
    if(uid != 1) 
    {
        printf("[P5] Error uid - [%d]\n",uid);
        return;
    }
    fseek(f,12,SEEK_CUR);
    int count;
    fread(&count,4,1,f);
    printf("[P5] Count - %d\n",count);
    fseek(f,40,SEEK_CUR);
    unsigned char * data = new unsigned char[size];
    fread(data,size-60,1,f);
    int cur = 0;
    for(int i = 0; i<count ;i++)
    {   
	cur++;                                                              
	memcpy(&uid,data + cur,4);                                        
	if(uid != 1) 
	{
	    printf("[P5] Error Entry [%d] uid - [%d]\n",i,uid);
	    break;
	}
	cur += 4;
	int of,s;
	memcpy(&of,data + cur,4);
//	printf("==%x\n",of);
	cur += 4;
	memcpy(&s,data + cur,4);
//	printf("%x\n",s);
	cur += 4;
	parsef5(of,s,f);
	cur += 48;
    }
    delete [] data;
    
}

void treeparse(rt e,FILE * f)
{
//    printf("+ %d++ %x ++ 0x%x ++ %x \n",e.type,e.uid,e.offset,e.size);
    switch(e.type)
    {
//	case 0:treeparse0(e.offset,e.size,f);break;
//	case 1:treeparse1(e.offset,e.size,f);break;
	case 2:treeparse2(e.offset,e.size,f);break;
//	case 3:treeparse3(e.offset,e.size,f);break;
//	case 4:treeparse4(e.offset,e.size,f);break;
	case 5:treeparse5(e.offset,e.size,f);break;
	default: printf("Error[Root]: Unknown - [%d]\n",e.type);
    }
}

int main(int argc, char** argv) 
{
    char infile[100];
    int dbglvl = 0;
    strcpy(infile,"Backup.arc");

    /* parse args */                                                             
    if(argc > 10 )                                                  
    {                                                                            
	printf("USAGE:\n%s", USAGE);                                              
	return -1;                                                                
    }                                                                            
    for (int x = 1; x < argc; x++) 
    {                                                 
	if(!strcmp(argv[x], "--help")) 
	{                                          
	    printf("USAGE:\n%s", USAGE);                                           
	    return 0;                                                              
	}                                                                         
	else if (!strcmp(argv[x], "-f")) 
	{                                  
	    x++;                                                                   
	    strncpy(infile, argv[x], sizeof(infile)-1);                 
	}                                                                         
	else if(!strcmp(argv[x], "-t"))
	{                                          
	    dbglvl++;                                                         
	}           
/*	
	else if (!strcmp(argv[x],"-d")) 
	{                                 
	    x++;                                                                   
	    dbglvl = atoi (argv[x]);                                            
	}                                    
*/
    }
    printf("Input file '%s'\n",infile);
    printf("Debug level '%d'\n",dbglvl);
    
    FILE * fp=fopen(infile,"rb");
    
    if(!fp)
    {
	printf("Where is Backup.arc???\n");
	return 1;
    }
    
    fseek(fp,0x3c,SEEK_SET);
    char fwsl;
    fread(&fwsl,1,1,fp);    
    char * fws = new char[fwsl+1];
    fread(fws,fwsl,1,fp);
    fws[fwsl]='\0';
    printf("Firmvare version - %s\n",fws);
    delete [] fws;
    
    fseek(fp,-0xe1,SEEK_END);
//    printf("%x\n",ftell(fp));
    int rootcount;
    fread(&rootcount,4,1,fp);
    printf("%d\n",rootcount);
    
    rt * troot = new rt[rootcount];
	printf("%d\n",sizeof(troot[1]));
    fread(troot,sizeof(rt),rootcount,fp);
    
    for(int i=0;i<rootcount;i++) treeparse(troot[i],fp);    
    
    delete [] troot;	
    fclose(fp);
    return 0;
}


