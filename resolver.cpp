#include "includes.h"
Resolver g_resolver{};;

void Resolver::OnBodyUpdate(Player* player, float value) {
	/*AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set data.
	data->m_body_proxy_old = data->m_body;
	data->m_body = value;*/
}

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (current->m_shot || current->m_mode == Modes::RESOLVE_FLICK || current->m_mode == Modes::RESOLVE_WALK || current->m_mode == Modes::RESOLVE_DISABLED)
			break;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant())
			return current;
	}

	return nullptr;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	ang_t  away;
	math::VectorAngles(record->m_origin - g_cl.m_local->m_vecOrigin(), away);
	return away.y;
}



void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.
	if (g_menu.main.aimbot.nospread.get())
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval;
	}

	// this record has a shot on it.
	if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
		if (record->m_lag <= 2)
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if (data->m_records.size() >= 2) {
			LagRecord* previous = data->m_records[1].get();

			if (previous)
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

class AntiFsAngle {
public:
	float m_base, m_add;
	float m_dist;
public:
	// ctor.
	__forceinline AntiFsAngle(float yaw, float add) {
		// set yaw.
		m_base = yaw;
		m_add = add;
		m_dist = 0.f;
	}
};

void Resolver::AntiFreestand(LagRecord* record, float& current_yaw, float multiplier) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos;
	record->m_player->GetEyePos(&enemypos);
	float away = GetAwayAngle(record);

	// construct vector of angles to test.
	std::vector< AntiFsAngle > angles{ };
	//angles.emplace_back(away);
	angles.emplace_back(away, 0.f);
	angles.emplace_back(away, 90.f);
	angles.emplace_back(away, -90.f);

	// start the trace at the your shoot pos.
	vec3_t start;
	g_cl.m_local->GetEyePos(&start);

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(math::NormalizedAngle(it->m_base + it->m_add))) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(math::NormalizedAngle(it->m_base + it->m_add))) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.
		 //g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	current_yaw = away;

	if (!valid)
		return;

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AntiFsAngle& a, const AntiFsAngle& b) {
			return a.m_dist > b.m_dist;
		});


	current_yaw = angles.front().m_base + (angles.front().m_add * multiplier);
}


void Resolver::SetMode(LagRecord* record) {
	if (record->m_flags & FL_ONGROUND) {
		if (record->m_anim_velocity.length_2d() > 0.0f)
			record->m_mode = RESOLVE_WALK;
		else
			record->m_mode = RESOLVE_STAND;
	}
	else
		record->m_mode = RESOLVE_AIR;
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	SetMode(record);
	MatchShot(data, record);

	LagRecord* previous = nullptr;
	if (data->m_records.size() > 1)
		previous = data->m_records[1].get();

	CCSGOPlayerAnimState* state = player->m_PlayerAnimState();
	if (!state)
		return;

	if (record->m_mode == RESOLVE_STAND) {
		if (previous && record->m_body != previous->m_body) {
			data->m_body_update = record->m_anim_time + 1.1f;
			data->m_can_predict = true;

			record->m_mode = RESOLVE_FLICK;
		}
		else if (data->m_body_update <= record->m_anim_time && data->m_can_predict) {
			data->m_body_update = record->m_anim_time + 1.1f;

			record->m_mode = RESOLVE_FLICK;
		}
	}

	if (g_menu.main.aimbot.nospread.get() == 1)
		record->m_eye_angles.x = 90.f;

	switch (record->m_mode) {
	case RESOLVE_WALK:
		ResolveWalk(data, record, state);
		break;
	case RESOLVE_STAND:
		ResolveStand(data, record, previous, state);
		break;
	case RESOLVE_AIR:
		ResolveAir(data, record, state);
		break;
	case RESOLVE_FLICK:
		ResolveLby(data, record, state);
		break;
	}
}
void Resolver::ResolvePoses(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// only do this bs when in air.
	if (record->m_mode == Modes::RESOLVE_AIR) {
		// ang = pose min + pose val x ( pose range )

		// lean_yaw
		player->m_flPoseParameter()[2] = g_csgo.RandomInt(0, 4) * 0.25f;

		// body_yaw
		player->m_flPoseParameter()[11] = g_csgo.RandomInt(1, 3) * 0.25f;
	}
}


void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state) {
	data->m_body_update = record->m_anim_time + 0.22f;
	data->m_can_predict = true;

	record->m_eye_angles.y = record->m_body;
	record->m_mode = RESOLVE_WALK;

	if (record->m_anim_velocity.length_2d() > 1.f) {
		data->m_stand_index = 0;
		data->m_body_index = 0;
		data->m_air_index = 0;

		data->m_walk_body = record->m_body;
		data->m_last_move_time = record->m_anim_time;

		data->m_prev_speed = data->m_last_speed;
		data->m_last_speed = record->m_anim_velocity.length_2d();

		if (data->m_last_speed > 30.f)
			data->m_ff_index = 0;
	}
}


void Resolver::ResolveStand(AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state) {
	float time_delta = record->m_anim_time - data->m_last_move_time;
	if (data->m_moved && time_delta < 0.22f) {
		record->m_eye_angles.y = data->m_walk_body;
		record->m_mode = RESOLVE_PREFLICK;
		return;
	}

	float away = GetAwayAngle(record);

	if (data->m_last_speed <= 25.f && data->m_last_speed >= data->m_prev_speed && data->m_ff_index < 3) {
		switch (data->m_stand_index % 3) {
		case 0:
			AntiFreestand(record, record->m_eye_angles.y, 1.f);
			break;
		case 1:
			record->m_eye_angles.y = away;
			break;
		case 2:
			AntiFreestand(record, record->m_eye_angles.y, -1.f);
			break;
		}
		record->m_mode = RESOLVE_FAKEFLICK;
		return;
	}

	if (data->m_stand_index % 4 == 0 && !data->m_moved)
		++data->m_stand_index;

	switch (data->m_stand_index % 4) {
	case 0:
		record->m_eye_angles.y = data->m_walk_body;
		break;
	case 1:
		AntiFreestand(record, record->m_eye_angles.y, 1.f);
		break;
	case 2:
		record->m_eye_angles.y = away;
		break;
	case 3:
		AntiFreestand(record, record->m_eye_angles.y, -1.f);
		break;
	}
}

void Resolver::ResolveLby(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state) {
	record->m_eye_angles.y = record->m_body;
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state) {
	float away = GetAwayAngle(record);

	if (data->m_air_index % 4 == 1 && !data->m_moved)
		++data->m_air_index;

	switch (data->m_air_index % 4) {
	case 0:
		record->m_eye_angles.y = away;
		break;
	case 1:
		record->m_eye_angles.y = data->m_walk_body;
		break;
	case 2:
		AntiFreestand(record, record->m_eye_angles.y, 1.f);
		break;
	case 3:
		AntiFreestand(record, record->m_eye_angles.y, -1.f);
		break;
	}

	record->m_mode = RESOLVE_AIR;
}