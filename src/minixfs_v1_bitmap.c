#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minixfs_v1.h"


/*****************************************************************************
 clear block
*****************************************************************************/
static void bclear(uint super_block_id, uint blkno)
{
    struct buf *bp;
    
    printl(__FILE__, __LINE__, "bclear(): super_block_id-%d blkno-0x%X\n", 
                                                    super_block_id, blkno);
    
    if((bp = bget(super_block_id, blkno)) == NULL)
    {
        printl(__FILE__, __LINE__, "bclear(): invalid struct buf *bp\n");
        return ;
    }
    memset(bp->data, 0, BLOCK_SIZE);
    bp->flag = UPD;
    brelse(bp);
}

/*****************************************************************************
 clear ino
*****************************************************************************/
static void iclear(uint super_block_id, uint ino)
{
    struct m_inode *mip;
    
    printl(__FILE__, __LINE__, "iclear(): super_block_id-%d ino-0x%X\n", 
                                                    super_block_id, ino);
    
    if((mip = iget(super_block_id, ino)) == NULL)
    {
        printl(__FILE__, __LINE__, 
                "iclear(): invalid struct m_inode *mip\n");
    }
    else
    {
        memset(mip, 0, sizeof(struct d_inode));
        mip->flag = UPD;
        irelse(mip);
    }
}

/*****************************************************************************
 文件块到磁盘块的映射
*****************************************************************************/
uint bmap(struct m_inode *mip, uint blkno, uint create)
{
    struct buf *bp;
    uint i;
    
    printl(__FILE__, __LINE__, "bmap(): file blk-0x%X\n", blkno);
    
    if(!(blkno >= 0 && blkno < (7 + 512 + 512 * 512)))
    {
        printl(__FILE__, __LINE__, "bmap(): invalid blkno\n");
        return 0;
    }
    
    // 直接块
    if(blkno < 7)
    {
        if(create && !mip->i_zone[blkno])
        {
            if(mip->i_zone[blkno] = balloc(mip->super_block_id))
            {
                mip->flag = UPD;
            }
            else
            {
                printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
                return 0;
            }
        }
        
        printl(__FILE__, __LINE__, "bmap(): blkno-0x%X\n", 
                                        mip->i_zone[blkno]);
        return mip->i_zone[blkno];
    }
    
    // 一次间接块
    blkno -= 7;
    if(blkno < 512)
    {
        if(create && !mip->i_zone[7])
        {
            if(mip->i_zone[7] = balloc(mip->super_block_id))
            {
                mip->flag = UPD;
            }
            else
            {
                printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
                return 0;
            }
        }
        
        if(!(mip->i_zone[7]))
        {
            return 0;
        }
        
        if(!(bp = bget(mip->super_block_id, mip->i_zone[7])))
        {
            printl(__FILE__, __LINE__, "bmap(): invalid struct buf *bp\n");
            return 0;
        }
        else
        {
            i = ((uint16 *)bp->data)[blkno];
        }
        
        if(create && !i)
        {
            if(i = balloc(mip->super_block_id))
            {
                ((uint16 *)bp->data)[blkno] = i;
                bp->flag = UPD;
                mip->flag = UPD;
            }
            else
            {
                printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
                brelse(bp);
                return 0;
            }
        }
        
        printl(__FILE__, __LINE__, "bmap(): blkno-0x%X\n", i);
        brelse(bp);
        return i;
    }
    
    // 二次间接块
    blkno -= 512;
    if(create && !mip->i_zone[8])
    {
        if(mip->i_zone[8] = balloc(mip->super_block_id))
        {
            mip->flag = UPD;
        }
        else
        {
            printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
            return 0;
        }
    }
    
    if(!(mip->i_zone[8]))
    {
        return 0;
    }
    
    if(!(bp = bget(mip->super_block_id, mip->i_zone[8])))
    {
        printl(__FILE__, __LINE__, "bmap(): invalid struct buf *bp\n");
        return 0;
    }
    else
    {
        i = ((uint16 *)bp->data)[blkno >> 9];
    }
    
    if(create && !i)
    {
        if(i = balloc(mip->super_block_id))
        {
            ((uint16 *)bp->data)[blkno >> 9] = i;
            bp->flag = UPD;
            mip->flag = UPD;
        }
        else
        {
            printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
            brelse(bp);
            return 0;
        }
    }
    brelse(bp);
    
    if(!i)
    {
        return 0;
    }
    
    if(!(bp = bget(mip->super_block_id, i)))
    {
        printl(__FILE__, __LINE__, "bmap(): invalid struct buf *bp\n");
        return 0;
    }
    else
    {
        i = ((uint16 *)bp->data)[blkno & 511];
    }
    
    if(create && !i)
    {
        if(i = balloc(mip->super_block_id))
        {
            ((uint16 *)bp->data)[blkno & 511] = i;
            bp->flag = UPD;
            mip->flag = UPD;
        }
        else
        {
            printl(__FILE__, __LINE__, "bmap(): block alloc fail!!!\n");
            brelse(bp);
            return 0;
        }
    }
    printl(__FILE__, __LINE__, "bmap(): blkno-0x%X\n", i);
    brelse(bp);
    return i;
}

