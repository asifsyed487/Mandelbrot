
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<math.h>
#include<sys/wait.h>
char xmin[]="0.286932";
char ymin[]="0.014287";
char image_width[]="500";
char image_height[]="500";
char max[]="1000";
char picture_name[50], zooming[50];
int thread_number[50];
int process_count;
float scale_zoom[60];
int zooming_scale()
{
    float zoom_from=2, zoom_to=0.00001, multiply;
    multiply= exp(log(zoom_to / zoom_from)/51);
    scale_zoom[1]=zoom_from;
    for(int i=2;i<=50;i++)
    {
        scale_zoom[i]=scale_zoom[i-1] * multiply;
    }

}

int main(int argc, char *argv[] )
{
    int number_of_process=50, number_of_thread=1;
    if(argc!=1&&argc!=3&&argc!=5)
    {
        fprintf(stderr, "mandelmovie.c:ERROR!");
    }
    if(argc>1)
    {
        if(argv[1][1]=='n')
        {
            number_of_thread= atoi(argv[2]);
            if(argc==5)
            {
                number_of_process=atoi(argv[4]);
            }
        }
        else
        {
            number_of_process=atoi(argv[2]);
            if(argc==5)
            {
                number_of_thread=atoi(argv[4]);
            }
        }
    }
    sprintf(thread_number,"%d", number_of_thread);
    int limit=number_of_process;
    pid_t pid;
    int i;
    zooming_scale();
    process_count=0;
    for(int i=1; i<=50; i++)
    {
        if(process_count ==limit)
        {
            wait(NULL);
            process_count--;
        }
        if(process_count<limit)
        {
            process_count++;
            pid = fork();
        }
        if(pid== -1)
        {
            printf("Error! Failed to create %dth child", i);
            exit(1);
        }
        else if(pid==0)
        {
            sprintf(picture_name, "m%d.bmp", i);
            sprintf(zooming, "%f", scale_zoom[i]);
            execlp("./mandel", "./mandel", "-x", xmin, "-y", ymin, "-s", zooming, "-w", image_width, "-H", image_height, "-m", max, "-n", thread_number, "-o", picture_name, NULL);

        }

    }
    while(process_count>0)
        {
            wait(NULL);
            process_count--;
        }
        system("ffmpeg -i m%d.bmp mandel.mpg");
}
