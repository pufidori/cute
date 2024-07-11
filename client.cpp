#include "includes.h"

Client g_cl{ };
char username[33] = "\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x90";
// init routine.
ulong_t __stdcall Client::init(void* arg) {
	// if not in interwebz mode, the driver will not set the username.
	g_cl.m_user = XOR("cute");

	// stop here if we failed to acquire all the data needed from csgo.
	if (!g_csgo.init())
		return 0;
	g_csgo.m_engine->ExecuteClientCmd((XOR("voice_modenable 1")));
	// welcome the user.
	g_notify.add(tfm::format(XOR("Welcome.\n")));
	g_notify.add(tfm::format(XOR("cute.vip by fruitydevteam\n")));
	g_notify.add(tfm::format(XOR("made with love <3\n")));
	g_notify.add(tfm::format(XOR("build - beta " __DATE__, __TIME__"\n")));
	g_cl.UnlockHiddenConvars();

	return 1;
}

void Client::UnlockHiddenConvars()
{
	if (!g_csgo.m_cvar)
		return;

	auto p = **reinterpret_cast<ConVar***>(g_csgo.m_cvar + 0x34);
	for (auto c = p->m_next; c != nullptr; c = c->m_next) {
		c->m_flags &= ~FCVAR_DEVELOPMENTONLY;
		c->m_flags &= ~FCVAR_HIDDEN;
		c->m_flags &= ~FCVAR_CHEAT;
	}
}

void Client::DrawHUD() {

	if (g_menu.main.misc.watermark1.get() == 0) {

		// Colors
		const auto col_background = Color(15, 15, 15, 200);

		// Get current time
		time_t t = std::time(nullptr);
		std::ostringstream time_stream;
		time_stream << std::put_time(std::localtime(&t), ("%H:%M:%S"));

		// Get round trip time in milliseconds
		int ms = std::max(0, static_cast<int>(std::round(g_cl.m_latency * 1000.f)));

		// Get tickrate
		int rate = static_cast<int>(std::round(1.f / g_csgo.m_globals->m_interval));

		// Construct watermark text
		std::string text = tfm::format(XOR(" cute.vip | build: :3 | ping %ims | %s "), ms, time_stream.str().c_str());
		render::FontSize_t size = render::menu_shade.size(text);

		// Draw background
		render::rect_filled(m_width - size.m_width - 18, 10, size.m_width + 8, size.m_height + 8, col_background);
		//render::rect_filled(m_width - size.m_width - 18, 10, size.m_width + 8, 2, col_accent);
		render::gradient1337(m_width - size.m_width - 18, 10, size.m_width + 8, 2, { 189, 207, 253, 255 }, { 253, 189, 202, 255 }); // Pastel Blue to Pastel Pink
		render::gradient1337(m_width - size.m_width - 18, 10, size.m_width + 8, 2, { 253, 189, 202, 255 }, { 189, 207, 253, 255 }); // Pastel Pink to White
		render::gradient1337(m_width - size.m_width - 18, 10, size.m_width + 8, 2, { 189, 207, 253, 255 }, { 253, 189, 202, 255 }); // White to Pastel Pink
		render::gradient1337(m_width - size.m_width - 18, 10, size.m_width + 8, 2, { 253, 189, 202, 255 }, { 189, 207, 253, 255 }); // Pastel Pink to Pastel Blue
		render::gradient1337(m_width - size.m_width - 18, 10, size.m_width + 8, 2, { 189, 207, 253, 255 }, { 253, 189, 202, 255 });

		// Draw text
		render::menu_shade.string(m_width - 14, 14, { 255, 175, 220, 255 }, text, render::ALIGN_RIGHT);
	}

	else if (g_menu.main.misc.watermark1.get() == 1) {

		// get time.
		time_t t = std::time(nullptr);
		std::ostringstream time_stream;
		time_stream << std::put_time(std::localtime(&t), ("%H:%M:%S"));

		// get round trip time in milliseconds.
		int ms = std::max(0, (int)std::round(g_cl.m_latency * 1000.f));

		// get tickrate.
		int rate = (int)std::round(1.f / g_csgo.m_globals->m_interval);

		std::string text = tfm::format(XOR("cute.vip | ms:%i | %s "), ms, time_stream.str().c_str());
		render::FontSize_t size = render::hud.size(text);

		// background.
		render::rect_filled(m_width - size.m_width - 20, 10, size.m_width + 10, size.m_height + 2, { 240, 110, 140, 130 });

		// text.
		render::hud.string(m_width - 15, 10, { 240, 160, 180, 250 }, text, render::ALIGN_RIGHT);
	}

	else if (g_menu.main.misc.watermark1.get() == 2) {
		// get time.
		time_t t = std::time(nullptr);
		std::ostringstream time;
		time << std::put_time(std::localtime(&t), ("%H:%M:%S"));

		// get round trip time in milliseconds.
		int ms = std::max(0, (int)std::round(g_cl.m_latency * 1000.f));

		// get tickrate.
		int rate = (int)std::round(1.f / g_csgo.m_globals->m_interval);
		//auto nci = g_csgo.m_engine->GetNetChannelInfo();
	//	char latency_str[32];

		std::string text = tfm::format(XOR("cute.vip | lt: 1.2f + 1.5f |"));

		render::FontSize_t size = render::hud.size(text);

		// background.
		//render::rect_filled(m_width - size.m_width - 20, 10, size.m_width + 10, size.m_height + 2, { 240, 110, 140, 130 });

		// text.
		render::hud31.string(m_width - 8, 6, { 255, 255, 255, 255 }, text, render::ALIGN_RIGHT);
	}
}

