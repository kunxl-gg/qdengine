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
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_named_object.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdNamedObject::qdNamedObject() : owner_(0),
	trigger_reference_count_(0),
#ifdef _QUEST_EDITOR
	ref_owner_(0),
#endif
	flags_(0) {
}

qdNamedObject::qdNamedObject(const qdNamedObject &obj) : qdNamedObjectBase(obj),
	owner_(obj.owner_),
#ifdef _QUEST_EDITOR
	ref_owner_(obj.ref_owner_),
#endif
	flags_(obj.flags_),
	trigger_reference_count_(0) {
}

qdNamedObject::~qdNamedObject() {
}

qdNamedObject &qdNamedObject::operator = (const qdNamedObject &obj) {
	if (this == &obj) return *this;

	*static_cast<qdNamedObjectBase *>(this) = obj;

	flags_ = obj.flags_;
	owner_ = obj.owner_;
#ifdef _QUEST_EDITOR
	ref_owner_ = obj.ref_owner_;
#endif
	return *this;
}

qdNamedObject *qdNamedObject::owner(qdNamedObjectType tp) const {
	qdNamedObject *p = owner();

	while (p) {
		if (p -> named_object_type() == tp) return p;
		p = p -> owner();
	}

	return NULL;
}

bool qdNamedObject::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	flags_ = fh.readSint32LE();
	return true;
}

bool qdNamedObject::load_data(qdSaveStream &fh, int save_version) {
	fh > flags_;

	return true;
}

bool qdNamedObject::save_data(Common::SeekableWriteStream &fh) const {
	fh.writeSint32BE(flags_);
	return true;
}

bool qdNamedObject::save_data(qdSaveStream &fh) const {
	fh < flags_;

	return true;
}

Common::String qdNamedObject::toString() {
	Common::String res;

	int owners_count = 0;

	const qdNamedObject *p = owner();
	while (p) {
		p = p -> owner();
		owners_count++;
	}

	for (int i = owners_count; i > 0; i--) {
		const qdNamedObject *p = this;
		for (int j = 0; j < i; j++)
			p = p->owner();

		if (p->name()) {
			res += Common::String((char *)transCyrillic(p->name()));
			res += "::";
		}
	}

	if (name())
		res += Common::String((char *)transCyrillic(name()));

	return res;
}

const char *objectType2str(int id) {
	const char *names[] = {
		"GENERIC",
		"SCALE_INFO",
		"TRIGGER_CHAIN",
		"SOUND",
		"ANIMATION",
		"ANIMATION_INFO",
		"COORDS_ANIMATION",
		"OBJ_STATE",
		"STATIC_OBJ",
		"ANIMATED_OBJ",
		"MOVING_OBJ",
		"MOUSE_OBJ",
		"SCENE",
		"LOCATION",
		"DISPATCHER",

		"ANIMATION_SET",
		"GRID_ZONE",
		"VIDEO",
		"INVENTORY",
		"MINIGAME",
		"MUSIC_TRACK",
		"GRID_ZONE_STATE",

		"SOUND_INFO",
		"ANIMATION_SET_INFO",

		"GAME_END",
		"COUNTER",

		"FONT_INFO",
	};

	if (id >= QD_NAMED_OBJECT_MAX_TYPE || id < 0)
		return "???";

	return names[id];
}

} // namespace QDEngine
