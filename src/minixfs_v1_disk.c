#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minixfs_v1.h"


/*****************************************************************************
 获取超级块
 super_block_id: 四个主分区之一(0 1 2 3)
 msb: 内存超级块结构
*****************************************************************************/
int read_super_block(uint super_block_id, struct m_super_block *msb)
{
    char data[BLOCK_SIZE];
    struct partition part;
    struct super_block *sb;
    
    printl(__FILE__, __LINE__, "read_super_block(): super_block_id-%d\n", 
                                                        super_block_id);
    
    if(!(super_block_id >= 0 && super_block_id < 4))
    {
        return -1;
    }
    else
    {
        read_part(super_block_id, &part);
        bread(part.start_sect + 2, data); // 分区第二逻辑块才是超级块
        memcpy((char *)msb, data, sizeof(struct super_block));
        msb->firstimapzone = 2;
        msb->firstzmapzone = msb->firstimapzone + msb->s_imap_block;
        msb->firstinodezone = msb->firstzmapzone + msb->s_zmap_block;
                                            
        return 0;
    }
}

/*****************************************************************************
 获取分区表
 part_id: 四个主分区之一(0 1 2 3)
 part: 
*****************************************************************************/
int read_part(uint part_id, struct partition *part)
{
    char data[BLOCK_SIZE];
    
    printl(__FILE__, __LINE__, "read_part(): part_id-%d\n", part_id);
    
    if(!(part_id >= 0 && part_id < 4))
    {
        return -1;
    }
    else
    {
        bread(0, data);
        memcpy((char *)part, &data[0x1BE + part_id * sizeof(struct partition)], 
                                            sizeof(struct partition));
                                            
        return 0;
    }
}

/*****************************************************************************
 从磁盘读取1024字节到缓冲区中
 super_block_id: 分区编号
 blkno: 编号从0 -- super_block.s_nzones - 1
*****************************************************************************/
struct buf *bget(uint super_block_id, uint blkno)
{
    struct buf *bp;
    struct partition part;
    
    printl(__FILE__, __LINE__, "bget(): super_block_id-%d blk-0x%X\n", 
                                                super_block_id, blkno);
    
    bp = (struct buf*)calloc(1, sizeof(struct buf));
    if(bp == NULL)
    {
        return NULL;
    }
    
    bp->data = (char *)malloc(BLOCK_SIZE);
    if(bp->data == NULL)
    {
        free(bp);
        return NULL;
    }
    
    read_part(super_block_id, &part);
    bp->blkno = blkno;
    bp->sector_offset = part.start_sect + blkno * 2;
    bread(bp->sector_offset, bp->data);
    
    return bp;
}

/*****************************************************************************
 释放读取到磁盘的数据
 bp: 缓冲结构指针
*****************************************************************************/
void brelse(struct buf *bp)
{
    // 测试非空指针,如果bp为空那么就不能释放bp->data
    if(bp == NULL)
    {
        return ;
    }
    else if(bp->data == NULL)
    {
        free(bp);
        return ;
    }
    
    printl(__FILE__, __LINE__, 
        "brelse(): bp->blkno-0x%X bp->flag-%d\n", 
                            bp->blkno, bp->flag);
    
    // 缓冲区如果更新了则需要写回到磁盘
    if(bp->flag == UPD)
    {
        bwrite(bp->sector_offset, bp->data);
    }
    free(bp->data); // 要先释放bp->data
    free(bp);
}

/*****************************************************************************
 读逻辑块
 sector_offset: 以512字节为单位;硬盘分区起始扇区存在奇偶问题.
 ptr: 缓冲区指针;虽然偏移以512字节为单位,但每次读取1024字节.
*****************************************************************************/
void bread(uint sector_offset, char *ptr)
{
    fseek(minixfs_v1_image_fp, sector_offset * SECTOR_SIZE, SEEK_SET);
    
    printl(__FILE__, __LINE__, "bread(): sector_offset-0x%X\n", 
                                                sector_offset);
    
    if(fread(ptr, 1, BLOCK_SIZE, minixfs_v1_image_fp) != BLOCK_SIZE)
    {
        panic(__FILE__, __LINE__, "bread(): bytes != BLOCK_SIZE\n");
    }
}

/*****************************************************************************
 写逻辑块
 sector_offset: 以512字节为单位;硬盘分区起始扇区存在奇偶问题.
 ptr: 缓冲区指针;虽然偏移以512字节为单位,但每次写入1024字节.
*****************************************************************************/
void bwrite(uint sector_offset, const char *ptr)
{
    fseek(minixfs_v1_image_fp, sector_offset * SECTOR_SIZE, SEEK_SET);
    
    printl(__FILE__, __LINE__, "bwrite(): sector_offset-0x%X\n", 
                                                sector_offset);
    
    if(fwrite(ptr, 1, BLOCK_SIZE, minixfs_v1_image_fp) != BLOCK_SIZE)
    {
        panic(__FILE__, __LINE__, "bwrite(): bytes != BLOCK_SIZE\n");
    }
}