/*
 *  ZIPfile.cpp
 *  ZIP
 *
 *  Created by andrew on 5/03/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "ZIPfile.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _MACINTOSH_
# include <unistd.h>
# include <utime.h>
# include <sys/stat.h>
#else
# include <direct.h>
# include <io.h>
#endif

#include "unzip.h"
#include "zip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
//#define MAXFILENAME (256)
#ifdef _WINDOWS_
#define USEWIN32IOAPI
#include "iowin32.h"
#endif


/* change_file_date : change the date/time of a file
 filename : the filename of the file where date/time must be modified
 dosdate : the new date at the MSDos format (4 bytes)
 tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char* filename, uLong dosdate, tm_unz tmu_date){
#ifdef _WINDOWS_
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;
	
	hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
					   0,NULL,OPEN_EXISTING,0,NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
	LocalFileTimeToFileTime(&ftLocal,&ftm);
	SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
	CloseHandle(hFile);
#else
#ifdef _MACINTOSH_
	struct utimbuf ut;
	struct tm newdate;
	newdate.tm_sec = tmu_date.tm_sec;
	newdate.tm_min=tmu_date.tm_min;
	newdate.tm_hour=tmu_date.tm_hour;
	newdate.tm_mday=tmu_date.tm_mday;
	newdate.tm_mon=tmu_date.tm_mon;
	if (tmu_date.tm_year > 1900)
		newdate.tm_year=tmu_date.tm_year - 1900;
	else
		newdate.tm_year=tmu_date.tm_year ;
	newdate.tm_isdst=-1;
	
	ut.actime=ut.modtime=mktime(&newdate);
	utime(filename,&ut);
#endif
#endif
}


/* mymkdir and change_file_date are not 100 % portable
 As I don't know well Unix, I wait feedback for the unix portion */

int mymkdir(const char* dirname){
    int ret=0;
#ifdef _WINDOWS_
    ret = mkdir(dirname);
#else
#ifdef _MACINTOSH_
    ret = mkdir (dirname, 0775);
#endif
#endif
    return ret;
}

int makedir (const char* newdir){
	char *buffer ;
	char *p;
	int  len = (int)strlen(newdir);
	
	if (len <= 0)
		return 0;
	
	buffer = (char*)malloc(len+1);
	strcpy(buffer,newdir);
	
	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (mymkdir(buffer) == 0)
    {
		free(buffer);
		return 1;
    }
	
	p = buffer+1;
	while (1)
    {
		char hold;
		
		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
			printf("couldn't create directory %s\n",buffer);
			free(buffer);
			return 0;
        }
		if (hold == 0)
			break;
		*p++ = hold;
    }
	free(buffer);
	return 1;
}

#ifdef _WINDOWS_
uLong filetime(const char* f, tm_zip* tmzip, uLong* dt){
               /* name of file to get info on */
            /* return value: access, modific. and creation times */
          /* dostime */

	int ret = 0;
	{
		FILETIME ftLocal;
		HANDLE hFind;
		WIN32_FIND_DATA  ff32;
		
		hFind = FindFirstFile(f,&ff32);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
			FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
			FindClose(hFind);
			ret = 1;
		}
	}
	return ret;
}
#else
#ifdef _MACINTOSH_
uLong filetime(const char *f, tm_zip* tmzip, uLong* dt){
	/* name of file to get info on */
	/* return value: access, modific. and creation times */
	/* dostime */
	
	int ret=0;
	struct stat s;        /* results of stat() */
	struct tm* filedate;
	time_t tm_t=0;
	
	if (strcmp(f,"-")!=0)
	{
		char name[MAX_PATH_LEN+1];
		int len = strlen(f);
		if (len > MAX_PATH_LEN)
			len = MAX_PATH_LEN;
		
		strncpy(name, f,MAX_PATH_LEN-1);
		/* strncpy doesnt append the trailing NULL, of the string is too long. */
		name[ MAX_PATH_LEN ] = '\0';
		
		if (name[len - 1] == '/')
			name[len - 1] = '\0';
		/* not all systems allow stat'ing a file with / appended */
		if (stat(name,&s)==0)
		{
			tm_t = s.st_mtime;
			ret = 1;
		}
	}
	filedate = localtime(&tm_t);
	
	tmzip->tm_sec  = filedate->tm_sec;
	tmzip->tm_min  = filedate->tm_min;
	tmzip->tm_hour = filedate->tm_hour;
	tmzip->tm_mday = filedate->tm_mday;
	tmzip->tm_mon  = filedate->tm_mon ;
	tmzip->tm_year = filedate->tm_year;
	
	return ret;
}
#else
uLong filetime(f, tmzip, dt)
char *f;                /* name of file to get info on */
tm_zip *tmzip;             /* return value: access, modific. and creation times */
uLong *dt;             /* dostime */
{
    return 0;
}
#endif
#endif