void Client::KillFeed( ) {
	if( !g_menu.main.misc.killfeed.get( ) )
		return;

	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// get the addr of the killfeed.
	KillFeed_t* feed = ( KillFeed_t* ) g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
	if( !feed )
		return;

	int size = feed->notices.Count( );
	if( !size )
		return;

	for( int i{ }; i < size; ++i ) {
		NoticeText_t* notice = &feed->notices[ i ];

		// this is a local player kill, delay it.
		if( notice->fade == 1.5f )
			notice->fade = FLT_MAX;
	}
}

void Client::MotionBlur()
{
	if (!g_csgo.m_cvar)
		return;

	int value = g_menu.main.misc.motion_blur.get();
	static auto mat_motion_blur_enabled = g_csgo.m_cvar->FindVar(HASH("mat_motion_blur_enabled"));
	static auto mat_motion_blur_strength = g_csgo.m_cvar->FindVar(HASH("mat_motion_blur_strength"));
	if (value > 0) {
		mat_motion_blur_enabled->SetValue(1);
		mat_motion_blur_strength->SetValue(value);
	}
	else {
		mat_motion_blur_enabled->SetValue(0);
		mat_motion_blur_strength->SetValue(0);
	}
}

void Client::ClanTag()
{
	// lambda function for setting our clantag.
	auto SetClanTag = [&](std::string tag) -> void {
		using SetClanTag_t = int(__fastcall*)(const char*, const char*);
		static auto SetClanTagFn = pattern::find(g_csgo.m_engine_dll, XOR("53 56 57 8B DA 8B F9 FF 15")).as<SetClanTag_t>();

		SetClanTagFn(tag.c_str(), XOR("cute.vip"));
		};

	std::string szClanTag = XOR("cute.vip");
	std::string szSuffix = XOR("");
	static int iPrevFrame = 0;
	static bool bReset = false;
	int iCurFrame = ((int)(g_csgo.m_globals->m_curtime * 2.f)) % (szClanTag.size() * 2);

	if (g_menu.main.misc.clantag.get()) {
		// are we in a new frame?
		static auto is_freeze_period = false;
		if (g_csgo.m_gamerules->m_bFreezePeriod())
		{
			if (is_freeze_period)
			{
				SetClanTag("cute.vip");
			}
			is_freeze_period = false;
			return;
		}

		is_freeze_period = true;

		if (iPrevFrame != int(g_csgo.m_globals->m_curtime * 2.6) % 35) {
			switch (int(g_csgo.m_globals->m_curtime * 2.6) % 35) {
			case 0: SetClanTag(XOR("         c")); break;
			case 1: SetClanTag(XOR("        cu")); break;
			case 2: SetClanTag(XOR("       cut")); break;
			case 3: SetClanTag(XOR("      cute")); break;
			case 4: SetClanTag(XOR("     cute.")); break;
			case 5: SetClanTag(XOR("    cute.v")); break;
			case 6: SetClanTag(XOR("   cute.vi")); break;
			case 7: SetClanTag(XOR("  cute.vip")); break;
			case 8: SetClanTag(XOR(" cute.vip ")); break;
			case 9: SetClanTag(XOR("cute.vip  ")); break;
			case 10:SetClanTag(XOR("ute.vip   ")); break;
			case 11:SetClanTag(XOR("te.vip    ")); break;
			case 12:SetClanTag(XOR("e.vip     ")); break;
			case 13:SetClanTag(XOR(".vip      ")); break;
			case 14:SetClanTag(XOR("vip       ")); break;
			case 15:SetClanTag(XOR("ip        ")); break;
			case 16:SetClanTag(XOR("p         ")); break;
			case 17:SetClanTag(XOR("          ")); break;
			default:;
			}
			iPrevFrame = int(g_csgo.m_globals->m_curtime * 2.6) % 35;
		}

		// do we want to reset after untoggling the clantag?
		bReset = true;
	}
	else {
		// reset our clantag.
		if (bReset) {
			SetClanTag(XOR(""));
			bReset = false;
		}
	}
}

