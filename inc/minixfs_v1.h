#ifndef __MINIXFS_V1_H__
#define __MINIXFS_V1_H__

typedef unsigned char           uint8;
typedef unsigned short          uint16;
typedef unsigned int            uint32;
typedef unsigned int            uint;

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

/*****************************************************************************
    minixfs_v1_core.c
*****************************************************************************/
extern FILE *minixfs_v1_image_fp; // 全局变量
extern FILE *minixfs_v1_log_fp;
void constructor(char *minixfs_v1_image_name);
void destructor(void);
/*****************************************************************************
    minixfs_v1_debug.c
*****************************************************************************/
void panic(char *src_file, int src_line, const char *format, ...);
void printl(char *src_file, int src_line, const char *format, ...);
void print_part(struct partition *part);
void print_m_super_block(struct m_super_block *msb);
void print_ino_value(struct m_inode *mip);
/*****************************************************************************
    minixfs_v1_disk.c
*****************************************************************************/
int read_super_block(uint super_block_id, struct m_super_block *msb);
int read_part(uint part_id, struct partition *part);
struct buf *bget(uint super_block_id, uint blkno);
void brelse(struct buf *bp);
void bread(uint sector_offset, char *ptr);
void bwrite(uint sector_offset, const char *ptr);
/*****************************************************************************
    minixfs_v1_inode.c
*****************************************************************************/
struct m_inode *iget(uint super_block_id, uint ino);
void irelse(struct m_inode *mip);
/*****************************************************************************
    minixfs_v1_bitmap.c
*****************************************************************************/
uint bmap(struct m_inode *mip, uint blkno, uint create);
uint balloc(uint super_block_id);
void bfree(uint super_block_id, uint blkno);
uint ialloc(uint super_block_id);
void ifree(uint super_block_id, uint ino);
/*****************************************************************************
    minixfs_v1_namei.c
*****************************************************************************/
struct buf *find_entry(struct m_inode **dir, char *name, int namelen, 
                                            struct dir_entry **res_dir);
struct m_inode *get_dir(uint super_block_id, char *pathname);
struct m_inode *dir_namei(uint super_block_id, char *pathname, int *namelen, 
                                                                char **name);
struct m_inode *namei(uint super_block_id, char *pathname);
struct buf *add_entry(struct m_inode *dir, char *name, int namelen, 
                                        struct dir_entry **res_dir);
/*****************************************************************************
    minixfs_v1_itrunc.c
*****************************************************************************/
void itrunc(struct m_inode *mip);
/*****************************************************************************
    minixfs_v1_system.c
*****************************************************************************/
struct m_inode *touch(uint super_block_id, char *pathname, uint mode);
/*****************************************************************************
    minixfs_v1_swap.c
*****************************************************************************/
void extract_file(FILE *fp, struct m_inode *mip);
int add_file(FILE *fp, uint super_block_id, char *pathname);

#endif