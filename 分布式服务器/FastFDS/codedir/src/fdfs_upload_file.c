#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fdfs_client.h"

int my_upload_file1(const char* conf_file,const char* local_file,char* file_id)
{
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;

	if ((result=fdfs_client_init(conf_file)) != 0)
	{
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}

	*group_name = '\0';
	if ((result=tracker_query_storage_store(pTrackerServer, \
	                &storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		fprintf(stderr, "tracker_query_storage fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
		return result;
	}

	result = storage_upload_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_file, NULL, \
			NULL, 0, group_name, file_id);
	if (result == 0)
	{
		printf("%s\n", file_id);
	}
	else
	{
		fprintf(stderr, "upload file fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}


// 使用多进程的方式实现
int my_upload_file2(const char* conf_file,const char* local_file,char* file_id,int size)
{
    // 创建匿名管道用于父子进程通信
    int fd[2];
    int ret=pipe(fd);
    
    if(ret==-1)
    {
        perror("pipe error");
        exit(0);
    }

    // 创建子进程
    pid_t pid=fork();

    // 子进程
    if(pid==0)
    {
        // 标准输出重定向到管道的写端
        dup2(fd[1],STDOUT_FILENO);

        // 子进程从管道写不需要读
        close(fd[0]);

        // 执行execlp命令
        execlp("fdfs_upload_file","xxx",conf_file,local_file,NULL);

        perror("execpl error");
    }
    else
    {
        // 父进程读取管道信息
        close(fd[1]);
        read(fd[0],file_id,size);

        // 回收子进程PCB
        wait(NULL);
    }

}

