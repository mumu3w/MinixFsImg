#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


/*****************************************************************************
 文件名比较
*****************************************************************************/
static int match(int namelen, const char *name, struct dir_entry *de)
{
    if(!de || !de->ino || namelen > NAME_LEN)
    {
        printl(__FILE__, __LINE__, 
                    "match(): (!de || !de->ino || namelen > NAME_LEN)\n");
		printl(__FILE__, __LINE__, 
                    "match(): %s\n", name);
        return 0;
    }
    
    if((strlen(de->name) == namelen) && (!strncmp(de->name, name, namelen)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
 从指定目录搜索条目
*****************************************************************************/
struct buf *find_entry(struct m_inode **dir, char *name, int namelen, 
                                            struct dir_entry **res_dir)
{
    uint entries, blkno, i;
    struct buf *bp;
    struct dir_entry *de;
    
    if(namelen > NAME_LEN || !namelen)
    {
        printl(__FILE__, __LINE__, 
                    "find_entry(): (namelen > NAME_LEN || !namelen)\n");
        return NULL;
    }
    
    // 假如在某目录下创建了5个文件,加上两个特殊条目,总数是7;
    // 现在删除5个文件其中的任何一个文件,某目录的大小还是7;
    // 如果现在重新添加一个文件,那么条目的位置将是原来被删
    // 除文件的位置,这时某目录大小不变;如果现在再添加一个文件
    // 某目录大小将增加
    entries = (*dir)->i_size / sizeof(struct dir_entry);
    *res_dir = NULL;
    // 目录最小为2 * sizeof(struct dir_entry),目录文件第一块为空
    // 这显然不正常
    if(!(blkno = (*dir)->i_zone[0]))
    {
        printl(__FILE__, __LINE__, 
                    "find_entry(): (!(blkno) = (*dir)->i_zone[0])\n");
        return NULL;
    }
    
    if(!(bp = bget((*dir)->super_block_id, blkno)))
    {
        printl(__FILE__, __LINE__, "find_entry(): invalid struct buf *bp\n");
        return NULL;
    }
    i = 0;
    de = (struct dir_entry *)bp->data;
    while(i < entries)
    {
        // 一个逻辑块搜索完则加载下一块
        if((char *)de >= bp->data + BLOCK_SIZE)
        {
            brelse(bp);
            bp = NULL;
            if(!(blkno = bmap(*dir, i/(BLOCK_SIZE/sizeof(struct dir_entry)), 
                        0)) || !(bp = bget((*dir)->super_block_id, blkno)))
            {
                i += (BLOCK_SIZE/sizeof(struct dir_entry));
                continue;
            }
            de = (struct dir_entry *)bp->data;
        }
        
        if(match(namelen, name, de))
        {
            *res_dir = de;
            return bp;
        }
        
        de++;
        i++;
    }
    // 找不到目录项
    brelse(bp);
    return NULL;
}

/*****************************************************************************
 获取路径i节点
 pathname = "/root"  返回根目录
 pathname = "/root/"  返回root目录
 pathname = "/root/test"  返回root目录
 pathname = "/root/test/"  返回test目录
 也就是说返回最后一个'/'之前的目录节点
*****************************************************************************/
struct m_inode *get_dir(uint super_block_id, char *pathname)
{
    char *thisname;
    struct m_inode *mip;
    struct buf *bp;
    struct dir_entry *de;
    int namelen, inr;
    char c;
        
    if((c = *pathname) == '/')
    {
        mip = iget(super_block_id, ROOT_INO);
        if(!mip)
        {
            printl(__FILE__, __LINE__, 
                        "get_dir(): invalid struct m_inode *mip\n");
            return NULL;
        }
        pathname++;
    }
    else
    {
        printl(__FILE__, __LINE__, "get_dir(): invalid pathname\n");
        return NULL;
    }
    
    while(1)
    {
        thisname = pathname;
        if(!S_ISDIR(mip->i_mode))
        {
            irelse(mip);
            printl(__FILE__, __LINE__, "get_dir(): not a directory\n");
            return NULL;
        }
        
        for(namelen = 0; (c = *(pathname++))&&(c != '/'); namelen++)
        {
            // nothing
        }
        if(!c)
        {
            return mip;
        }
        if(!(bp = find_entry(&mip, thisname, namelen, &de)))
        {
            irelse(mip);
            printl(__FILE__, __LINE__, "get_dir(): invalid struct buf *bp\n");
            return NULL;
        }
        inr = de->ino;
        brelse(bp);
        irelse(mip);
        if(!(mip = iget(super_block_id, inr)))
        {
            printl(__FILE__, __LINE__, 
                        "get_dir(): invalid struct m_inode *mip\n");
            return NULL;
        }
    }
}

/*****************************************************************************
 获取路径i节点,以及最顶层目录名
 pathname = "/root/test/"  namelen = 0  name = NULL
 pathname = "/root/test"  namelen = 4  name = "test"
*****************************************************************************/
struct m_inode *dir_namei(uint super_block_id, char *pathname, int *namelen, 
                                                                char **name)
{
    struct m_inode *dir;
    char *basename;
    char c;
    
    if(!(dir = get_dir(super_block_id, pathname)))
    {
        printl(__FILE__, __LINE__, 
                        "dir_namei(): invalid struct m_inode *dir\n");
        return NULL;
    }
    basename = pathname;
    while(c = *pathname++)
    {
        if(c == '/')
        {
            basename = pathname;
        }
    }
    *namelen = pathname - basename - 1;
    *name = basename;
    return dir;
}

/*****************************************************************************
 路径名转i节点
*****************************************************************************/
struct m_inode *namei(uint super_block_id, char *pathname)
{
    char *basename;
    struct buf *bp;
    struct m_inode *dir;
    struct dir_entry *de;
    int inr, namelen;
    
    if(!(dir = dir_namei(super_block_id, pathname, &namelen, &basename)))
    {
        printl(__FILE__, __LINE__, "namei(): invalid path\n");
        return NULL;
    }
    // pathname = "/root/test/"  namelen = 0,直接返回,因为已经是最顶层目录节点
    if(!namelen)
    {
        return dir;
    }
    // pathname = "/root/test"  namelen = 4  name = "test" 继续搜索最顶层节点
    if(!(bp = find_entry(&dir, basename, namelen, &de)))
    {
        printl(__FILE__, __LINE__, "namei(): file not found\n");
        irelse(dir);
        return NULL;
    }
    inr = de->ino;
    brelse(bp);
    irelse(dir);
    
    if(!(dir = iget(super_block_id, inr)))
    {
        printl(__FILE__, __LINE__, "namei(): get inode failure\n");
        return NULL;
    }
    return dir;
}

/*****************************************************************************
 添加目录项
*****************************************************************************/
struct buf *add_entry(struct m_inode *dir, char *name, int namelen, 
                                        struct dir_entry **res_dir)
{
    struct buf *bp;
    struct dir_entry *de;
    int blkno, i, j;
    
    *res_dir = NULL;
    if(namelen > NAME_LEN - 1 || !namelen)
    {
        printl(__FILE__, __LINE__, 
                    "add_entry(): (namelen > NAME_LEN - 1 || !namelen)\n");
        return NULL;
    }
    
    if(!(blkno = dir->i_zone[0]))
    {
        printl(__FILE__, __LINE__, 
                    "add_entry(): (!(blkno = dir->i_zone[0]))\n");
        return NULL;
    }
    if(!(bp = bget(dir->super_block_id, blkno)))
    {
        printl(__FILE__, __LINE__, 
                "add_entry(): (!(bp = bget(dir->super_block_id, blkno)))\n");
        return NULL;
    }
    
    i = 0;
    de = (struct dir_entry *)bp->data;
    while(1)
    {
        if((char *)de >= bp->data + BLOCK_SIZE)
        {
            brelse(bp);
            bp = NULL;
            
            // 返回块号或创建块
            blkno = bmap(dir, i/(BLOCK_SIZE/sizeof(struct dir_entry)), 1);
            if(!blkno)
            {
                printl(__FILE__, __LINE__, "add_entry(): (!blkno)\n");
                return NULL;
            }
            if(!(bp = bget(dir->super_block_id, blkno)))
            {
                i += (BLOCK_SIZE/sizeof(struct dir_entry));
                continue;
            }
            de = (struct dir_entry *)bp->data;
        }
        
        if(i * sizeof(struct dir_entry) >= dir->i_size)
        {
            de->ino = 0;
            dir->i_size = (i + 1) * sizeof(struct dir_entry);
            dir->flag = UPD;
        }
        
        // 目录项结构的i节点成员为0代表是空目录项
        if(!de->ino)
        {
            for(j = 0; j < NAME_LEN; j++)
            {
                de->name[j] = (j < namelen) ? *(name + j) : '\0';
            }
            bp->flag = UPD;
            *res_dir = de;
            return bp;
        }
        
        de++;
        i++;
    }
}