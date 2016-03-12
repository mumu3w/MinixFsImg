#ifndef __MINIXFSRW_H__
#define __MINIXFSRW_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>


// 文件类型
#define S_IFMT                  00170000
#define S_IFREG                 0100000
#define S_IFBLK                 0060000
#define S_IFDIR                 0040000
#define S_IFCHR                 0020000
#define S_IFIFO                 0010000
// 文件属性
#define S_ISUID                 0004000
#define S_ISGID                 0002000
#define S_ISVTX                 0001000 
#define S_ISREG(m)              (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)              (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)              (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)              (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)             (((m) & S_IFMT) == S_IFIFO)

#define SECTOR_SIZE             512
#define BLOCK_SIZE              1024
#define MINIXFS_V1              0x137F
#define ROOT_INO                1
#define UPD                     1
#define IPB                     (BLOCK_SIZE/sizeof(struct d_inode))

struct buf
{
    uint8   flag;
    char    *data;
    uint16  blkno;
    uint32  sector_offset;
};

struct partition
{
    uint8   boot_ind;
    uint8   head;
    uint8   sector;
    uint8   cyl;
    uint8   sys_ind;
    uint8   end_head;
    uint8   end_sector;
    uint8   end_cyl;
    uint32  start_sect;
    uint32  nr_sects;
};

struct super_block
{
    uint16  s_ninodes;
    uint16  s_nzones;
    uint16  s_imap_block;
    uint16  s_zmap_block;
    uint16  s_firstdatazone;
    uint16  s_log_zine_size;
    uint32  s_max_size;
    uint16  s_magic;
};

struct m_super_block
{
    uint16  s_ninodes;
    uint16  s_nzones;
    uint16  s_imap_block;
    uint16  s_zmap_block;
    uint16  s_firstdatazone;
    uint16  s_log_zine_size;
    uint32  s_max_size;
    uint16  s_magic;
    // mem
    uint16  firstimapzone;
    uint16  firstzmapzone;
    uint16  firstinodezone;
};

struct d_inode
{
    uint16  i_mode;
    uint16  i_uid;
    uint32  i_size;
    uint32  i_mtime;
    uint8   i_gid;
    uint8   i_nlinks;
    uint16  i_zone[9];
};

struct m_inode
{
    uint16  i_mode;
    uint16  i_uid;
    uint32  i_size;
    uint32  i_mtime;
    uint8   i_gid;
    uint8   i_nlinks;
    uint16  i_zone[9];
    // mem
    uint8   super_block_id;
    uint8   flag;
    uint16  ino;
};

#define NAME_LEN    14
struct dir_entry
{
    uint16  ino;
    char    name[NAME_LEN];
};
 

int read_part(unsigned int  part_id, struct partition *part);
void constructor(const char *minixfs_v1_image_name);
static inline unsigned int get_file_size(FILE *fp);
static inline void panic(const char *src_name, int src_line, 
									const char *format, ...);
static inline char *bread(unsigned int off_bytes);
static inline void bwrite(unsigned int off_bytes, const char *buf);



#ifndef NODEBUG
static inline void print_part(struct partition *part)
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
#endif

#endif // __MINIXFSRW_H__
