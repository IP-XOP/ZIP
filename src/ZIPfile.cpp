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

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
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

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,err,1,fout)!=1)
                    {
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

int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password){
    uLong i;
    unz_global_info gi;
    int err;
    FILE* fout=NULL;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) != UNZ_OK)
            break;

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
	int err = 0;
	char dirname[MAX_PATH_LEN+1]; // Output full path 
	char temppath[MAX_PATH_LEN+1];
	char zipfilename[MAX_PATH_LEN+1];
    const char *password=NULL;
    char filename_try[MAX_PATH_LEN+16] = "";
    int opt_do_extract=1;
    int opt_do_extract_withoutpath=0;
    int opt_overwrite=0;
    int opt_extractdir=1;
    unzFile uf=NULL;

	memset(dirname, 0, MAX_PATH_LEN+1);
	memset(zipfilename, 0, MAX_PATH_LEN+1);
	
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

		HFSToPosixPath(temppath, dirname, 1);
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
		err = CANNOT_OPEN_ZIPFILE;
		goto done;
    }

	if(opt_do_extract==1){
        if (opt_extractdir && chdir(dirname)) {
			err = CANNOT_CHANGE_DIR;
			goto done;
        }
        if(err = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password)){
			err = PROBLEM_EXTRACTING_ZIP;
			goto done;
			}
       }
	
	if(uf)
		unzCloseCurrentFile(uf);

done:
	return err;

}