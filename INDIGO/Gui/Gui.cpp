#include "Gui.h"
#include "../Font.h"

using namespace Client;

//[enc_string_enable /]
//[junk_enable /]

ImFont* icons;
ImFont* main;

bool bIsGuiInitalize = false;
bool bIsGuiVisible = false;
WNDPROC WndProc_o = nullptr;

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

LRESULT WINAPI GUI_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

CGui::CGui() {}

CGui::~CGui()
{
	ImGui_ImplDX9_Shutdown();
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


void CGui::GUI_Init(IDirect3DDevice9 * pDevice)
{
	HWND hWindow = FindWindowA("Valve001", 0);

	ImGui_ImplDX9_Init(hWindow, pDevice);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	io.IniFilename = GuiFile.c_str();

	icons = io.Fonts->AddFontFromMemoryCompressedTTF(icons_compressed_data, icons_compressed_size, 25.f);
	main = io.Fonts->AddFontFromMemoryCompressedTTF(main_compressed_data, main_compressed_size, 15.f);

	style.Alpha = 0.f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMinSize = ImVec2(200, 200);
	style.FramePadding = ImVec2(4, 2);
	style.ItemSpacing = ImVec2(6, 3);
	style.ItemInnerSpacing = ImVec2(6, 4);
	style.Alpha = 1.f;
	style.WindowRounding = 0.f;
	style.FrameRounding = 0.f;
	style.IndentSpacing = 6.0f;
	style.ItemInnerSpacing = ImVec2(3, 4);
	style.ColumnsMinSpacing = 50.0f;
	style.GrabMinSize = 14.0f;
	style.GrabRounding = 16.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 16.0f;
	style.AntiAliasedLines = true;
	style.AntiAliasedShapes = true;

	BlueSheme();

	ImGui_ImplDX9_CreateDeviceObjects();

	WndProc_o = (WNDPROC)SetWindowLongA(hWindow, GWL_WNDPROC, (LONG)(LONG_PTR)GUI_WndProc);

	bIsGuiInitalize = true;
}

void CGui::GUI_Begin_Render()
{
	ImGui_ImplDX9_NewFrame();
}

void CGui::GUI_End_Render()
{
	ImGui::Render();
}

LRESULT WINAPI GUI_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool is_down = false;
	static bool is_clicked = false;
	static bool check_closed = false;

	if (GUI_KEY_DOWN(VK_INSERT))
	{
		is_clicked = false;
		is_down = true;
	}
	else if (!GUI_KEY_DOWN(VK_INSERT) && is_down)
	{
		is_clicked = true;
		is_down = false;
	}
	else
	{
		is_clicked = false;
		is_down = false;
	}

	if (!bIsGuiVisible && !is_clicked && check_closed)
	{
		string msg = "cl_mouseenable " + to_string(!bIsGuiVisible);
		Interfaces::Engine()->ClientCmd_Unrestricted2(msg.c_str());
		check_closed = false;
	}

	if (is_clicked)
	{
		bIsGuiVisible = !bIsGuiVisible;

		string msg = "cl_mouseenable " + to_string(!bIsGuiVisible);
		Interfaces::Engine()->ClientCmd_Unrestricted2(msg.c_str());

		if (!check_closed)
			check_closed = true;
	}

	if (bIsGuiVisible && ImGui_ImplDX9_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProcA(WndProc_o, hwnd, uMsg, wParam, lParam);
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

void CGui::GUI_Draw_Elements()
{
	if (!bIsGuiInitalize || Interfaces::Engine()->IsTakingScreenshot() || !Interfaces::Engine()->IsActiveApp())
		return;

	g_pGui->GUI_Begin_Render();

	ImGui::GetIO().MouseDrawCursor = bIsGuiVisible;

	bool bOpenTimer = (bIsGuiVisible || (bC4Timer && iC4Timer));

	g_pGui->MainFont();
	if (Settings::Misc::misc_Spectators) g_pMisc->OnRenderSpectatorList();

	if (g_pEsp && Settings::Esp::esp_BombTimer && bOpenTimer)
	{
		ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;

		ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);

		ImGui::SetNextWindowSize(ImVec2(125.f, 30.f));

		ImGui::PushFont(main);

		if (ImGui::Begin("Bomb Timer", &bOpenTimer,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::Text(C4_TIMER_STRING, g_pEsp->fC4Timer);
			ImGui::End();
		}

		ImGui::GetStyle().WindowMinSize = OldMinSize;
	}


	if (Settings::Misc::misc_KnifeSkinChanger && bIsGuiVisible)
	{
		ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;

		ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);

		ImGui::SetNextWindowSize(ImVec2(500.f, 280.f));
		ImGui::PushFont(main);

		if (ImGui::Begin("Weapon Changer", &Settings::Misc::misc_KnifeSkinChanger,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
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

			static int skinKnifePages = 0;
			if (ImGui::Button("Skin", ImVec2(235.f, 25.0f)))
			{
				skinKnifePages = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Knife", ImVec2(235.f, 25.0f)))
			{
				skinKnifePages = 1;
			}

			if (skinKnifePages == 0)
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

				ImGui::ComboBoxArray(WeaponSkin.c_str(), &iWeaponSelectSkinIndex, WeaponSkins[iWeaponID].SkinNames);

				ImGui::Combo("Weapon Quality", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
				ImGui::SliderFloat("Weapon Wear", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear, 0.001f, 1.f);
				ImGui::InputInt("Weapon StatTrak", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak, 1, 100, ImGuiInputTextFlags_CharsDecimal);

				ImGui::Separator();

				ImGui::Combo("Gloves Skin", &Settings::Skin::gloves_skin, gloves_listbox_items,
					IM_ARRAYSIZE(gloves_listbox_items));

				ImGui::Separator();

				ImGui::PopItemWidth();

				if (ImGui::Button("Apply##Skin"))
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
			}
			if (skinKnifePages == 1)
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

				ImGui::Combo("Knife CT Model", &Settings::Skin::knf_ct_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));
				ImGui::Combo("Knife TT Model", &Settings::Skin::knf_tt_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));

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

				ImGui::SliderFloat("Knife CT Wear", &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear, 0.001f, 1.f);
				ImGui::Combo("Knife CT Quality", &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
				ImGui::ComboBoxArray(KnifeCTModel.c_str(), &iSelectKnifeCTSkinIndex, KnifeSkins[iKnifeCTModelIndex].SkinNames);

				ImGui::Separator();

				ImGui::SliderFloat("Knife TT Wear", &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear, 0.001f, 1.f);
				ImGui::Combo("Knife TT Quality", &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
				ImGui::ComboBoxArray(KnifeTTModel.c_str(), &iSelectKnifeTTSkinIndex, KnifeSkins[iKnifeTTModelIndex].SkinNames);

				if (ImGui::Button("Apply##Skin"))
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
			ImGui::End();
		}

		ImGui::GetStyle().WindowMinSize = OldMinSize;
	}


	if (Settings::Misc::misc_ProfileChanger && bIsGuiVisible)
	{
		ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;

		ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);
		ImGui::SetNextWindowSize(ImVec2(550.f, 320.f));
		ImGui::PushFont(main);

		if (ImGui::Begin("Profile Changer", &Settings::Misc::misc_ProfileChanger,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
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

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			static int profilePages = 0;
			if (ImGui::Button("Commend/Level/Rank", ImVec2(270.f, 25.0f)))
			{
				profilePages = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Medal", ImVec2(270, 25.0f)))
			{
				profilePages = 1;
			}

			if (profilePages == 0)
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
				ImGui::PushItemWidth(460.f);
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
			if (profilePages == 1)
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushItemWidth(460.f);
				static int medal_id = 0;
				if (ImGui::Button("Add") && medal_id != 0) {
					Settings::InventoryChanger::medals.insert(Settings::InventoryChanger::medals.end(), medal_id);
					medal_id = 0;
				}
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
				if (ImGui::Button("Apply##Medals")) {
					SendClientHello();

				}
			}

			ImGui::End();
		}

		ImGui::GetStyle().WindowMinSize = OldMinSize;
	}

	if (Settings::Misc::misc_ClanEdit && bIsGuiVisible)
	{
		ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;

		ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);

		ImGui::SetNextWindowSize(ImVec2(500.f, 250.f));
		ImGui::PushFont(main);

		if (ImGui::Begin("Clan Changer", &Settings::Misc::misc_ClanEdit,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
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

			

			ImGui::End();
		}

		ImGui::GetStyle().WindowMinSize = OldMinSize;
	}



	if (bIsGuiVisible)
	{
		int pX, pY;
		Interfaces::InputSystem()->GetCursorPosition(&pX, &pY);

		ImGui::GetIO().MousePos.x = (float)pX;
		ImGui::GetIO().MousePos.y = (float)pY;

		Client::OnRenderGUI();
	}

	g_pGui->GUI_End_Render();
}


void CGui::BlueSheme()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;

	style.ScrollbarSize = 0.0f;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.29f, 0.29f, 0.29f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.59f, 0.59f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.39f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.35f, 0.35f, 0.35f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


}

void CGui::MainFont()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;
	ImGui::PushFont(main);
	style.ScrollbarSize = 0.0f;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.29f, 0.29f, 0.29f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.59f, 0.59f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.39f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.35f, 0.35f, 0.35f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void CGui::IconsFont()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;
	ImGui::PushFont(icons);
	style.ScrollbarSize = 0.0f;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.29f, 0.29f, 0.29f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.59f, 0.59f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.39f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.35f, 0.35f, 0.35f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	IMGUI_API bool ComboBoxArray(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		const ImVec2 itemSpacing = style.ItemSpacing;
		const ImVec4 color = style.Colors[ImGuiCol_Button];
		const ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		const ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
		const ImVec4 colorText = style.Colors[ImGuiCol_Text];
		style.ItemSpacing.x = 2.5;
		style.ItemSpacing.y = 1;
		const ImVec4 colorSelectedTab = ImVec4(color.x, color.y, color.z, color.w*0.5f);
		const ImVec4 colorSelectedTabHovered = ImVec4(colorHover.x, colorHover.y, colorHover.z, colorHover.w*0.5f);
		const ImVec4 colorSelectedTabText = ImVec4(colorText.x*0.8f, colorText.y*0.8f, colorText.z*0.8f, colorText.w*0.8f);

		if (tabSize>0 && (tabIndex<0 || tabIndex >= tabSize))
		{
			if (!tabOrder)
			{
				tabIndex = 0;
			}
			else
			{
				tabIndex = -1;
			}
		}

		float windowWidth = 0.f, sumX = 0.f;
		windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);

		const bool isMMBreleased = ImGui::IsMouseReleased(2);
		int justClosedTabIndex = -1, newtabIndex = tabIndex;

		bool selection_changed = false; bool noButtonDrawn = true;

		for (int j = 0, i; j < tabSize; j++)
		{
			i = tabOrder ? tabOrder[j] : j;
			if (i == -1) continue;

			if (sumX > 0.f)
			{
				sumX += style.ItemSpacing.x;
				sumX += ImGui::CalcTextSize(tabLabels[i]).x + 2.f*style.FramePadding.x;

				if (sumX>windowWidth)
				{
					sumX = 0.f;
				}
			}

			if (i != tabIndex)
			{
				// Push the style
				style.Colors[ImGuiCol_Button] = colorSelectedTab;
				style.Colors[ImGuiCol_ButtonActive] = colorSelectedTab;
				style.Colors[ImGuiCol_ButtonHovered] = colorSelectedTabHovered;
				style.Colors[ImGuiCol_Text] = colorSelectedTabText;
			}
			// Draw the button
			ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
			if (ImGui::Button(tabLabels[i], ImVec2(65.f, 67.f))) { selection_changed = (tabIndex != i); newtabIndex = i; }
			ImGui::PopID();
			if (i != tabIndex)
			{
				// Reset the style
				style.Colors[ImGuiCol_Button] = color;
				style.Colors[ImGuiCol_ButtonActive] = colorActive;
				style.Colors[ImGuiCol_ButtonHovered] = colorHover;
				style.Colors[ImGuiCol_Text] = colorText;
			}
			noButtonDrawn = false;

			if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

		}

		tabIndex = newtabIndex;

		// Change selected tab when user closes the selected tab
		if (tabIndex == justClosedTabIndex && tabIndex >= 0)
		{
			tabIndex = -1;
			for (int j = 0, i; j < tabSize; j++)
			{
				i = tabOrder ? tabOrder[j] : j;
				if (i == -1)
				{
					continue;
				}
				tabIndex = i;
				break;
			}
		}

		// Restore the style
		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		style.Colors[ImGuiCol_Text] = colorText;
		style.ItemSpacing = itemSpacing;

		return selection_changed;
	}
}