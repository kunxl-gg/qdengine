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

#ifndef __SYNCRO_TIMER__
#define __SYNCRO_TIMER__

typedef unsigned int time_type;

namespace QDEngine {

class SyncroTimer {
public:
	// Main property
	time_type operator()() const {
		return round(time);
	}
	// Last delta
	time_type delta() const {
		return time - time_prev;
	}

	SyncroTimer() {
		set(1, 15, 100);
		time_prev = time = 1;
		time_offset = 0;
		time_speed = 1;
	}

	void set(int syncro_by_clock_, time_type time_per_frame_, time_type max_time_interval_) {
		syncro_by_clock = syncro_by_clock_;
		time_per_frame = time_per_frame_;
		max_time_interval = max_time_interval_;
	}

	SyncroTimer &adjust() {
		time_prev = time;

		if (syncro_by_clock) {
			float t = float(g_system->getMillis());
			float dt = (t - time - time_offset) * time_speed;
			if (dt > max_time_interval)
				dt = max_time_interval;
			time += dt;
			time_offset = t - time;
		}
		return *this;
	}

	void next_frame() {
		if (syncro_by_clock)
			adjust();
		else {
			time_prev = time;
			time += time_per_frame * time_speed;
//			time += round(time_per_frame*time_speed);
		}
	}

	void skip() {
		if (syncro_by_clock)
			time_offset = g_system->getMillis() - time;
	}

	void setTime(time_type t) {
		time_prev = time = t;
		time_offset = syncro_by_clock ? g_system->getMillis() - time : 0;
	}

	void setSpeed(float speed) {
		time_speed = speed;
	}

private:
	float time;
	float time_prev;
	float time_offset;
	time_type max_time_interval;
	time_type time_per_frame;
	int syncro_by_clock;
	float time_speed;
};

} // namespace QDEngine

#endif // __SYNCRO_TIMER__
