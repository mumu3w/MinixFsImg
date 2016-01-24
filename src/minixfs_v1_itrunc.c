#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


/*****************************************************************************
 �ͷ�һ�μ�ӿ�
*****************************************************************************/
static void free_ind(uint super_block_id, uint block)
{
    struct buf *bp;
    uint16 *p;
    
    if(!block)
    {
        return ;
    }
    
    if(bp = bget(super_block_id, block))
    {
        p = (uint16 *)bp->data;
        for(int i = 0; i < 512; i++, p++)
        {
            if(*p)
            {
                bfree(super_block_id, *p);
            }
        }
        
        brelse(bp);
    }
    
    bfree(super_block_id, block);
}

/*****************************************************************************
 �ͷŶ��μ�ӿ�
*****************************************************************************/
static void free_dind(uint super_block_id, uint block)
{
    struct buf *bp;
    uint16 *p;
    
    if(!block)
    {
        return ;
    }
    
    if(bp = bget(super_block_id, block))
    {
        p = (uint16 *)bp->data;
        for(int i = 0; i < 512; i++, p++)
        {
            if(*p)
            {
                free_ind(super_block_id, *p);
            }
        }
        
        brelse(bp);
    }
    
    bfree(super_block_id, block);
}

/*****************************************************************************
 �ļ��ض�
*****************************************************************************/
void itrunc(struct m_inode *mip)
{
    // �����з�ʱ��ͨ�ļ���Ŀ¼
    if(!(S_ISREG(mip->i_mode) || S_ISDIR(mip->i_mode)))
    {
        return ;
    }
    
    for(int i = 0; i < 7; i++)
    {
        if(mip->i_zone[i])
        {
            bfree(mip->super_block_id, mip->i_zone[i]);
            mip->i_zone[i] = 0;
        }
    }
    
    free_ind(mip->super_block_id, mip->i_zone[7]);
    mip->i_zone[7] = 0;
    free_dind(mip->super_block_id, mip->i_zone[8]);
    mip->i_zone[8] = 0;
    
    mip->i_size = 0;
    mip->flag = UPD;
}