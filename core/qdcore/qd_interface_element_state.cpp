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
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"

#include "qdengine/core/qdcore/qd_sound.h"
#include "qdengine/core/qdcore/qd_animation.h"

#include "qdengine/core/qdcore/qd_interface_element.h"
#include "qdengine/core/qdcore/qd_interface_dispatcher.h"
#include "qdengine/core/qdcore/qd_interface_element_state.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceElementState::qdInterfaceElementState() : state_mode_(DEFAULT_MODE), prev_state_mode_(DEFAULT_MODE) {
}

qdInterfaceElementState::qdInterfaceElementState(const qdInterfaceElementState &st) : qdInterfaceObjectBase(st),
	events_(st.events_),
	state_mode_(st.state_mode_),
	prev_state_mode_(st.prev_state_mode_) {
	for (int i = 0; i < NUM_MODES; i ++)
		modes_[i] = st.modes_[i];

	register_resources();
}

qdInterfaceElementState::~qdInterfaceElementState() {
	unregister_resources();
}

qdInterfaceElementState &qdInterfaceElementState::operator = (const qdInterfaceElementState &st) {
	if (this == &st) return *this;

	unregister_resources();

	this -> qdInterfaceObjectBase::operator = (st);

	events_ = st.events_;

	for (int i = 0; i < NUM_MODES; i ++)
		modes_[i] = st.modes_[i];

	register_resources();

	state_mode_ = st.state_mode_;
	prev_state_mode_ = st.prev_state_mode_;

	return *this;
}

bool qdInterfaceElementState::save_script(XStream &fh, int indent) const {
	for (int i = 0; i < indent; i ++) fh < "\t";
	fh < "<interface_element_state";

	if (name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	fh < ">\r\n";

	for (int j = 0; j < events_.size(); j ++) {
		for (int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<event type=\"" <= int(events_[j].event()) < "\"";

		if (events_[j].has_data())
			fh < " event_data=\"" < qdscr_XML_string(events_[j].event_data()) < "\"";

		if (events_[j].is_before_animation())
			fh < " before_animation=\"1\"";

		if (events_[j].activation() != qdInterfaceEvent::EVENT_ACTIVATION_CLICK)
			fh < " activation_type=\"" <= (int)events_[j].activation() < "\"";

		fh < "/>\r\n";
	}

	for (int i = 0; i < NUM_MODES; i ++) {
		if (has_state_mode(state_mode_t(i)))
			modes_[i].save_script(fh, i, indent + 1);
	}

	for (int i = 0; i < indent; i ++) fh < "\t";
	fh < "</interface_element_state>\r\n";
	return true;
}

bool qdInterfaceElementState::load_script(const xml::tag *p) {
	int num_events = 0;
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_INTERFACE_EVENT:
			num_events++;
			break;
		}
	}

	if (num_events)
		events_.reserve(num_events);

	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_INTERFACE_ELEMENT_STATE_MODE:
			if (const xml::tag * tg = it -> search_subtag(QDSCR_TYPE))
				modes_[state_mode_t(xml::tag_buffer(*tg).get_int())].load_script(&*it);
			break;
		case QDSCR_INTERFACE_EVENT: {
			qdInterfaceEvent::event_t ev = qdInterfaceEvent::EVENT_NONE;
			qdInterfaceEvent::activation_t act = qdInterfaceEvent::EVENT_ACTIVATION_CLICK;
			const char *ev_data = "";
			bool anm_flag = false;

			if (const xml::tag * tg = it -> search_subtag(QDSCR_TYPE))
				ev = qdInterfaceEvent::event_t(xml::tag_buffer(*tg).get_int());
			if (const xml::tag * tg = it -> search_subtag(QDSCR_INTERFACE_EVENT_DATA))
				ev_data = tg -> data();
			if (const xml::tag * tg = it -> search_subtag(QDSCR_INTERFACE_EVENT_BEFORE_ANIMATION)) {
				if (xml::tag_buffer(*tg).get_int())
					anm_flag = true;
			}
			if (const xml::tag * tg = it -> search_subtag(QDSCR_INTERFACE_EVENT_ACTIVATION_TYPE))
				act = qdInterfaceEvent::activation_t(xml::tag_buffer(*tg).get_int());

			events_.push_back(qdInterfaceEvent(ev, ev_data, anm_flag, act));
		}
		break;
		}
	}

	register_resources();

	return true;
}

