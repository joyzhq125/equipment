#include <stdio.h>
#include <sys/stat.h>  
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
int trave_dir_latest_file(char* path)
{
    DIR *d; 
    struct dirent *file; 
    struct stat buf;    

    if(!(d = opendir(path)))
    {
        printf("error opendir %s!!!\n",path);
        return -1;
    }
    chdir(path);//Add this, so that it can scan the children dir
    while((file = readdir(d)) != NULL)
    {
	printf("file is %s\n",file->d_name);
        if(strncmp(file->d_name, ".", 1) == 0 ||strncmp(file->d_name, "..", 1) == 0)
            continue;
        if(stat(file->d_name, &buf) >= 0 && !S_ISDIR(buf.st_mode))
        {
            printf("aaaa%s\n",file->d_name);
            printf("file size=%d\n",buf.st_size);
            printf("file last modify time=%d\n",buf.st_mtime);
        }

    }
    closedir(d);
    return 0;
}
main()
{
 	char timeString[64] = {0};
	tm       *temptm; 
	time_t filetime_latest;
	#if (defined(__i386__)) 
		printf("__i386__\r\n");
	#elif (defined(__x86_64__))
		printf("__x86_64__\r\n");
	#else
		printf("null\r\n");
	#endif
	//printf("aaaaa\n");
	trave_dir_latest_file("//var//log//EManager/");

	struct timeval  timeVal;
	gettimeofday( &timeVal, NULL );
	strftime( timeString, sizeof(timeString), "%Y%m%d", localtime(&timeVal.tv_sec));

	//获取0点时间戳
	temptm = localtime(&timeVal.tv_sec);     
	temptm->tm_hour = 0;    
	temptm->tm_min =0;    
	temptm->tm_sec = 0;
	filetime_latest =mktime(temptm);

	printf("time is %d cur is %d\n",filetime_latest,timeVal.tv_sec);

}



