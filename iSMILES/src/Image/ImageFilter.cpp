#include <math.h>
#include <algorithm>
#include "ImageFilter.h"

namespace gga
{
    static inline int ROUND (double x) { return (int) (x + 0.5);}
// Square:
#define SQR(x) ((x) * (x))
//    static inline double SQR(double x) { return x*x;}

    
    struct UnsharpMaskParams
    {
      double  radius;
      double  amount;
      int     threshold;
    };

    static void blurLine (const double* ctable, const double* cmatrix, const int cmatrix_length,
                        const unsigned char* src, unsigned char* dest, const int len, const int bytes)
    {
        const double *cmatrix_p;
        const double *ctable_p;
        const unsigned char  *src_p;
        const unsigned char  *src_p1;
        const int     cmatrix_middle = cmatrix_length / 2;
        int           row;
        int           i, j;

        /* This first block is the same as the optimized version --
        * it is only used for very small pictures, so speed isn't a
        * big concern.
        */
        if (cmatrix_length > len)
        {
            for (row = 0; row < len; row++)
            {
                /* find the scale factor */
                double scale = 0;

                for (j = 0; j < len; j++)
                {
                    /* if the index is in bounds, add it to the scale counter */
                    if (j + cmatrix_middle - row >= 0 &&
                        j + cmatrix_middle - row < cmatrix_length)
                    scale += cmatrix[j];
                }

                src_p = src;

                for (i = 0; i < bytes; i++)
                {
                    double sum = 0;

                    src_p1 = src_p++;

                    for (j = 0; j < len; j++)
                    {
                        if (j + cmatrix_middle - row >= 0 &&
                            j + cmatrix_middle - row < cmatrix_length)
                        sum += *src_p1 * cmatrix[j];

                        src_p1 += bytes;
                    }

                    *dest++ = (unsigned char) ROUND (sum / scale);
                }
            }
        }
        else
        {
            /* for the edge condition, we only use available info and scale to one */
            for (row = 0; row < cmatrix_middle; row++)
            {
                /* find scale factor */
                double scale = 0;

                for (j = cmatrix_middle - row; j < cmatrix_length; j++)
                scale += cmatrix[j];

                src_p = src;

                for (i = 0; i < bytes; i++)
                {
                    double sum = 0;

                    src_p1 = src_p++;

                    for (j = cmatrix_middle - row; j < cmatrix_length; j++)
                    {
                        sum += *src_p1 * cmatrix[j];
                        src_p1 += bytes;
                    }

                    *dest++ = (unsigned char) ROUND (sum / scale);
                }
            }

            /* go through each pixel in each col */
            for (; row < len - cmatrix_middle; row++)
            {
                src_p = src + (row - cmatrix_middle) * bytes;

                for (i = 0; i < bytes; i++)
                {
                    double sum = 0;

                    cmatrix_p = cmatrix;
                    src_p1 = src_p;
                    ctable_p = ctable;

                    for (j = 0; j < cmatrix_length; j++)
                    {
                        sum += cmatrix[j] * *src_p1;
                        src_p1 += bytes;
                        ctable_p += 256;
                    }

                    src_p++;
                    *dest++ = (unsigned char) ROUND (sum);
                }
            }

            /* for the edge condition, we only use available info and scale to one */
            for (; row < len; row++)
            {
                /* find scale factor */
                double scale = 0;

                for (j = 0; j < len - row + cmatrix_middle; j++)
                scale += cmatrix[j];

                src_p = src + (row - cmatrix_middle) * bytes;

                for (i = 0; i < bytes; i++)
                {
                    double sum = 0;

                    src_p1 = src_p++;

                    for (j = 0; j < len - row + cmatrix_middle; j++)
                    {
                        sum += *src_p1 * cmatrix[j];
                        src_p1 += bytes;
                    }

                    *dest++ = (unsigned char) ROUND (sum / scale);
                }
            }
        }
    }

