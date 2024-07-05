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
#include "common/file.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_trigger_profiler.h"

#ifdef __QD_TRIGGER_PROFILER__

#include "qdengine/qdcore/qd_trigger_chain.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

const char *const qdTriggerProfiler::activation_comline_ = "trigger_profiler";

int qdTriggerProfiler::record_text_format_ = PROFILER_TEXT_TIME | PROFILER_TEXT_TRIGGER_NAME | PROFILER_TEXT_SCENE_NAME;

qdTriggerProfilerRecord::qdTriggerProfilerRecord() : time_(0),
	event_(ELEMENT_STATUS_UPDATE),
	trigger_id_(0),
	element_id_(0),
	link_id_(0),
	status_(0) {
}

qdTriggerProfilerRecord::qdTriggerProfilerRecord(unsigned int tm, event_t ev, const qdTriggerChain *trigger, int el_id, int lnk_id, int st) : time_(tm),
	event_(ev),
	trigger_id_(0),
	element_id_(el_id),
	link_id_(lnk_id),
	status_(st) {
	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
		for (qdTriggerChainList::const_iterator it = p->trigger_chain_list().begin(); it != p->trigger_chain_list().end(); ++it) {
			if (*it == trigger) break;
			trigger_id_++;
		}
	}
}

qdTriggerProfilerRecord::qdTriggerProfilerRecord(const qdTriggerProfilerRecord &rec) {
	*this = rec;
}

qdTriggerProfilerRecord::~qdTriggerProfilerRecord() {
}

qdTriggerProfilerRecord &qdTriggerProfilerRecord::operator = (const qdTriggerProfilerRecord &rec) {
	if (this == &rec) return *this;

	time_ = rec.time_;

	event_ = rec.event_;

	trigger_id_ = rec.trigger_id_;
	element_id_ = rec.element_id_;
	link_id_ = rec.link_id_;
	status_ = rec.status_;

	return *this;
}

bool qdTriggerProfilerRecord::save(Common::SeekableWriteStream &fh) const {
	warning("STUB: Test qdTriggerProfilerRecord::save(Common::SeekableWriteStream &fh)");

	fh.writeUint32LE(time_);
	fh.writeSint32LE((int)event_);
	fh.writeSint32LE(trigger_id_);
	fh.writeSint32LE(element_id_);
	fh.writeSint32LE(link_id_);
	fh.writeSint32LE(status_);

	return true;
}

bool qdTriggerProfilerRecord::save(XStream &fh) const {
	warning("STUB: qdTriggerProfilerRecord::save(XStream &fh)");
	return true;
}

bool qdTriggerProfilerRecord::load(Common::SeekableReadStream &fh) {
	warning("STUB: Test qdTriggerProfilerRecord::load(Common::SeekableReadStream &fh)");
	int ev;

	time_ = fh.readUint32LE();
	ev = fh.readSint32LE();
	trigger_id_ = fh.readSint32LE();
	element_id_ = fh.readSint32LE();
	link_id_ = fh.readSint32LE();
	status_ = fh.readSint32LE();

	event_ = event_t(ev);
	return true;
}

bool qdTriggerProfilerRecord::load(XStream &fh) {
	warning("STUB: qdTriggerProfilerRecord::load(XStream &fh)");
	return true;
}

qdTriggerProfiler::qdTriggerProfiler() : is_logging_enabled_(false), is_read_only_(true) {
	work_file_ = "trigger_profiler.dat";
}

qdTriggerProfiler::~qdTriggerProfiler() {
	if (is_logging_enabled_)
		save_to_work_file();
}

bool qdTriggerProfiler::save_to_work_file() const {
	warning("STUB: qdTriggerProfiler::save_to_work_file()");
	Common::DumpFile fh;

	fh.writeUint32LE(records_.size());
	for (auto &it : records_) {
		it.save(fh);
	}

	fh.close();
	return true;
}

bool qdTriggerProfiler::load_from_work_file() {
	warning("STUB: qdTriggerProfiler::load_from_work_file()");
	Common::File fh;

	records_.clear();

	if (fh.open(work_file_.c_str())) {
		int size;
		size = fh.readSint32LE();
		records_.resize(size);
		for (record_container_t::iterator it = records_.begin(); it != records_.end(); ++it)
			it->load(fh);

		fh.close();
		return true;
	}

	return false;
}

