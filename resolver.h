
#pragma once

class ShotRecord;

class Resolver {
public:
	enum Modes : size_t {
		RESOLVE_DISABLED = 0,
		RESOLVE_WALK,
		RESOLVE_PREFLICK,
		RESOLVE_STAND,
		RESOLVE_LBY,
		RESOLVE_SPIN,
		RESOLVE_DISTORTION,
		RESOLVE_STAND1,
		RESOLVE_STAND2,
		RESOLVE_BODY,
		RESOLVE_AIR,
		RESOLVE_FLICK,
		RESOLVE_FAKEFLICK,
		RESOLVE_STOPPED_MOVING,
		RESOLVE_OVERRIDE,
		RESOLVE_LASTMOVE,
		RESOLVE_UNKNOWM,
		RESOLVE_BRUTEFORCE,
	};

public:
	LagRecord* FindIdealRecord(AimPlayer* data);
	LagRecord* FindLastRecord(AimPlayer* data);

	//LagRecord* FindFirstRecord(AimPlayer* data);

	void OnBodyUpdate(Player* player, float value);
	float GetAwayAngle(LagRecord* record);

	void MatchShot(AimPlayer* data, LagRecord* record);
	void SetMode(LagRecord* record);

	void ResolveAngles(Player* player, LagRecord* record);
	void AntiFreestand(LagRecord* record, float& current_yaw, float multiplier);
	void ResolveWalk(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state);
	void ResolveStand(AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state);
	void ResolveLby(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state);
	void ondistortion(Player* player, LagRecord* record);
	void ResolveAir(AimPlayer* data, LagRecord* record, CCSGOPlayerAnimState* state);
	void ResolvePoses(Player* player, LagRecord* record);

public:
	std::array< vec3_t, 64 > m_impacts;
	int	   iPlayers[64];
	bool   m_step_switch;
	int    m_random_lag;
	float  m_next_random_update;
	float  m_random_angle;
	float  m_direction;
	float  m_auto;
	float  m_auto_dist;
	float  m_auto_last;
	float  m_view;

	class PlayerResolveRecord
	{
	public:
		struct AntiFreestandingRecord
		{
			int right_damage = 0, left_damage = 0;
			float right_fraction = 0.f, left_fraction = 0.f;
		};

	public:
		AntiFreestandingRecord m_sAntiEdge;
	};

	PlayerResolveRecord player_resolve_records[33];
};

extern Resolver g_resolver;