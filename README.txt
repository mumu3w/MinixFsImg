Minix v1 FileSystem tools

2016.03.07
        match还是出了问题,如果name是de->name的子句,那将匹配一个可能的错误文件
            if(!strncmp(de->name, name, namelen)) 
            if((strlen(de->name) == namelen) && \
                (!strncmp(de->name, name, namelen))

2016.03.06 
        最初match函数测试是对的, 由于疏忽将namelen写成了NAME_LEN
            if(!strncmp(de->name, name, NAME_LEN)) 
            if(!strncmp(de->name, name, namelen)) 
        这个问题直接导致了文件名比较失败,结果就是只能向根目录添加
        文件.

2016.01.24:
        0.02