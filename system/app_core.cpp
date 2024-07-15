/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/archive.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/system/app_core.h"
#include "qdengine/qdengine.h"
#include "qdengine/xutil/xutil.h"

namespace QDEngine {

void *app_hWnd = 0;

void *appGetHandle() {
	return app_hWnd;
}

void appSetHandle(void *hwnd) {
	app_hWnd = hwnd;
}

unsigned app_memory_usage() {
	unsigned size = 0;

	warning("STUB: app_memory_usage()");
#if 0
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	MEMORY_BASIC_INFORMATION Buffer;
	VirtualQuery(SystemInfo.lpMinimumApplicationAddress, &Buffer,  sizeof(Buffer));
	while (Buffer.BaseAddress < SystemInfo.lpMaximumApplicationAddress) {
		if (Buffer.State == MEM_COMMIT && !(Buffer.Type & MEM_MAPPED) && Buffer.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READ))
			size += Buffer.RegionSize;
		void *prev_address = Buffer.BaseAddress;
		VirtualQuery((char *)Buffer.BaseAddress + Buffer.RegionSize, &Buffer,  sizeof(Buffer));
		if (prev_address == Buffer.BaseAddress)
			break;
	}
#endif

	return size;
}

namespace app_io {

bool is_file_exist(const char *file_name) {
	/*  XStream fh(0);

	    if(fh.open(file_name,XS_IN)){
	        fh.close();
	        return true;
	    }

	    return false;*/
	bool hasFile = SearchMan.hasFile(Common::Path(file_name));
	warning("STUB: is_file_exist(): %s %d", file_name, hasFile);

	return hasFile;
}

const char *strip_path(const char *file_name) {
	static XBuffer name_buf(_MAX_PATH);
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];


	_splitpath(file_name, NULL, NULL, fname, ext);

	name_buf.init();
	name_buf.operator < (fname).operator < (ext);


	return name_buf.c_str();
}

bool is_directory_exist(const char *dir_name) {
	warning("STUB: is_directory_exist()");
#if 0
	 DWORD attr = GetFileAttributes(dir_name);
	if (attr != -1 && attr & FILE_ATTRIBUTE_DIRECTORY)
		return true;
#endif
	return false;
}

const char *strip_file_name(const char *path) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(path, drive, dir, NULL, NULL);

	name_buf.init();
	name_buf.operator < (drive).operator < (dir);

	return name_buf.c_str();

}

const char *get_ext(const char *file_name) {
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(file_name, NULL, NULL, fname, ext);

	name_buf.init();
	name_buf.operator < (ext);

	return name_buf.c_str();
}

const char *change_ext(const char *file_name, const char *new_ext) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	static XBuffer name_buf(_MAX_PATH);

	_splitpath(file_name, drive, dir, fname, NULL);

	name_buf.init();
	name_buf.operator < (drive).operator < (dir).operator < (fname).operator < (new_ext);

	return name_buf.c_str();
}

unsigned file_size(const char *file_name) {
	Common::File fh;
	if (fh.open(file_name))
		return fh.size();

	return 0;
}

std::string path_to_file_name(const std::string &str) {
	std::string conv_str = str;
	for (int i = 0; i < conv_str.length(); i++)
		if ('\\' == conv_str[i])
			conv_str[i] = '_';
	return conv_str;
}

bool relat_path(const char *path) {
	int len = strlen(path);
	if (len < 2) return true;

	if (('.' == path[0]) && ('.' == path[1])) return false;
	if (':' == path[1]) return false;

	return true;
}

void adjust_dir_end_slash(std::string &str) {
	if ((str.size() > 0) && ('\\' != str[str.size() - 1]))
		str = str + '\\';
}

}; /* namespace app_io */

} // namespace QDEngine
