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

#ifndef QDENGINE_MINIGAMES_PUZZLE_ALL_H
#define QDENGINE_MINIGAMES_PUZZLE_ALL_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdPuzzleAllMiniGameInterface : public qdMiniGameInterface {
public:
	qdPuzzleAllMiniGameInterface(Common::String dll, Common::Language language) : _dll(dll), _language(language) {}
	~qdPuzzleAllMiniGameInterface() {};

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "PuzzleAll::init(), lang: %d", _language);

		_engine = engine_interface;
		_scene = _engine->current_scene_interface();
		if (!_scene)
			return 0;

		if (_dll == "DLL\\Puzzle_ep08.dll") {
			_pieceCoords[0].y = 188;
			_pieceCoords[1].y = 181;
			_pieceCoords[2].x = 462;
			_pieceCoords[2].y = 188;
			_pieceCoords[3].y = 181;
			_pieceCoords[6].x = 462;
			_pieceCoords[10].x = 462;
			_pieceCoords[0].x = 217;
			_pieceCoords[1].x = 337;
			_pieceCoords[3].x = 575;
			_pieceCoords[4].x = 224;
			_pieceCoords[4].y = 302;
			_pieceCoords[5].x = 338;
			_pieceCoords[5].y = 301;
			_pieceCoords[6].y = 301;
			_pieceCoords[7].x = 582;
			_pieceCoords[7].y = 301;
			_pieceCoords[8].x = 217;
			_pieceCoords[8].y = 415;
			_pieceCoords[9].x = 337;
			_pieceCoords[9].y = 422;
			_pieceCoords[10].y = 414;
			_pieceCoords[11].x = 575;
			_pieceCoords[11].y = 421;
		}

		for (int i = 0; i < 12; i++)
			_pieces[i] = _scene->object_interface(Common::String::format("\xee\xe1\xfa\xe5\xea\xf2_%i", i + 1).c_str());	// "объект_%i"

		_objFinal = _scene->object_interface("$\xf4\xe8\xed\xe0\xeb"); // "$финал"
		_objRan = _scene->object_interface("$\xe7\xe0\xef\xf3\xf1\xf2\xe8\xeb\xe8");	// "$запустили""
		_wasInited = false;

		if (_objFinal->is_state_active("\xed\xe5\xf2")) { // "нет"
			_isFinal = false;

			_minDepthPiece = findMinDepthPiece();
			return true;
		} else {
			if (_objFinal->is_state_active("\xe4\xe0"))	// "да"
				_isFinal = true;

			_minDepthPiece = findMinDepthPiece();
			return true;
		}

		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		debugC(3, kDebugMinigames, "PuzzleAll::quant(%f)", dt);

		if (!_wasInited && _objRan->is_state_active("\xed\xe5\xf2")) {	// "нет"
			_rotatingPiece = -1;
			_currentPieceState = -1;
			_pieceIsPut = false;
			_isFinal = false;
			_wasInited = 1;
			_objRan->set_state("\xe4\xe0");	// "да"
		}

		if (_isFinal)
			return true;

		if (!checkSolution() || _scene->mouse_object_interface()) {
			qdMinigameObjectInterface *mouseObj = _scene->mouse_object_interface();

			if (_pieceIsPut) {
				for (int i = 0; i < 12; i++) {
					if (_pieces[i]->is_state_active("to_inv_flag_0")
							|| _pieces[i]->is_state_active("to_inv_flag_90")
							|| _pieces[i]->is_state_active("to_inv_flag_180")
							|| _pieces[i]->is_state_active("to_inv_flag_270")) {

						if (_pieces[i]->is_state_active("to_inv_flag_0"))
							_currentPieceState = _pieces[i]->state_index("inv_0");
						else if (_pieces[i]->is_state_active("to_inv_flag_90"))
							_currentPieceState = _pieces[i]->state_index("inv_90");
						else if (_pieces[i]->is_state_active("to_inv_flag_180"))
							_currentPieceState = _pieces[i]->state_index("inv_180");
						else if (_pieces[i]->is_state_active("to_inv_flag_270"))
							_currentPieceState = _pieces[i]->state_index("inv_270");

						_rotatingPiece = i;
						_pieces[i]->set_state("to_inv");
						_pieceIsPut = false;
					}
				}
			}

			if (mouseObj) {
				if (_rotatingPiece != -1) {
					_pieces[_rotatingPiece]->set_state(_currentPieceState);
					_currentPieceState = -1;
					_rotatingPiece = -1;
				}
			}

			if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN)) {
				mgVect2i mousePos = _engine->mouse_cursor_position();
				qdMinigameObjectInterface *obj = _scene->mouse_object_interface();

				if (obj) {
					if (obj->is_state_active("inv_0"))
						obj->set_state("0");
					else if (obj->is_state_active("inv_90"))
						obj->set_state("90");
					else if (obj->is_state_active("inv_180"))
						obj->set_state("180");
					else if (obj->is_state_active("inv_270"))
						obj->set_state("270");

					_minDepthPiece -= 250.0;

					mgVect3f coords = _scene->screen2world_coords(mousePos, _minDepthPiece);
					obj->set_R(coords);

					snapPieces();
				} else {
					_pieceIsPut = true;
				}
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN)) {
			qdMinigameObjectInterface *obj = _scene->mouse_object_interface();

			if (obj) {
				if (obj->is_state_active("inv_0"))
					obj->set_state("inv_270");
				else if (obj->is_state_active("inv_90"))
					obj->set_state("inv_0");
				else if (obj->is_state_active("inv_180"))
					obj->set_state("inv_90");
				else if (obj->is_state_active("inv_270"))
					obj->set_state("inv_180");
			}
		}

		return true;
	}

	bool checkSolution() {
		if (_scene->mouse_object_interface())
			return false;

		for (int i = 0; i < 12; i++)
			if (!_pieces[0]->is_state_active("0"))
				return false;

		mgVect2i piecePos;

		for (int i = 0; i < 12; i++) {
			piecePos = _pieces[i]->screen_R();

			if (ABS(_pieceCoords[i].x - piecePos.x) > 10 ||
				ABS(_pieceCoords[i].y - piecePos.y) > 10)
				return false;
		}

		_isFinal = true;
		_objFinal->set_state("\xe4\xe0");	// "да"

		_minDepthPiece -= 250.0;

		mgVect3f coords = _scene->screen2world_coords(_objFinal->screen_R(), _minDepthPiece);
		_objFinal->set_R(coords);

		return true;
	}

	void snapPieces() {
		mgVect2i piecePos;
		mgVect3f newPiecePos;

		for (int i = 0; i < 12; i++) {
			piecePos = _pieces[i]->screen_R();
			float depth = _scene->screen_depth(_pieces[i]->R());

			if (_pieces[i]->is_state_active("0")) {
				if (ABS(_pieceCoords[i].x - piecePos.x) <= 10) {
					if (ABS(_pieceCoords[i].y - piecePos.y) <= 10) {
						piecePos.x = _pieceCoords[i].x;
						piecePos.y = _pieceCoords[i].y;

						newPiecePos = _scene->screen2world_coords(piecePos, depth);
						_pieces[i]->set_R(newPiecePos);
					}
				}
			}
		}
	}

	//! Деинициализация игры.
	bool finit() {
		debugC(1, kDebugMinigames, "PuzzleAll::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	/// Версия интерфейса игры, трогать не надо.
	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	float findMinDepthPiece() {
		float min = 100000.0;

		for (int i = 0; i < 12; i++) {
			float depth = _scene->screen_depth(_pieces[i]->R());

			if (min > depth)
				min = depth;
		}

		return min;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_pieces[12];
	Vect2i _pieceCoords[12];

	qdMinigameObjectInterface *_objFinal = nullptr;
	qdMinigameObjectInterface *_objRan = nullptr;

	bool _wasInited = false;
	bool _isFinal = false;

	float _minDepthPiece = 0.0;
	int _rotatingPiece = -1;
	bool _pieceIsPut = true;
	int _currentPieceState = 0;

	Common::String _dll;
	Common::Language _language;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_PUZZLE_ALL_H