/* calculate the CRC32 of a file,
 because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const char* filenameinzip,void*buf,unsigned long size_buf,unsigned long* result_crc)
{
	unsigned long calculate_crc=0;
	int err=ZIP_OK;
	FILE * fin = fopen(filenameinzip,"rb");
	unsigned long size_read = 0;
	unsigned long total_read = 0;
	if (fin==NULL)
	{
		err = ZIP_ERRNO;
	}
	
    if (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf,1,size_buf,fin);
            if (size_read < size_buf)
                if (feof(fin)==0)
				{
					printf("error in reading %s\n",filenameinzip);
					err = ZIP_ERRNO;
				}
			
            if (size_read>0)
                calculate_crc = crc32(calculate_crc, (const Bytef*)buf, size_read);
            total_read += size_read;
			
        } while ((err == ZIP_OK) && (size_read>0));
	
    if (fin)
        fclose(fin);
	
    *result_crc=calculate_crc;
    printf("file %s crc %x\n",filenameinzip,calculate_crc);
    return err;
}

int check_exist_file(const char* filename){
    FILE* ftestexist;
    int ret = 1;
    ftestexist = fopen(filename,"rb");
    if (ftestexist==NULL)
        ret = 0;
    else
        fclose(ftestexist);
    return ret;
}



int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, const int* popt_overwrite, const char* password){
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;
	
    unz_file_info file_info;
    uLong ratio=0;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	
    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }
	
    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }
	
    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }
	
    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;
		
        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;
		
        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }
		
        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
				skip = 1;
				
			} 
			
        }
		
        if ((skip==0) && (err==UNZ_OK))
        {
            fout=fopen(write_filename,"wb");
			
            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
				(filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }
			
            if (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }
		
        if (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);
			
            do{
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0){
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,err,1,fout)!=1){
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
				fclose(fout);
			
            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }
		
        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }
	
    free(buf);
    return err;
}

int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password, Handle fileNames){
    uLong i;
    unz_global_info gi;
    int err;
    FILE* fout=NULL;
	const char* sep = ";";
	
    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
	
    for (i=0;i<gi.number_entry;i++){
		char filename_inzip[256];
        unz_file_info file_info;

		if (do_extract_currentfile(uf,&opt_extract_without_path,
								   &opt_overwrite,
								   password) != UNZ_OK)
            break;

		err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		
		if(err = PtrAndHand((Ptr)&filename_inzip, fileNames, sizeof(char)*strlen(filename_inzip)))
			return err;
		if(err = PtrAndHand((Ptr)sep, fileNames, sizeof(char)))
			return err;
						
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }
	
    return 0;
}

int
ExecuteZIPfile(ZIPfileRuntimeParamsPtr p)
{
	int err = 0, err2 = 0;
	char dirname[MAX_PATH_LEN+1]; // Output full path 
	char temppath[MAX_PATH_LEN+1];
	char zipfilename[MAX_PATH_LEN+1];
    const char *password=NULL;
    char filename_try[MAX_PATH_LEN+16] = "";
    int opt_do_extract=1;
    int opt_do_extract_withoutpath=0;
    int opt_overwrite=0;
    int opt_extractdir=1;
	Handle fileNames = NULL;
    unzFile uf=NULL;
	
	memset(dirname, 0, MAX_PATH_LEN+1);
	memset(zipfilename, 0, MAX_PATH_LEN+1);
	fileNames = NewHandle(0);
	
	//if you want to overwrite existing files
	if (p->OFlagEncountered) {
		opt_overwrite = 1;
	}
	
	// Main parameters.
	
	if (p->pathEncountered) {
		if(!p->path){
			err = NULL_STRING_HANDLE;
			goto done;
		}
		if(err = GetCStringFromHandle(p->path, temppath, MAX_PATH_LEN))
			goto done;
		if(err = GetNativePath(temppath,dirname))
			goto done;
#ifdef _MACINTOSH_
		HFSToPosixPath(dirname, dirname, 1);
#endif
	}
	
	if (p->fileEncountered) {
		// Parameter: p->file (test for NULL handle before using)
		if(!p->file){
			err = NULL_STRING_HANDLE;
			goto done;
		}
		if(err = GetCStringFromHandle(p->file, temppath, MAX_PATH_LEN))
			goto done;
		if(err = GetNativePath(temppath,zipfilename))
			goto done;
#ifdef _MACINTOSH_
		HFSToPosixPath(zipfilename, zipfilename, 0);
#endif
	}
	
	//will extract without paths, everything will be flattened
	opt_do_extract = opt_do_extract_withoutpath = 1;
	
    if (zipfilename!=NULL)
    {
		
#        ifdef USEWIN32IOAPI
        zlib_filefunc_def ffunc;
#        endif
		
        strncpy(filename_try, zipfilename,MAX_PATH_LEN-1);
        /* strncpy doesnt append the trailing NULL, of the string is too long. */
        filename_try[ MAX_PATH_LEN ] = '\0';
		
