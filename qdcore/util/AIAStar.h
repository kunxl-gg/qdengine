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

//K-D Lab / Balmer
#pragma once

#include "common/multimap.h"


///////////////////////////AIAStar/////////////////////
//AIAStar::FindPath поиск пути из точки from
//в точку IsEndPoint.
//Боле-менее оптимизированный на случай квадратной сетки

/*
class Heuristic
{
    float GetH(int x,int y);//Предполагаемые затраты на продвижение из pos1 к окончанию
    float GetG(int x1,int y1,int x2,int y2);//Затраты на продвижение из pos1 в pos2
    bool IsEndPoint(int x,int y);//Рекурсия должна окончиться здесь
    //то есть класс AIAStar позволяет задавать несколько точек окончания поиска пути
};
*/

namespace QDEngine {

template<class Heuristic, class TypeH = float>
class AIAStar {
public:
	typedef Common::MultiMap<TypeH, Vect2i> type_point_map;

	struct OnePoint {
		TypeH g;//Затраты на продвижение до этой точки
		TypeH h;//Предполагаемые затраты на продвижение до финиша
		int used;
		OnePoint *parent;
		bool is_open;

		inline TypeH f() {
			return g + h;
		}
	};
protected:
	int dx, dy;
	OnePoint *chart;
	type_point_map open_map;

	int is_used_num;//Если is_used_num==used, то ячейка используется

	int num_point_examine;//количество посещённых ячеек
	int num_find_erase;//Сколько суммарно искали ячейки для удаления
	Heuristic *heuristic;
public:
	AIAStar();
	~AIAStar();

	void Init(int dx, int dy);
	bool FindPath(Vect2i from, Heuristic *h, Std::vector<Vect2i> &path, int directions_count = 8);
	void GetStatistic(int *num_point_examine, int *num_find_erase);

	//Debug
	OnePoint *GetInternalBuffer() {
		return chart;
	};
	int GetUsedNum() {
		return is_used_num;
	}
protected:
	void clear();
	inline Vect2i PosBy(OnePoint *p) {
		int offset = p - chart;
		Vect2i pos;
		pos.x = offset % dx;
		pos.y = offset / dx;
		return pos;
	}
};

template<class Heuristic, class TypeH>
AIAStar<Heuristic, TypeH>::AIAStar() {
	chart = NULL;
	heuristic = NULL;
}

template<class Heuristic, class TypeH>
void AIAStar<Heuristic, TypeH>::Init(int _dx, int _dy) {
	dx = _dx;
	dy = _dy;

	int size = dx * dy;
	chart = new OnePoint[size];
	clear();
}

template<class Heuristic, class TypeH>
void AIAStar<Heuristic, TypeH>::clear() {
	int size = dx * dy;
	is_used_num = 0;
	for (int i = 0; i < size; i++)
		chart[i].used = 0;
}

template<class Heuristic, class TypeH>
AIAStar<Heuristic, TypeH>::~AIAStar() {
	delete[] chart;
}

template<class Heuristic, class TypeH>
bool AIAStar<Heuristic, TypeH>::FindPath(Vect2i from, Heuristic *hr, Std::vector<Vect2i> &path, int directions_count) {
	num_point_examine = 0;
	num_find_erase = 0;

	is_used_num++;
	open_map.clear();
	path.clear();
	if (is_used_num == 0)
		clear();//Для того, чтобы вызвалась эта строчка, необходимо гиганское время
	assert(from.x >= 0 && from.x < dx && from.y >= 0 && from.y < dy);
	heuristic = hr;

	OnePoint *p = chart + from.y * dx + from.x;
	p->g = 0;
	p->h = heuristic->GetH(from.x, from.y);
	p->used = is_used_num;
	p->is_open = true;
	p->parent = NULL;

	open_map.insert(typename type_point_map::value_type(p->f(), from));

	const int sx[8] = { 0, -1, 0, +1, -1, +1, +1, -1,};
	const int sy[8] = {-1, 0, +1, 0, -1, -1, +1, +1 };

//	const int sx[size_child]={ 0,-1, 0,+1};
//	const int sy[size_child]={-1, 0,+1, 0};

	const int size_child = directions_count;

	while (!open_map.empty()) {
		typename type_point_map::iterator low = open_map.begin();
		Vect2i pt = (*low).second;
		OnePoint *parent = chart + pt.y * dx + pt.x;

		parent->is_open = false;
		open_map.erase(low);

		if (heuristic->IsEndPoint(pt.x, pt.y)) {
			//сконструировать путь
			Vect2i vp;
			while (parent) {
				vp = PosBy(parent);;
				path.push_back(vp);

				if (parent->parent) {
					Vect2i pp;
					pp = PosBy(parent->parent);
					assert(abs(vp.x - pp.x) <= 1 &&
					       abs(vp.y - pp.y) <= 1);
				}

				parent = parent->parent;
			}
			assert(vp.x == from.x && vp.y == from.y);
			Common::reverse(path.begin(), path.end());
			return true;
		}

		//для каждого наследника child узла parent
		for (int i = 0; i < size_child; i++) {
			Vect2i child = Vect2i(pt.x + sx[i], pt.y + sy[i]);
			num_point_examine++;

			if (child.x < 0 || child.y < 0 ||
			        child.x >= dx || child.y >= dy)continue;
			p = chart + child.y * dx + child.x;


			TypeH addg = heuristic->GetG(pt.x, pt.y, child.x, child.y);
			TypeH newg = parent->g + addg;

			if (p->used == is_used_num) {
				if (!p->is_open)continue;
				if (p->g <= newg)continue;

				//Удаляем элемент из open_map
				TypeH f = p->f();
				typename type_point_map::iterator cur = open_map.find(p->f());
				bool erase = false;
				while (cur != open_map.end()) {
					if ((*cur).first != f)break;
					if ((*cur).second.x == child.x && (*cur).second.y == child.y) {
						open_map.erase(cur);
						erase = true;
						break;
					}
					num_find_erase++;
					cur++;
				}
				num_find_erase++;
				//assert(erase);
				if (!erase)
					continue;
			}

			p->parent = parent;
			/*
			            {
			                Vect2i pp=PosBy(parent);
			                Vect2i pc=PosBy(p);
			                assert(abs(pc.x-pp.x)<=1 &&
			                        abs(pc.y-pp.y)<=1);
			            }
			*/
			p->g = newg;
			p->h = heuristic->GetH(child.x, child.y);

			open_map.insert(typename type_point_map::value_type(p->f(), child));

			p->is_open = true;
			p->used = is_used_num;
		}
	}

	return false;
}

template<class Heuristic, class TypeH>
void AIAStar<Heuristic, TypeH>::GetStatistic(
    int *p_num_point_examine, int *p_num_find_erase) {
	if (p_num_point_examine)
		*p_num_point_examine = num_point_examine;
	if (p_num_find_erase)
		*p_num_find_erase = num_find_erase;
}

///////////////////////AIAStarGraph/////////////
//AIAStarGraph - Так-же поиск пути, но ориентированный на поиск в произвольном графе
/*
class Node
{
    typedef ... iterator;
    iterator begin();//Работа со списком связанных с этой Node нод.
    iterator end();

    void* AIAStarPointer;//Используется в AIAStarGraph
};

class Heuristic
{
    float GetH(Node* pos);//Предполагаемые затраты на продвижение из pos1 к окончанию
    float GetG(Node* pos1,Node* pos2);//Затраты на продвижение из pos1 в pos2
    bool IsEndPoint(Node* pos);//Рекурсия должна окончиться здесь
    //то есть класс AIAStar позволяет задавать несколько точек окончания поиска пути
};
*/

template<class Heuristic, class Node, class TypeH = float>
class AIAStarGraph {
public:
	struct OnePoint;
	typedef Common::MultiMap<TypeH, OnePoint *> type_point_map;

