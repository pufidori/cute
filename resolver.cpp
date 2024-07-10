#include "includes.h"
Resolver g_resolver{};

void Resolver::OnBodyUpdate(Player* player, float angleOffset) {
    AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
    float new_body = player->m_flLowerBodyYawTarget() + angleOffset;


    //negushook yes.
    if (player->m_vecVelocity().length_2d() > 0.1f || !(player->m_fFlags() & FL_ONGROUND)) {
        data->m_body_proxy_updated = false;
        data->m_body_proxy_old = new_body;
        data->m_body_proxy = new_body;
        return;
    }

    if (fabsf(math::AngleDiff(new_body, data->m_body_proxy)) >= 15.f) {
        data->m_body_proxy_old = data->m_body_proxy;
        data->m_body_proxy = new_body;
        data->m_body_proxy_updated = true;
    }
}

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
    LagRecord* first_valid = nullptr;

    if (data->m_records.empty())
        return nullptr;

    for (const auto& it : data->m_records) {
        if (it->dormant() || it->immune() || !it->valid())
            continue;

        LagRecord* current = it.get();

        if (!first_valid)
            first_valid = current;

        if (current->m_shot || current->m_mode == Modes::RESOLVE_FLICK || current->m_mode == Modes::RESOLVE_WALK || current->m_mode == Modes::RESOLVE_DISABLED)
            break;
    }

    return first_valid ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
    if (data->m_records.empty())
        return nullptr;

    for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
        LagRecord* current = it->get();

        if (current->valid() && !current->immune() && !current->dormant())
            return current;
    }

    return nullptr;
}

float Resolver::GetAwayAngle(LagRecord* record) {
    ang_t away;
    math::VectorAngles(record->m_origin - g_cl.m_local->m_vecOrigin(), away);
    return away.y;
}

void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
    if (g_menu.main.aimbot.nospread.get())
        return;

    float shoot_time = -1.f;
    Weapon* weapon = data->m_player->GetActiveWeapon();
    if (weapon) {
        shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval;
    }

    if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
        if (record->m_lag <= 2)
            record->m_shot = true;
        else if (data->m_records.size() >= 2) {
            LagRecord* previous = data->m_records[1].get();
            if (previous)
                record->m_eye_angles.x = previous->m_eye_angles.x;
        }
    }
}

class AntiFsAngle {
public:
    float m_base, m_add, m_dist;

    __forceinline AntiFsAngle(float yaw, float add) {
        m_base = yaw;
        m_add = add;
        m_dist = 0.f;
    }
};

void Resolver::AntiFreestand(LagRecord* record, float& current_yaw, float multiplier) {
    constexpr float STEP{ 4.f };
    constexpr float RANGE{ 32.f };

    vec3_t enemypos;
    record->m_player->GetEyePos(&enemypos);
    float away = GetAwayAngle(record);

    std::vector<AntiFsAngle> angles{ {away, 0.f}, {away, 90.f}, {away, -90.f} };

    vec3_t start;
    g_cl.m_local->GetEyePos(&start);

    bool valid{ false };

    for (auto& it : angles) {
        vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(math::NormalizedAngle(it.m_base + it.m_add))) * RANGE,
                    enemypos.y + std::sin(math::deg_to_rad(math::NormalizedAngle(it.m_base + it.m_add))) * RANGE,
                    enemypos.z };

        vec3_t dir = end - start;
        float len = dir.normalize();

        if (len <= 0.f)
            continue;

        for (float i{ 0.f }; i < len; i += STEP) {
            vec3_t point = start + (dir * i);
            int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

            if (!(contents & MASK_SHOT_HULL))
                continue;

            float mult = 1.f;
            if (i > (len * 0.5f))
                mult = 1.25f;
            if (i > (len * 0.75f))
                mult = 1.25f;
            if (i > (len * 0.9f))
                mult = 2.f;

            it.m_dist += (STEP * mult);
            valid = true;
        }
    }

    current_yaw = away;
    if (!valid)
        return;

    std::sort(angles.begin(), angles.end(), [](const AntiFsAngle& a, const AntiFsAngle& b) { return a.m_dist > b.m_dist; });

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

    float ao = 0.0f;

    if (record->m_velocity.length_2d() < 1.0f && record->m_flags & FL_ONGROUND) {
        ao = -90.0f;
    }
    else {
        // high quality detection area :3
        if (record->m_velocity.length_2d() > 100.0f) {
            ao = 30.0f;
        }
        else {
            ao = -30.0f;
        }
    }

    switch (record->m_mode) {
    case RESOLVE_WALK:
        ResolveWalk(data, record, state);
        break;
    case RESOLVE_STAND:
        ResolveStand(data, record, previous, state);
        OnBodyUpdate(record->m_player, ao);
        ondistortion(record->m_player, record);
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

    if (record->m_mode == Modes::RESOLVE_AIR) {
        player->m_flPoseParameter()[2] = g_csgo.RandomInt(0, 4) * 0.25f;
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

    record->m_eye_angles.clamp();
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state) {
    data->m_body_update = record->m_anim_time + 0.34f;
    data->m_can_predict = true;
    // hi mrs.pufidori
    float speed = record->m_anim_velocity.length_2d();
    float delta = fabsf(math::NormalizedAngle(record->m_eye_angles.x - data->m_upd_time));

    if (speed < 10.f && record->m_anim_velocity.z < 10.f && record->m_flags & FL_ONGROUND) {
        record->m_eye_angles.x = record->m_body;
    }
    else {
        record->m_eye_angles.y = record->m_body;
        record->m_mode = RESOLVE_AIR;
    }
}

void Resolver::ondistortion(Player* player, LagRecord* record) {
    // i just tried somethings sorry its fucked up
    if (record->m_mode == Modes::RESOLVE_SPIN) {
        record->m_eye_angles.y += 180.0f;
    }
    else if (record->m_mode == Modes::RESOLVE_DISTORTION) {
        record->m_eye_angles.y += 90.0f;
    }
    else {
        record->m_eye_angles.y += 45.0f;
    }

    record->m_eye_angles.y = math::NormalizedAngle(record->m_eye_angles.y);

    record->m_eye_angles.x += 10.0f;
    record->m_eye_angles.y = (record->m_eye_angles.x, -89.0f, 89.0f);
}


void Resolver::ResolveLby(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state) {
    data->m_body_update = record->m_anim_time + 1.1f;
    data->m_can_predict = true;

    // jokushkahax skeet2*17dump by nigfgas
    float time = g_csgo.m_globals->m_curtime;

    float lby = state->m_time_to_align_lower_body;
    float yaw = math::NormalizedAngle(state->m_eye_yaw);

    if (time >= data->m_body_update) {
        record->m_eye_angles.y = lby;
        record->m_mode = RESOLVE_LBY;
    }
    else if (fabs(math::NormalizedAngle(lby - yaw)) < 35.f) {
        record->m_eye_angles.y = yaw;
        record->m_mode = RESOLVE_LBY;
    }
    else {
        record->m_eye_angles.y = lby;
        record->m_mode = RESOLVE_LBY;
    }
}
