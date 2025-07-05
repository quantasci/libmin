//--------------------------------------------------------------------------------
// Copyright 2007-2022 (c) Quanta Sciences, Rama Hoetzlein, ramakarl.com
//
// * Derivative works may append the above copyright notice but should not remove or modify earlier notices.
//
// MIT License:
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
// associated documentation files (the "Software"), to deal in the Software without restriction, including without 
// limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
#include "imageformat_tga.h"

unsigned char *CImageFormatTga::getRGBA( FILE *s, unsigned char* rgba, int size )
{
    // Read in RGBA data for a 32bit image.     
    unsigned char temp;
    int bread;
    int i;

    if( rgba == NULL )
        return 0;

    bread = (int) fread( rgba, sizeof (unsigned char), size, s ); 

    // TGA is stored in BGRA, make it RGBA  
    if( bread != size )
    {
        free( rgba );
        return 0;
    }

    for( i = 0; i < size; i += 4 )
    {
        temp = rgba[i];
        rgba[i] = rgba[i + 2];
        rgba[i + 2] = temp;
    }

    return rgba;
}

unsigned char *CImageFormatTga::getRGB( FILE *s, unsigned char* rgb, int size )
{
    // Read in RGB data for a 24bit image.     
    unsigned char temp;
    int bread;
    int i;

    if( rgb == NULL )
        return 0;

	//printf ( 'tga ', INFO, "%p %p %d\n", s, rgb, size );

    bread = (int) fread( rgb, sizeof (unsigned char), size, s );

    if(bread != size)
    {
        free( rgb );
        return 0;
    }

    // TGA is stored in BGR, make it RGB  
    for( i = 0; i < size; i += 3 )
    {
        temp = rgb[i];
        rgb[i] = rgb[i + 2];
        rgb[i + 2] = temp;
    }

    return rgb;
}

unsigned char *CImageFormatTga::getGray( FILE *s, unsigned char* grayData, int size )
{
    // Gets the grayscale image data.  Used as an alpha channel.    
    int bread;

    if( grayData == NULL )
        return 0;

    bread = (int)fread( grayData, sizeof (unsigned char), size, s );

    if( bread != size )
    {
        free( grayData );
        return 0;
    }
    return grayData;
}

void CImageFormatTga::writeRGBA( FILE *s, const unsigned char *externalImage, int size )
{
    // Read in RGBA data for a 32bit image. 
    unsigned char *rgba;
    int bread;
    int i;

    rgba = (unsigned char *) malloc( size * 4 );

    // switch RGBA to BGRA
    for( i = 0; i < size * 4; i += 4 )
    {
        rgba[i + 0] = externalImage[i + 2];
        rgba[i + 1] = externalImage[i + 1];
        rgba[i + 2] = externalImage[i + 0];
        rgba[i + 3] = externalImage[i + 3];
    }

    bread = (int) fwrite( rgba, sizeof (unsigned char), size * 4, s ); 
    free( rgba );
}

void CImageFormatTga::writeRGB( FILE *s, const unsigned char *externalImage, int size )
{
    // Read in RGBA data for a 32bit image. 
    unsigned char *rgb;
    int bread;
    int i;

    rgb = (unsigned char *)malloc( size * 3 );

    // switch RGB to BGR
    for( i = 0; i < size * 3; i += 3 )
    {
        rgb[i + 0] = externalImage[i + 2];
        rgb[i + 1] = externalImage[i + 1];
        rgb[i + 2] = externalImage[i + 0];
    }

    bread = (int)fwrite( rgb, sizeof (unsigned char), size * 3, s ); 
    free( rgb );
}

void CImageFormatTga::writeGrayAsRGB( FILE *s, const unsigned char *externalImage, int size )
{
    // Read in RGBA data for a 32bit image. 
    unsigned char *rgb;
    int bread;
    int i;

    rgb = (unsigned char *)malloc( size * 3 );

    // switch RGB to BGR
    int j = 0;
    for( i = 0; i < size * 3; i += 3, j++ )
    {
        rgb[i + 0] = externalImage[j];
        rgb[i + 1] = externalImage[j];
        rgb[i + 2] = externalImage[j];
    }

    bread = (int)fwrite( rgb, sizeof (unsigned char), size * 3, s ); 
    free( rgb );
}

void CImageFormatTga::writeGray( FILE *s, const unsigned char *externalImage, int size )
{
    // Gets the grayscale image data.  Used as an alpha channel.
    int bread;

    bread = (int)fwrite( externalImage, sizeof (unsigned char), size, s );
}



