/*
 *	minixfsrw.c
 *
 *	向Minix FileSystem v1添加与提取文件.
 *
 *	(C) 2016.3	Mumu3w@outlook.com
 */
 
#include "minixfsrw.h"

 
FILE *minixfs_v1_image_fp;  // 镜像文件指针
char *minixfs_v1_image_buf; // 将整个文件读入内存


int main(int argc, char *argv[])
{
	
}

/*****************************************************************************
 获取分区表
 part_id: 四个主分区之一(0 1 2 3)
 part: 
*****************************************************************************/
int read_part(unsigned int  part_id, struct partition *part)
{
    char *data;
    
    if(!(part_id >= 0 && part_id < 4))
    {
        return -1;
    }
    else
    {
        data = bread(0);
        memcpy((char *)part, 
			&data[0x1BE + part_id * sizeof(struct partition)], 
                                    sizeof(struct partition));
                                            
        return 0;
    }
}

/*****************************************************************************
 初始化
*****************************************************************************/
void constructor(const char *minixfs_v1_image_name)
{
	unsigned int file_size, read_bytes;
	
    if(NULL == (minixfs_v1_image_fp = fopen(minixfs_v1_image_name, "rb+")))
    {
        panic(__FILE__, __LINE__, "constructor(): \"%s\" Can't open\n", 
												minixfs_v1_image_name);
    }
	
	file_size = get_file_size(minixfs_v1_image_fp);
	minixfs_v1_image_buf = (char *)malloc(file_size);
	if(minixfs_v1_image_buf == NULL)
	{
		panic(__FILE__, __LINE__, 
						"constructor(): memory allocation failed\n");
	}
	read_bytes = fread(minixfs_v1_image_buf, 1, file_size, 
											minixfs_v1_image_fp);
	assert(read_bytes != file_size);
}

/*****************************************************************************
 获取文件长度
*****************************************************************************/
static inline unsigned int get_file_size(FILE *fp)
{
    unsigned int file_len, file_pos;
    
    file_pos = ftell(fp); // 保存当前位置
    
    fseek(fp, 0L, SEEK_END);
    file_len = ftell(fp); // 计算文件长度
    
    fseek(fp, file_pos, SEEK_SET);
    
    return file_len;
}

/*****************************************************************************
 无法恢复的错误
*****************************************************************************/
static inline void panic(const char *src_name, int src_line, 
									const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "%s:%d:", src_file, src_line);
    vfprintf(stderr, format, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}
 
 /*****************************************************************************
 读逻辑块
 off_bytes: (512 * n)缓冲区字节偏移;硬盘分区起始扇区存在奇偶问题.
 返回指向缓冲区相对应块的指针.
*****************************************************************************/
static inline char *bread(unsigned int off_bytes)
{
	assert(&minixfs_v1_image_buf[off_bytes] 
			< (minixfs_v1_image_buf + get_file_size(minixfs_v1_image_fp)));
			
    return &minixfs_v1_image_buf[off_bytes];
}

/*****************************************************************************
 写逻辑块
 off_bytes: (512 * n)缓冲区字节偏移;硬盘分区起始扇区存在奇偶问题.
 buf: 缓冲区指针;虽然偏移以512字节为单位,但每次写入1024字节.
*****************************************************************************/
static inline void bwrite(unsigned int off_bytes, const char *buf)
{
    assert(&minixfs_v1_image_buf[off_bytes] 
			< (minixfs_v1_image_buf + get_file_size(minixfs_v1_image_fp)));
			
	memcpy(&minixfs_v1_image_buf[off_bytes], buf, BLOCK_SIZE);
}