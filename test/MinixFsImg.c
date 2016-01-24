#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


void PrintUsage(const char *str1);


int main(int argc, char *argv[])
{
	if(argc != 6 || ((argc == 2) && (!stricmp(argv[1], "-h"))))
	{
		PrintUsage(argv[0]);
		return 0;
	}
	
	if(argc == 6 && (!stricmp(argv[1], "-a")))
    {
		FILE *fp;
		if(NULL == (fp = fopen(argv[5], "rb")))
		{
			fprintf(stderr, "Can't open %s\n", argv[5]);
			exit(EXIT_FAILURE);
		}
		constructor(argv[2]);
    
		if(add_file(fp, atoi(argv[3]), argv[4]))
		{
			fprintf(stderr, "error: Add file to image\n");
		}
		else
		{
			fprintf(stderr, "ok: Add file to image\n");
		}
		
		fclose(fp);
		destructor();
    }
	
    if(argc == 6 && (!stricmp(argv[1], "-c")))
    {
		FILE *fp;
		if(NULL == (fp = fopen(argv[5], "wb")))
		{
			fprintf(stderr, "Can't open %s\n", argv[5]);
			exit(EXIT_FAILURE);
		}
		constructor(argv[2]);
	
		struct m_inode *mip;
		mip = namei(atoi(argv[3]), argv[4]);
		if(!mip)
		{
			fprintf(stderr, "error: Copy file from image\n");
		}
		else
		{
			extract_file(fp, mip);
			fprintf(stderr, "ok: Copy file from image\n");
		}
		
		fclose(fp);
		destructor();
    }

	return 0;
}

void PrintUsage(const char *str1)
{
	printf("(C) Mumu3w@outlook.com  %s  (MinixFsImage 0.02)\n\n", __DATE__);
	printf("Usage: %s <-a | -c | -h> Image ID Pathname FileName\n", str1);
	printf("  -a Add file to image\n");
	printf("  -c Copy file from image\n");
	printf("  -h Show this usage\n");
}