/*****************************************************************************
 块的分配
*****************************************************************************/
uint balloc(uint super_block_id)
{
    struct buf *bp;
    struct m_super_block msb;
    uint bi, b, blkno;
    uint8 m;
    
    printl(__FILE__, __LINE__, "balloc(): super_block_id-%d\n", 
                                                super_block_id);
                                                
    if(read_super_block(super_block_id, &msb))
    {
        printl(__FILE__, __LINE__, "balloc(): invalid super_block_id\n");
        return 0; // 正常返回的块号一定是
                  // (blkno >= msb.s_firstdatazone || blkno < msb.s_nzones)
    }
    // minix fs v1支持的最大分区是64M,也就是需要8块逻辑快
    for(b = 0; b < msb.s_nzones - msb.s_firstdatazone + 1; b += 8192)
    {
        if((bp = bget(super_block_id, msb.firstzmapzone + b/8192)) == NULL)
        {
            printl(__FILE__, __LINE__, "balloc(): invalid struct buf *bp\n");
            return 0;
        }
        for(bi = 0; (bi<8192)&&(b+bi<msb.s_nzones - msb.s_firstdatazone + 1); 
                                                                        bi++)
        {
            m = 0x01 << (bi % 8);
            if((bp->data[bi/8] & m) == 0)
            {
                // block = s_firstdatazone - 1
                blkno = msb.s_firstdatazone + b + bi - 1;
                printl(__FILE__, __LINE__, 
                        "balloc(): alloc blk-0x%X zmap-0x%X bit-0x%X\n", 
                        blkno, msb.firstzmapzone + b/8192, b + bi);
                bp->data[bi/8] |= m;
                bp->flag = UPD;
                brelse(bp);
                bclear(super_block_id, blkno);
                return blkno;
            }
        }
        brelse(bp);
    }
    return 0;
}

/*****************************************************************************
 块的回收
*****************************************************************************/
void bfree(uint super_block_id, uint blkno)
{
    struct buf *bp;
    struct m_super_block msb;
    uint bi;
    uint8 m;
    
    printl(__FILE__, __LINE__, "bfree(): super_block_id-%d blkno-0x%X\n", 
                                                    super_block_id, blkno);
    
    if(read_super_block(super_block_id, &msb))
    {
        printl(__FILE__, __LINE__, "bfree(): invalid super_block_id\n");
        return ;
    }
    
    if(blkno < msb.s_firstdatazone || blkno >= msb.s_nzones)
    {
        printl(__FILE__, __LINE__, "bfree(): invalid blkno\n");
        return ;
    }
    
    blkno -= (msb.s_firstdatazone - 1); // (blkno/8192)计算位图所在的块
    if((bp = bget(super_block_id, msb.firstzmapzone + blkno/8192)) == NULL)
    {
        printl(__FILE__, __LINE__, "bfree(): invalid struct buf *bp\n");
        return ;
    }
    bi = blkno % 8192;    // 76543210  FEDCBA98
    m = 0x01 << (bi % 8); // 01010101B 10101010B
                          // 位图的位顺序如上
    if(!(bp->data[bi/8] & m)) // 释放的块位图已经被清位这显然哪里出错了
    {
        printl(__FILE__, __LINE__, "bfree(): bit already cleared?\n");
        brelse(bp);
        return ;
    }
    bp->data[bi/8] &= ~m;
    bp->flag = UPD;
    brelse(bp);
}

/*****************************************************************************
 i节点分配
*****************************************************************************/
uint ialloc(uint super_block_id)
{
    struct buf *bp;
    struct m_super_block msb;
    uint bi, b, ino;
    uint8 m;
    
    printl(__FILE__, __LINE__, "ialloc(): super_block_id-%d\n", 
                                                super_block_id);
                                                
    if(read_super_block(super_block_id, &msb))
    {
        printl(__FILE__, __LINE__, "ialloc(): invalid super_block_id\n");
        return 0;
    }
    // i节点从1开始,根目录的i节点是1
    for(b = 0; b < msb.s_ninodes; b += 8192)
    {
        if((bp = bget(super_block_id, msb.firstimapzone + b/8192)) == NULL)
        {
            printl(__FILE__, __LINE__, "ialloc(): invalid struct buf *bp\n");
            return 0;
        }
        for(bi = 0; bi < 8192 && bi < msb.s_ninodes; bi++)
        {
            m = 0x01 << (bi % 8);
            if((bp->data[bi/8] & m) == 0)
            {
                ino = b + bi;
                printl(__FILE__, __LINE__, 
                        "ialloc(): alloc ino-0x%X imap-0x%X bit-0x%X\n", 
                        ino, msb.firstimapzone + b/8192, b + bi);
                bp->data[bi/8] |= m; // 置位
                bp->flag = UPD;
                brelse(bp);
                iclear(super_block_id, ino);
                
                return ino;
            }
        }
    }
    
    return 0;
}

/*****************************************************************************
 i节点回收
*****************************************************************************/
void ifree(uint super_block_id, uint ino)
{
    struct buf *bp;
    struct m_super_block msb;
    uint bi;
    uint8 m;
    
    printl(__FILE__, __LINE__, "ifree(): super_block_id-%d ino-0x%X\n", 
                                                    super_block_id, ino);
    
    if(read_super_block(super_block_id, &msb))
    {
        printl(__FILE__, __LINE__, "ifree(): invalid super_block_id\n");
        return ;
    }
    if((bp = bget(super_block_id, msb.firstimapzone + ino/8192)) == NULL)
    {
        printl(__FILE__, __LINE__, "ifree(): invalid struct buf *bp\n");
        return ;
    }
    
    bi = ino % 8192;      // 76543210  FEDCBA98
    m = 0x01 << (bi % 8); // 01010101B 10101010B
                          // 位图的位顺序如上
    if(!(bp->data[bi/8] & m)) // 释放的块位图已经被清位这显然哪里出错了
    {
        printl(__FILE__, __LINE__, "ifree(): bit already cleared?\n");
        brelse(bp);
        return ;
    }
    bp->data[bi/8] &= ~m;
    bp->flag = UPD;
    brelse(bp);
}