void Client::Skybox()
{
	static auto sv_skyname = g_csgo.m_cvar->FindVar(HASH("sv_skyname"));
	if (g_menu.main.misc.skyboxchange.get()) {
		switch (g_menu.main.misc.skybox.get()) {
		case 0: //Tibet
			//sv_skyname->SetValue("cs_tibet");
			sv_skyname->SetValue(XOR("cs_tibet"));
			break;
		case 1: //Embassy
			//sv_skyname->SetValue("embassy");
			sv_skyname->SetValue(XOR("embassy"));
			break;
		case 2: //Italy
			//sv_skyname->SetValue("italy");
			sv_skyname->SetValue(XOR("italy"));
			break;
		case 3: //Daylight 1
			//sv_skyname->SetValue("sky_cs15_daylight01_hdr");
			sv_skyname->SetValue(XOR("sky_cs15_daylight01_hdr"));
			break;
		case 4: //Cloudy
			//sv_skyname->SetValue("sky_csgo_cloudy01");
			sv_skyname->SetValue(XOR("sky_csgo_cloudy01"));
			break;
		case 5: //Night 1
			sv_skyname->SetValue(XOR("sky_csgo_night02"));
			break;
		case 6: //Night 2
			//sv_skyname->SetValue("sky_csgo_night02b");
			sv_skyname->SetValue(XOR("sky_csgo_night02b"));
			break;
		case 7: //Night Flat
			//sv_skyname->SetValue("sky_csgo_night_flat");
			sv_skyname->SetValue(XOR("sky_csgo_night_flat"));
			break;
		case 8: //Day HD
			//sv_skyname->SetValue("sky_day02_05_hdr");
			sv_skyname->SetValue(XOR("sky_day02_05_hdr"));
			break;
		case 9: //Day
			//sv_skyname->SetValue("sky_day02_05");
			sv_skyname->SetValue(XOR("sky_day02_05"));
			break;
		case 10: //Rural
			//sv_skyname->SetValue("sky_l4d_rural02_ldr");
			sv_skyname->SetValue(XOR("sky_l4d_rural02_ldr"));
			break;
		case 11: //Vertigo HD
			//sv_skyname->SetValue("vertigo_hdr");
			sv_skyname->SetValue(XOR("vertigo_hdr"));
			break;
		case 12: //Vertigo Blue HD
			//sv_skyname->SetValue("vertigoblue_hdr");
			sv_skyname->SetValue(XOR("vertigoblue_hdr"));
			break;
		case 13: //Vertigo
			//sv_skyname->SetValue("vertigo");
			sv_skyname->SetValue(XOR("vertigo"));
			break;
		case 14: //Vietnam
			//sv_skyname->SetValue("vietnam");
			sv_skyname->SetValue(XOR("vietnam"));
			break;
		case 15: //Dusty Sky
			//sv_skyname->SetValue("sky_dust");
			sv_skyname->SetValue(XOR("sky_dust"));
			break;
		case 16: //Jungle
			sv_skyname->SetValue(XOR("jungle"));
			break;
		case 17: //Nuke
			sv_skyname->SetValue(XOR("nukeblank"));
			break;
		case 18: //Office
			sv_skyname->SetValue(XOR("office"));
			//game::SetSkybox(XOR("office"));
			break;
		default:
			break;
		}
	}

	/*
	Checkbox	FogOverride; // butt
	Colorpicker	FogColor; // color
	Slider		FogStart; // slider
	Slider		FogEnd; // slider
	Slider		Fogdensity; // slider
	*/
	//g_menu.main.visuals.FogColor.get().r(), g_menu.main.visuals.FogColor.get().g(), g_menu.main.visuals.FogColor.get().b()

	float destiny = g_menu.main.visuals.Fogdensity.get() / 100.f;

	static const auto fog_enable = g_csgo.m_cvar->FindVar(HASH("fog_enable"));
	fog_enable->SetValue(1); 
	static const auto fog_override = g_csgo.m_cvar->FindVar(HASH("fog_override"));
	fog_override->SetValue(g_menu.main.visuals.FogOverride.get()); 
	static const auto fog_color = g_csgo.m_cvar->FindVar(HASH("fog_color"));
	fog_color->SetValue(std::string(std::to_string(g_menu.main.visuals.FogColor.get().r()) + " " + std::to_string(g_menu.main.visuals.FogColor.get().g()) + " " + std::to_string(g_menu.main.visuals.FogColor.get().b())).c_str()); 
	static const auto fog_start = g_csgo.m_cvar->FindVar(HASH("fog_start"));
	fog_start->SetValue(g_menu.main.visuals.FogStart.get()); 
	static const auto fog_end = g_csgo.m_cvar->FindVar(HASH("fog_end"));
	fog_end->SetValue(g_menu.main.visuals.FogEnd.get()); 
	static const auto fog_destiny = g_csgo.m_cvar->FindVar(HASH("fog_maxdensity"));
	fog_destiny->SetValue(destiny); 
}

