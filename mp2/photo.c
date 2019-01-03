/* tab:4
 *
 * photo.c - photo display functions
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       3
 * Creation Date: Fri Sep 9 21:44:10 2011
 * Filename:      photo.c
 * History:
 *    SL    1    Fri Sep 9 21:44:10 2011
 *        First written(based on mazegame code).
 *    SL    2    Sun Sep 11 14:57:59 2011
 *        Completed initial implementation of functions.
 *    SL    3    Wed Sep 14 21:49:44 2011
 *        Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"


/* types local to this file(declared in types.h) */

/*
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;            /* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/*
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have
 * been stored as 2:2:2-bit RGB values(one byte each), including
 * transparent pixels(value OBJ_CLR_TRANSP).  As with the room photos,
 * pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of the
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;  /* defines height and width */
    uint8_t*       img;  /* pixel data               */
};


/* file-scope variables */

/*
 * The room currently shown on the screen.  This value is not known to
 * the mode X code, but is needed when filling buffers in callbacks from
 * that code(fill_horiz_buffer/fill_vert_buffer).  The value is set
 * by calling prep_room.
 */
static const room_t* cur_room = NULL;

/*
 * Define Octree Node Struct
 */
struct octree_node {
  uint32_t red, green, blue;
  uint32_t pixel_count;
  uint16_t l2_rgb_val, l4_rgb_val, palette_index;
};


/*
 * fill_horiz_buffer
 *   DESCRIPTION: Given the(x,y) map pixel coordinate of the leftmost
 *                pixel of a line to be drawn on the screen, this routine
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS:(x,y) -- leftmost pixel of line to be drawn
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]) {
    int            idx;   /* loop index over pixels in the line          */
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */
    int            yoff;  /* y offset into object image                  */
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo(cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ? view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate(cur_room); NULL != obj; obj = obj_next(obj)) {
        obj_x = obj_get_x(obj);
        obj_y = obj_get_y(obj);
        img = obj_image(obj);

        /* Is object outside of the line we're drawing? */
        if (y < obj_y || y >= obj_y + img->hdr.height || x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
            continue;
        }

        /* The y offset of drawing is fixed. */
        yoff = (y - obj_y) * img->hdr.width;

        /*
         * The x offsets depend on whether the object starts to the left
         * or to the right of the starting point for the line being drawn.
         */
        if (x <= obj_x) {
            idx = obj_x - x;
            imgx = 0;
        }
        else {
            idx = 0;
            imgx = x - obj_x;
        }

        /* Copy the object's pixel data. */
        for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
            pixel = img->img[yoff + imgx];

            /* Don't copy transparent pixels. */
            if (OBJ_CLR_TRANSP != pixel) {
                buf[idx] = pixel;
            }
        }
    }
}


/*
 * fill_vert_buffer
 *   DESCRIPTION: Given the(x,y) map pixel coordinate of the top pixel of
 *                a vertical line to be drawn on the screen, this routine
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS:(x,y) -- top pixel of line to be drawn
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void fill_vert_buffer(int x, int y, unsigned char buf[SCROLL_Y_DIM]) {
    int            idx;   /* loop index over pixels in the line          */
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */
    int            xoff;  /* x offset into object image                  */
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo(cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ? view->img[view->hdr.width *(y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate(cur_room); NULL != obj; obj = obj_next(obj)) {
        obj_x = obj_get_x(obj);
        obj_y = obj_get_y(obj);
        img = obj_image(obj);

        /* Is object outside of the line we're drawing? */
        if (x < obj_x || x >= obj_x + img->hdr.width ||
            y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
            continue;
        }

        /* The x offset of drawing is fixed. */
        xoff = x - obj_x;

        /*
         * The y offsets depend on whether the object starts below or
         * above the starting point for the line being drawn.
         */
        if (y <= obj_y) {
            idx = obj_y - y;
            imgy = 0;
        }
        else {
            idx = 0;
            imgy = y - obj_y;
        }

        /* Copy the object's pixel data. */
        for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
            pixel = img->img[xoff + img->hdr.width * imgy];

            /* Don't copy transparent pixels. */
            if (OBJ_CLR_TRANSP != pixel) {
                buf[idx] = pixel;
            }
        }
    }
}


/*
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t image_height(const image_t* im) {
    return im->hdr.height;
}


/*
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t image_width(const image_t* im) {
    return im->hdr.width;
}

/*
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t photo_height(const photo_t* p) {
    return p->hdr.height;
}


/*
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t photo_width(const photo_t* p) {
    return p->hdr.width;
}


/*
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void prep_room(const room_t* r) {
    /* Record the current room. */
    photo_t* p = room_photo(r);
    fill_palette_room(p->palette);
    cur_room = r;
}


