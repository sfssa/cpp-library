#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "fdfs_upload_file.h"

int main(int argc,char* argv[])
{
    char file_id[1024]={0};
    my_upload_file1("/etc/fdfs/client.conf","main.c",file_id);

    printf("file_id: %s\n",file_id);
    printf("========================\n");
    my_upload_file2("/etc/fdfs/client.conf","main.c",file_id,sizeof(file_id));
    printf("file_id: %s\n",file_id);
    return 0;
}