//Spotify shit
void Client::SpotifyDisplay() {

	if (!g_menu.main.misc.whitelist.get())
		return;

	for (auto hwnd = GetTopWindow(0); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {
		if (!IsWindowVisible(hwnd))
			continue;

		const auto length = GetWindowTextLengthW(hwnd);
		if (length == 0)
			continue;

		wchar_t filename[300];
		DWORD pid{ 0 };
		GetWindowThreadProcessId(hwnd, &pid);

		const auto handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
		GetModuleFileNameExW(handle, nullptr, filename, 300);

		const auto sane_filename = std::wstring{ filename };

		int pos_y = 35;
	//	if (g_menu.main.misc.watermark1.get())
			//pos_y = 35;
		//else
			//pos_y = pos_y;

		int alpha = sin(abs(fmod(-math::pi + (g_csgo.m_globals->m_curtime * (2 / .75)), (math::pi * 2)))) * 225;
		if (alpha < 0)
			alpha = alpha * (-1);

		CloseHandle(handle);

		if (sane_filename.find(L"Spotify.exe") != std::string::npos) {
			wchar_t title[300];
			if (!GetWindowTextW(hwnd, title, 300))
				return;
			else {
				const auto sane_title = std::wstring{ title };
				std::string ascii_title = { sane_title.begin() , sane_title.end() };

				// forcing lowercase on title and artist
				std::for_each(ascii_title.begin(), ascii_title.end(), [](char& c) {
					c = ::tolower(c);
					});
				std::cout << ascii_title << std::endl;

				// note - cole; the & symbol just fuckes the pos up and idk why so we do this -_-
				std::replace(ascii_title.begin(), ascii_title.end(), '&', '-');
				std::cout << ascii_title << std::endl;

				if (sane_title.find(L"-") != std::string::npos)
					render::menu_shade.string(m_width - 4, pos_y, { 225, 225, 225, 225 }, ascii_title.c_str(), render::ALIGN_RIGHT);
				else
					render::menu_shade.string(m_width - 4, pos_y, { 225, 225, 225, alpha }, XOR("paused"), render::ALIGN_RIGHT);
			}
		}
	}
}


void Client::OnPaint() {
	// update screen size.
	g_csgo.m_engine->GetScreenSize(m_width, m_height);

	// render stuff.
	g_visuals.think();
	g_grenades.paint();
	g_notify.think();

	DrawHUD();
	KillFeed();
	SpotifyDisplay();

	g_visuals.IndicateAngles();

	events::player_say;

	// menu goes last.
	g_gui.think();
}

void Client::OnMapload() {
	// store class ids.
	g_netvars.SetupClassData();

	// createmove will not have been invoked yet.
	// but at this stage entites have been created.
	// so now we can retrive the pointer to the local player.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	// world materials.
	Visuals::ModulateWorld();

	// init knife shit.
	g_skins.load();

	g_cl.m_setupped = false;
	m_sequences.clear();

	// if the INetChannelInfo pointer has changed, store it for later.
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo();

	if (g_csgo.m_net) {
		g_hooks.m_net_channel.reset();
		g_hooks.m_net_channel.init(g_csgo.m_net);
		g_hooks.m_net_channel.add(INetChannel::PROCESSPACKET, util::force_cast(&Hooks::ProcessPacket));
		g_hooks.m_net_channel.add(INetChannel::SENDDATAGRAM, util::force_cast(&Hooks::SendDatagram));
	}
}

void update_lerp() {
	static auto cl_interp = g_csgo.m_cvar->FindVar(HASH("cl_interp"));
	static auto cl_updaterate = g_csgo.m_cvar->FindVar(HASH("cl_updaterate"));
	static auto cl_interp_ratio = g_csgo.m_cvar->FindVar(HASH("cl_interp_ratio"));

	g_cl.m_lerp = fmaxf(cl_interp->GetFloat(), cl_interp_ratio->GetFloat() / cl_updaterate->GetFloat());
}

void Client::StartMove(CUserCmd* cmd) {
	// save some usercmd stuff.
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;

	// get local ptr.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());
	if (!m_local) {
		m_setupped = false;
		return;
	}



	if (m_local->m_fFlags() & FL_FROZEN || m_local->m_iTeamNum() < 2) {
		m_setupped = false;
	}

	m_pressing_move = (m_buttons & (IN_LEFT) || m_buttons & (IN_FORWARD) || m_buttons & (IN_BACK) ||
		m_buttons & (IN_RIGHT) || m_buttons & (IN_MOVELEFT) || m_buttons & (IN_MOVERIGHT) ||
		m_buttons & (IN_JUMP));

	// store max choke
	// TODO; 11 -> m_bIsValveDS
	m_max_lag = (m_local->m_fFlags() & FL_ONGROUND) ? g_menu.main.antiaim.fakelag_limit.get() : g_menu.main.antiaim.fakelag_limit.get() - 1;
	m_lag = g_csgo.m_cl->m_choked_commands;

	update_lerp();

	m_latency = g_csgo.m_net->GetLatency(INetChannel::FLOW_OUTGOING);
	m_latency2 = g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING);

	math::clamp(m_latency, 0.f, 1.f);
	m_latency_ticks = game::TIME_TO_TICKS(m_latency);
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + m_latency_ticks;

	// processing indicates that the localplayer is valid and alive.
	m_processing = m_local && m_local->alive();
	if (!m_processing) {
		m_setupped = false;
		return;
	}

	// make sure prediction has ran on all usercommands.
	// because prediction runs on frames, when we have low fps it might not predict all usercommands.
	// also fix the tick being inaccurate.
	g_inputpred.UpdateGamePrediction(g_cl.m_cmd);

	// store some stuff about the local player.
	m_flags = m_local->m_fFlags();

	// ...
	m_shot = false;
}