/*
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t* read_obj_image(const char* fname) {
    FILE*    in;        /* input file               */
    image_t* img = NULL;    /* image structure          */
    uint16_t x;            /* index over image columns */
    uint16_t y;            /* index over image rows    */
    uint8_t  pixel;        /* one pixel from the file  */

    /*
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen(fname, "r+b")) ||
        NULL == (img = malloc(sizeof (*img))) ||
        NULL != (img->img = NULL) || /* false clause for initialization */
        1 != fread(&img->hdr, sizeof (img->hdr), 1, in) ||
        MAX_OBJECT_WIDTH < img->hdr.width ||
        MAX_OBJECT_HEIGHT < img->hdr.height ||
        NULL == (img->img = malloc
        (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
        if (NULL != img) {
            if (NULL != img->img) {
                free(img->img);
            }
            free(img);
        }
        if (NULL != in) {
            (void)fclose(in);
        }
        return NULL;
    }

    /*
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order(top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

        /* Loop over columns from left to right. */
        for (x = 0; img->hdr.width > x; x++) {

            /*
             * Try to read one 8-bit pixel.  On failure, clean up and
             * return NULL.
             */
            if (1 != fread(&pixel, sizeof (pixel), 1, in)) {
                free(img->img);
                free(img);
                (void)fclose(in);
                return NULL;
            }

            /* Store the pixel in the image data. */
            img->img[img->hdr.width * y + x] = pixel;
        }
    }

    /* All done.  Return success. */
    (void)fclose(in);
    return img;
}


/*
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t* read_photo(const char* fname) {
    FILE*    in;    /* input file               */
    photo_t* p = NULL;    /* photo structure          */
    uint16_t x;        /* index over image columns */
    uint16_t y;        /* index over image rows    */
    uint16_t pixel;    /* one pixel from the file  */

    /*
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen(fname, "r+b")) ||
        NULL == (p = malloc(sizeof (*p))) ||
        NULL != (p->img = NULL) || /* false clause for initialization */
        1 != fread(&p->hdr, sizeof (p->hdr), 1, in) ||
        MAX_PHOTO_WIDTH < p->hdr.width ||
        MAX_PHOTO_HEIGHT < p->hdr.height ||
        NULL == (p->img = malloc
        (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
        if (NULL != p) {
            if (NULL != p->img) {
                free(p->img);
            }
            free(p);
        }
        if (NULL != in) {
            (void)fclose(in);
        }
        return NULL;
    }
    
    /* OCTREE INITIALIZATION START */
    uint32_t image_size = p->hdr.width * p->hdr.height;
    uint32_t i;
    uint32_t red_avg, green_avg, blue_avg;
    uint32_t pixels[image_size];
    struct octree_node L2[OCTREE_L2_NODES];
    struct octree_node L4[OCTREE_L4_NODES];
    uint32_t l4_palette_index[OCTREE_L4_NODES];

    /* Initialize L2 Nodes of Octree, set everything to 0 */
    for(i = 0; i < OCTREE_L2_NODES; i++) {
      L2[i].red = L2[i].green = L2[i].blue = 0;
      L2[i].pixel_count = 0;
      L2[i].l2_rgb_val = L4[i].l4_rgb_val = 0;
      L2[i].palette_index = 0;
    }

    /* Initialize L4 Nodes of Octree, set everything to 0 */
    for(i = 0; i < OCTREE_L4_NODES; i++) {
      L4[i].red = L4[i].green = L4[i].blue = 0;
      L4[i].pixel_count = 0;
      L4[i].l2_rgb_val = L4[i].l4_rgb_val = 0;
      L4[i].palette_index = 0;
      l4_palette_index[i] = 0;
    }

    /*
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order(top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) {

        /* Loop over columns from left to right. */
        for (x = 0; p->hdr.width > x; x++) {

            /*
             * Try to read one 16-bit pixel.  On failure, clean up and
             * return NULL.
             */
            if (1 != fread(&pixel, sizeof (pixel), 1, in)) {
                free(p->img);
                free(p);
                (void)fclose(in);
                return NULL;
            }
            /*
             * 16-bit pixel is coded as 5:6:5 RGB(5 bits red, 6 bits green,
             * and 6 bits blue).  We change to 2:2:2, which we've set for the
             * game objects.  You need to use the other 192 palette colors
             * to specialize the appearance of each photo.
             *
             * In this code, you need to calculate the p->palette values,
             * which encode 6-bit RGB as arrays of three uint8_t's.  When
             * the game puts up a photo, you should then change the palette
             * to match the colors needed for that photo.
             */
            /* p->img[p->hdr.width * y + x] = (((pixel >> 14) << 4) | (((pixel >> 9) & 0x3) << 2) | ((pixel >> 3) & 0x3)); */

            /* OCTREE */
            /* Save pixel RGB values to a new array for future use */
            pixels[p->hdr.width * y + x] = pixel;
            /* Save value of pixel to Level 4 Pixel Array of Octree */
            /* Convert 16 bit RGB value to 12 bit RGB value for L4 */
            i = l4_rgb(pixel);
            /* Save value of RGB pixel to struct */
            /* Shift values of pixels and use bitmask to get individual RGB values */
            L4[i].red += (pixel >> 11) & 0x1F;
		        L4[i].green += (pixel >> 5) & 0x3F;
		        L4[i].blue += pixel & 0x1F;
            /* Increment number of times pixel RGB value is shown in image */
            L4[i].pixel_count++;
            /* Convert L4 RGB value to a L2 RGB value for specific color */
            L4[i].l2_rgb_val = l2_rgb(pixel);
            /* Save original L4 RGB value */
            L4[i].l4_rgb_val = i;
        }
    }
    (void)fclose(in);

    /* Sort the L4 pixel array of Octree by the pixel count */
    qsort(L4, OCTREE_L4_NODES, sizeof(struct octree_node), qsort_compare);

    /* Save the color of the first 128 nodes to the palette and save
     * their position to the new palette array
     */
    for(i = 0; i < OCTREE_L4_FINAL; i++) {
      L4[i].palette_index = PALETTE_OFFSET + i;
      l4_palette_index[L4[i].l4_rgb_val] = i;
      if(L4[i].pixel_count > 0) {
        red_avg = L4[i].red / L4[i].pixel_count;
        green_avg = L4[i].green / L4[i].pixel_count;
        blue_avg = L4[i].blue / L4[i].pixel_count;
      } else {
        red_avg = green_avg = blue_avg = 0;
      }
      p->palette[i][0] = (uint8_t) (red_avg & 0x1F) << 1;
		  p->palette[i][1] = (uint8_t) (green_avg & 0x3F);
		  p->palette[i][2] = (uint8_t) (blue_avg & 0x1F) << 1;
    }
    /* For the rest of the colors after the first 128, assign them the color
     * values of that of Level 2. Calculate the colors of level 2 by first
     * taking the average of the values of the pixels.
     */
    for(i = OCTREE_L4_FINAL; i < OCTREE_L4_NODES; i++) {
      l4_palette_index[L4[i].l4_rgb_val] = i;
      if(L4[i].l2_rgb_val < PALETTE_OFFSET) {
        L2[L4[i].l2_rgb_val].red += L4[i].red;
        L2[L4[i].l2_rgb_val].green += L4[i].green;
        L2[L4[i].l2_rgb_val].blue += L4[i].blue;
        L2[L4[i].l2_rgb_val].pixel_count += L4[i].pixel_count;
      }
    }
    /* After taking the combined sums and counting their frequency,
     * take the average and save the colors to the palette
     */
    for(i = 0; i < OCTREE_L2_NODES; i++) {
      L2[i].palette_index = PALETTE_OFFSET + OCTREE_L4_FINAL + i;
      if(L2[i].pixel_count > 0) {
        red_avg = L2[i].red / L2[i].pixel_count;
        green_avg = L2[i].green / L2[i].pixel_count;
        blue_avg = L2[i].blue / L2[i].pixel_count;
      } else {
        red_avg = green_avg = blue_avg = 0;
      }
      p->palette[i + OCTREE_L4_FINAL][0] = (uint8_t) (red_avg & 0x1F) << 1;
		  p->palette[i + OCTREE_L4_FINAL][1] = (uint8_t) (green_avg & 0x3F);
		  p->palette[i + OCTREE_L4_FINAL][2] = (uint8_t) (blue_avg & 0x1F) << 1;
    }
    /* Save the palette index of the colors after the 128 colors
     * to the initial L4 array
     */
    for(i = OCTREE_L4_FINAL; i < OCTREE_L4_NODES; i++) {
      if(L4[i].l2_rgb_val < PALETTE_OFFSET) {
        L4[i].palette_index = L2[L4[i].l2_rgb_val].palette_index;
      }
    }
    /* Save the correct palette index to the picture structure used to create
     * the image of the room 
     */
    for(i = 0; i < image_size; i++) {
      p->img[i] = L4[l4_palette_index[l4_rgb(pixels[i])]].palette_index;
    }
    return p;
}

