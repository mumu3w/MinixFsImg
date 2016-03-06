#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


/*****************************************************************************
 获取文件长度
*****************************************************************************/
static uint get_file_size(FILE *fp)
{
    uint file_len, file_pos;
    
    file_pos = ftell(fp); // 保存当前位置
    
    fseek(fp, 0L, SEEK_END);
    file_len = ftell(fp); // 计算文件长度
    
    fseek(fp, file_pos, SEEK_SET);
    
    return file_len;
}

/*****************************************************************************
 提取文件
*****************************************************************************/
void extract_file(FILE *fp, struct m_inode *mip)
{
    uint blkno, blkoff;
    struct buf *bp;
	int i;
    
    if(((mip->i_mode >> 12) & 017) != 010)
    {
        printl(__FILE__, __LINE__, "extract_file(): not ordinary file\n");
        return;
    }
    
    blkoff = mip->i_size % BLOCK_SIZE;
    blkno = mip->i_size / BLOCK_SIZE + (blkoff > 0 ? 1 : 0);
    
    for(i = 0; i < blkno-1; i++)
    {
        bp = bget(mip->super_block_id, bmap(mip, i, 0));
        fwrite(bp->data, sizeof(char), BLOCK_SIZE, fp);
        brelse(bp);
    }
    // 最后一块做特殊处理
    bp = bget(mip->super_block_id, bmap(mip, blkno-1, 0));
    if(blkoff > 0)
    {
        fwrite(bp->data, sizeof(char), blkoff, fp);
    }
    else
    {
        fwrite(bp->data, sizeof(char), BLOCK_SIZE, fp);
    }
    brelse(bp);
}

/*****************************************************************************
 添加文件
*****************************************************************************/
int add_file(FILE *fp, uint super_block_id, char *pathname)
{
    uint blkno, blkoff;
    struct buf *bp;
	struct m_inode *mip;
	struct m_super_block msb;
	int i;
	
	read_super_block(super_block_id, &msb);
	if(msb.s_magic != MINIXFS_V1)
	{
		panic(__FILE__, __LINE__, "add_file(): msb.s_magic != MINIXFS_V1\n");
		return -1;
	}
    
	mip = touch(super_block_id, pathname, 0755);
    if(!mip)
    {
		printl(__FILE__, __LINE__, "add_file(): failed add a file\n");
        return -1;
    }
    
    blkoff = get_file_size(fp) % BLOCK_SIZE;
    blkno = get_file_size(fp) / BLOCK_SIZE + (blkoff > 0 ? 1 : 0);
    
	for(i = 0; i < blkno-1; i++)
	{
		bp = bget(mip->super_block_id, bmap(mip, i, 1));
		fread(bp->data, sizeof(char), BLOCK_SIZE, fp);
		bp->flag = UPD;
		brelse(bp);
	}
    // 最后一块做特殊处理
	bp = bget(mip->super_block_id, bmap(mip, blkno-1, 1));
	if(blkoff > 0)
	{
		fread(bp->data, sizeof(char), blkoff, fp);
	}
	else
	{
		fread(bp->data, sizeof(char), BLOCK_SIZE, fp);
	}
	bp->flag = UPD;
    brelse(bp);
	
	mip->i_size = get_file_size(fp);
	mip->flag = UPD;
	irelse(mip);
	
	return 0;
}