void Client::BackupPlayers(bool restore) {

	// restore stuff.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!g_aimbot.IsValidTarget(player))
			continue;

		if (!player->m_BoneCache().m_pCachedBones)
			continue;


		if (restore)
			g_aimbot.m_backup[i - 1].restore(player);
		else
			g_aimbot.m_backup[i - 1].store(player);
	}
}

void Client::DoMove() {
	penetration::PenetrationOutput_t tmp_pen_data{ };

	// backup strafe angles (we need them for input prediction)
	m_strafe_angles = m_cmd->m_view_angles;

	if (!(m_flags & FL_ONGROUND) && g_input.GetKeyState(g_menu.main.misc.instant_stop_in_air.get()))
		g_aimbot.m_stop_air = true;

	if (g_aimbot.m_stop_air) {

		if (g_cl.m_local->m_vecVelocity().length_2d() > 10.f)
			g_movement.NullVelocity();
		else
			g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.f;
	}


	// run movement code before input prediction.
	g_movement.JumpRelated();
	g_movement.Strafe();
	g_movement.FakeWalk();
	g_movement.AutoStop();
	g_movement.AutoPeek(g_cl.m_cmd, m_strafe_angles.y);
	g_movement.FastStop();

	g_aimbot.m_stop_air = false;
	g_aimbot.m_stop = false;

	// predict input.
	g_inputpred.RunGamePrediction(g_cl.m_cmd);

	if (g_csgo.m_gamerules->m_bFreezePeriod() || (g_cl.m_flags & FL_FROZEN))
		return;

	g_cl.m_shoot_pos = g_aimbot.UpdateShootPosition(-10.f);

	// restore original angles after input prediction
	m_cmd->m_view_angles = m_view_angles;

	// convert viewangles to directional forward vector.
	math::AngleVectors(m_view_angles, &m_forward_dir);

	// reset shit.
	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;

	// store weapon stuff.
	m_weapon = m_local->GetActiveWeapon();

	if (m_weapon) {
		m_weapon_info = m_weapon->GetWpnData();
		m_weapon_id = m_weapon->m_iItemDefinitionIndex();
		m_weapon_type = m_weapon_info->m_weapon_type;

		// ensure weapon spread values / etc are up to date.
		if (m_weapon_type != WEAPONTYPE_GRENADE)
			m_weapon->UpdateAccuracyPenalty();

		// run autowall once for penetration crosshair if we have an appropriate weapon.
		if (m_weapon_type != WEAPONTYPE_KNIFE && m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE) {
			penetration::PenetrationInput_t in;
			in.m_from = m_local;
			in.m_target = nullptr;
			in.m_pos = m_shoot_pos + (m_forward_dir * m_weapon_info->m_range);
			in.m_damage = 1.f;
			in.m_damage_pen = 1.f;
			in.m_can_pen = true;

			// run autowall.
			penetration::run(&in, &tmp_pen_data);
		}

		g_movement.m_max_weapon_speed = g_cl.m_local->m_bIsScoped() ?
			m_weapon_info->m_max_player_speed_alt :
			m_weapon_info->m_max_player_speed;

		// set pen data for penetration crosshair.
		m_pen_data = tmp_pen_data;

		// can the player fire.
		m_player_fire = g_csgo.m_globals->m_curtime >= m_local->m_flNextAttack() && !g_csgo.m_gamerules->m_bFreezePeriod() && !(g_cl.m_flags & FL_FROZEN);

		UpdateRevolverCock();
		m_weapon_fire = CanFireWeapon();
	}






	// grenade prediction.
	g_grenades.think();

	// run fakelag.
	g_hvh.SendPacket();

	// run aimbot.
	g_aimbot.think();

	// run antiaims.
	g_hvh.AntiAim();
}

