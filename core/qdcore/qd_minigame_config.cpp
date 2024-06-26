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
#include "common/str.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/qdcore/qd_setup.h"
#include "qdengine/core/qdcore/qd_minigame_config.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdMinigameConfigParameter::qdMinigameConfigParameter() : data_type_(PRM_DATA_STRING) {
	data_count_ = 1;
	is_data_valid_ = true;
}

qdMinigameConfigParameter::~qdMinigameConfigParameter() {
}

bool qdMinigameConfigParameter::load_script(const xml::tag *p) {
	set_data_string(p -> data());

	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
//			case QDSCR_COMMENT:
//				set_comment(it -> data());
//				break;
//			case QDSCR_TYPE:
//				set_data_type(data_type_t(xml::tag_buffer(*it).get_int()));
//				break;
		case QDSCR_SIZE:
			set_data_count(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdMinigameConfigParameter::save_script(XStream &fh, int indent) const {
	for (int i = 0; i < indent; i ++) fh < "\t";
	fh < "<minigame_config_prm";

	fh < " name=\"" < qdscr_XML_string(name()) < "\"";
//	fh < " type=\"" <= int(data_type_) < "\"";

//	if(!comment_.empty())
//		fh < " comment=\"" < qdscr_XML_string(comment()) < "\"";

	if (data_count_ > 1)
		fh < " size=\"" <= data_count_ < "\"";

	fh < ">";

	fh < qdscr_XML_string(data_string());

	fh < "</minigame_config_prm>\r\n";

	return true;
}

bool qdMinigameConfigParameter::validate_data() {
	is_data_valid_ = true;
	return true;
}

bool qdMinigameConfigParameter::load_ini(const char *ini_file, const char *ini_section) {
	set_name(ini_section);
	char *str = getIniKey(ini_file, ini_section, "type");
	if (strlen(str)) {
		if (!scumm_stricmp(str, "string"))
			set_data_type(PRM_DATA_STRING);
		else if (!scumm_stricmp(str, "file"))
			set_data_type(PRM_DATA_FILE);
		else if (!scumm_stricmp(str, "object"))
			set_data_type(PRM_DATA_OBJECT);
	}

	str = getIniKey(ini_file, ini_section, "count");
	if (strlen(str))
		set_data_count(atoi(str));

	str = getIniKey(ini_file, ini_section, "value");
	if (strlen(str)) {
		set_data_string(str);
		int cnt = 0;

	}

	str = getIniKey(ini_file, ini_section, "comment");
	if (strlen(str))
		set_comment(str);

	return true;
}
} // namespace QDEngine