#        ifdef USEWIN32IOAPI
        fill_win32_filefunc(&ffunc);
        uf = unzOpen2(zipfilename,&ffunc);
#        else
        uf = unzOpen(zipfilename);
#        endif
        if (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2(filename_try,&ffunc);
#            else
            uf = unzOpen(filename_try);
#            endif
        }
    }
	
    if (uf==NULL){
		err2 = CANNOT_OPEN_ZIPFILE;
		goto done;
    }
	
	if(opt_do_extract==1){
        if (opt_extractdir && chdir(dirname)) {
			err2 = CANNOT_CHANGE_DIR;
			goto done;
        }
        if(err2 = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password, fileNames)){
			err2 = PROBLEM_EXTRACTING_ZIP;
			goto done;
		}
	}
	
done:
if(uf)
	unzCloseCurrentFile(uf);
if(err2)
	SetOperationNumVar("V_flag", 1);
else
	SetOperationNumVar("V_flag", 0);
if(!err2){
	char* fnames = NULL;
	fnames = (char*)malloc(sizeof(char)*GetHandleSize(fileNames)+1);
	if(fnames==NULL)
		err = NOMEM;
	err = GetCStringFromHandle(fileNames, fnames, GetHandleSize(fileNames));
	if(!err)
		err = SetOperationStrVar("S_unzippedfiles", fnames);
	if(fnames)
		free(fnames);
}
if(fileNames)
	DisposeHandle(fileNames);
	
	return err;
	
}

int
ExecuteZIPzipfiles(ZIPzipfilesRuntimeParamsPtr p)
{
	int err = 0, err2 = 0;
	char zipfile[MAX_PATH_LEN+1];
	char temppath[MAX_PATH_LEN+1];
	char zipFileIn[MAX_PATH_LEN+1];
	int numfiles = 0;
	
	int i, len;
	int dot_found=0;
	
    int opt_overwrite=0;
    int opt_compress_level=Z_DEFAULT_COMPRESSION;
    char filename_try[MAX_PATH_LEN+16];
    int zipok;
	int size_buf=0;
    void* buf=NULL;
    const char* password=NULL;
	zipFile zf = NULL;
	int errclose;
	FILE * fin = NULL;
	
	
	memset(zipfile, 0, MAX_PATH_LEN);
	memset(zipFileIn, 0, MAX_PATH_LEN);	
	
	// Main parameters.
	if (p->OFlagEncountered) {
		opt_overwrite = 1;
	}
	
	if (p->AFlagEncountered) {
		opt_overwrite = 2;
	}
	
	// Main parameters.
	
	if (p->zipfileEncountered) {
		// Parameter: p->zipfile (test for NULL handle before using)
		if(!p->zipfile){
			err = NULL_STRING_HANDLE;
			goto done;
		}
		if(err = GetCStringFromHandle(p->zipfile, temppath, MAX_PATH_LEN))
			goto done;
		if(err = GetNativePath(temppath,zipfile))
			goto done;
		#ifdef _MACINTOSH_
		HFSToPosixPath(zipfile, zipfile, 1);
		#endif
	}
	
	if (p->filesEncountered) {
		int* paramsSet = &p->filesParamsSet[0];
		Handle strH;
		int ii;
		
		for(ii=0; ii<100; ii++) {
			if (paramsSet[ii] == 0)
				break;		// No more parameters.
			strH = p->files[ii];
			if (strH == NULL) {
				err = USING_NULL_STRVAR;
				goto done;
			}
			numfiles += 1;
		}
	}
	
	size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL){
		err = NOMEM;
		goto done;
	}
    
	zipok = 1 ;
	strncpy(filename_try, zipfile, MAX_PATH_LEN-1);
	/* strncpy doesnt append the trailing NULL, of the string is too long. */
	filename_try[ MAX_PATH_LEN ] = '\0';
	
	len=(int)strlen(filename_try);
	for (i=0;i<len;i++)
		if (filename_try[i]=='.')
			dot_found=1;
	
	if (dot_found==0)
		strcat(filename_try,".zip");
	
	if (opt_overwrite==2){
		/* if the file don't exist, we not append file */
		if (check_exist_file(filename_try)==0)
			opt_overwrite=1;
	} else {
        if (opt_overwrite==0)
            if (check_exist_file(filename_try)!=0)
				goto done;
	}