void Client::EndMove(CUserCmd* cmd) {

	OnCreateMove();

	// store this when choke cycle reset.
	if (!g_csgo.m_cl->m_choked_commands) {
		m_real_angle = m_cmd->m_view_angles;
		m_frame_shit = g_csgo.m_globals->m_tick_count;
	}

	m_cmd->m_view_angles.SanitizeAngle();

	// fix our movement.
	g_movement.FixMove(cmd, m_strafe_angles);
	g_movement.MoonWalk(cmd);




	// this packet will be sent.
	if (*m_packet) {

		g_cl.m_upd_time_test = g_csgo.m_globals->m_tick_count + 1;

		g_hvh.m_step_switch = (bool)g_csgo.RandomInt(0, 1);

		// we are sending a packet, so this will be reset soon.
		// store the old value.
		m_old_lag = m_lag;

		// get radar angles.
		m_radar = cmd->m_view_angles;
		m_radar.normalize();

	}




	// store some values for next tick.
	m_old_packet = *m_packet;
	m_old_shot = m_buttons & IN_ATTACK && g_cl.m_weapon_fire;
}

void Client::OnTick(CUserCmd* cmd) {
	// TODO; add this to the menu.
	if (g_menu.main.misc.ranks.get() && cmd->m_buttons & IN_SCORE) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll(&msg);
	}

	// store some data and update prediction.
	StartMove(cmd);

	// not much more to do here.
	if (!m_processing)
		return;

	// save the original state of players.
	BackupPlayers(false);

	// run all movement related code.
	DoMove();

	// store stome additonal stuff for next tick
	// sanetize our usercommand if needed and fix our movement.
	EndMove(cmd);

	// restore the players.
	BackupPlayers(true);

	// restore curtime/frametime
	// and prediction seed/player.
	g_inputpred.RestoreGamePrediction(g_cl.m_cmd);
}

void Client::SetAngles() {
	if (!g_cl.m_local || !g_cl.m_processing) {
		g_cl.m_updated_values = false;
		return;
	}

	// apply the rotation.
	g_cl.m_local->SetAbsAngles(ang_t(0, m_abs_yaw, 0));

	 //set radar angles.
	if (g_csgo.m_input->CAM_IsThirdPerson())
		g_csgo.m_prediction->SetLocalViewAngles(m_radar);
}

void Client::OnCreateMove() {

	if (g_cl.m_lag > 0)
		return;

	// test
	m_should_try_upd = true;

	// update time.
	m_anim_frame = g_csgo.m_globals->m_curtime - m_anim_time;
	m_anim_time = g_csgo.m_globals->m_curtime;

	// current angle will be animated.
	m_angle = g_cl.m_cmd->m_view_angles;

	if (m_flags & FL_ONGROUND) {
		if (m_local->m_vecVelocity().length_2d() > 0.1f) {
			m_body = m_angle.y;
			m_body_pred = m_anim_time + 0.22f;
		}

		// standing update every 1.1s
		else if (m_anim_time > m_body_pred) {
			m_body = m_angle.y;
			m_body_pred = m_anim_time + 1.1f;
		}
	}
}

