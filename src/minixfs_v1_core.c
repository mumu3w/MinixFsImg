#include <stdio.h>
#include <stdlib.h>
#include "minixfs_v1.h"


FILE *minixfs_v1_image_fp;
FILE *minixfs_v1_log_fp;
#define LOG_NAME    "minixfs_v1.log"


/*****************************************************************************
    初始化
*****************************************************************************/
void constructor(char *minixfs_v1_image_name)
{
    if(NULL == (minixfs_v1_image_fp = fopen(minixfs_v1_image_name, "rb+")))
    {
        panic(__FILE__, __LINE__, "constructor(): \"%s\" Can't open\n", 
                                            minixfs_v1_image_name);
    }
    
    if(NULL == (minixfs_v1_log_fp = fopen(LOG_NAME, "w")))
    {
        panic(__FILE__, __LINE__, "constructor(): \"%s\" Can't open\n", 
                                            LOG_NAME);
    }
}

/*****************************************************************************
    释放资源
*****************************************************************************/
void destructor(void)
{
    if(minixfs_v1_image_fp != NULL)
    {
        fclose(minixfs_v1_image_fp);
    }
    
    if(minixfs_v1_log_fp != NULL)
    {
        fclose(minixfs_v1_image_fp);
    }
}