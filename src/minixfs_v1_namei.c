#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixfs_v1.h"


/*****************************************************************************
 �ļ����Ƚ�
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
 ��ָ��Ŀ¼������Ŀ
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
    
    // ������ĳĿ¼�´�����5���ļ�,��������������Ŀ,������7;
    // ����ɾ��5���ļ����е��κ�һ���ļ�,ĳĿ¼�Ĵ�С����7;
    // ��������������һ���ļ�,��ô��Ŀ��λ�ý���ԭ����ɾ
    // ���ļ���λ��,��ʱĳĿ¼��С����;������������һ���ļ�
    // ĳĿ¼��С������
    entries = (*dir)->i_size / sizeof(struct dir_entry);
    *res_dir = NULL;
    // Ŀ¼��СΪ2 * sizeof(struct dir_entry),Ŀ¼�ļ���һ��Ϊ��
    // ����Ȼ������
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
        // һ���߼����������������һ��
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
    // �Ҳ���Ŀ¼��
    brelse(bp);
    return NULL;
}

/*****************************************************************************
 ��ȡ·��i�ڵ�
 pathname = "/root"  ���ظ�Ŀ¼
 pathname = "/root/"  ����rootĿ¼
 pathname = "/root/test"  ����rootĿ¼
 pathname = "/root/test/"  ����testĿ¼
 Ҳ����˵�������һ��'/'֮ǰ��Ŀ¼�ڵ�
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
 ��ȡ·��i�ڵ�,�Լ����Ŀ¼��
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
 ·����תi�ڵ�
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
    // pathname = "/root/test/"  namelen = 0,ֱ�ӷ���,��Ϊ�Ѿ������Ŀ¼�ڵ�
    if(!namelen)
    {
        return dir;
    }
    // pathname = "/root/test"  namelen = 4  name = "test" �����������ڵ�
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
 ���Ŀ¼��
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
            
            // ���ؿ�Ż򴴽���
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
        
        // Ŀ¼��ṹ��i�ڵ��ԱΪ0�����ǿ�Ŀ¼��
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