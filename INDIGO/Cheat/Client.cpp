#include "Client.h"
#include "../Font.h"
#include "../Gui/Gui.h"
#include <ctime>

//[enc_string_enable /]
//[junk_enable /]


namespace Client
{
	//[swap_lines]
	int	iScreenWidth = 0;
	int	iScreenHeight = 0;

	string BaseDir = "C:/AidanVIP/Configs/";
	string LogFile = "";
	string GuiFile = "";
	string IniFile = "";

	vector<string> ConfigList;

	Vector2D	g_vCenterScreen = Vector2D(0.f, 0.f);

	CPlayers*	g_pPlayers = nullptr;
	CRender*	g_pRender = nullptr;
	CGui*		g_pGui = nullptr;

	CAimbot*	g_pAimbot = nullptr;
	CTriggerbot* g_pTriggerbot = nullptr;
	CEsp*		g_pEsp = nullptr;
	CRadar*		g_pRadar = nullptr;
	CKnifebot*	g_pKnifebot = nullptr;
	CSkin*		g_pSkin = nullptr;
	CMisc*		g_pMisc = nullptr;

	CInventoryChanger* g_pInventoryChanger = nullptr;
	CInventoryChanger1* g_pInventoryChanger1 = nullptr;

	bool		bC4Timer = false;
	int			iC4Timer = 40;

	int			iWeaponID = 0;
	int			iWeaponSelectIndex = WEAPON_DEAGLE;
	int			iWeaponSelectSkinIndex = -1;
	//[/swap_lines]

	void ReadConfigs(LPCTSTR lpszFileName)
	{
		if (!strstr(lpszFileName, "gui.ini"))
		{
			ConfigList.push_back(lpszFileName);
		}
	}

	void RefreshConfigs()
	{
		ConfigList.clear();
		string ConfigDir = "C:/AidanVIP/Configs/*.ini";
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}

	bool SendClientHello()
	{
		CMsgClientHello Message;

		Message.set_client_session_need(1);
		Message.clear_socache_have_versions();

		void* ptr = malloc(Message.ByteSize() + 8);

		if (!ptr)
			return false;

		((uint32_t*)ptr)[0] = k_EMsgGCClientHello | ((DWORD)1 << 31);
		((uint32_t*)ptr)[1] = 0;

		Message.SerializeToArray((void*)((DWORD)ptr + 8), Message.ByteSize());

		bool result = Interfaces::SteamGameCoordinator()->SendMessage(k_EMsgGCClientHello | ((DWORD)1 << 31), ptr, Message.ByteSize() + 8) == k_EGCResultOK;

		free(ptr);

		return result;
	}

	bool SendMMHello()
	{
		CMsgGCCStrike15_v2_MatchmakingClient2GCHello Message;
		void* ptr = malloc(Message.ByteSize() + 8);
		if (!ptr)
			return false;

		auto unMsgType = k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31);
		((uint32_t*)ptr)[0] = unMsgType;
		((uint32_t*)ptr)[1] = 0;

		Message.SerializeToArray((void*)((DWORD)ptr + 8), Message.ByteSize());