/*
 * l2_rgb
 *   DESCRIPTION: Convert original RBG pixel value from 16 bits
 *                to 6 bits for level 2 of Octree
 *   INPUTS: pixel
 *   OUTPUTS: none
 *   RETURN VALUE: pixel value for L2
 *   SIDE EFFECTS: none
 */
int l2_rgb(uint16_t pixel) {
	return (((pixel >> 14) << 4) | (((pixel >> 9) & 0x03) << 2) | ((pixel >> 3) & 0x03));
}

/*
 * l4_rgb
 *   DESCRIPTION: Convert original RBG pixel value from 16 bits
 *                to 12 bits for level 4 of Octree
 *   INPUTS: pixel
 *   OUTPUTS: none
 *   RETURN VALUE: pixel value for L4
 *   SIDE EFFECTS: none
 */
int l4_rgb(uint16_t pixel) {
	return (((pixel >> 12) << 8) | (((pixel >> 7) & 0x0F) << 4) | ((pixel >> 1) & 0x0F));
}

/*
 * qsort_compare
 *   DESCRIPTION: Sort values of L4 pixel values by the pixel_count 
 *                in the image to determine color to be used in palette
 *   INPUTS: pointers to values of L4 pixels for comparison
 *   OUTPUTS: none
 *   RETURN VALUE: whether a pixel is larger or smaller than the other
 *   SIDE EFFECTS: none
 */
int qsort_compare(const void* a1, const void* b2) {
	const struct octree_node* a = a1;
	const struct octree_node* b = b2;
	return (b->pixel_count - a->pixel_count);
}
