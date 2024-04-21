/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#define _NO_ZIP_
#include "common/textconsole.h"

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/graphics/gr_dispatcher.h"
#include "qdengine/core/system/graphics/gr_font.h"
#include "qdengine/core/qdcore/qd_file_manager.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

grFont::grFont() : alpha_buffer_(NULL) {
	size_x_ = size_y_ = 0;
	alpha_buffer_sx_ = alpha_buffer_sy_ = 0;

	chars_.reserve(256);
}

grFont::~grFont() {
	delete alpha_buffer_;
}

bool grFont::load(const char *fname) {
	warning("STUB: grFont::load()");
#if 0
	XBuffer str(MAX_PATH);
	str < fname < ".tga";
	XStream fh;

	fh.open(str, XS_IN);
	if (load_alpha(fh)) {
		str.init();
		str < fname < ".idx";

		fh.open(str, XS_IN);
		if (load_index(fh))
			return true;
	}
#endif
	return false;
}

bool grFont::load_index(XStream &fh) {
	int buf_sz = fh.size();
	char *buf = new char[buf_sz];

	fh.read(buf, buf_sz);
	fh.close();

	XBuffer XBuf(buf, buf_sz);

	int num_ch, sx, sy;
	XBuf >= sx >= sy >= num_ch;

	grFontChar chr;
	for (int i = 0; i < num_ch; i ++) {
		int x, y, sx, sy;
		XBuf >= chr.code_ >= x >= y >= sx >= sy;
		chr.region_ = grScreenRegion(x, y, sx, sy);
		chars_.push_back(chr);

		if (sx > size_x_) size_x_ = sx;
		if (sy > size_y_) size_y_ = sy;
	}

	delete buf;

	return true;
}

bool grFont::load_index(XZipStream &fh) {
	int buf_sz = fh.size();
	char *buf = new char[buf_sz];

	fh.read(buf, buf_sz);
	fh.close();

	XBuffer XBuf(buf, buf_sz);

	int num_ch, sx, sy;
	XBuf >= sx >= sy >= num_ch;

	grFontChar chr;
	for (int i = 0; i < num_ch; i ++) {
		int x, y, sx, sy;
		XBuf >= chr.code_ >= x >= y >= sx >= sy;
		chr.region_ = grScreenRegion(x, y, sx, sy);
		chars_.push_back(chr);

		if (sx > size_x_) size_x_ = sx;
		if (sy > size_y_) size_y_ = sy;
	}

	delete buf;

	return true;
}

bool grFont::load_alpha(XStream &fh) {
	unsigned char header[18];
	fh.read(header, 18);

	if (header[0]) // Length of Image ID field
		fh.seek(header[0], XS_CUR);

	if (header[1]) // Color map type (0 is no color map)
		return false;

	if (header[2] != 2 && header[2] != 3) // TGA file type
		return false;

	int sx = alpha_buffer_sx_ = header[12] + (header[13] << 8);
	int sy = alpha_buffer_sy_ = header[14] + (header[15] << 8);

	int colors = header[16];
	int flags = header[17];

	int ssx = sx * colors / 8;

	alpha_buffer_ = new unsigned char[ssx * sy];

	if (!(flags & 0x20)) {
		int idx = (sy - 1) * ssx;
		for (int i = 0; i < sy; i ++) {
			fh.read(alpha_buffer_ + idx, ssx);
			idx -= ssx;
		}
	} else
		fh.read(alpha_buffer_, ssx * sy);

	fh.close();

	return true;
}

bool grFont::load_alpha(XZipStream &fh) {
	unsigned char header[18];
	fh.read(header, 18);

	if (header[0]) // Length of Image ID field
		return false;
//		fh.seek(header[0],XS_CUR);

	if (header[1]) // Color map type (0 is no color map)
		return false;

	if (header[2] != 2 && header[2] != 3) // TGA file type
		return false;

	int sx = alpha_buffer_sx_ = header[12] + (header[13] << 8);
	int sy = alpha_buffer_sy_ = header[14] + (header[15] << 8);

	int colors = header[16];
	int flags = header[17];

	int ssx = sx * colors / 8;

	alpha_buffer_ = new unsigned char[ssx * sy];

	if (!(flags & 0x20)) {
		int idx = (sy - 1) * ssx;
		for (int i = 0; i < sy; i ++) {
			fh.read(alpha_buffer_ + idx, ssx);
			idx -= ssx;
		}
	} else
		fh.read(alpha_buffer_, ssx * sy);

	fh.close();

	return true;
}