bool CImageFormatTga::Load (char *filename, ImageX* img )
{
	StartFormat ( filename, img, ImageOp::Loading );

	// Loads up a targa file. Supported types are 8, 24 and 32 
    // uncompressed images.
    unsigned char type[4];
    unsigned char info[7];
    FILE *tga = NULL;
    int size = 0;
    
    if( !(tga = fopen( filename, "rb" )) ) {
        m_eStatus = ImageOp::FileNotFound;
		return false;
	}

    fread( &type, sizeof (char), 3, tga );   // Read in colormap info and image type, byte 0 ignored
    fseek( tga, 12, SEEK_SET);			   // Seek past the header and useless info
    fread( &info, sizeof (char), 6, tga );

    if( type[1] != 0 || (type[2] != 2 && type[2] != 3) ) {
		m_eStatus = ImageOp::InvalidFile;
        return false;
	}

    m_xres = info[0] + info[1] * 256; 
    m_yres = info[2] + info[3] * 256;
    m_bpp  = info[4]; 

    // Make sure we are loading a supported type  
    if( m_bpp  != 32 && m_bpp != 24 && m_bpp != 8 ) {
		m_eStatus = ImageOp::InvalidFile;
		return false;
	}     
	ImageOp::Format eNewFormat;	

	size = m_xres * m_yres;
	
	switch ( m_bpp ) {     // *NOTE* BitsPerPixel is NOT bits per channel
	case 32:	eNewFormat = ImageOp::RGBA8;	size *= 4;	break;		// 8-bit, 4 channel
	case 24:	eNewFormat = ImageOp::RGB8; 	size *= 3;	break;	    // 8-bit, 3 channel
	case 8:		eNewFormat = ImageOp::RGB8;	            	break;      // 8-bit, 1 channel
	};

	// Allocate image
  m_pImg->Resize ( m_xres, m_yres, eNewFormat );
	unsigned char* buf = m_pImg->GetData ();
	if ( buf == 0x0 ) {
		printf ( "TGA Error: No new image data created.\n" );
	}

	// Read data		
	//printf ( "Load TGA: %d x %d x %d, %d\n", m_Xres, m_Yres, m_BitsPerPixel, size );
	switch ( m_bpp ) {     // *NOTE* BitsPerPixel is NOT bits per channel
	case 32:	getRGBA( tga, buf, size );	break;
	case 24:	getRGB( tga, buf, size );	break;
	case 8:		getGray( tga, buf, size );	break;
	};

  // Flip Y
  int stride = size / m_yres;
  unsigned char* tmp = (unsigned char*) malloc(stride);
  for (int y = 0; y < m_yres / 2; y++) {
      memcpy(tmp, buf + (y * stride), stride);
      memcpy(buf + (y * stride), buf + ((m_yres - y - 1) * stride), stride);
      memcpy(buf + ((m_yres - y - 1) * stride), tmp, stride);
  }

  fclose( tga );

  return true;
}	 


/* 
TGA::TGAError ImageFormatTga::saveFromExternalData( const char *name, int w, int h, TGA::TGAFormat fmt, const unsigned char *externalImage )
{
    static unsigned char type[] = {0,0,2};
    static unsigned char dummy[] = {0,0,0,0,0,0,0,0,0};
    static unsigned char info[] = {0,0,0,0,0,0};
    FILE *s = NULL;
    int size = 0;
    
    if( !(s = fopen( name, "wb" )) )
        return TGA_FILE_NOT_FOUND;

    fwrite( type, sizeof (char), 3, s );   // Read in colormap info and image type, byte 0 ignored
    fwrite( dummy, sizeof (char), 9, s );   // Read in colormap info and image type, byte 0 ignored

    info[0] = w & 0xFF;
    info[1] = (w>>8) & 0xFF;
    info[2] = h & 0xFF;
    info[3] = (h>>8) & 0xFF;
    switch(fmt)
    {
    case ALPHA:
        info[4] = 8;
        break;
    case RGB:
        info[4] = 24;
        break;
    case RGBA:
        info[4] = 32;
        break;
    }
    fwrite( info, sizeof (char), 6, s );

    size = w*h;
    switch(fmt)
    {
    case ALPHA:
        writeGray(s, externalImage, size);
        break;
    case RGB:
        //writeGrayAsRGB(s, externalImage, size);
        break;
    case RGBA:
        writeRGBA(s, externalImage, size);
        break;
    }

    fclose( s );

    return TGA_NO_ERROR;
}*/


