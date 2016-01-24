#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "minixfs_v1.h"


/*****************************************************************************
 获取1970-1-1到现在的秒数
*****************************************************************************/
static uint get_seconds(void)
{
    time_t seconds;
    time(&seconds);
    return seconds;
}

/*****************************************************************************
 读取i节点结构到内存
*****************************************************************************/
struct m_inode *iget(uint super_block_id, uint ino)
{
    struct m_super_block msb;
    uint blkno, blkoff;
    struct buf *bp;
    struct m_inode *mip;
    
    printl(__FILE__, __LINE__, "iget(): super_block_id-%d ino-0x%X\n", 
                                                super_block_id, ino);
    
    if(read_super_block(super_block_id, &msb))
    {
        printl(__FILE__, __LINE__, "iget(): invalid super_block_id\n");
        return NULL;
    }
    
    if(ino == 0 || ino >= msb.s_ninodes)
    {
        printl(__FILE__, __LINE__, "iget(): invalid ino\n");
        return NULL;
    }
    
    blkno = (ino - 1) / IPB;
    blkoff = ((ino - 1) % IPB) * sizeof(struct d_inode);
    
    if((bp = bget(super_block_id, msb.firstinodezone + blkno)) == NULL)
    {
        printl(__FILE__, __LINE__, "iget(): invalid struct buf *bp\n");
        return NULL;
    }
    
    if((mip = (struct m_inode *)calloc(1, sizeof(struct m_inode))) == NULL)
    {
        printl(__FILE__, __LINE__, "iget(): invalid struct m_inode *mip\n");
        brelse(bp);
        return NULL;
    }
    
    mip->super_block_id = super_block_id;
    mip->ino = ino;
    memcpy((char *)mip, bp->data + blkoff, sizeof(struct d_inode));
    brelse(bp);
    
    return mip;
}

/*****************************************************************************
 释放i节点内容
*****************************************************************************/
void irelse(struct m_inode *mip)
{
    struct m_super_block msb;
    uint blkno, blkoff;
    struct buf *bp;
    
    if(!(mip && mip->ino))
    {
        printl(__FILE__, __LINE__, "irelse(): invalid inode\n");
        return ;
    }
    
    printl(__FILE__, __LINE__, "irelse(): ino-0x%X\n", mip->ino);
    
    if(mip->flag == UPD)
    {
        if(read_super_block(mip->super_block_id, &msb))
        {
            printl(__FILE__, __LINE__, "irelse(): invalid super_block_id\n");
            return ;
        }
        
        blkno = (mip->ino - 1) / IPB;
        blkoff = ((mip->ino - 1) % IPB) * sizeof(struct d_inode);
        if((bp = bget(mip->super_block_id, msb.firstinodezone + blkno)) == 
                                                                        NULL)
        {
            printl(__FILE__, __LINE__, "irelse(): invalid struct buf *bp\n");
            return ;
        }
        mip->i_mtime = get_seconds(); // 更新时间
        memcpy(bp->data + blkoff, mip, sizeof(struct d_inode));
        bp->flag = UPD;
        brelse(bp);
    }
    
    free(mip);
}