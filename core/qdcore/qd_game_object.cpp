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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/stream.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/qdcore/qd_game_object.h"
#include "qdengine/core/qdcore/qd_camera.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGameObject::qdGameObject() : r_(0, 0, 0),
	parallax_offset_(0.0f, 0.0f),
	screen_r_(0, 0),
	screen_depth_(0.0f) {
}

qdGameObject::qdGameObject(const qdGameObject &obj) : qdNamedObject(obj),
	r_(obj.r_),
	parallax_offset_(obj.parallax_offset_),
	screen_r_(obj.screen_r_),
	screen_depth_(obj.screen_depth_) {
}

qdGameObject::~qdGameObject() {
}

qdGameObject &qdGameObject::operator = (const qdGameObject &obj) {
	if (this == &obj) return *this;

	*static_cast<qdNamedObject *>(this) = obj;

	r_ = obj.r_;
	parallax_offset_ = obj.parallax_offset_;

	screen_r_ = obj.screen_r_;
	screen_depth_ = obj.screen_depth_;

	return *this;
}

bool qdGameObject::load_script_body(const xml::tag *p) {
	int fl;
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_POS2D:
			xml::tag_buffer(*it) > r_.x > r_.y;
			r_.z = 0.0f;
			break;
		case QDSCR_POS3D:
			xml::tag_buffer(*it) > r_.x > r_.y > r_.z;
			break;
		case QDSCR_FLAG:
			xml::tag_buffer(*it) > fl;
			set_flag(fl);
			break;
		case QDSCR_PARALLAX_OFFSET:
			xml::tag_buffer(*it) > parallax_offset_.x > parallax_offset_.y;
			break;
		}
	}

	return true;
}

bool qdGameObject::save_script_body(XStream &fh, int indent) const {
	for (int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<pos_3d>" <= r_.x < " " <= r_.y < " " <= r_.z < "</pos_3d>\r\n";

	if (parallax_offset_.x || parallax_offset_.y) {
		for (int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<parallax_offset>" <= parallax_offset_.x < " " <= parallax_offset_.y < "</parallax_offset>\r\n";
	}

	if (flags()) {
		for (int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<flag>" <= flags() < "</flag>\r\n";
	}

	return true;
}
#ifdef _QUEST_EDITOR
const Vect2i &qdGameObject::screen_pos() {
	update_screen_pos();
	return screen_r_;
}
#endif

void qdGameObject::quant(float dt) {
}

bool qdGameObject::update_screen_pos() {
	if (!check_flag(QD_OBJ_SCREEN_COORDS_FLAG)) {
		if (const qdCamera * cp = qdCamera::current_camera()) {
#ifndef _QUEST_EDITOR
			Vect3f v = cp -> global2camera_coord(R());

			if (check_flag(QD_OBJ_FIXED_SCREEN_COORDS_FLAG)) {
				screen_r_ = cp -> camera_coord2rscr(v);
				screen_r_.x += cp -> get_scr_sx() / 2;
				screen_r_.y = cp -> get_scr_sy() / 2 - screen_r_.y;

				screen_depth_ = 0.0f;
			} else {
				screen_r_ = cp -> camera_coord2scr(v);

				Vect3f rr = R();
				rr.z = cp -> get_grid_center().z;
				screen_depth_ = cp -> global2camera_coord(rr).z;
			}

			if (parallax_offset_.x || parallax_offset_.y) {
				screen_r_.x += round(float(parallax_offset_.x) * cp -> scrolling_phase_x());
				screen_r_.y += round(float(parallax_offset_.y) * cp -> scrolling_phase_y());
			}
#else
			Vect3f v = cp -> global2camera_coord(R());
			screen_r_ = cp -> camera_coord2scr(v);

			v = R();
			v.z = cp -> get_grid_center().z;
			screen_depth_ = cp -> global2camera_coord(v).z;
#endif
		} else
			return false;
	} else {
		screen_r_ = Vect2i(R().xi(), R().yi());
		screen_depth_ = 0.0f;
	}

	return true;
}

bool qdGameObject::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	if (!qdNamedObject::load_data(fh, saveVersion)) return false;

	r_.x = fh.readSint32LE();
	r_.y = fh.readSint32LE();
	r_.z = fh.readSint32LE();
	return true;
}

bool qdGameObject::load_data(qdSaveStream &fh, int save_version) {
	if (!qdNamedObject::load_data(fh, save_version)) return false;

	fh > r_.x > r_.y > r_.z;

	return true;
}

bool qdGameObject::save_data(Common::SeekableWriteStream &fh) {
	if (!qdNamedObject::save_data(fh)) {
		return false;
	}

	fh.writeFloatLE(r_.x);
	fh.writeFloatLE(r_.y);
	fh.writeFloatLE(r_.z);
}

bool qdGameObject::save_data(qdSaveStream &fh) const {
	if (!qdNamedObject::save_data(fh)) return false;

	fh < r_.x < r_.y < r_.z;

	return true;
}

bool qdGameObject::init() {
	drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);

	drop_flag(QD_OBJ_STATE_CHANGE_FLAG | QD_OBJ_IS_IN_TRIGGER_FLAG | QD_OBJ_STATE_CHANGE_FLAG | QD_OBJ_IS_IN_INVENTORY_FLAG);

#ifndef _QUEST_EDITOR
	drop_flag(QD_OBJ_HIDDEN_FLAG);
#endif

	return true;
}
} // namespace QDEngine
