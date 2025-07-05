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
//*************************************

#include <assert.h>
 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_defs.h"
#include "imagex.h"
#include "imageformat_png.h"

//#include "pnginfo.h"		// for PNG 1.5
#include "file_png.h"

/* -- FOR PNG 1.5
typedef jmp_buf* (*png_set_longjmp_fnPtr)(png_structp png_ptr, png_longjmp_ptr longjmp_fn, size_t jmp_buf_size);
png_set_longjmp_fnPtr mypng_set_longjmp_fnPtr = 0;

extern "C" {
   jmp_buf* png_set_longjmp_fn(png_structp png_ptr, png_longjmp_ptr longjmp_fn, size_t jmp_buf_size)
   { 
	   if (mypng_set_longjmp_fnPtr) { return (*mypng_set_longjmp_fnPtr)(png_ptr, longjmp_fn, jmp_buf_size); }
       return 0;
   }
}*/

bool CImageFormatPng::Load (char *filename, ImageX* img )
{
	StartFormat ( filename, img, ImageOp::Loading );

	bool bGrey = false;

	//-------------------------- Using PNG helpers
	std::vector<unsigned char> out;
	unsigned int nx, ny;
	unsigned error = lodepng::decode( out, nx, ny, filename, (bGrey ? LCT_GREY : LCT_RGBA), (bGrey ? 16 : 8));

	if (error) {
		//nvprintf("png read error: %s\n", lodepng_error_text(error));
		return false;		
	}	

	// Determine output parameters	
	char color_type = 6;
	ImageOp::Format eNewFormat;
	switch (color_type) {
	case 0:	eNewFormat = ImageOp::BW8;		break;		// each pixel is 1x grayscale 
	case 2: eNewFormat = ImageOp::RGB8;		break;		// each pixel is RGB triple
	case 6: eNewFormat = ImageOp::RGBA8;	break;		// each pixel is RGBA 
	default:
		m_eStatus = ImageOp::DepthNotSupported; return false;
		break;
	}
	// Allocate new image space or use existing one
	m_pImg->Resize (nx, ny, eNewFormat);

	int stride = m_pImg->GetBytesPerRow();
	XBYTE *dest = m_pImg->GetData();
	for (uint y = 0; y < ny; y++)
		memcpy(dest + y*stride, &out[y*stride], stride);

	return true;
}


bool CImageFormatPng::Save (char *filename, ImageX* img )
{
	StartFormat ( filename, img, ImageOp::Saving );

	int ch = 3;

	switch ( m_pImg->GetFormat() ) {
	case ImageOp::BW8: 	ch = 1; break;
	case ImageOp::BW16: ch = 1; break;
	case ImageOp::BW32: ch = 1; break;		
	case ImageOp::RGB8: ch = 3; break;
	case ImageOp::RGBA8: ch = 4; break;
	};

	save_png ( filename, m_pImg->GetData(), m_pImg->GetWidth(), m_pImg->GetHeight(), ch );

	return true;
	
	
	/* 
	//----------------------------- Using PNG library
	// Open file for output
	FILE* png_file;
	fopen_s ( &png_file, filename, "wb" );
	if ( png_file == NULL) {		
		m_eStatus = ImageOp::InvalidFile;		
		return false;
	}

	// initialize stuff 
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr) {
		m_eStatus = ImageOp::InvalidFile;		
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		m_eStatus = ImageOp::InvalidFile;		
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) { 
		m_eStatus = ImageOp::InvalidFile; 
		return false; 
	}				

	png_init_io( png_ptr, png_file );

	int width = m_pOrigImage->GetWidth();
	int height = m_pOrigImage->GetHeight();
	png_byte bit_depth = 8;
	png_byte color_type = PNG_COLOR_TYPE_RGB;
	png_set_IHDR( png_ptr, info_ptr, width, height,
		     bit_depth, color_type, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// write row data
	XBYTE *src = GetData ( m_pOrigImage );	
	int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	png_bytep row = (png_bytep) png_malloc( png_ptr, row_bytes );			// Allocate memory in PNG memory space
	for (int y=0; y < height; y++) {		
		memcpy ( row, src + (height-1-y)*row_bytes, row_bytes );		// Copy data Luna Image to PNG memory
		png_write_row ( png_ptr, row );
	}	
	png_free ( png_ptr, row );						// Free PNG memory	

	png_write_end( png_ptr, NULL );

	if ( info_ptr != NULL ) png_free_data ( png_ptr, info_ptr, PNG_FREE_ALL, -1 );
	if ( png_ptr != NULL ) png_destroy_write_struct ( &png_ptr, (png_infopp) NULL );

    fclose( png_file );
	*/
}