	struct OnePoint {
		TypeH g;//Затраты на продвижение до этой точки
		TypeH h;//Предполагаемые затраты на продвижение до финиша
		int used;
		OnePoint *parent;
		bool is_open;

		Node *node;
		typename type_point_map::iterator self_it;

		inline TypeH f() {
			return g + h;
		}
	};
protected:
	Std::vector<OnePoint> chart;
	type_point_map open_map;

	int is_used_num;//Если is_used_num==used, то ячейка используется

	int num_point_examine;//количество посещённых ячеек
	int num_find_erase;//Сколько суммарно искали ячейки для удаления
	Heuristic *heuristic;
public:
	AIAStarGraph();
	~AIAStarGraph();

	//Общее количество узлов. Константа, которая не должна меняться,
	//пока существует класс, указывающий на неё.
	void Init(Std::vector<Node> &all_node);

	bool FindPath(Node *from, Heuristic *h, Std::vector<Node *> &path);
	void GetStatistic(int *num_point_examine, int *num_find_erase);

	//Debug
	OnePoint *GetInternalBuffer() {
		return chart;
	};
	int GetUsedNum() {
		return is_used_num;
	}
protected:
	void clear();
	inline Node *PosBy(OnePoint *p) {
		return p->node;
	}
};

template<class Heuristic, class Node, class TypeH>
AIAStarGraph<Heuristic, Node, TypeH>::AIAStarGraph() {
	heuristic = NULL;
}

template<class Heuristic, class Node, class TypeH>
void AIAStarGraph<Heuristic, Node, TypeH>::Init(Std::vector<Node> &all_node) {
	int size = all_node.size();
	chart.resize(size);

	for (int i = 0; i < size; i++) {
		OnePoint *c = &chart[i];
		c->node = &all_node[i];
		c->node->AIAStarPointer = (void *)c;
	}
	clear();
}

template<class Heuristic, class Node, class TypeH>
void AIAStarGraph<Heuristic, Node, TypeH>::clear() {
	is_used_num = 0;
	for (auto &it : chart) {
		it.used = 0;
	}
}

template<class Heuristic, class Node, class TypeH>
AIAStarGraph<Heuristic, Node, TypeH>::~AIAStarGraph() {
}

template<class Heuristic, class Node, class TypeH>
bool AIAStarGraph<Heuristic, Node, TypeH>::FindPath(Node *from, Heuristic *hr, Std::vector<Node *> &path) {
	num_point_examine = 0;
	num_find_erase = 0;

	is_used_num++;
	open_map.clear();
	path.clear();
	if (is_used_num == 0)
		clear();//Для того, чтобы вызвалась эта строчка, необходимо гиганское время
	heuristic = hr;

	OnePoint *p = (OnePoint *)from->AIAStarPointer;
	Node *from_node = p->node;
	p->g = 0;
	p->h = heuristic->GetH(p->node);
	p->used = is_used_num;
	p->is_open = true;
	p->parent = NULL;

	p->self_it = open_map.insert(type_point_map::value_type(p->f(), p));

	while (!open_map.empty()) {
		typename type_point_map::iterator low = open_map.begin();

		OnePoint *parent = low->second;
		Node *node = parent->node;

		parent->is_open = false;
		open_map.erase(low);

		if (heuristic->IsEndPoint(node)) {
			//сконструировать путь
			Node *np;
			while (parent) {
				np = PosBy(parent);
				assert(parent->used == is_used_num);

				path.push_back(np);
				parent = parent->parent;
			}
			assert(np == from_node);
			reverse(path.begin(), path.end());
			return true;
		}

		//для каждого наследника child узла parent
		for (auto &it : *node) {
			Node *cur_node = *it;
			OnePoint *op = (OnePoint *)cur_node->AIAStarPointer;
			num_point_examine++;

			TypeH addg = heuristic->GetG(node, cur_node);
			TypeH newg = parent->g + addg;

			if (op->used == is_used_num) {
				if (!op->is_open)continue;
				if (op->g <= newg)continue;

				open_map.erase(op->self_it);
				num_find_erase++;
			}

			op->parent = parent;
			op->g = newg;
			op->h = heuristic->GetH(cur_node);

			op->self_it = open_map.insert(type_point_map::value_type(op->f(), op));

			op->is_open = true;
			op->used = is_used_num;
		}
	}

	return false;
}

template<class Heuristic, class Node, class TypeH>
void AIAStarGraph<Heuristic, Node, TypeH>::GetStatistic(
    int *p_num_point_examine, int *p_num_find_erase) {
	if (p_num_point_examine)
		*p_num_point_examine = num_point_examine;
	if (p_num_find_erase)
		*p_num_find_erase = num_find_erase;
}

///////////////////////AIFindMaxium/////////////

/*
struct Maps
{
    //Значение чего нибудь в точке (x,y)
    TypeH Get(int x,int y);

    //Дальнейшие поиски можно прекратить
    bool IsOptiumGood(TypeH optium,int x,int y);
};
*/

//Ищет минимальное значение, но не по всей карте
template<class Maps>
Vect2i AIFindMinium(int x, int y,
                    Maps &maps,
                    int dx, int dy) {
	typename Maps::TypeH optium = maps.Get(x, y);
	int optiumx = x, optiumy = y;

	int maxi = MAX(MAX(x, dx - x), MAX(y, dy - y));
	for (int i = 1; i < maxi; i++) {
		int curx, cury;
		int xmin = MAX(0, x - i), xmax = MIN(dx - 1, x + i);
		int ymin = MAX(0, y - i), ymax = MIN(dy - 1, y + i);
		//up
		cury = y - i;
		if (cury >= 0)
			for (curx = xmin; curx <= xmax; curx++) {
				typename Maps::TypeH o = maps.Get(curx, cury);
				if (o < optium) {
					optium = o;
					optiumx = curx;
					optiumy = cury;
				}
			}

		//down
		cury = y + i;
		if (cury < dy)
			for (curx = xmin; curx <= xmax; curx++) {
				typename Maps::TypeH o = maps.Get(curx, cury);
				if (o < optium) {
					optium = o;
					optiumx = curx;
					optiumy = cury;
				}
			}

		//left
		curx = x - i;
		if (curx >= 0)
			for (cury = ymin; cury <= ymax; cury++) {
				typename Maps::TypeH o = maps.Get(curx, cury);
				if (o < optium) {
					optium = o;
					optiumx = curx;
					optiumy = cury;
				}
			}

		//right
		curx = x + i;
		if (curx < dx)
			for (cury = ymin; cury <= ymax; cury++) {
				typename Maps::TypeH o = maps.Get(curx, cury);
				if (o < optium) {
					optium = o;
					optiumx = curx;
					optiumy = cury;
				}
			}

		if (maps.IsOptiumGood(optium, optiumx, optiumy))
			break;
	}

	Vect2i p = {optiumx, optiumy};
	return p;
}
} // namespace QDEngine