    /* generates a 1-D convolution matrix to be used for each pass of
     * a two-pass gaussian blur.  Returns the length of the matrix.
     */
    static int makeConvolveMatrix (double radius, double **cmatrix_p)
    {
      double *cmatrix;
      double  std_dev;
      double  sum;
      int     matrix_length;
      int     i, j;

      /* we want to generate a matrix that goes out a certain radius
       * from the center, so we have to go out ceil(rad-0.5) pixels,
       * inlcuding the center pixel.  Of course, that's only in one direction,
       * so we have to go the same amount in the other direction, but not count
       * the center pixel again.  So we double the previous result and subtract
       * one.
       * The radius parameter that is passed to this function is used as
       * the standard deviation, and the radius of effect is the
       * standard deviation * 2.  It's a little confusing.
       */
      radius = fabs (radius) + 1.0;

      std_dev = radius;
      radius = std_dev * 2;

      /* go out 'radius' in each direction */
      matrix_length = (int)(2 * ceil (radius - 0.5) + 1);
      if (matrix_length <= 0)
        matrix_length = 1;

      *cmatrix_p = new double[matrix_length];
      cmatrix = *cmatrix_p;

      /*  Now we fill the matrix by doing a numeric integration approximation
       * from -2*std_dev to 2*std_dev, sampling 50 points per pixel.
       * We do the bottom half, mirror it to the top half, then compute the
       * center point.  Otherwise asymmetric quantization errors will occur.
       *  The formula to integrate is e^-(x^2/2s^2).
       */

      /* first we do the top (right) half of matrix */
      for (i = matrix_length / 2 + 1; i < matrix_length; i++)
        {
          double base_x = i - (matrix_length / 2) - 0.5;

          sum = 0;
          for (j = 1; j <= 50; j++)
            {
              double r = base_x + 0.02 * j;

              if (r <= radius)
                sum += exp (- SQR (r) / (2 * SQR (std_dev)));
            }

          cmatrix[i] = sum / 50;
        }

      /* mirror the thing to the bottom half */
      for (i = 0; i <= matrix_length / 2; i++)
        cmatrix[i] = cmatrix[matrix_length - 1 - i];

      /* find center val -- calculate an odd number of quanta to make it symmetric,
       * even if the center point is weighted slightly higher than others. */
      sum = 0;
      for (j = 0; j <= 50; j++)
        sum += exp (- SQR (- 0.5 + 0.02 * j) / (2 * SQR (std_dev)));

      cmatrix[matrix_length / 2] = sum / 51;

      /* normalize the distribution by scaling the total sum to one */
      sum = 0;
      for (i = 0; i < matrix_length; i++)
        sum += cmatrix[i];

      for (i = 0; i < matrix_length; i++)
        cmatrix[i] = cmatrix[i] / sum;

      return matrix_length;
    }