		bool result = Interfaces::SteamGameCoordinator()->SendMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31), ptr, Message.ByteSize() + 8) == k_EGCResultOK;

		free(ptr);
		return result;
	}

	bool Initialize(IDirect3DDevice9* pDevice)
	{
		g_pPlayers = new CPlayers();
		g_pRender = new CRender(pDevice);
		g_pGui = new CGui();

		g_pAimbot = new CAimbot();
		g_pEsp = new CEsp();
		g_pSkin = new CSkin();
		g_pMisc = new CMisc();

		g_pInventoryChanger = new CInventoryChanger();
		g_pInventoryChanger1 = new CInventoryChanger1();

		GuiFile = BaseDir + "\\" + "gui.ini";
		IniFile = BaseDir + "\\" + "settings.ini";

		g_pSkin->InitalizeSkins();

		Settings::LoadSettings(IniFile);

		iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

		g_pGui->GUI_Init(pDevice);

		RefreshConfigs();
		
		
		Interfaces::Engine()->ClientCmd_Unrestricted2("clear");
		Interfaces::Engine()->ClientCmd_Unrestricted2("toggleconsole");
		Sleep(1000);

		Interfaces::Engine()->ClientCmd_Unrestricted2("echo -------------------------------------------------");
		Interfaces::Engine()->ClientCmd_Unrestricted2("echo aidan.vip release v1.0");
		Interfaces::Engine()->ClientCmd_Unrestricted2("echo pastebin.com/c032L28c for changelog");
		Interfaces::Engine()->ClientCmd_Unrestricted2("echo -------------------------------------------------");
		return true;
	}

	void Shutdown()
	{
		DELETE_MOD(g_pPlayers);
		DELETE_MOD(g_pRender);
		DELETE_MOD(g_pGui);

		DELETE_MOD(g_pAimbot);
		DELETE_MOD(g_pTriggerbot);
		DELETE_MOD(g_pEsp);
		DELETE_MOD(g_pRadar);
		DELETE_MOD(g_pKnifebot);
		DELETE_MOD(g_pSkin);
		DELETE_MOD(g_pMisc);
	}


	void OnRender()
	{
		if (g_pRender && !Interfaces::Engine()->IsTakingScreenshot() && Interfaces::Engine()->IsActiveApp())
		{
			g_pRender->BeginRender();

			if (g_pGui)
				g_pGui->GUI_Draw_Elements();

			Interfaces::Engine()->GetScreenSize(iScreenWidth, iScreenHeight);

			g_vCenterScreen.x = iScreenWidth / 2.f;
			g_vCenterScreen.y = iScreenHeight / 2.f;




			if (Settings::Esp::esp_Watermark)
			{
				//bool rainbow; 
				static float rainbow;
				rainbow += 0.005f;
				if (rainbow > 1.f) rainbow = 0.f;
				g_pRender->Text(15, 15, false, true, Color::FromHSB(rainbow, 1.f, 1.f), WATER_MARK);
			}

			if (Settings::Esp::esp_Cheatbuild)
				g_pRender->Text(15, 45, false, true, Color::White(), "Last cheat compile: %s : %s", __DATE__, __TIME__);


			{
				if (g_pEsp)
					g_pEsp->OnRender();

				if (g_pMisc)
				{
					g_pMisc->OnRender();
					g_pMisc->OnRenderSpectatorList();
				}
			}

			std::time_t result = std::time(nullptr);

			if (Settings::Esp::esp_Time)
				g_pRender->Text(15, 30, false, true, Color::White(), std::asctime(std::localtime(&result)));

			g_pRender->EndRender();
		}
	}

	void OnLostDevice()
	{
		if (g_pRender)
			g_pRender->OnLostDevice();

		if (g_pGui)
			ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void OnResetDevice()
	{
		if (g_pRender)
			g_pRender->OnResetDevice();

		if (g_pGui)
			ImGui_ImplDX9_CreateDeviceObjects();
	}

	void OnRetrieveMessage(void* ecx, void* edx, uint32_t *punMsgType, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize)
	{
		g_pInventoryChanger->PostRetrieveMessage(punMsgType, pubDest, cubDest, pcubMsgSize);
		g_pInventoryChanger1->PostRetrieveMessage(punMsgType, pubDest, cubDest, pcubMsgSize);
	}

	void OnSendMessage(void* ecx, void* edx, uint32_t unMsgType, const void* pubData, uint32_t cubData)
	{

		void* pubDataMutable = const_cast<void*>(pubData);
		g_pInventoryChanger->PreSendMessage(unMsgType, pubDataMutable, cubData);
		g_pInventoryChanger1->PreSendMessage(unMsgType, pubDataMutable, cubData);
	}

	void OnCreateMove(CUserCmd* pCmd)
	{
		if (g_pPlayers && Interfaces::Engine()->IsInGame())
		{
			g_pPlayers->Update();

			if (g_pEsp)
				g_pEsp->OnCreateMove(pCmd);

			if (IsLocalAlive())
			{
				if (!bIsGuiVisible)
				{
					int iWeaponSettingsSelectID = GetWeaponSettingsSelectID();

					if (iWeaponSettingsSelectID >= 0)
						iWeaponID = iWeaponSettingsSelectID;
				}

				if (g_pAimbot)
					g_pAimbot->OnCreateMove(pCmd, g_pPlayers->GetLocal());

				if (g_pMisc)
					g_pMisc->OnCreateMove(pCmd);

				backtracking->legitBackTrack(pCmd);

			}
		}
	}

	void OnFireEventClientSideThink(IGameEvent* pEvent)
	{
		if (!strcmp(pEvent->GetName(), "player_connect_full") ||
			!strcmp(pEvent->GetName(), "round_start") ||
			!strcmp(pEvent->GetName(), "cs_game_disconnected"))
		{
			if (g_pPlayers)
				g_pPlayers->Clear();

			if (g_pEsp)
				g_pEsp->OnReset();
		}

		if (Interfaces::Engine()->IsConnected())
		{
			hitmarker::singleton()->initialize();

			if (g_pEsp)
				g_pEsp->OnEvents(pEvent);

			if (g_pSkin)
				g_pSkin->OnEvents(pEvent);
		}
	}

	void OnFrameStageNotify(ClientFrameStage_t Stage)
	{
		if (Interfaces::Engine()->IsInGame() && Interfaces::Engine()->IsConnected())
		{
			/*
			ConVar* sv_cheats = Interfaces::GetConVar()->FindVar("sv_cheats");
			SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
			sv_cheats_spoofed->SetInt(1);
			*/

			if (g_pMisc)
				g_pMisc->FrameStageNotify(Stage);

			Skin_OnFrameStageNotify(Stage);
			Gloves_OnFrameStageNotify(Stage);
		}
	}

	void OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t &state,
		const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
	{
		if (Interfaces::Engine()->IsInGame() && ctx && pCustomBoneToWorld)
		{
			if (g_pEsp)
				g_pEsp->OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

			if (g_pMisc)
				g_pMisc->OnDrawModelExecute();
		}
	}

	void OnPlaySound(const Vector* pOrigin, const char* pszSoundName)
	{
		if (!pszSoundName || !Interfaces::Engine()->IsInGame())
			return;

		if (!strstr(pszSoundName, "bulletLtoR") &&
			!strstr(pszSoundName, "rics/ric") &&
			!strstr(pszSoundName, "impact_bullet"))
		{
			if (g_pEsp && IsLocalAlive() && Settings::Esp::esp_Sound && pOrigin)
			{
				if (!GetVisibleOrigin(*pOrigin))
					g_pEsp->SoundEsp.AddSound(*pOrigin);
			}
		}
	}

	void OnPlaySound(const char* pszSoundName)
	{
		if (g_pMisc)
			g_pMisc->OnPlaySound(pszSoundName);
	}

	void OnOverrideView(CViewSetup* pSetup)
	{
		if (g_pMisc)
			g_pMisc->OnOverrideView(pSetup);
	}

	void OnGetViewModelFOV(float& fov)
	{
		if (g_pMisc)
			g_pMisc->OnGetViewModelFOV(fov);
	}


	void DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
	{
		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

		Color colColor(0, 0, 0, 255);

		flRainbow += flSpeed;
		if (flRainbow > 1.f) flRainbow = 0.f;

		for (int i = 0; i < width; i++)
		{
			float hue = (1.f / (float)width) * i;
			hue -= flRainbow;
			if (hue < 0.f) hue += 1.f;

			Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
			windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height), colRainbow.GetU32());
		}
	}

	void OnRenderGUI()
	{
		ImGui::SetNextWindowSize(ImVec2(570.f, 375.f));

		g_pGui->MainFont();
		if (ImGui::Begin(CHEAT_NAME, &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			static float flRainbow;
			float flSpeed = 0.0005f;
			int curWidth = 1;
			ImVec2 curPos = ImGui::GetCursorPos();
			ImVec2 curWindowPos = ImGui::GetWindowPos();
			curPos.x += curWindowPos.x;
			curPos.y += curWindowPos.y;
			int size;
			int y;
			Interfaces::Engine()->GetScreenSize(y, size);
			DrawRectRainbow(curPos.x - 10, curPos.y - 5, ImGui::GetWindowSize().x + size, curPos.y + -4, flSpeed, flRainbow);

			ImGui::BeginGroup();
			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType > 1)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType = 1;

			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit > 1)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit = 1;

			if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot > 5)
				Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot = 5;

			g_pGui->IconsFont();
			const char* tabNames[] = {
				"!" , "@" ,
				"%" , "$" , "#" };

			static int tabOrder[] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
			static int tabSelected = 0;
			const bool tabChanged = ImGui::TabLabels(tabNames,
				sizeof(tabNames) / sizeof(tabNames[0]),
				tabSelected, tabOrder);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			float SpaceLineOne = 120.f;
			float SpaceLineTwo = 220.f;
			float SpaceLineThr = 320.f;

			ImGui::EndGroup();
			ImGui::SameLine();

			g_pGui->MainFont();

			if (tabSelected == 0) // Aimbot
			{
				static int aimPages = 0;
				ImGui::BeginGroup();
				ImGui::BeginChild(1, ImVec2(485, 338), true);
				{
					if (ImGui::Button("Main", ImVec2(230.0f, 25.0f)))
					{
						aimPages = 0;
					}
					ImGui::SameLine();
					if (ImGui::Button("Controls", ImVec2(230.0f, 25.0f)))
					{
						aimPages = 1;
					}

					if (aimPages == 0)
					{
						const char* iHitSound[] =
						{
							"Off",
							"Default",
							"Anime",
							"Bameware",
							"Gamesense",
						};

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Deathmatch", &Settings::Aimbot::aim_Deathmatch);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("WallAttack", &Settings::Aimbot::aim_WallAttack);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("CheckSmoke", &Settings::Aimbot::aim_CheckSmoke);

						ImGui::Checkbox("AntiJump", &Settings::Aimbot::aim_AntiJump);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Draw Fov", &Settings::Aimbot::aim_DrawFov);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("DrawSpot", &Settings::Aimbot::aim_DrawSpot);

						ImGui::Checkbox("Backtrack", &Settings::Aimbot::aim_Backtrack);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Draw Ticks", &Settings::Aimbot::aim_DrawBacktrack);
						ImGui::SliderInt("Ticks", &Settings::Aimbot::aim_Backtracktickrate, 1, 12);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Hitmarker", &Settings::Esp::esp_HitMarker);
						ImGui::Combo("##HITSOUND", &Settings::Esp::esp_HitMarkerSound, iHitSound, ARRAYSIZE(iHitSound));
						ImGui::SameLine();
						ImGui::Text("Sound");

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
					}
					if (aimPages == 1)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(110.f);
						ImGui::Text("Current Weapon: ");
						ImGui::SameLine();
						ImGui::Combo("##AimWeapon", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Active", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active);

						if (iWeaponID <= 9)
						{
							ImGui::SameLine();
							ImGui::Checkbox("Autopistol", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol);
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(362.f);
						ImGui::SliderInt("Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth, 1, 300);
						ImGui::SliderInt("Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov, 1, 300);
						ImGui::PopItemWidth();

						const char* AimFovType[] = { "Dynamic" , "Static" };
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("Fov Type", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType, AimFovType, IM_ARRAYSIZE(AimFovType));

						ImGui::PopItemWidth();

						const char* Aimspot[] = { "Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" };
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("Aimspot", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot, Aimspot, IM_ARRAYSIZE(Aimspot));
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(362.f);
						ImGui::SliderInt("Shot Delay", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay, 0, 200);
						ImGui::SliderInt("RCS", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs, 0, 100);
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (iWeaponID >= 10 && iWeaponID <= 30)
						{
							ImGui::PushItemWidth(362.f);
							ImGui::SliderInt("RCS Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov, 1, 300);
							ImGui::SliderInt("RCS Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth, 1, 300);
							ImGui::PopItemWidth();

							const char* ClampType[] = { "All Target" , "Shot" , "Shot + Target" };
							ImGui::PushItemWidth(362.f);
							ImGui::Combo("Clamp", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType, ClampType, IM_ARRAYSIZE(ClampType));
							ImGui::PopItemWidth();

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
						}
					}
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			else if (tabSelected == 1) // Visuals
			{
				const char* skybox_items[] =
				{
					"cs_baggage_skybox_",
					"cs_tibet",
					"embassy",
					"italy",
					"jungle",
					"nukeblank",
					"office",
					"sky_cs15_daylight01_hdr",
					"sky_cs15_daylight02_hdr",
					"sky_cs15_daylight03_hdr",
					"sky_cs15_daylight04_hdr",
					"sky_csgo_cloudy01",
					"sky_csgo_night02",
					"sky_csgo_night02b",
					"sky_csgo_night_flat",
					"sky_day02_05_hdr",
					"sky_day02_05",
					"sky_dust",
					"sky_l4d_rural02_ldr",
					"sky_venice",
					"vertigo_hdr",
					"vertigoblue_hdr",
					"vertigo",
					"vietnam"
					"amethyst",
					"bartuc_canyon",
					"bartuc_grey_sky",
					"blue1",
					"blue2",
					"blue3",
					"blue5",
					"blue6",
					"cssdefault",
					"dark1",
					"dark2",
					"dark3",
					"dark4",
					"dark5",
					"extreme_glaciation",
					"green1",
					"green2",
					"green3",
					"green4",
					"green5",
					"greenscreen",
					"greysky",
					"night1",
					"night2",
					"night3",
					"night4",
					"night5",
					"orange1",
					"orange2",
					"orange3",
					"orange4",
					"orange5",
					"orange6",
					"persistent_fog",
					"pink1",
					"pink2",
					"pink3",
					"pink4",
					"pink5",
					"polluted_atm",
					"toxic_atm",
					"water_sunset﻿",
				};

				const char* material_items[] =
				{
					"Glass",
					"Crystal",
					"Gold",
					"Dark Chrome",
					"Plastic Glass",
					"Velvet",
					"Crystal Blue",
					"Detailed Gold"
				};

				const char* armtype_items[] =
				{
					"Arms Only",
					"Arms + Weapon"
				};
				static int visPages = 0;

				ImGui::BeginGroup();
				ImGui::BeginChild(1, ImVec2(485, 338), true);
				{
					if (ImGui::Button("Main", ImVec2(230.3f, 25.0f)))
					{
						visPages = 0;
					}
					ImGui::SameLine();
					if (ImGui::Button("Colours", ImVec2(230.3f, 25.0f)))
					{
						visPages = 1;
					}

					if (visPages == 0)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						string style_1 = "Box";
						string style_2 = "Corner Box";
						string style_3 = "None";

						const char* items1[] = { style_1.c_str() , style_2.c_str() , style_3.c_str() };

						ImGui::PushItemWidth(339.f);
						ImGui::Combo("Type", &Settings::Esp::esp_Style, items1, IM_ARRAYSIZE(items1));
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Team", &Settings::Esp::esp_Team);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Enemy", &Settings::Esp::esp_Enemy);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Bomb", &Settings::Esp::esp_Bomb);
						ImGui::SameLine(SpaceLineThr);
						ImGui::Checkbox("Line", &Settings::Esp::esp_Line);

						ImGui::Checkbox("Outline", &Settings::Esp::esp_Outline);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Name", &Settings::Esp::esp_Name);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Rank", &Settings::Esp::esp_Rank);
						ImGui::SameLine(SpaceLineThr);
						ImGui::Checkbox("Weapon", &Settings::Esp::esp_Weapon);

						ImGui::Checkbox("Ammo", &Settings::Esp::esp_Ammo);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Distance", &Settings::Esp::esp_Distance);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Skeleton", &Settings::Esp::esp_Skeleton);
						ImGui::SameLine(SpaceLineThr);
						if (ImGui::Checkbox("Nades", &Settings::Esp::esp_GrenadePrediction))
						{
							if (!Interfaces::Engine()->IsInGame())
								Settings::Esp::esp_GrenadePrediction = false;
						}
						ImGui::Checkbox("Chicken", &Settings::Esp::esp_Chicken);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Fish", &Settings::Esp::esp_Fish);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Radar", &Settings::Esp::esp_Radar);
						ImGui::SameLine(SpaceLineThr);
						ImGui::Checkbox("Left Hand Knife", &Settings::Misc::misc_LeftHandKnife);
						
						ImGui::Checkbox("Defusing", &Settings::Esp::isdefusing);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						string hpbar_1 = "None";
						string hpbar_2 = "Number";
						string hpbar_3 = "Bottom";
						string hpbar_4 = "Left";

						const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
						ImGui::Combo("Esp Health", &Settings::Esp::esp_Health, items3, IM_ARRAYSIZE(items3));

						string arbar_1 = "None";
						string arbar_2 = "Number";
						string arbar_3 = "Bottom";
						string arbar_4 = "Right";

						const char* items4[] = { arbar_1.c_str() , arbar_2.c_str() , arbar_3.c_str() , arbar_4.c_str() };
						ImGui::Combo("Esp Armor", &Settings::Esp::esp_Armor, items4, IM_ARRAYSIZE(items4));

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing(); 

						string visible_1 = "Enemy";
						string visible_2 = "Team";
						string visible_3 = "All";
						string visible_4 = "Only Visible";

						const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

						ImGui::PushItemWidth(339.f);
						ImGui::Combo("Visible", &Settings::Esp::esp_Visible, items2, IM_ARRAYSIZE(items2));

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						string chams_1 = "None";
						string chams_2 = "Flat";
						string chams_3 = "Texture";

						const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
						ImGui::Combo("Chams", &Settings::Esp::esp_Chams, items5, IM_ARRAYSIZE(items5));
						ImGui::Combo("Chams Visible", &Settings::Esp::esp_ChamsVisible, items2, IM_ARRAYSIZE(items2));

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Chams Materials", &Settings::Misc::misc_ChamsMaterials);
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("##CHAMSMATERIALS", &Settings::Misc::misc_ChamsMaterialsList, material_items, ARRAYSIZE(material_items));

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Text("Effects (can cause crashes/not responding)");

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Columns(2, nullptr, false);

						ImGui::Checkbox("Chrome World", &Settings::Esp::esp_ChromeWorld);
						ImGui::Checkbox("Minecraft World", &Settings::Esp::esp_MinecraftMode);
						ImGui::Checkbox("Snow", &Settings::Misc::misc_Snow);

						ImGui::NextColumn();

						ImGui::Checkbox("LSD World", &Settings::Esp::esp_LSDMode);
						ImGui::Checkbox("Night Mode", &Settings::Esp::esp_NightMode);
						ImGui::Checkbox("Disable Postprocess", &Settings::Misc::misc_EPostprocess);

						ImGui::PushItemWidth(425.f);
						ImGui::Columns(1, nullptr, false);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(380.f);
						ImGui::Checkbox("Asus Walls", &Settings::Esp::esp_AsusWalls);
						ImGui::SliderInt("Walls Opacity", &Settings::Esp::esp_WallsOpacity, 0, 100);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						
						ImGui::PushItemWidth(425.f);
						ImGui::Text("Skybox");
						ImGui::SameLine();
						if (ImGui::Combo("", &Settings::Misc::misc_CurrentSky, skybox_items, IM_ARRAYSIZE(skybox_items)))
						{
							Settings::Misc::misc_SkyName = skybox_items[Settings::Misc::misc_CurrentSky];
						}

					}
					if (visPages == 1)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Text("ESP");

						ImGui::Separator();
						ImGui::ColorEdit3("Esp Color Hit Marker", Settings::Esp::esp_HitMarkerColor);
						ImGui::ColorEdit3("Esp Color CT", Settings::Esp::esp_Color_CT);
						ImGui::ColorEdit3("Esp Color TT", Settings::Esp::esp_Color_TT);
						ImGui::ColorEdit3("Esp Color Visible CT", Settings::Esp::esp_Color_VCT);
						ImGui::ColorEdit3("Esp Color Visible TT", Settings::Esp::esp_Color_VTT);
						ImGui::Spacing();
						ImGui::Separator();

						ImGui::Text("Chams");

						ImGui::Separator();
						ImGui::ColorEdit3("Chams Color CT", Settings::Esp::chams_Color_CT);
						ImGui::ColorEdit3("Chams Color TT", Settings::Esp::chams_Color_TT);
						ImGui::ColorEdit3("Chams Color Visible CT", Settings::Esp::chams_Color_VCT);
						ImGui::ColorEdit3("Chams Color Visible TT", Settings::Esp::chams_Color_VTT);
						
					}
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			else if (tabSelected == 2) // Misc
			{
				ImGui::BeginGroup();
				ImGui::BeginChild(1, ImVec2(485, 338), true);
				{
					
						ImGui::Checkbox("Auto Accept", &Settings::Misc::misc_AutoAccept);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("No Flash", &Settings::Misc::misc_NoFlash);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("No Smoke", &Settings::Misc::misc_NoSmoke);
						ImGui::SameLine(SpaceLineThr);
						ImGui::Checkbox("No Hands", &Settings::Misc::misc_NoHands);
						ImGui::Checkbox("Wire Hands", &Settings::Misc::misc_WireHands);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Chat Spam", &Settings::Misc::misc_spamregular);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Spectators", &Settings::Misc::misc_Spectators);
						ImGui::SameLine(SpaceLineThr);
						ImGui::Checkbox("Sniper Crosshair", &Settings::Misc::misc_AwpAim);
						ImGui::Checkbox("Punch", &Settings::Misc::misc_Punch);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("Bhop", &Settings::Misc::misc_Bhop);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("Random Spam", &Settings::Misc::misc_spamrandom);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(390.f);

						ImGui::Checkbox("Fov Changer", &Settings::Misc::misc_FovChanger);
						ImGui::PushItemWidth(390.f);
						ImGui::SliderInt("View", &Settings::Misc::misc_FovView, 1, 170);
						ImGui::SliderInt("Model View", &Settings::Misc::misc_FovModelView, 1, 190);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(350.f);

						string clan_1 = "None";
						string clan_2 = "Clear";
						string clan_3 = "aidan.vip";
						string clan_4 = "aidan.vip No-name";
						string clan_5 = "Valve";
						string clan_6 = "Valve No-name";
						string clan_7 = "Animation";
						const char* items5[] = { clan_1.c_str() , clan_2.c_str() , clan_3.c_str() , clan_4.c_str() , clan_5.c_str() , clan_6.c_str() , clan_7.c_str() };
						ImGui::Combo("Clan Changer", &Settings::Misc::misc_Clan, items5, IM_ARRAYSIZE(items5));

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						static char misc_CustomClanTag[128] = { 0 };
						ImGui::PushItemWidth(350.f);
						ImGui::InputText("Custom Clan Tag", misc_CustomClanTag, 128);
						ImGui::PopItemWidth();
						if (ImGui::Button("Apply"))
						{
							Engine::ClanTagApply(misc_CustomClanTag);
						}
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			else if (tabSelected == 3)
			{
				ImGui::BeginGroup();
				ImGui::BeginChild(1, ImVec2(485, 338), true);
				static int profilePages = 0;
				{
					if (ImGui::Button("Skin", ImVec2(112.f, 25.0f)))
					{
						profilePages = 0;
					}
					ImGui::SameLine();
					if (ImGui::Button("Knife", ImVec2(112.f, 25.0f)))
					{
						profilePages = 1;
					}
					ImGui::SameLine();
					if (ImGui::Button("Profile", ImVec2(112.f, 25.0f)))
					{
						profilePages = 2;
					}
					ImGui::SameLine();
					if (ImGui::Button("Medal", ImVec2(112.f, 25.0f)))
					{
						profilePages = 3;
					}

					if (profilePages == 0)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						const char* quality_items[] =
						{
							"Normal","Genuine","Vintage","Unusual","Community","Developer",
							"Self-Made","Customized","Strange","Completed","Tournament"
						};

						const char* gloves_listbox_items[25] =
						{
							"default",
							"bloodhound_black_silver","bloodhound_snakeskin_brass","bloodhound_metallic","handwrap_leathery",
							"handwrap_camo_grey","slick_black","slick_military","slick_red","sporty_light_blue","sporty_military",
							"handwrap_red_slaughter","motorcycle_basic_black","motorcycle_mint_triangle","motorcycle_mono_boom",
							"motorcycle_triangle_blue","specialist_ddpat_green_camo","specialist_kimono_diamonds_red",
							"specialist_emerald_web","specialist_orange_white","handwrap_fabric_orange_camo","sporty_purple",
							"sporty_green","bloodhound_guerrilla","slick_snakeskin_yellow"
						};
						//[enc_string_enable /]


						ImGui::Text("Current Weapon: %s", pWeaponData[iWeaponID]);

						ImGui::PushItemWidth(362.f);

						static int iOldWeaponID = -1;

						ImGui::Combo("Weapon##WeaponSelect", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));

						iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

						if (iOldWeaponID != iWeaponID)
							iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

						iOldWeaponID = iWeaponID;

						string WeaponSkin = pWeaponData[iWeaponID];
						WeaponSkin += " Skin";

						if (ImGui::ComboBoxArray(WeaponSkin.c_str(), &iWeaponSelectSkinIndex, WeaponSkins[iWeaponID].SkinNames))
						{
							if (Settings::Esp::esp_GrenadePrediction)
							{
								Settings::Esp::esp_GrenadePrediction = false;
								if (iWeaponSelectSkinIndex >= 0) {
									g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];
									ForceFullUpdate();
								}
								Settings::Esp::esp_GrenadePrediction = true;
							}
							else
							{
								if (iWeaponSelectSkinIndex >= 0) {
									g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];
								}
								ForceFullUpdate();
							}
						}

						if (ImGui::Combo("Weapon Quality", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items)))
						{
							ForceFullUpdate();
						}
						if (ImGui::SliderFloat("Weapon Wear", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear, 0.001f, 1.f))
						{
							ForceFullUpdate();
						}
						ImGui::InputInt("Weapon StatTrak", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak, 1, 100, ImGuiInputTextFlags_CharsDecimal);

						ImGui::Separator();

						if (ImGui::Combo("Gloves Skin", &Settings::Skin::gloves_skin, gloves_listbox_items,
							IM_ARRAYSIZE(gloves_listbox_items)))
						{
							ForceFullUpdate();
						}
					}
					if (profilePages == 1)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						const char* knife_models_items[] =
						{
							"Default","Bayonet","Flip","Gut","Karambit" ,"M9 Bayonet",
							"Huntsman","Falchion","Bowie","Butterfly","Shadow Daggers"
						};

						const char* quality_items[] =
						{
							"Normal","Genuine","Vintage","Unusual","Community","Developer",
							"Self-Made","Customized","Strange","Completed","Tournament"
						};

						if (ImGui::Combo("Knife CT Model", &Settings::Skin::knf_ct_model, knife_models_items, IM_ARRAYSIZE(knife_models_items)))
						{
							ForceFullUpdate();
						}
						if (ImGui::Combo("Knife TT Model", &Settings::Skin::knf_tt_model, knife_models_items, IM_ARRAYSIZE(knife_models_items)))
						{
							ForceFullUpdate();
						}

						ImGui::Separator();

						static int iSelectKnifeCTSkinIndex = -1;
						static int iSelectKnifeTTSkinIndex = -1;

						int iKnifeCTModelIndex = Settings::Skin::knf_ct_model;
						int iKnifeTTModelIndex = Settings::Skin::knf_tt_model;

						static int iOldKnifeCTModelIndex = -1;
						static int iOldKnifeTTModelIndex = -1;

						if (iOldKnifeCTModelIndex != iKnifeCTModelIndex && Settings::Skin::knf_ct_model)
							iSelectKnifeCTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, false);

						if (iOldKnifeTTModelIndex != iKnifeTTModelIndex && Settings::Skin::knf_tt_model)
							iSelectKnifeTTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, true);

						iOldKnifeCTModelIndex = iKnifeCTModelIndex;
						iOldKnifeTTModelIndex = iKnifeTTModelIndex;

						string KnifeCTModel = knife_models_items[Settings::Skin::knf_ct_model];
						string KnifeTTModel = knife_models_items[Settings::Skin::knf_tt_model];

						KnifeCTModel += " Skin##KCT";
						KnifeTTModel += " Skin##KTT";

						if (ImGui::SliderFloat("Knife CT Wear", &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear, 0.001f, 1.f))
						{
							ForceFullUpdate();
						}
						if (ImGui::Combo("Knife CT Quality", &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items)))
						{
							ForceFullUpdate();
						}
						if (ImGui::ComboBoxArray(KnifeCTModel.c_str(), &iSelectKnifeCTSkinIndex, KnifeSkins[iKnifeCTModelIndex].SkinNames))
						{
							if (Settings::Esp::esp_GrenadePrediction)
							{
								Settings::Esp::esp_GrenadePrediction = false;
								if (iSelectKnifeCTSkinIndex >= 0 && Settings::Skin::knf_ct_model > 0) {
									Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
									ForceFullUpdate();
								}
								else
								{
									Settings::Skin::knf_ct_skin = 0;
									ForceFullUpdate();
								}

								if (iSelectKnifeTTSkinIndex >= 0 && Settings::Skin::knf_tt_model > 0) {
									Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
									ForceFullUpdate();
								}
								else
								{
									Settings::Skin::knf_tt_skin = 0;
									ForceFullUpdate();
								}
								Settings::Esp::esp_GrenadePrediction = true;
							}
							else
							{
								if (iSelectKnifeCTSkinIndex >= 0 && Settings::Skin::knf_ct_model > 0) {
									Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
								}
								else
								{
									Settings::Skin::knf_ct_skin = 0;
								}

								if (iSelectKnifeTTSkinIndex >= 0 && Settings::Skin::knf_tt_model > 0) {
									Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
								}
								else
								{
									Settings::Skin::knf_tt_skin = 0;
								}
								ForceFullUpdate();
							}
						}

						ImGui::Separator();

						if (ImGui::SliderFloat("Knife TT Wear", &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear, 0.001f, 1.f))
						{
							ForceFullUpdate();
						}
						if (ImGui::Combo("Knife TT Quality", &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items)))
						{
							ForceFullUpdate();
						}
						if (ImGui::ComboBoxArray(KnifeTTModel.c_str(), &iSelectKnifeTTSkinIndex, KnifeSkins[iKnifeTTModelIndex].SkinNames))
						{
							if (Settings::Esp::esp_GrenadePrediction)
							{
								Settings::Esp::esp_GrenadePrediction = false;
								if (iSelectKnifeCTSkinIndex >= 0 && Settings::Skin::knf_ct_model > 0) {
									Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
									ForceFullUpdate();
								}
								else
								{
									Settings::Skin::knf_ct_skin = 0;
									ForceFullUpdate();
								}

								if (iSelectKnifeTTSkinIndex >= 0 && Settings::Skin::knf_tt_model > 0) {
									Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
									ForceFullUpdate();
								}
								else
								{
									Settings::Skin::knf_tt_skin = 0;
									ForceFullUpdate();
								}
								Settings::Esp::esp_GrenadePrediction = true;
							}
							else
							{
								if (iSelectKnifeCTSkinIndex >= 0 && Settings::Skin::knf_ct_model > 0) {
									Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
								}
								else
								{
									Settings::Skin::knf_ct_skin = 0;
								}

								if (iSelectKnifeTTSkinIndex >= 0 && Settings::Skin::knf_tt_model > 0) {
									Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
								}
								else
								{
									Settings::Skin::knf_tt_skin = 0;
								}
								ForceFullUpdate();
							}
						}
					}

					if (profilePages == 2)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						
						const char* ranklist[] =
						{
							"OFF",
							"Silver I",
							"Silver II",
							"Silver III",
							"Silver IV",
							"Silver Elite",
							"Silver Elite Master",

							"Gold Nova I",
							"Gold Nova II",
							"Gold Nova III",
							"Gold Nova Master",
							"Master Guardian I",
							"Master Guardian II",

							"Master Guardian Elite",
							"Distinguished Master Guardian",
							"Legendary Eagle",
							"Legendary Eagle Master",
							"Supreme Master First Class",
							"Global Elite"

						};
						ImGui::PushItemWidth(362.f);
						ImGui::SliderInt("Level (1-40)", &Settings::Misc::misc_Level, 1, 40);
						ImGui::Combo(("Rank"), &Settings::Misc::misc_Rank_Combo, ranklist, ARRAYSIZE(ranklist));
						if (Settings::Misc::misc_Rank_Combo >= 1)
						{
							Settings::Misc::misc_Rank = Settings::Misc::misc_Rank_Combo;
						}
						else
						{
							Settings::Misc::misc_Rank = 0;
						}
						ImGui::InputInt("XP", &Settings::Misc::misc_XP);
						ImGui::InputInt("Wins", &Settings::Misc::misc_Wins);
						ImGui::Separator();
						ImGui::InputInt("Friendly", &Settings::Misc::misc_Friendly);
						ImGui::InputInt("Leader", &Settings::Misc::misc_Leader);
						ImGui::InputInt("Teacher", &Settings::Misc::misc_Teacher);

						ImGui::Spacing();
						if (ImGui::Button("Apply"))
						{
							SendMMHello();
						}

					}
					if (profilePages == 3)
					{
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(362.f);
						static int medal_id = 0;
						
						ImGui::InputInt("Medal ID", &medal_id);

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
						ImGui::ListBoxHeader("Medal list");
						for (int m = 0; m < Settings::InventoryChanger::medals.size(); m++) {
							if (ImGui::Selectable(std::to_string(Settings::InventoryChanger::medals[m]).c_str())) {
								if (Settings::InventoryChanger::equipped_medal == Settings::InventoryChanger::medals[m]) {
									Settings::InventoryChanger::equipped_medal = 0;
									Settings::InventoryChanger::equipped_medal_override = false;
								}
								Settings::InventoryChanger::medals.erase(Settings::InventoryChanger::medals.begin() + m);
							}
						}
						ImGui::ListBoxFooter();
						ImGui::PopStyleColor();

						ImGui::Checkbox("Equipped Medal", &Settings::InventoryChanger::equipped_medal_override);
						if (Settings::InventoryChanger::equipped_medal_override) {
							static int equipped_medal = 0;
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
							if (ImGui::Combo("Equipped Medal", &equipped_medal, [](void* data, int idx, const char** out_text)
							{
								*out_text = std::to_string(Settings::InventoryChanger::medals[idx]).c_str();
								return true;
							}, nullptr, Settings::InventoryChanger::medals.size(), 5)) {
								Settings::InventoryChanger::equipped_medal = Settings::InventoryChanger::medals[equipped_medal];
							}
							ImGui::PopStyleColor();
						}
						ImGui::Spacing();
						if (ImGui::Button("Add") && medal_id != 0) {
							Settings::InventoryChanger::medals.insert(Settings::InventoryChanger::medals.end(), medal_id);
							medal_id = 0;
						}
						if (ImGui::Button("Apply##Medals")) {
							SendClientHello();

						}
					}
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			else if (tabSelected == 4) // Config
			{
				ImGui::BeginGroup();
				ImGui::BeginChild(1, ImVec2(485, 338), true);
				{
					static int iConfigSelect = 0;
					static int iMenuSheme = 1;
					static char ConfigName[64] = { 0 };

					ImGui::ComboBoxArray("Select Config", &iConfigSelect, ConfigList);

					ImGui::Separator();

					if (ImGui::Button("Load Config"))
					{
						Settings::LoadSettings("C:/AidanVIP/Configs/" + ConfigList[iConfigSelect]);
						SendMMHello();
					}
					ImGui::SameLine();
					if (ImGui::Button("Save Config"))
					{
						Settings::SaveSettings("C:/AidanVIP/Configs/" + ConfigList[iConfigSelect]);
					}
					ImGui::SameLine();
					if (ImGui::Button("Refresh Config List"))
					{
						RefreshConfigs();
					}

					ImGui::Separator();

					ImGui::InputText("Config Name", ConfigName, 64);

					if (ImGui::Button("Create & Save new Config"))
					{
						string ConfigFileName = ConfigName;

						if (ConfigFileName.size() < 1)
						{
							ConfigFileName = "settings";
						}

						Settings::SaveSettings("C:/AidanVIP/Configs/" + ConfigFileName + ".ini");
						RefreshConfigs();
					}

					ImGui::Separator();

					if (ImGui::Button("Crash Ayyware"))
					{
						ConVar* Name = Interfaces::GetConVar()->FindVar("name");
						*(int*)((DWORD)&Name->fnChangeCallback + 0xC) = 0;

						Name->SetValue("GETCRASHEDAYY?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????");
					}
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
		}
		ImGui::End();
	}
}