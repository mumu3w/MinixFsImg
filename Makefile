RM		= -del
CP		= copy
CC		= gcc
AR		= ar
CFLAGS	= -g -Werror -I./inc
LDFLAGS	= -s -L. -lminixfs_v1


APPS	= MinixFsImg

LIBNAME = libminixfs_v1.a
LIBOBJ	= src/minixfs_v1_core.o \
			src/minixfs_v1_disk.o \
			src/minixfs_v1_debug.o \
			src/minixfs_v1_inode.o \
			src/minixfs_v1_bitmap.o \
			src/minixfs_v1_swap.o \
			src/minixfs_v1_namei.o \
			src/minixfs_v1_itrunc.o \
			src/minixfs_v1_system.o
			
INCNAME	= inc/minixfs_v1.h


.PHONY: all clean
all : $(LIBNAME) $(APPS)


MinixFsImg : test/MinixFsImg.c $(INCNAME)
	$(CC) $(CFLAGS) -o $@ test/MinixFsImg.c $(LDFLAGS)

$(LIBNAME) : $(LIBOBJ)
	$(AR) rcs $(LIBNAME) $(LIBOBJ)

src/minixfs_v1_core.o : src/minixfs_v1_core.c $(INCNAME)

src/minixfs_v1_disk.o : src/minixfs_v1_disk.c $(INCNAME)

src/minixfs_v1_debug.o : src/minixfs_v1_debug.c $(INCNAME)

src/minixfs_v1_inode.o : src/minixfs_v1_inode.c $(INCNAME)

src/minixfs_v1_bitmap.o : src/minixfs_v1_bitmap.c $(INCNAME)

src/minixfs_v1_swap.o : src/minixfs_v1_swap.c $(INCNAME)

src/minixfs_v1_namei.o : src/minixfs_v1_namei.c $(INCNAME)

src/minixfs_v1_itrunc.o : src/minixfs_v1_itrunc.c $(INCNAME)

src/minixfs_v1_system.o : src/minixfs_v1_system.c $(INCNAME)

%o : %c

clean:
	@$(RM) *.exe *.o *.dat $(LIBNAME) src\*.o