    /* ----------------------- gen_lookup_table ----------------------- */
    /* generates a lookup table for every possible product of 0-255 and
       each value in the convolution matrix.  The returned array is
       indexed first by matrix position, then by input multiplicand (?)
       value.
    */
    static double* makeLookupTable (const double *cmatrix, int cmatrix_length)
    {
      double       *lookup_table   = new double [cmatrix_length * 256];
      double       *lookup_table_p = lookup_table;
      const double *cmatrix_p      = cmatrix;

      for (int i = 0; i < cmatrix_length; i++)
        {
          for (int j = 0; j < 256; j++)
            *(lookup_table_p++) = *cmatrix_p * (double) j;

          cmatrix_p++;
        }

      return lookup_table;
    }

/* Perform an unsharp mask on the region, given a source region, dest.
 * region, width and height of the regions, and corner coordinates of
 * a subregion to act upon.  Everything outside the subregion is unaffected.
 */
static void unsharpRegion (const unsigned char* srcPR, unsigned char* destPR, int bytes,
                            double radius, double amount, int threshold, int x1, int x2, int y1, int y2)
{
    int     width   = x2 - x1 + 1;
    int     height  = y2 - y1 + 1;
    double *cmatrix = 0;
    int     cmatrix_length;
    double *ctable  = 0;
    int     row, col;
 
    // generate convolution matrix and make sure it's smaller than each dimension
    cmatrix_length = makeConvolveMatrix (radius, &cmatrix);

    // generate lookup table
    ctable = makeLookupTable (cmatrix, cmatrix_length);

    //allocate buffers
    unsigned char  *src  = new unsigned char [std::max (width, height)];
    unsigned char  *dest = new unsigned char [std::max (width, height)];

    // blur the rows
    for (row = 0; row < height; row++)
    {
      memcpy(src, srcPR + x1 + (y1 + row) * width, width - x1);
      blurLine (ctable, cmatrix, cmatrix_length, src, dest, width, bytes);
      memcpy(destPR + x1 + (y1 + row) * width, dest , width - x1);
    }

    // blur the cols
    for (col = 0; col < width; col++)
    {
        for (row = 0; row < height; row++)
            src[row] = destPR[x1 + col + (y1 + row) * width];
        blurLine (ctable, cmatrix, cmatrix_length, src, dest, height, bytes);
        for (row = 0; row < height; row++)
            destPR[x1 + col + (y1 + row) * width] = dest[row];
    }

    //merge the source and destination (which currently contains the blurred version) images
    for (row = 0; row < height; row++)
    {
      const unsigned char *s = src;
      unsigned char       *d = dest;
      int          u, v;

      // get source row
      memcpy(src, srcPR + x1 + (y1 + row) * width, width - x1);
      // get dest row
      memcpy(dest, destPR + x1 + (y1 + row) * width, width - x1);

      // combine both
        for (u = 0; u < width; u++)
        {
            for (v = 0; v < bytes; v++)
            {
                int value;
                int diff = *s - *d;

                /* do tresholding */
                if (abs (2 * diff) < threshold)
                diff = 0;

                value = int(*s++ + amount * diff);
                *d++ = value > 0 ? (value <= 255 ? value : 255) : 0;
            }
        }
        memcpy(destPR + x1 + (y1 + row) * width, dest , width - x1);
    }

  delete [] dest;
  delete [] src;
  delete [] ctable;
  delete [] cmatrix;
}
//==============================================================================

