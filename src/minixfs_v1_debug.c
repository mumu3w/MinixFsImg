#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "minixfs_v1.h"


/*****************************************************************************
    无法恢复的错误
*****************************************************************************/
void panic(char *src_file, int src_line, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "%s:%d:", src_file, src_line);
    vfprintf(stderr, format, ap);
    va_end(ap);
    
    destructor(); // 释放资源
    exit(EXIT_FAILURE);
}

/*****************************************************************************
    print log
*****************************************************************************/
void printl(char *src_file, int src_line, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
#ifdef DEBUG
#ifdef LOG
	fprintf(minixfs_v1_log_fp, "%s:%d:", src_file, src_line);
    vfprintf(minixfs_v1_log_fp, format, ap);
#else
    fprintf(stderr, "%s:%d:", src_file, src_line);
    vfprintf(stderr, format, ap);
#endif
#endif
    va_end(ap);
}

/*****************************************************************************
    print partition
*****************************************************************************/
void print_part(struct partition *part)
{
    printf("boot_ind                0x%.2X\n", part->boot_ind);
    printf("head                    0x%.2X\n", part->head);
    printf("sector                  0x%.2X\n", part->sector);
    printf("cyl                     0x%.4X\n", part->cyl);
    printf("sys_ind                 0x%.2X\n", part->sys_ind);
    printf("end_head                0x%.2X\n", part->end_head);
    printf("end_sector              0x%.2X\n", part->end_sector);
    printf("end_cyl                 0x%.4X\n", part->end_cyl);
    printf("start_sect              0x%.8X\n", part->start_sect);
    printf("nr_sects                0x%.8X\n", part->nr_sects);
}

/*****************************************************************************
    print m_super_block
*****************************************************************************/
void print_m_super_block(struct m_super_block *msb)
{
    printf("s_ninodes               0x%X\n", msb->s_ninodes);
    printf("s_nzones                0x%X\n", msb->s_nzones);
    printf("s_imap_block            0x%X\n", msb->s_imap_block);
    printf("s_zmap_block            0x%X\n", msb->s_zmap_block);
    printf("s_firstdatazone         0x%X\n", msb->s_firstdatazone);
    printf("s_log_zine_size         0x%X\n", msb->s_log_zine_size);
    printf("s_max_size              0x%X\n", msb->s_max_size);
    printf("s_magic                 0x%X\n", msb->s_magic);
    printf("firstimapzone           0x%X\n", msb->firstimapzone);
    printf("firstzmapzone           0x%X\n", msb->firstzmapzone);
    printf("firstinodezone          0x%X\n", msb->firstinodezone);
}

/*****************************************************************************
    print m_inode
*****************************************************************************/
void print_ino_value(struct m_inode *mip)
{
    printf("file_type               0%o\n", (mip->i_mode & S_IFMT) >> 12);
    printf("i_mode                  0%o\n", mip->i_mode & 0777);
    printf("i_uid                   0x%X\n", mip->i_uid);
    printf("i_size                  0x%X\n", mip->i_size);
    printf("i_mtime                 0x%X\n", mip->i_mtime);
    printf("i_gid                   0x%X\n", mip->i_gid);
    printf("i_nlinks                0x%X\n", mip->i_nlinks);
    printf("i_zone[0]               0x%X\n", mip->i_zone[0]);
    printf("i_zone[1]               0x%X\n", mip->i_zone[1]);
    printf("i_zone[2]               0x%X\n", mip->i_zone[2]);
    printf("i_zone[3]               0x%X\n", mip->i_zone[3]);
    printf("i_zone[4]               0x%X\n", mip->i_zone[4]);
    printf("i_zone[5]               0x%X\n", mip->i_zone[5]);
    printf("i_zone[6]               0x%X\n", mip->i_zone[6]);
    printf("i_zone[7]               0x%X\n", mip->i_zone[7]);
    printf("i_zone[8]               0x%X\n", mip->i_zone[8]);
    printf("super_block_id          0x%X\n", mip->super_block_id);
    printf("flag                    0x%X\n", mip->flag);
    printf("inode                   0x%X\n", mip->ino);
}