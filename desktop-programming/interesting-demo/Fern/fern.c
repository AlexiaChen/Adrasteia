/*******************************************************************************************************
*
*    You need to install the library called cairo,in which it is a 2D vector graphics library.
*  
*  the manual of the cairo you can click here (http://www.cairographics.org/documentation/)
*
*  To get the header files installed as well may require asking for a -dev or -devel package as follows:
*
*  For Debian and Debian derivatives including Ubuntu:
*
*  sudo apt-get install libcairo2-dev
*
* 
* @Author: AlexiaChen
* @brief:  the Demo of fractal Fern
********************************************************************************************************/



#include <cairo.h>
#include <time.h>


int main(int argc, char *argv[]){
    
       
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,  640,480 );  
    cairo_t* cr = cairo_create(surface);  
  
    int itera;
    float x = 0, y = 0, Probability = 0, temp= 0;
    
    //the Construction of the Barnsley fern Aglorithm I read the Wiki(http://en.wikipedia.org/wiki/Barnsley_fern) for reference
    
    srand( time( NULL ) );
    
    for ( itera =0 ; itera < 1000000; itera++ ) {
        
        Probability = rand() % 100;
        // 1%
        if ( Probability < 1 ) {
            x = 0;
            y = 0.16 * y;
        }
        // 85%
        if ( Probability >= 1 && Probability < 86 ) {
            temp = 0.85 * x + 0.04 * y;
            y = -0.04*x + 0.85 * y + 1.6;
            x = temp;
        }
        //  7%
        if ( Probability >= 86 && Probability < 93 ) {
            temp = 0.2 * x - 0.26 * y;
            y = 0.23 * x + 0.22 * y + 1.6;
            x = temp;
        }
        //  7%
        if ( Probability >= 93 ) {
            temp = -0.15 * x + 0.28 * y;
            y = 0.26 * x + 0.24 * y +0.44;
            x = temp;
        }
        //draw a green pixel , referred to http://lists.cairographics.org/archives/cairo/2008-April/013741.html ,because i wanna use a funtion like putpixel for purpose
        cairo_rectangle (cr, 45*x+230,480-45*y, 1, 1);
	cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);  
	cairo_fill (cr);
    }
    
    cairo_surface_write_to_png(surface, "fern.png");  
    //destory drawing enviroment
    cairo_destroy(cr);  
    cairo_surface_destroy (surface);  
        
    return 0;
}