    void unsharpMaskImage(Image* img, const double radius, const double sigma, const double amount,const int threshold)
    {
        int x1=0, y1=0, x2=img->getWidth()-1, y2=img->getHeight()-1;
        size_t size = img->getWidth()*img->getHeight();
        unsigned char* dest = new unsigned char[size];
        unsharpRegion ((const unsigned char*)img->getPixels(), dest, 1, radius, amount, threshold, x1, x2, y1, y2);
        memcpy(img->getPixels(), dest, size);
        delete [] dest;
    }


//==============================================================================
/*
<?php 

/ * 

New:  
- In version 2.1 (February 26 2007) Tom Bishop has done some important speed enhancements. 
- From version 2 (July 17 2006) the script uses the imageconvolution function in PHP  
version >= 5.1, which improves the performance considerably. 


Unsharp masking is a traditional darkroom technique that has proven very suitable for  
digital imaging. The principle of unsharp masking is to create a blurred copy of the image 
and compare it to the underlying original. The difference in colour values 
between the two images is greatest for the pixels near sharp edges. When this  
difference is subtracted from the original image, the edges will be 
accentuated.  

The Amount parameter simply says how much of the effect you want. 100 is 'normal'. 
Radius is the radius of the blurring circle of the mask. 'Threshold' is the least 
difference in colour values that is allowed between the original and the mask. In practice 
this means that low-contrast areas of the picture are left unrendered whereas edges 
are treated normally. This is good for pictures of e.g. skin or blue skies. 

Any suggenstions for improvement of the algorithm, expecially regarding the speed 
and the roundoff errors in the Gaussian blur process, are welcome. 

* / 

function UnsharpMask($img, $amount, $radius, $threshold)    {  

////////////////////////////////////////////////////////////////////////////////////////////////   
////   
////                  Unsharp Mask for PHP - version 2.1.1   
////   
////    Unsharp mask algorithm by Torstein H?nsi 2003-07.   
////             thoensi_at_netcom_dot_no.   
////               Please leave this notice.   
////   
///////////////////////////////////////////////////////////////////////////////////////////////   



    // $img is an image that is already created within php using  
    // imgcreatetruecolor. No url! $img must be a truecolor image.  

    // Attempt to calibrate the parameters to Photoshop:  
    if ($amount > 500)    $amount = 500;  
    $amount = $amount * 0.016;  
    if ($radius > 50)    $radius = 50;  
    $radius = $radius * 2;  
    if ($threshold > 255)    $threshold = 255;  
      
    $radius = abs(round($radius));     // Only integers make sense.  
    if ($radius == 0) {  
        return $img; imagedestroy($img); break;        }  
    $w = imagesx($img); $h = imagesy($img);  
    $imgCanvas = imagecreatetruecolor($w, $h);  
    $imgBlur = imagecreatetruecolor($w, $h);  
      

    // Gaussian blur matrix:  
    //                          
    //    1    2    1          
    //    2    4    2          
    //    1    2    1          
    //                          
    //////////////////////////////////////////////////  
          

    if (function_exists('imageconvolution')) { // PHP >= 5.1   
            $matrix = array(   
            array( 1, 2, 1 ),   
            array( 2, 4, 2 ),   
            array( 1, 2, 1 )   
        );   
        imagecopy ($imgBlur, $img, 0, 0, 0, 0, $w, $h);  
        imageconvolution($imgBlur, $matrix, 16, 0);   
    }   
    else {   

    // Move copies of the image around one pixel at the time and merge them with weight  
    // according to the matrix. The same matrix is simply repeated for higher radii.  
        for ($i = 0; $i < $radius; $i++)    {  
            imagecopy ($imgBlur, $img, 0, 0, 1, 0, $w - 1, $h); // left  
            imagecopymerge ($imgBlur, $img, 1, 0, 0, 0, $w, $h, 50); // right  
            imagecopymerge ($imgBlur, $img, 0, 0, 0, 0, $w, $h, 50); // center  
            imagecopy ($imgCanvas, $imgBlur, 0, 0, 0, 0, $w, $h);  

            imagecopymerge ($imgBlur, $imgCanvas, 0, 0, 0, 1, $w, $h - 1, 33.33333 ); // up  
            imagecopymerge ($imgBlur, $imgCanvas, 0, 1, 0, 0, $w, $h, 25); // down  
        }  
    }  

    if($threshold>0){  
        // Calculate the difference between the blurred pixels and the original  
        // and set the pixels  
        for ($x = 0; $x < $w-1; $x++)    { // each row 
            for ($y = 0; $y < $h; $y++)    { // each pixel  
                      
                $rgbOrig = ImageColorAt($img, $x, $y);  
                $rOrig = (($rgbOrig >> 16) & 0xFF);  
                $gOrig = (($rgbOrig >> 8) & 0xFF);  
                $bOrig = ($rgbOrig & 0xFF);  
                  
                $rgbBlur = ImageColorAt($imgBlur, $x, $y);  
                  
                $rBlur = (($rgbBlur >> 16) & 0xFF);  
                $gBlur = (($rgbBlur >> 8) & 0xFF);  
                $bBlur = ($rgbBlur & 0xFF);  
                  
                // When the masked pixels differ less from the original  
                // than the threshold specifies, they are set to their original value.  
                $rNew = (abs($rOrig - $rBlur) >= $threshold)   
                    ? max(0, min(255, ($amount * ($rOrig - $rBlur)) + $rOrig))   
                    : $rOrig;  
                $gNew = (abs($gOrig - $gBlur) >= $threshold)   
                    ? max(0, min(255, ($amount * ($gOrig - $gBlur)) + $gOrig))   
                    : $gOrig;  
                $bNew = (abs($bOrig - $bBlur) >= $threshold)   
                    ? max(0, min(255, ($amount * ($bOrig - $bBlur)) + $bOrig))   
                    : $bOrig;  
                  
                  
                              
                if (($rOrig != $rNew) || ($gOrig != $gNew) || ($bOrig != $bNew)) {  
                        $pixCol = ImageColorAllocate($img, $rNew, $gNew, $bNew);  
                        ImageSetPixel($img, $x, $y, $pixCol);  
                    }  
            }  
        }  
    }  
    else{  
        for ($x = 0; $x < $w; $x++)    { // each row  
            for ($y = 0; $y < $h; $y++)    { // each pixel  
                $rgbOrig = ImageColorAt($img, $x, $y);  
                $rOrig = (($rgbOrig >> 16) & 0xFF);  
                $gOrig = (($rgbOrig >> 8) & 0xFF);  
                $bOrig = ($rgbOrig & 0xFF);  
                  
                $rgbBlur = ImageColorAt($imgBlur, $x, $y);  
                  
                $rBlur = (($rgbBlur >> 16) & 0xFF);  
                $gBlur = (($rgbBlur >> 8) & 0xFF);  
                $bBlur = ($rgbBlur & 0xFF);  
                  
                $rNew = ($amount * ($rOrig - $rBlur)) + $rOrig;  
                    if($rNew>255){$rNew=255;}  
                    elseif($rNew<0){$rNew=0;}  
                $gNew = ($amount * ($gOrig - $gBlur)) + $gOrig;  
                    if($gNew>255){$gNew=255;}  
                    elseif($gNew<0){$gNew=0;}  
                $bNew = ($amount * ($bOrig - $bBlur)) + $bOrig;  
                    if($bNew>255){$bNew=255;}  
                    elseif($bNew<0){$bNew=0;}  
                $rgbNew = ($rNew << 16) + ($gNew <<8) + $bNew;  
                    ImageSetPixel($img, $x, $y, $rgbNew);  
            }  
        }  
    }  
    imagedestroy($imgCanvas);  
    imagedestroy($imgBlur);  
      
    return $img;  

} 
?> 

//==============================================================================
    MagickExport Image *UnsharpMaskImageChannel(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double amount,const double threshold,ExceptionInfo *exception)
{
#define SharpenImageTag  "Sharpen/Image"

  CacheView
    *image_view,
    *unsharp_view;

  Image
    *unsharp_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    bias;

  MagickRealType
    quantum_threshold;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  unsharp_image=BlurImageChannel(image,channel,radius,sigma,exception);
  if (unsharp_image == (Image *) NULL)
    return((Image *) NULL);
  quantum_threshold=(MagickRealType) QuantumRange*threshold;
  
  //  Unsharp-mask image.
  
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&bias);
  image_view=AcquireCacheView(image);
  unsharp_view=AcquireCacheView(unsharp_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict unsharp_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(unsharp_view,0,y,unsharp_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    unsharp_indexes=GetCacheViewAuthenticIndexQueue(unsharp_view);
    pixel=bias;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          pixel.red=p->red-(MagickRealType) q->red;
          if (fabs(2.0*pixel.red) < quantum_threshold)
            pixel.red=(MagickRealType) GetRedPixelComponent(p);
          else
            pixel.red=(MagickRealType) p->red+(pixel.red*amount);
          SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
        }
      if ((channel & GreenChannel) != 0)
        {
          pixel.green=p->green-(MagickRealType) q->green;
          if (fabs(2.0*pixel.green) < quantum_threshold)
            pixel.green=(MagickRealType) GetGreenPixelComponent(p);
          else
            pixel.green=(MagickRealType) p->green+(pixel.green*amount);
          SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
        }
      if ((channel & BlueChannel) != 0)
        {
          pixel.blue=p->blue-(MagickRealType) q->blue;
          if (fabs(2.0*pixel.blue) < quantum_threshold)
            pixel.blue=(MagickRealType) GetBluePixelComponent(p);
          else
            pixel.blue=(MagickRealType) p->blue+(pixel.blue*amount);
          SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
        }
      if ((channel & OpacityChannel) != 0)
        {
          pixel.opacity=p->opacity-(MagickRealType) q->opacity;
          if (fabs(2.0*pixel.opacity) < quantum_threshold)
            pixel.opacity=(MagickRealType) GetOpacityPixelComponent(p);
          else
            pixel.opacity=p->opacity+(pixel.opacity*amount);
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          pixel.index=unsharp_indexes[x]-(MagickRealType) indexes[x];
          if (fabs(2.0*pixel.index) < quantum_threshold)
            pixel.index=(MagickRealType) unsharp_indexes[x];
          else
            pixel.index=(MagickRealType) unsharp_indexes[x]+(pixel.index*
              amount);
          unsharp_indexes[x]=ClampToQuantum(pixel.index);
        }
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(unsharp_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_UnsharpMaskImageChannel)
#endif
        proceed=SetImageProgress(image,SharpenImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  unsharp_image->type=image->type;
  unsharp_view=DestroyCacheView(unsharp_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    unsharp_image=DestroyImage(unsharp_image);
  return(unsharp_image);
}
*/

}
