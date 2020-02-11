//-----------------------------------------------------------------------------
// File: globe.cpp
// Desc: sample showing how to texture a sphere with OpenGL, glu quadric
//
// Autumn 2000 - Ge Wang - implementation
//        ( RGB file loading routine by David Blythe, SGI )
//-----------------------------------------------------------------------------
#include <GL/glut.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------

struct GeImageData;
bool ge_read_image( const char * filename, GeImageData * pImgData );
unsigned * ge_read_rgb( const char * name, int * width,
                        int * height, int * comp );

//-----------------------------------------------------------------------------
// Name: struct GeImageData
// Desc: holds basic image data
//-----------------------------------------------------------------------------
struct GeImageData
{
    int width;
    int height;
    unsigned * bits;

    GeImageData( int w = 0, int h = 0, unsigned * p = 0 )
        : width( w ), height( h ), bits( p )
    { }
};

//-----------------------------------------------------------------------------
// global variables and #defines
//-----------------------------------------------------------------------------


// whether to animate
GLboolean g_rotate = GL_TRUE;

// texture and glu data
GLUquadricObj * g_sphere = NULL;
GeImageData g_imgData;
GLuint g_img = 0;

GLfloat g_angle_x = 27.0f;
GLfloat g_inc = 0.0f;
char g_filename[128] = "D:/CG LABBBBBB/Test globe/globe/map.rgb";

// light position
GLfloat g_light_position[4] = { 2.0f, 1.2f, 4.0f, 1.0f };

void initialize (){
    // enable lighting
    glEnable( GL_LIGHTING );
    // enable lighting for front
    glLightModeli( GL_FRONT, GL_TRUE );
    // material have diffuse and ambient lighting
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    // enable color
    glEnable( GL_COLOR_MATERIAL );
    // enable light 0
    glEnable( GL_LIGHT0 );

    // initialize our sphere
    g_sphere = gluNewQuadric( );
    gluQuadricNormals( g_sphere, ( GLenum )GLU_SMOOTH );
    gluQuadricTexture( g_sphere, GL_TRUE );

    // set texture state
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    // read the texture
    if( !ge_read_image( g_filename, &g_imgData ) )
    {
        printf( "couldn't open file '%s'...\n", g_filename );
        printf( "usage: globe <filename>.rgb\n" );
        exit(1);
    }

    // generate OpenGL texture
    glGenTextures( 1, &g_img );
    glBindTexture( GL_TEXTURE_2D, g_img );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        g_imgData.width,
        g_imgData.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        g_imgData.bits
        );
}

//-----------------------------------------------------------------------------
// Name: ge_read_image( )
// Desc: reads an RGB file into pImgData
//-----------------------------------------------------------------------------
bool ge_read_image( const char * filename, GeImageData * pImgData )
{
    int c;
    if( !filename || !pImgData )
        return false;

    // zero out the memory
    memset( pImgData, 0, sizeof( GeImageData ) );

    pImgData->bits = ge_read_rgb( filename, &pImgData->width,
                                  &pImgData->height, &c );

    return ( pImgData->bits != 0 );
}

//-----------------------------------------------------------------------------
/* texture loading - by David Blythe, SGI */
/* a simplistic routine for reading an SGI .rgb image file. */
//-----------------------------------------------------------------------------

void
bwtorgba(unsigned char *b,unsigned char *l,int n) {
    while(n--) {
        l[0] = *b;
        l[1] = *b;
        l[2] = *b;
        l[3] = 0xff;
        l += 4; b++;
    }
}

void
rgbtorgba(unsigned char *r,unsigned char *g,unsigned char *b,
          unsigned char *l,int n) {
    while(n--) {
        l[0] = r[0];
        l[1] = g[0];
        l[2] = b[0];
        l[3] = 0xff;
        l += 4; r++; g++; b++;
    }
}

void
rgbatorgba(unsigned char *r,unsigned char *g,unsigned char *b,
           unsigned char *a,unsigned char *l,int n) {
    while(n--) {
        l[0] = r[0];
        l[1] = g[0];
        l[2] = b[0];
        l[3] = a[0];
        l += 4; r++; g++; b++; a++;
    }
}

typedef struct _ImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short xsize, ysize, zsize;
    unsigned int min, max;
    unsigned int wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB;
    unsigned long rleEnd;
    unsigned int *rowStart;
    int *rowSize;
} ImageRec;

static void
ConvertShort(unsigned short *array, unsigned int length) {
    unsigned short b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        *array++ = (b1 << 8) | (b2);
    }
}