#        ifdef USEWIN32IOAPI
	zlib_filefunc_def ffunc;
	fill_win32_filefunc(&ffunc);
	zf = zipOpen2(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        else
	zf = zipOpen(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif
	
	if (zf == NULL){
		err2= PROBLEM_ZIPPING;
		goto done;
	} else
		
        for (i=0 ; (i<numfiles) && (err2==ZIP_OK);i++){

 
                int size_read;
				if(err = GetCStringFromHandle(p->files[i], temppath, MAX_PATH_LEN))
					goto done;
				if(err = GetNativePath(temppath,zipFileIn))
					goto done;
				#ifdef _MACINTOSH_
				HFSToPosixPath(zipFileIn, zipFileIn, 1);
				#endif
		
                const char* filenameinzip = zipFileIn;
                zip_fileinfo zi;
                unsigned long crcFile=0;
				
                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.internal_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip, &zi.tmz_date,&zi.dosDate);
				
				/*
				 err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
				 NULL,0,NULL,0,NULL / * comment * /,
				 (opt_compress_level != 0) ? Z_DEFLATED : 0,
				 opt_compress_level);
				 */
                if ((password != NULL) && (err2==ZIP_OK))
                    err2 = getFileCrc(filenameinzip,buf,size_buf,&crcFile);
					
					err2 = zipOpenNewFileInZip3(zf,filenameinzip,&zi,
											   NULL,0,NULL,0,NULL /* comment*/,
											   (opt_compress_level != 0) ? Z_DEFLATED : 0,
											   opt_compress_level,0,
											   /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
											   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
											   password,crcFile);
					
					if (err2 != ZIP_OK){
					} else {
						fin = fopen(filenameinzip,"rb");
						if (fin==NULL)
							err2=PROBLEM_ZIPPING;
					}
				
                if (err2 == ZIP_OK)
                    do {
                        err2 = ZIP_OK;
                        size_read = (int)fread(buf,1,size_buf,fin);
                        if (size_read < size_buf)
                            if (feof(fin)==0)
								err2 = PROBLEM_ZIPPING;
						
                        if (size_read>0)
							err2 = zipWriteInFileInZip (zf,buf,size_read);
                    } while ((err2 == ZIP_OK) && (size_read>0));
				
                if (fin){
                    fclose(fin);
					fin = NULL;
				}
				
				if (err2<0)
					err2=PROBLEM_ZIPPING;
				else
					err2 = zipCloseFileInZip(zf);
            
        }


done:
if(buf)
	free(buf);
if(fin)
	fclose(fin);
errclose = zipClose(zf,NULL);
if (errclose != ZIP_OK)
	err2 = PROBLEM_ZIPPING;

if(err2)
	SetOperationNumVar("V_flag", 1);
else
	SetOperationNumVar("V_flag", 0);
	
return err;

}
