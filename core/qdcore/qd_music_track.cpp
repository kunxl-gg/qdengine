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

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"
#include "qdengine/core/qdcore/qd_music_track.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdMusicTrack::qdMusicTrack() : volume_(256) {
	toggle_cycle(true);
}

qdMusicTrack::qdMusicTrack(const qdMusicTrack &trk) : qdConditionalObject(trk),
	file_name_(trk.file_name_),
	volume_(trk.volume_) {
}

qdMusicTrack::~qdMusicTrack() {
}

qdMusicTrack &qdMusicTrack::operator = (const qdMusicTrack &trk) {
	if (this == &trk) return *this;

	*static_cast<qdConditionalObject *>(this) = trk;

	file_name_ = trk.file_name_;
	volume_ = trk.volume_;

	return *this;
}

bool qdMusicTrack::load_script(const xml::tag *p) {
	load_conditions_script(p);

	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			clear_flags();
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_MUSIC_TRACK_CYCLED:
			if (xml::tag_buffer(*it).get_int())
				toggle_cycle(true);
			else
				toggle_cycle(false);
			break;
		case QDSCR_FILE:
			set_file_name(Common::Path(it->data(), '\\').toString().c_str());
			break;
		case QDSCR_SOUND_VOLUME:
			set_volume(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdMusicTrack::save_script(class XStream &fh, int indent) const {
	for (int i = 0; i < indent; i ++) fh < "\t";
	fh < "<music_track";

	if (name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	fh < " flags=\"" <= flags() < "\"";

	if (!file_name_.empty())
		fh < " file=\"" < qdscr_XML_string(file_name_.c_str()) < "\"";

	if (is_cycled())
		fh < " cycled=\"1\"";

	if (volume_ != 256)
		fh < " volume=\"" <= volume_ < "\"";

	if (has_conditions()) {
		fh < ">\r\n";
		save_conditions_script(fh, indent);
		for (int i = 0; i < indent; i ++) fh < "\t";
		fh < "</music_track>";
	} else
		fh < "/>\r\n";

	return true;
}

qdConditionalObject::trigger_start_mode qdMusicTrack::trigger_start() {
	if (qdGameDispatcher * dp = qd_get_game_dispatcher()) {
		dp -> play_music_track(this);
		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}
} // namespace QDEngine
