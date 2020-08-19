#include "bitmap.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
struct work
{
    double xmin, xmax, ymin, ymax;
    int thread_start, thread_end, max, image_height, image_width;
    struct bitmap *bm;
};

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int n);
void *mandelthread(void *arg);

void show_help()
{
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-n <number_of_threads>    The maximum number of threads (default=1)\n");
    printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>  X coordinate of image center point. (default=0)\n");
    printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
    printf("-W <pixels> Width of the image in pixels. (default=500)\n");
    printf("-H <pixels> Height of the image in pixels. (default=500)\n");
    printf("-o <file>   Set output file. (default=mandel.bmp)\n");
    printf("-h          Show this help text.\n");
    printf("\nSome examples are:\n");
    printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
    printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
    printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
    char c;

    // These are the default configuration values used
    // if no command line arguments are given.

    const char *outfile = "mandel.bmp";
    double xcenter = 0.286932;
    double ycenter = 0.014287;
    double scale = 0.0005;
    int image_width = 500;
    int image_height = 500;
    int max = 1000;
    int number_of_thread=1;
    int number_of_process;


    // For each command line argument given,
    // override the appropriate configuration value.

    while((c = getopt(argc,argv,"n:x:y:s:W:H:m:o:h"))!=-1)
    {
        switch(c)
        {
        case 'n':
            number_of_thread = atoi(optarg);
            break;
        case 'x':
            xcenter = atof(optarg);
            break;
        case 'y':
            ycenter = atof(optarg);
            break;
        case 's':
            scale = atof(optarg);
            break;
        case 'W':
            image_width = atoi(optarg);
            break;
        case 'H':
            image_height = atoi(optarg);
            break;
        case 'm':
            max = atoi(optarg);
            break;
        case 'o':
            outfile = optarg;
            break;
        case 'h':
            show_help();
            exit(1);
            break;
        }
    }

    // Display the configuration of the image.
    printf("number of thread: n=%d mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",number_of_thread,xcenter,ycenter,scale,max,outfile);

    // Create a bitmap of the appropriate size.
    struct bitmap *bm = bitmap_create(image_width,image_height);

    // Fill it with a dark blue, for debugging
    bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

    // Compute the Mandelbrot image
    compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max,number_of_thread);

    // Save the image in the stated file.
    if(!bitmap_save(bm,outfile))
    {
        fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
        return 1;
    }

    return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int n)
{
    int i,j;
    pthread_t t[n];
    struct work p[n];
    int width = bitmap_width(bm);
    int height = bitmap_height(bm);
    for(i=0; i<n; i++)
    {
        if(i!=n)
        {
            if(i==0)
            {
                p[i].thread_start = 0;
                p[i].image_width=width;
                p[i].image_height=height;
                p[i].xmin=xmin;
                p[i].xmax=xmax;
                p[i].ymin=ymin;
                p[i].ymax=ymax;
                p[i].max=max;
                p[i].bm=bm;
                p[i].thread_end=(height/n)-1;
            }
            else
            {
                p[i].thread_start=p[i-1].thread_end+1;
                p[i].image_width=width;
                p[i].image_height=height;
                p[i].xmin=xmin;
                p[i].xmax=xmax;
                p[i].ymin=ymin;
                p[i].ymax=ymax;
                p[i].max=max;
                p[i].bm=bm;
                p[i].thread_end= p[i].thread_start + (p[i-1].thread_end - p[i-1].thread_start);
            }
        }
    }
    for(i=0; i<n; i++)
    {
        pthread_create(&t[i], NULL, mandelthread, &p[i]);

    }
     for(i=0; i<n; i++)
    {

        pthread_join(t[i], NULL);
    }

}

void *mandelthread(void *arg)
{
    int i,j;
    struct work *p=(struct work*)arg;

    // For every pixel in the image...

    for(j=p->thread_start; j<p->image_height; j++)
    {

        for(i=0; i<p->image_width; i++)
        {

            // Determine the point in x,y space for that pixel.
            double x = p->xmin + i*(p->xmax-p->xmin)/p->image_width;
            double y = p->ymin + j*(p->ymax-p->ymin)/p->image_height;

            // Compute the iterations at that point.
            int iters = iterations_at_point(x,y,p->max);

            // Set the pixel in the bitmap.
            bitmap_set(p->bm,i,j,iters);
        }
    }
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
    double x0 = x;
    double y0 = y;

    int iter = 0;

    while( (x*x + y*y <= 4) && iter < max )
    {

        double xt = x*x - y*y + x0;
        double yt = 2*x*y + y0;

        x = xt;
        y = yt;

        iter++;
    }

    return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
    int color1, color2, color3;

    if (i < 255)
    {
        color1 = 16*i;
        color2 = 8*(255 - i)/max;
        color3 = 2;
    }
    else if (i < 128 )
    {
        color1 = 16*(128 - i)/max;
        color2 = 8*i;
        color3 = 4;
    }
    else if (i < 64)
    {
        color1 = 16*(i - 64);
        color2 = 8*(64- i)/max;
        color3 = 6;
    }
    else
    {
        color1 = i*16;
        color2 = i*8;
        color3 = 3;
    }
    return MAKE_RGBA(color1, color2, color3, 0);
}




