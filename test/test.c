#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


int main(int argc, char *argv[])
{
	struct m_inode *mip;
	FILE *fp;
	
	if(argc != 5)
	{
		panic(__FILE__, __LINE__, 
				"Usage: %s Image super_block_id pathname filename\n", 
				argv[0]);
	}
	
	fp = fopen(argv[4], "rb");
	if(!fp)
	{
		panic(__FILE__, __LINE__, "Can't open %s\n", argv[4]);
	}
	
	constructor(argv[1]);
	
	add_file(fp, atoi(argv[2]), argv[3]);
	
	destructor();
	return 0;
}