static void
ConvertUint(unsigned *array, unsigned int length) {
    unsigned int b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        b3 = *ptr++;
        b4 = *ptr++;
        *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static ImageRec *ImageOpen(const char *fileName)
{
    union {
        int testWord;
        char testByte[4];
    } endianTest;
    ImageRec *image;
    int swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
        swapFlag = 1;
    } else {
        swapFlag = 0;
    }

    image = (ImageRec *)malloc(sizeof(ImageRec));
    if (image == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    if ((image->file = fopen(fileName, "rb")) == NULL) {
        perror(fileName);
        //exit(1);
        return NULL;
    }

    fread(image, 1, 12, image->file);

    if (swapFlag) {
        ConvertShort(&image->imagic, 6);
    }

    image->tmp = (unsigned char *)malloc(image->xsize*256);
    image->tmpR = (unsigned char *)malloc(image->xsize*256);
    image->tmpG = (unsigned char *)malloc(image->xsize*256);
    image->tmpB = (unsigned char *)malloc(image->xsize*256);
    if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
        image->tmpB == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }

    if ((image->type & 0xFF00) == 0x0100) {
        x = image->ysize * image->zsize * (int) sizeof(unsigned);
        image->rowStart = (unsigned *)malloc(x);
        image->rowSize = (int *)malloc(x);
        if (image->rowStart == NULL || image->rowSize == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(1);
        }
        image->rleEnd = 512 + (2 * x);
        fseek(image->file, 512, SEEK_SET);
        fread(image->rowStart, 1, x, image->file);
        fread(image->rowSize, 1, x, image->file);
        if (swapFlag) {
            ConvertUint(image->rowStart, x/(int) sizeof(unsigned));
            ConvertUint((unsigned *)image->rowSize, x/(int) sizeof(int));
        }
    }
    return image;
}

static void
ImageClose(ImageRec *image) {
    fclose(image->file);
    free(image->tmp);
    free(image->tmpR);
    free(image->tmpG);
    free(image->tmpB);
    free(image);
}

static void
ImageGetRow(ImageRec *image, unsigned char *buf, int y, int z) {
    unsigned char *iPtr, *oPtr, pixel;
    int count;

    if ((image->type & 0xFF00) == 0x0100) {
        fseek(image->file, (long) image->rowStart[y+z*image->ysize],
              SEEK_SET);
        fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],
              image->file);

        iPtr = image->tmp;
        oPtr = buf;
        for (;;) {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count) {
                return;
            }
            if (pixel & 0x80) {
                while (count--) {
                    *oPtr++ = *iPtr++;
                }
            } else {
                pixel = *iPtr++;
                while (count--) {
                    *oPtr++ = pixel;
                }
            }
        }
    } else {
        fseek(image->file,
              512+(y*image->xsize)+(z*image->xsize*image->ysize),
              SEEK_SET);
        fread(buf, 1, image->xsize, image->file);
    }
}

unsigned * ge_read_rgb( const char * name, int * width, int * height,
                       int * components) {
    unsigned *base, *lptr;
    unsigned char *rbuf, *gbuf, *bbuf, *abuf;
    ImageRec *image;
    int y;

    image = ImageOpen(name);

    if(!image)
        return NULL;
    (*width)=image->xsize;
    (*height)=image->ysize;
    (*components)=image->zsize;
    base = (unsigned *)malloc(image->xsize*image->ysize*sizeof(unsigned));
    rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    if(!base || !rbuf || !gbuf || !bbuf)
        return NULL;
    lptr = base;
    for(y=0; y<image->ysize; y++) {
        if(image->zsize>=4) {
            ImageGetRow(image,rbuf,y,0);
            ImageGetRow(image,gbuf,y,1);
            ImageGetRow(image,bbuf,y,2);
            ImageGetRow(image,abuf,y,3);
            rgbatorgba(rbuf,gbuf,bbuf,abuf,
              (unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
        } else if(image->zsize==3) {
            ImageGetRow(image,rbuf,y,0);
            ImageGetRow(image,gbuf,y,1);
            ImageGetRow(image,bbuf,y,2);
            rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
        } else {
            ImageGetRow(image,rbuf,y,0);
            bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
        }
    }
    ImageClose(image);
    free(rbuf);
    free(gbuf);
    free(bbuf);
    free(abuf);

    return (unsigned *) base;
}

void displayFunc( )
{
    static GLfloat x = 0.0f;

    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if( g_img )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, g_img );
    }

    // save the current matrix
    glPushMatrix( );

        // rotate the view point
        glRotatef( g_angle_x, 1.0f, 0.0f, 0.0f );
        g_angle_x += g_inc;

        // rotate the planet
        glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
        glRotatef( x, 0.0f, 0.0f, 1.0f );
        x += .4f;

        // draw the sphere
        glColor3f( 1.0f, 1.0f, 1.0f );
        gluSphere( g_sphere, 1.28f, 24, 24 );

    glPopMatrix( );

    if( g_img )
        glDisable( GL_TEXTURE_2D );

    glFlush( );
    glutSwapBuffers( );
}