void Client::UpdateInformation() {

	if (!g_cl.m_local || !g_cl.m_processing || !g_csgo.m_engine->IsInGame()) {
		m_has_updated = false;
		return;
	}

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	if (m_spawn_time != m_local->m_flSpawnTime()) {

		// reset animstate
		game::ResetAnimationState(state);

		// store new spawn time
		m_spawn_time = m_local->m_flSpawnTime();

		// reset this
		m_has_updated = false;
	}

	// set those so we use raw velocity
	g_cl.m_local->SetAbsVelocity(g_cl.m_local->m_vecVelocity());
	g_cl.m_local->m_iEFlags() &= ~(0x1000 | 0x800);

	// disable eye pos interpolation
	g_cl.m_local->m_fFlags() |= 0xF0;

	// null out incorrect data
	g_cl.m_local->some_ptr() = nullptr;

	if (g_csgo.m_input->CAM_IsThirdPerson())
		*reinterpret_cast<ang_t*>(uintptr_t(g_cl.m_local) + 0x31C4 + 0x4) = m_real_angle; // cancer.

	// backup global vars.
	const float backup_curtime = g_csgo.m_globals->m_curtime;
	const float backup_realtime = g_csgo.m_globals->m_realtime;
	const float backup_frametime = g_csgo.m_globals->m_frametime;
	const float backup_abs_frametime = g_csgo.m_globals->m_abs_frametime;
	const float backup_interp = g_csgo.m_globals->m_interp_amt;
	const int backup_framecount = g_csgo.m_globals->m_frame;
	const int backup_tickcount = g_csgo.m_globals->m_tick_count;

	const float time{ m_local->m_flOldSimulationTime() + g_csgo.m_globals->m_interval };
	const int ticks = game::TIME_TO_TICKS(time);

	// correct time and frametime to match server simulation.
	g_csgo.m_globals->m_curtime = time;
	g_csgo.m_globals->m_realtime = time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_abs_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frame = ticks;
	g_csgo.m_globals->m_tick_count = ticks;
	g_csgo.m_globals->m_interp_amt = 0.f;

	// set this var for later use
	m_real_update = m_local->m_flSimulationTime() != m_last_sim_time;

	if (m_real_update) {

		// get last networked layers.
		g_cl.m_local->GetAnimLayers(g_cl.m_layers);

		// store new simtime
		m_last_sim_time = m_local->m_flSimulationTime();

		// get current origin.
		vec3_t cur = m_local->m_vecOrigin();

		// get prevoius origin.
		vec3_t prev = m_net_pos.empty() ? cur : m_net_pos.front().m_pos;

		// check if we broke lagcomp.
		m_lagcomp = (cur - prev).length_sqr() > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front(g_csgo.m_globals->m_curtime, cur);
	}

	bool update_anims = m_should_try_upd && !g_menu.main.misc.sync.get() || m_real_update && g_menu.main.misc.sync.get();

	// credits: evitable
	// if time has changed, animations have updated
//	if( time != state->m_last_update_time ) {
	if (update_anims) {

		// reset this
		m_should_try_upd = false;

		if (!m_real_update)
			g_cl.m_local->GetAnimLayers(m_backup_layers);

		// set the nointerp flag.
		if (!g_menu.main.misc.interpolation.get())
			g_cl.m_local->m_fEffects() |= EF_NOINTERP;

		// remove body lean; also inair foot fix
		if (g_menu.main.misc.bodeeeelean.get()) {
			m_backup_layers[12].m_weight = g_cl.m_layers[12].m_weight = 0.f;
			if (!(m_local->m_fFlags() & FL_ONGROUND)) {
				state->feet_cycle() = 0.0f; // or any constant value that makes sense
				state->feet_yaw_rate() = 0.0f;
			}
		}

		//below is something i want to do in the future, dk if it is possible but, in air static legs.
		//g_cl.m_local->m_flPoseParameter()[PoseParam::JUMP_FALL] = 1.0f;


		// call original, bypass hook.
		g_hooks.m_bUpdatingCSALP = true;
		g_cl.m_local->UpdateClientSideAnimation();
		g_hooks.m_bUpdatingCSALP = false;

		if (g_menu.main.antiaim.allow_land.get()) {
			int value = g_menu.main.antiaim.landangle.get();

			if (state->m_landing && (m_local->m_fFlags() & FL_ONGROUND) && !state->m_dip_air && state->m_dip_cycle > 0.f)
				m_angle.x = value;
		}

		// nignog
		if (!m_real_update)
			g_cl.m_local->SetAnimLayers(m_backup_layers);

		// get last networked poses.
		// if( !g_csgo.m_cl->m_choked_commands ){
		g_cl.m_local->GetPoseParameters(g_cl.m_poses);

		// set this as true 
		m_has_updated = true;

		// store updated abs yaw.
		g_cl.m_abs_yaw = state->m_foot_yaw;

		// save updated data.
		m_rotation = g_cl.m_local->m_angAbsRotation();
		m_speed = state->m_speed;
		m_ground = state->m_ground;

		// set the nointerp flag.
		g_cl.m_local->m_fEffects() &= ~EF_NOINTERP;
	}

	// restore globals
	g_csgo.m_globals->m_realtime = backup_realtime;
	g_csgo.m_globals->m_curtime = backup_curtime;
	g_csgo.m_globals->m_frametime = backup_frametime;
	g_csgo.m_globals->m_abs_frametime = backup_abs_frametime;
	g_csgo.m_globals->m_frame = backup_framecount;
	g_csgo.m_globals->m_tick_count = backup_tickcount;
	g_csgo.m_globals->m_interp_amt = backup_interp;


	if (!m_has_updated)
		return;

	// set vars to last networked data
	m_local->SetPoseParameters(m_poses);
	m_local->SetAnimLayers(m_layers);
	m_local->SetAbsAngles(ang_t(0.f, m_abs_yaw, 0.f));

	// invalidate bone accessor and cache
	m_local->InvalidateBoneCache();

	// setup bones for current frame
	m_setupped = g_bone_handler.SetupBonesOnetap(m_local, m_local_bones, g_menu.main.misc.interpolation.get());
}


