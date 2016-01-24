#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


/*****************************************************************************
 建立一个空普通文件
*****************************************************************************/
struct m_inode *touch(uint super_block_id, char *pathname, uint mode)
{
    struct buf *bp;
    struct dir_entry *de;
    struct m_inode *mip, *dir;
    char *basename;
    int namelen;
    uint ino;
    
    mip = namei(super_block_id, pathname);
    if(mip)
    {
        if(S_ISREG(mip->i_mode))
        {
            itrunc(mip);
            return mip;
        }
        else
        {
            printl(__FILE__, __LINE__, "touch(): not ordinary file\n");
            return NULL;
        }
    }
    else
    {
        dir = dir_namei(super_block_id, pathname, &namelen, &basename);
        if(!dir)
        {
            printl(__FILE__, __LINE__, "touch(): invalid path\n");
            return NULL;
        }
        
        if(!namelen)
        {
            irelse(dir);
            printl(__FILE__, __LINE__, "touch(): invalid path\n");
            return NULL;
        }
        
        bp = add_entry(dir, basename, namelen, &de);
        if(!bp)
        {
            irelse(dir);
            printl(__FILE__, __LINE__, 
                        "touch(): failed add a directory entry\n");
            return NULL;
        }
        ino = ialloc(super_block_id);
        if(!ino)
        {
            irelse(dir);
            printl(__FILE__, __LINE__, 
                        "touch(): failed add a inode\n");
            return NULL;
        }
        mip = iget(super_block_id, ino);
        if(!mip)
        {
            irelse(dir);
            ifree(super_block_id, ino);
            printl(__FILE__, __LINE__, 
                        "touch(): failed get inode\n");
            return NULL;
        }
        de->ino = ino;
        bp->flag = UPD;
        brelse(bp);
        mip->i_mode = (010 << 12) | mode;
        mip->i_nlinks = 1;
        mip->flag = UPD;
        dir->i_nlinks++;
        dir->flag = UPD;
        irelse(dir);
        return mip;
    }
}