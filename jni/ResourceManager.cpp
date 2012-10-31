/*  
	Copyright (c) 2012, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/ 

#include "ResourceManager.h"
#include <algorithm>
#include <zip.h>
#include <png.h>

#ifdef _DEBUG

#include  <io.h>

static bool fileExists(const char *filename) {
   return _access( filename, 0 ) != -1;
}

static void *loadFile(const char *filename, int &fsize) {
	FILE *pFile = fopen(filename, "rb");
	if(!pFile) 
		return 0;
    fseek (pFile, 0, SEEK_END);
    fsize = ftell (pFile);
	rewind(pFile);
	char *buffer = new char[fsize+1];

	int size = fsize;
	char *buf = buffer;
	while(size > 0) {
		size_t rs = fread(buf, 1, std::min(size, 0x8000), pFile);
		if(rs <= 0) {
			fclose(pFile);
			delete buffer;
			return 0;
		}
		buf += rs;
		size -= rs;
	}
	buffer[fsize] = 0;
    fclose(pFile);
	return buffer;
}

#endif

static ResourceManager *rm = 0;

ResourceManager* ResourceManager::init(const char *apkFilename) {
	if(rm)
		delete rm;
	return rm = new ResourceManager(apkFilename);
}

void ResourceManager::destroy() {
	if(rm) {
		delete rm;
		rm = 0;
	}
}

ResourceManager* ResourceManager::instance() {
	return rm;
}

ResourceManager::ResourceManager(const char *apkFilename): archive(0) {
	archive = zip_open(apkFilename, 0, NULL);

	if(archive) {
		//Just for debug, print APK contents
		int numFiles = zip_get_num_files(archive);
		for (int i=0; i<numFiles; i++) {
			const char* name = zip_get_name(archive, i, 0);
			if (name == NULL) {
	//			LOGE("Error reading zip file name at index %i : %s", zip_strerror(APKArchive));
				return;
			}
	//		LOGI("File %i : %s\n", i, name);
		}
	}
}

ResourceManager::~ResourceManager() {
	if(archive)
		zip_close(archive);
}

ResourceManager::file ResourceManager::open(const char *filename) {
#ifdef _DEBUG
	if(fileExists(filename)) {
		FILE *fp = fopen(filename, "rb");
		return file(filename, fp);
	}
#endif
	zip_file *zfile = zip_fopen(archive, filename, 0);
	return file(filename, zfile);
}

unsigned int ResourceManager::file::read(void *buf, unsigned int size) {
#ifdef _DEBUG
	if(ffile) 
		return fread(buf, 1, size, ffile);
#endif
	if(!zfile)
		return 0;
	ssize_t rs = zip_fread(zfile, buf, size);
	return rs>0 ? rs : 0;
}

bool ResourceManager::file::rewind() {
#ifdef _DEBUG
	if(ffile) {
		::rewind(ffile);
		return true;
	}
#endif
	if(!zfile)
		return false;
	zip_fclose(zfile);
	zfile = zip_fopen(ResourceManager::instance()->archive, filename.c_str(), 0);
	return true;
}

void ResourceManager::file::close() {
#ifdef _DEBUG
	if(ffile) {
		fclose(ffile);
		return;
	}
#endif
	if(zfile)
		zip_fclose(zfile);
}

static ResourceManager::file pngfile;

static void png_zip_read(png_structp png_ptr, png_bytep data, png_size_t length) {
	pngfile.read(data, length);
}

void *ResourceManager::loadFile(const char *filename, int &fsize) {
#ifdef _DEBUG
	if(fileExists(filename)) 
		return ::loadFile(filename, fsize);
#endif

	int fidx = zip_name_locate(archive, filename, 0);
	if( fidx == -1)
		return 0;

	struct zip_stat st;
	if( zip_stat_index(archive, fidx, 0, &st) == -1 )
		return 0;

	zip_file *zfile = zip_fopen_index(archive, fidx, 0);
	if (!zfile)
		return 0;

	char *buffer = new char[ st.size+1 ];
	if(!buffer)
		return 0;

	int size = st.size;
	char *buf = buffer;
	while(size > 0) {
		ssize_t rs = zip_fread(zfile, buf, std::min(size, 0x8000) );
		if(rs <= 0) {
			zip_fclose(zfile);
			delete buffer;
			return 0;
		}
		buf += rs;
		size -= rs;
	}
	buffer[st.size] = 0;
	zip_fclose(zfile);

	fsize = st.size;
	return buffer;
}


unsigned char *ResourceManager::readPNG(const char *filename, int &width, int &height) {
	file file = open(filename);
	if(!file)
	    return 0;

	//header for testing if it is a png
	png_byte header[8];

	//read the header
	file.read(header, 8);

	//test if png
	int is_png = !png_sig_cmp(header, 0, 8);
	if(!is_png) {
	//    LOGE("Not a png file : %s", filename);
		file.close();
		return 0;
	}

	//create png struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) {
	//    LOGE("Unable to create png struct : %s", filename);
		file.close();
		return 0;
	}

	//create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
	//    LOGE("Unable to create png info : %s", filename);
		file.close();
		return 0;
	}

	//create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if(!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	//    LOGE("Unable to create png end info : %s", filename);
		file.close();
		return 0;
	}

	//png error stuff, not sure libpng man suggests this.
//	if(setjmp(png_jmpbuf(png_ptr))) {
//	//    LOGE("Error during setjmp : %s", filename);
//	    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
//		file.close();
//		return 0;
//	}

	//init png reading
	//png_init_io(png_ptr, fp);
	pngfile = file;
	png_set_read_fn(png_ptr, NULL, png_zip_read);

	//let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);

	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	//variables to pass to get info
	int bit_depth, color_type;
	png_uint_32 twidth, theight;

	// get info about png
	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	//update width and height based on png info
	width = twidth;
	height = theight;

	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// Row size in bytes.
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// Allocate the image_data as a big block, to be given to opengl
	png_byte *image_data = new png_byte[rowbytes * height];
	if (!image_data) {
	    //clean up memory and close stuff
	    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	//    LOGE("Unable to allocate image_data while loading %s ", filename);
		file.close();
		return 0;
	}

	  //row_pointers is for pointing to image_data for reading the png with libpng
	png_bytep *row_pointers = new png_bytep[height];
	if (!row_pointers) {
	    //clean up memory and close stuff
	    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	    delete[] image_data;
	//    LOGE("Unable to allocate row_pointer while loading %s ", filename);
		file.close();
		return 0;
	}
	  // set the individual row_pointers to point at the correct offsets of image_data
	for (int i = 0; i < height; ++i)
		row_pointers[height - 1 - i] = image_data + i * rowbytes;

	//read the png into image_data through row_pointers
	png_read_image(png_ptr, row_pointers);

	//clean up memory and close stuff
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	delete[] row_pointers;
	file.close();

	return image_data;
}