void Client::MouseFix(CUserCmd* cmd) {
	/*
	  FULL CREDITS TO:
	  - chance ( for reversing it )
	  - polak ( for having this in aimware )
	  - llama ( for having this in onetap and confirming )
	*/

	// purpose is to fix mouse dx/dy - there is a noticeable difference once fixed

	static ang_t delta_viewangles{ };
	ang_t delta = cmd->m_view_angles - delta_viewangles;

	static ConVar* sensitivity = g_csgo.m_cvar->FindVar(HASH("sensitivity"));

	if (delta.x != 0.f) {
		static ConVar* m_pitch = g_csgo.m_cvar->FindVar(HASH("m_pitch"));

		int final_dy = static_cast<int>((delta.x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		if (final_dy <= 32767) {
			if (final_dy >= -32768) {
				if (final_dy >= 1 || final_dy < 0) {
					if (final_dy <= -1 || final_dy > 0)
						final_dy = final_dy;
					else
						final_dy = -1;
				}
				else {
					final_dy = 1;
				}
			}
			else {
				final_dy = 32768;
			}
		}
		else {
			final_dy = 32767;
		}

		cmd->m_mousedy = static_cast<short>(final_dy);
	}

	if (delta.y != 0.f) {
		static ConVar* m_yaw = g_csgo.m_cvar->FindVar(HASH("m_yaw"));

		int final_dx = static_cast<int>((delta.y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		if (final_dx <= 32767) {
			if (final_dx >= -32768) {
				if (final_dx >= 1 || final_dx < 0) {
					if (final_dx <= -1 || final_dx > 0)
						final_dx = final_dx;
					else
						final_dx = -1;
				}
				else {
					final_dx = 1;
				}
			}
			else {
				final_dx = 32768;
			}
		}
		else {
			final_dx = 32767;
		}

		cmd->m_mousedx = static_cast<short>(final_dx);
	}

	delta_viewangles = cmd->m_view_angles;
}

void Client::print( const std::string text, ... ) {
	va_list     list;
	int         size;
	std::string buf;

	if( text.empty( ) )
		return;

	va_start( list, text );

	// count needed size.
	size = std::vsnprintf( 0, 0, text.c_str( ), list );

	// allocate.
	buf.resize( size );

	// print to buffer.
	std::vsnprintf( buf.data( ), size + 1, text.c_str( ), list );

	va_end( list );

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf(g_gui.m_color, XOR( "[cute] " ) );
	g_csgo.m_cvar->ConsoleColorPrintf( colors::white, buf.c_str( ) );
}

bool Client::CanFireWeapon() {

	// the player cant fire.
	if (!m_player_fire)
		return false;

	if (m_weapon_type == WEAPONTYPE_GRENADE)
		return false;

	// if we have no bullets, we cant shoot.
	if (m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1() < 1)
		return false;

	// do we have any burst shots to handle?
	if ((m_weapon_id == GLOCK || m_weapon_id == FAMAS) && m_weapon->m_iBurstShotsRemaining() > 0) {
		// new burst shot is coming out.
		if (g_csgo.m_globals->m_curtime >= m_weapon->m_fNextBurstShot())
			return true;
	}

	// r8 revolver.
	if (m_weapon_id == REVOLVER) {
		int act = m_weapon->m_Activity();

		// mouse1.
		if (!m_revolver_fire) {
			if ((act == 185 || act == 193) && m_revolver_cock == 0)
				return g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack();

			return false;
		}
	}

	// yeez we have a normal gun.
	if (g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack())
		return true;

	return false;
}

void Client::UpdateRevolverCock( ) {
	// default to false.
	m_revolver_fire = false;

	// reset properly.
	if( m_revolver_cock == -1 )
		m_revolver_cock = 0;

	// we dont have a revolver.
	// we have no ammo.
	// player cant fire
	// we are waiting for we can shoot again.
	if( m_weapon_id != REVOLVER || m_weapon->m_iClip1( ) < 1 || !m_player_fire || g_csgo.m_globals->m_curtime < m_weapon->m_flNextPrimaryAttack( ) ) {
		// reset.
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	// calculate max number of cocked ticks.
	// round to 6th decimal place for custom tickrates..
	int shoot = ( int ) ( 0.25f / ( std::round( g_csgo.m_globals->m_interval * 1000000.f ) / 1000000.f ) );

	// amount of ticks that we have to query.
	m_revolver_query = shoot - 1;

	// we held all the ticks we needed to hold.
	if( m_revolver_query == m_revolver_cock ) {
		// reset cocked ticks.
		m_revolver_cock = -1;

		// we are allowed to fire, yay.
		m_revolver_fire = true;
	}

	else {
		// we still have ticks to query.
		// apply inattack.
		if( m_revolver_query > m_revolver_cock )
			m_cmd->m_buttons |= IN_ATTACK;

		// count cock ticks.
		// do this so we can also count 'legit' ticks
		// that didnt originate from the hack.
		if( m_cmd->m_buttons & IN_ATTACK )
			m_revolver_cock++;

		// inattack was not held, reset.
		else m_revolver_cock = 0;
	}

	// remove inattack2 if cocking.
	if( m_revolver_cock > 0 )
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences( ) {
	if( !g_csgo.m_net )
		return;

	if( m_sequences.empty( ) || g_csgo.m_net->m_in_seq > m_sequences.front( ).m_seq ) {
		// store new stuff.
		m_sequences.emplace_front( g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq );
	}

	// do not save too many of these.
	while( m_sequences.size( ) > 2048 )
		m_sequences.pop_back( );
}
