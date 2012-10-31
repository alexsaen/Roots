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

#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#ifdef _DEBUG
#include "stdio.h"
#endif

#include <string>

struct zip_file;
struct zip;

class ResourceManager {
	zip* 	archive;
public:
	class	file {
		std::string		filename;
		zip_file		*zfile;
	public:
#ifdef _DEBUG
		FILE			*ffile;
						file(): zfile(0), ffile(0)													{}
						file(const char* fname, zip_file *f): filename(fname), zfile(f), ffile(0)	{}
						file(const char* fname, FILE *f): filename(fname), ffile(f), zfile(0)		{}
		bool			operator!()										{ return zfile==0 && ffile==0; }
#else
						file(): zfile(0)															{}
						file(const char* fname, zip_file *f): filename(fname), zfile(f)				{}
		bool			operator!()										{ return zfile==0; }
#endif
		unsigned int	read(void *buf, unsigned int size);
		bool			rewind();
		void 			close();
	};
					ResourceManager(const char *apkFilename);
					~ResourceManager();

	static ResourceManager*	init(const char *apkFilename);
	static void				destroy();
	static ResourceManager*	instance();

	file			open(const char *filename);

	unsigned char 	*readPNG(const char *filename, int &width, int &height);
	void 			*loadFile(const char *filename, int &fsize);
};

#endif /* RESOURCEMANAGER_H_ */
