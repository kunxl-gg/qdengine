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

#ifndef QDENGINE_CORE_QDCORE_CONDITIONAL_OBJECT_H
#define QDENGINE_CORE_QDCORE_CONDITIONAL_OBJECT_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_condition.h"
#include "qdengine/core/qdcore/qd_condition_group.h"
#include "qdengine/core/qdcore/qd_named_object.h"

namespace Common {
class SeekableWriteStream;
}

namespace QDEngine {

//! Объект с условиями.
class qdConditionalObject : public qdNamedObject {
public:
	qdConditionalObject();
	qdConditionalObject(const qdConditionalObject &obj);
	~qdConditionalObject();

	qdConditionalObject &operator = (const qdConditionalObject &obj);

	//! Режим проверки условий.
	enum ConditionsMode {
		//! "И" - должны выполниться все условия.
		CONDITIONS_AND,
		//! "ИЛИ" - достаточно выполнения одного из условий.
		CONDITIONS_OR
	};

	typedef std::vector<qdCondition> conditions_container_t;
	typedef std::vector<qdConditionGroup> condition_groups_container_t;

	//! Возвращает true, если условия выполняются.
	virtual bool check_conditions();
	//! Добавляет условие. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition(const qdCondition *p);
	//! Обновляет условие условие с порядковым номером num.
	bool update_condition(int num, const qdCondition &p);

	//! Удаляет условие.
	bool remove_conditon(int idx);

#ifdef _QUEST_EDITOR
	//! Удаляет все условия
	void remove_all_conditions();
	//! Удаляет все группы
	void remove_all_groups();
#endif //_QUEST_EDITOR

	//! Возвращает указатель на условие.
	const qdCondition *get_condition(int idx = 0) const {
		return &*(conditions_.begin() + idx);
	}

	//! Возвращает количество условий.
	int conditions_count() const {
		return conditions_.size();
	}

	//! Добавляет группу условий. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition_group(const qdConditionGroup *p);
	//! Обновляет группу условий с порядковым номером num.
	bool update_condition_group(int num, const qdConditionGroup &p);

	//! Удаляет группу условий.
	bool remove_conditon_group(int idx);

	//! Возвращает указатель на группу условий.
	const qdConditionGroup *get_condition_group(int idx = 0) const {
		return &*(condition_groups_.begin() + idx);
	}

	//! Возвращает количество групп условий.
	int condition_groups_count() const {
		return condition_groups_.size();
	}

	//! Возвращает режим проверки условий.
	ConditionsMode conditions_mode() const {
		return conditions_mode_;
	}
	//! Устанавливает режим проверки условий.
	void set_conditions_mode(ConditionsMode m) {
		conditions_mode_ = m;
	}

	//! Возвращает true, если список условий не пустой.
	bool has_conditions() const {
		return !conditions_.empty();
	}

	//! Обсчет логики условий, dt - время в секундах.
	void conditions_quant(float dt);

#ifdef _QUEST_EDITOR
	bool backup_object();
	bool restore_object();
#endif

	//! Инициализация объекта, вызывается при старте и перезепуске игры.
	virtual bool init();

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh) const;

#ifdef _QUEST_EDITOR
	bool init_objects();
	static void toggle_global_list(bool state) {
		enable_objects_list_ = state;
		if (!state)
			objects_list_.clear();
	}
	static void global_init();
#endif

	bool is_condition_in_group(int condition_idx) const;

	//! Варианты старта триггера.
	enum trigger_start_mode {
		//! запуск не удался
		TRIGGER_START_FAILED,
		//! включить активный режим у триггера
		TRIGGER_START_ACTIVATE,
		//! включить ждущий режим у триггера
		TRIGGER_START_WAIT
	};
	//! Активация триггера.
	virtual trigger_start_mode trigger_start() = 0;

	//! Возвращает true, если триггер может быть активирован при выполнении его внутренних условий.
	/**
	Объект должен принадлежать активной сцене (игровые объекты, зоны на сетке, музыка)
	или не зависеть от нее (сцены, мини-игры и т.д.).
	*/
	virtual bool trigger_can_start() const;

protected:

	//! Загрузка данных из скрипта.
	bool load_conditions_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_conditions_script(XStream &fh, int indent = 0) const;

private:

	//! Логика проверки условий - И/ИЛИ.
	ConditionsMode conditions_mode_;

	//! Условия.
	conditions_container_t conditions_;

	//! Группы условий.
	condition_groups_container_t condition_groups_;

#ifdef _QUEST_EDITOR
	static conditions_container_t backup_;
	static condition_groups_container_t groups_backup_;
	static ConditionsMode backup_mode_;

	//! Список всех объектов (для инициализации в редакторе).
	static std::list<qdConditionalObject *> objects_list_;
	static bool enable_objects_list_;
#endif

	bool check_group_conditions(const qdConditionGroup &gr);
};

} // namespace QDEngine

#endif /* QDENGINE_CORE_QDCORE_CONDITIONAL_OBJECT_H */