const char *qdTriggerProfiler::record_text(const qdTriggerProfilerRecord &rec, char separator) {
	static XBuffer text(1024, 1);
	text.init();

	if (record_text_format_ & PROFILER_TEXT_TIME) {
		// time = hsec * 10 + sec * 1000 + min * 60 * 1000 + hrs * 60 * 60 * 1000
		int hrs = rec.time() / (1000 * 60 * 60);
		if (hrs < 10) text < "0";
		text <= hrs < ":";

		int min = (rec.time() % (1000 * 60 * 60)) / (1000 * 60);
		if (min < 10) text < "0";
		text <= min < ":";

		int sec = (rec.time() % (1000 * 60)) / 1000;
		if (sec < 10) text < "0";
		text <= sec < ":";

		int hsec = (rec.time() % 1000) / 10;
		if (hsec < 10) text < "0";
		text <= hsec < separator;
	}

	if (record_text_format_ & PROFILER_TEXT_TRIGGER_NAME) {
		if (qdTriggerChain * tp = get_record_trigger(rec)) {
			if (tp->name())
				text < "[" < tp->name() < "]" < separator;
		}
	}

	switch (rec.event()) {
	case qdTriggerProfilerRecord::ELEMENT_STATUS_UPDATE:
		switch (rec.status()) {
		case qdTriggerElement::TRIGGER_EL_INACTIVE:
			text < "триггер выключен";
			break;
		case qdTriggerElement::TRIGGER_EL_WAITING:
			text < "триггер ждет";
			break;
		case qdTriggerElement::TRIGGER_EL_WORKING:
			text < "триггер работает";
			break;
		case qdTriggerElement::TRIGGER_EL_DONE:
			text < "триггер закончен";
			break;
		}
		text < separator;
		break;
	case qdTriggerProfilerRecord::PARENT_LINK_STATUS_UPDATE:
	case qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE:
		switch (rec.status()) {
		case qdTriggerLink::LINK_INACTIVE:
			text < "линк выключен";
			break;
		case qdTriggerLink::LINK_ACTIVE:
			text < "линк включен";
			break;
		case qdTriggerLink::LINK_DONE:
			text < "линк отработан";
			break;
		}
		text < separator;
		break;
	}

	if (qdTriggerElementPtr el = get_record_element(rec)) {
		switch (rec.event()) {
		case qdTriggerProfilerRecord::ELEMENT_STATUS_UPDATE:
			text < element_text(el);
			break;
		case qdTriggerProfilerRecord::PARENT_LINK_STATUS_UPDATE:
			if (qdTriggerLink * lp = get_record_link(rec))
				text < element_text(lp->element());
			else
				text < "???";

			text < " ->" < separator;

			text < element_text(el);
			break;
		case qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE:
			text < element_text(el);

			text < " ->" < separator;

			if (qdTriggerLink * lp = get_record_link(rec))
				text < element_text(lp->element());
			else
				text < "???";
			break;
		}
	}

	return text.c_str();
}

qdTriggerElementPtr qdTriggerProfiler::get_record_element(const qdTriggerProfilerRecord &rec) {
	if (qdTriggerChain * p = get_record_trigger(rec))
		return p->search_element(rec.element_id());

	return NULL;
}

qdTriggerLink *qdTriggerProfiler::get_record_link(const qdTriggerProfilerRecord &rec) {
	if (qdTriggerElementPtr p = get_record_element(rec)) {
		if (rec.event() == qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE)
			return p->find_child_link(rec.link_id());
		else
			return p->find_parent_link(rec.link_id());
	}

	return NULL;
}

qdTriggerChain *qdTriggerProfiler::get_record_trigger(const qdTriggerProfilerRecord &rec) {
	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
		qdTriggerChainList::const_iterator it = p->trigger_chain_list().begin();
		for (int i = 0; i < rec.trigger_id(); i++) {
			if (it != p->trigger_chain_list().end()) ++it;
		}

		if (it != p->trigger_chain_list().end()) return *it;
	}

	return NULL;
}

bool qdTriggerProfiler::evolve(int record_num) const {
	assert(record_num >= 0 && record_num < records_.size());

	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
		p->reset_triggers();

	for (int i = 0; i <= record_num; i++) {
		switch (records_[i].event()) {
		case qdTriggerProfilerRecord::ELEMENT_STATUS_UPDATE:
			if (qdTriggerElementPtr p = get_record_element(records_[i]))
				p->set_status(qdTriggerElement::ElementStatus(records_[i].status()));
			break;
		case qdTriggerProfilerRecord::PARENT_LINK_STATUS_UPDATE:
		case qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE:
			if (qdTriggerLink * p = get_record_link(records_[i]))
				p->set_status(qdTriggerLink::LinkStatus(records_[i].status()));
			break;
		}
	}

	return true;
}

qdTriggerProfiler &qdTriggerProfiler::instance() {
	static qdTriggerProfiler pr;

	return pr;
}

const char *qdTriggerProfiler::element_text(qdTriggerElementPtr el) {
	static XBuffer text(1024, 1);
	text.init();

	qdNamedObject *p;

	if (el->object()) {
		if (record_text_format_ & PROFILER_TEXT_SCENE_NAME) {
			if ((p = el->object()->owner(QD_NAMED_OBJECT_SCENE))) {
				if (p->name())
					text < p->name() < "::";
				else
					text < "???::";
			}
		}

		if ((p = el->object()->owner(QD_NAMED_OBJECT_ANIMATED_OBJ))) {
			if (p->name())
				text < p->name() < "::";
			else
				text < "???::";
		} else {
			if ((p = el->object()->owner(QD_NAMED_OBJECT_MOVING_OBJ))) {
				if (p->name())
					text < p->name() < "::";
				else
					text < "???::";
			} else {
				if ((p = el->object()->owner(QD_NAMED_OBJECT_GRID_ZONE))) {
					if (p->name())
						text < p->name() < "::";
					else
						text < "???::";
				}
			}
		}

		if (el->object()->name())
			text < el->object()->name();
		else
			text < "???";
	} else {
		if (el->ID() == qdTriggerElement::ROOT_ID)
			text < "Старт";
	}

	return text.c_str();
}

void qdTriggerProfiler::set_work_file(const char *fname) {
	if (NULL != fname) work_file_ = fname;
	else work_file_.clear();
}

#endif /* __QD_TRIGGER_PROFILER__ */
} // namespace QDEngine