bool qdInterfaceElementState::quant(float dt) {
	debugC(9, kDebugQuant, "qdInterfaceElementState::quant(%d)", dt);
	if (qdInterfaceElement * ep = dynamic_cast<qdInterfaceElement * >(owner())) {
		if (prev_state_mode_ == MOUSE_HOVER_MODE && state_mode() == DEFAULT_MODE)
			handle_events(qdInterfaceEvent::EVENT_ACTIVATION_HOVER, false);

		prev_state_mode_ = state_mode();

		switch (ep -> state_status(this)) {
		case qdInterfaceElement::STATE_INACTIVE:
			ep -> set_state(this);
			break;
		case qdInterfaceElement::STATE_DONE:
			if (state_mode() == EVENT_MODE) {
				debugC(3, kDebugQuant, "qdInterfaceElementState::quant(%d) - EVENT_MODE", dt);
				handle_events(qdInterfaceEvent::EVENT_ACTIVATION_CLICK, false);
			}


			set_state_mode(DEFAULT_MODE);
			break;
		}

		return true;
	}

	return false;
}

void qdInterfaceElementState::set_sound_file(const char *str, state_mode_t snd_id) {
	if (has_sound(snd_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			p -> remove_resource(sound_file(snd_id), this);

		modes_[snd_id].set_sound(NULL);
	}

	modes_[snd_id].set_sound_file(str);
	if (has_sound(snd_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			modes_[snd_id].set_sound(dynamic_cast<const qdSound * >(p -> add_resource(sound_file(snd_id), this)));
	}
}

void qdInterfaceElementState::set_animation_file(const char *name, state_mode_t anm_id) {
	if (has_animation(anm_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			p -> remove_resource(animation_file(anm_id), this);

		modes_[anm_id].set_animation(NULL);
	}

	modes_[anm_id].set_animation_file(name);
	if (has_animation(anm_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			modes_[anm_id].set_animation(dynamic_cast<const qdAnimation * >(p -> add_resource(animation_file(anm_id), this)));
	}
}

bool qdInterfaceElementState::adjust_res_file(const char *res_file, const char *dir, const char *&new_path, bool can_overwrite) {
	std::string corr_dir = dir;
	app_io::adjust_dir_end_slash(corr_dir);

	bool all_ok = true;
	new_path = NULL; // По умолчанию изменений с файлами не было
	// Если файл ресурса - звук, то правим положение звукового файла
	if (0 == strcmp(res_file, sound_file())) {
		QD_ADJUST_TO_REL_FILE_MEMBER(corr_dir, sound_file, set_sound_file, can_overwrite, all_ok);
		if (all_ok) new_path = sound_file();
		return all_ok;
	}
	// Анимация
	if (0 == strcmp(res_file, animation_file())) {
		QD_ADJUST_TO_REL_FILE_MEMBER(corr_dir, animation_file, set_animation_file, can_overwrite, all_ok);
		if (all_ok) new_path = animation_file();
		return all_ok;
	}

	return true; // Нет ресурса с таким файлом - все ок.
}

bool qdInterfaceElementState::unregister_resources() {
	bool res = true;

	for (int i = 0; i < NUM_MODES; i ++) {
		if (has_animation(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner())) {
				if (!p -> remove_resource(animation_file(state_mode_t(i)), this))
					res = false;

				modes_[i].set_animation(NULL);
			}
		}

		if (has_sound(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner())) {
				if (!p -> remove_resource(sound_file(state_mode_t(i)), this))
					res = false;

				modes_[i].set_sound(NULL);
			}
		}
	}

	return res;
}

bool qdInterfaceElementState::register_resources() {
	bool res = true;

	for (int i = 0; i < NUM_MODES; i ++) {
		if (has_animation(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
				modes_[i].set_animation(dynamic_cast<const qdAnimation * >(p -> add_resource(animation_file(state_mode_t(i)), this)));

			if (!modes_[i].animation()) res = false;
		}

		if (has_sound(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
				modes_[i].set_sound(dynamic_cast<const qdSound * >(p -> add_resource(sound_file(state_mode_t(i)), this)));

			if (!modes_[i].sound()) res = false;
		}
	}

	return res;
}

bool qdInterfaceElementState::has_state_mode(state_mode_t mode) const {
	switch (mode) {
	case DEFAULT_MODE:
		return true;
	case MOUSE_HOVER_MODE:
		if (has_animation(MOUSE_HOVER_MODE) || has_sound(MOUSE_HOVER_MODE) || has_contour(MOUSE_HOVER_MODE)) return true;
		return false;
	case EVENT_MODE:
		if (events_.size()) return true;
		if (has_animation(EVENT_MODE) || has_sound(EVENT_MODE) || has_contour(EVENT_MODE)) return true;
		return false;
	}

	return false;
}

bool qdInterfaceElementState::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	switch (ev) {
	case mouseDispatcher::EV_MOUSE_MOVE:
		if (state_mode() != EVENT_MODE) {
			if (has_state_mode(MOUSE_HOVER_MODE)) {
				set_state_mode(MOUSE_HOVER_MODE);
				if (prev_state_mode_ != MOUSE_HOVER_MODE)
					handle_events(qdInterfaceEvent::EVENT_ACTIVATION_HOVER, true);
				return true;
			}
		}
		break;
	case mouseDispatcher::EV_LEFT_DOWN:
	case mouseDispatcher::EV_RIGHT_DOWN:
		if (has_state_mode(EVENT_MODE)) {
			set_state_mode(EVENT_MODE);
			handle_events(qdInterfaceEvent::EVENT_ACTIVATION_CLICK, true);
			return true;
		}
		break;
	}

	return false;
}

bool qdInterfaceElementState::keyboard_handler(int vkey) {
	return false;
}

bool qdInterfaceElementState::handle_events(qdInterfaceEvent::activation_t activation_type, bool before_animation) {
	if (qdInterfaceDispatcher * dp = qdInterfaceDispatcher::get_dispatcher()) {
		for (int i = 0; i < events_.size(); i ++) {
			if (events_[i].activation() == activation_type && events_[i].is_before_animation() == before_animation) {
				dp -> handle_event(events_[i].event(), events_[i].event_data(), owner());
			}
		}

		return true;
	}

	return false;
}

bool qdInterfaceElementState::get_contour(state_mode_t mode, qdContour &cnt) const {
	modes_[mode].get_contour(cnt);
	return true;
}

bool qdInterfaceElementState::set_contour(state_mode_t mode, const qdContour &cnt) {
	modes_[mode].set_contour(cnt);
	return true;
}

bool qdInterfaceElementState::need_active_game() const {
	for (int i = 0; i < events_.size(); i ++) {
		if (events_[i].event() == qdInterfaceEvent::EVENT_SAVE_GAME)
			return true;
		if (events_[i].event() == qdInterfaceEvent::EVENT_CHANGE_PERSONAGE)
			return true;
		if (events_[i].event() == qdInterfaceEvent::EVENT_RESUME_GAME)
			return true;
		if (events_[i].event() == qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE)
			return true;
		if (events_[i].event() == qdInterfaceEvent::EVENT_SET_SAVE_MODE)
			return true;
	}

	return false;
}

const qdInterfaceEvent *qdInterfaceElementState::find_event(qdInterfaceEvent::event_t type) const {
	events_container_t::const_iterator it = std::find(events_.begin(), events_.end(), type);
	if (it != events_.end())
		return &*it;

	return NULL;
}

bool qdInterfaceElementState::has_event(qdInterfaceEvent::event_t type, const char *ev_data) const {
	for (events_container_t::const_iterator it = events_.begin(); it != events_.end(); ++it) {
		if (it -> event() == type) {
			if ((!ev_data && !it -> event_data()) || (it -> event_data() && !strcmp(ev_data, it -> event_data())))
				return true;
		}
	}

	return false;
}
} // namespace QDEngine
