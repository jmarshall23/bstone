/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include <map>
#include "3d_def.h"
#include "jm_lzh.h"


void CA_CacheScreen(
    int16_t chunk);

void VH_UpdateScreen();
void DrawHighScores();
void ClearMemory();

void DrawTopInfo(
    sp_type type);

void PreloadUpdate(
    uint16_t current,
    uint16_t total);

void INL_GetJoyDelta(
    uint16_t joy,
    int16_t* dx,
    int16_t* dy);

bool LoadTheGame(
    const std::string& file_name);

bool IN_CheckAck();


//
// End game message
//

char EndGameStr[] = { "    End current game?\n"
                      " Are you sure (Y or N)?" };


#define ENDGAMESTR (EndGameStr)

char QuitToDosStr[] = { "      Quit to DOS?\n"
                        " Are you sure (Y or N)?" };

#define FREEFONT(fontnum) { if (fontnum != STARTFONT + 2 && grsegs[fontnum]) { UNCACHEGRCHUNK(fontnum); } }


static const char* const CURGAME =
    "   Continuing past this\n"
    "  point will end the game\n"
    " you're currently playing.\n"
    "\n"
    " Start a NEW game? (Y/N)";

static const char* const GAMESVD =
    "There's already a game\n"
    "saved at this position.\n"
    "\n"
    "    Overwrite? (Y/N)";


bool EscPressed = false;

int16_t lastmenumusic;

int16_t MENUSONG = 0;
int16_t ROSTER_MUS = 0;
int16_t TEXTSONG = 0;


// ===========================================================================
//
// PRIVATE PROTOTYPES
//
// ===========================================================================

void CP_ReadThis(
    int16_t temp1);

void CP_OrderingInfo(
    int16_t temp1);

void DrawEpisodePic(
    int16_t w);

void DrawAllSoundLights(
    int16_t which);

void ReadGameNames();
void FreeMusic();

void CP_GameOptions(
    int16_t temp1);

void DrawGopMenu();
void CalibrateJoystick();
void ExitGame();

void CP_Switches(
    int16_t temp1);

void DrawSwitchMenu();

void DrawAllSwitchLights(
    int16_t which);

void DrawSwitchDescription(
    int16_t which);

// BBi
void cp_sound_volume(
    int16_t);

void cp_video(
    int16_t);
// BBi


extern bool refresh_screen;


// ===========================================================================
//
// LOCAL DATA...
//
// ===========================================================================

CP_iteminfo MainItems = { MENU_X, MENU_Y, 12, MM_READ_THIS, 0, 9, { 77, 1, 154, 9, 1 } };
CP_iteminfo GopItems = { MENU_X, MENU_Y + 30, 5, 0, 0, 9, { 77, 1, 154, 9, 1 } };
CP_iteminfo SndItems = { SM_X, SM_Y, 6, 0, 0, 7, { 87, -1, 144, 7, 1 } };
CP_iteminfo LSItems = { LSM_X, LSM_Y, 10, 0, 0, 8, { 86, -1, 144, 8, 1 } };
CP_iteminfo CtlItems = { CTL_X, CTL_Y, 7, -1, 0, 9, { 87, 1, 174, 9, 1 } };
CP_iteminfo CusItems = { CST_X, CST_Y + 7, 6, -1, 0, 15, { 54, -1, 203, 7, 1 } };
CP_iteminfo NewEitems = { NE_X, NE_Y, 6, 0, 0, 16, { 43, -2, 119, 16, 1 } };
CP_iteminfo NewItems = { NM_X, NM_Y, 4, 1, 0, 16, { 60, -2, 105, 16, 1 } };
CP_iteminfo SwitchItems = { MENU_X, 0, 0, 0, 0, 9, { 87, -1, 132, 7, 1 } };

// BBi
CP_iteminfo video_items = { MENU_X, MENU_Y + 30, 1, 0, 0, 9, { 77, -1, 154, 7, 1 } };
// BBi


CP_itemtype MainMenu[] = {
    { AT_ENABLED, "NEW MISSION", CP_NewGame, static_cast<uint8_t>(COAL_FONT) },
    { AT_READIT, "ORDERING INFO", CP_OrderingInfo },
    { AT_READIT, "INSTRUCTIONS", CP_ReadThis },
    { AT_ENABLED, "STORY", CP_BlakeStoneSaga },
    { AT_DISABLED, "", 0 },
    { AT_ENABLED, "GAME OPTIONS", CP_GameOptions },
    { AT_ENABLED, "HIGH SCORES", CP_ViewScores },
    { AT_ENABLED, "LOAD MISSION", reinterpret_cast<void (*)(int16_t)>(CP_LoadGame) },
    { AT_DISABLED, "SAVE MISSION", reinterpret_cast<void (*)(int16_t)>(CP_SaveGame) },
    { AT_DISABLED, "", 0 },
    { AT_ENABLED, "BACK TO DEMO", CP_ExitOptions },
    { AT_ENABLED, "LOGOFF", 0 }
};

CP_itemtype GopMenu[] = {
    // BBi
    { AT_ENABLED, "VIDEO", cp_video },
    // BBi

    { AT_ENABLED, "SOUND", CP_Sound },

    // BBi
    { AT_ENABLED, "SOUND VOLUME", cp_sound_volume },
    // BBi

    { AT_ENABLED, "CONTROLS", CP_Control },
    { AT_ENABLED, "SWITCHES", CP_Switches }
};

CP_itemtype SndMenu[] = {
    { AT_ENABLED, "NONE", 0 },
    { AT_ENABLED, "ADLIB/SOUND BLASTER", 0 },
    { AT_DISABLED, "", 0 },
    { AT_DISABLED, "", 0 },
    { AT_ENABLED, "NONE", 0 },
    { AT_ENABLED, "ADLIB/SOUND BLASTER", 0 }
};

CP_itemtype CtlMenu[] = {
    { AT_DISABLED, "MOUSE ENABLED", 0 },
    { AT_DISABLED, "JOYSTICK ENABLED", 0 },
    { AT_DISABLED, "USE JOYSTICK PORT 2", 0 },
    { AT_DISABLED, "GRAVIS GAMEPAD ENABLED", 0 },
    { AT_DISABLED, "CALIBRATE JOYSTICK", 0 },
    { AT_DISABLED, "MOUSE SENSITIVITY", MouseSensitivity },
    { AT_ENABLED, "CUSTOMIZE CONTROLS", CustomControls }
};

CP_itemtype SwitchMenu[] = {
    { AT_ENABLED, "LIGHTING", 0 },
    { AT_ENABLED, "REBA ATTACK INFO", 0 },
    { AT_ENABLED, "SHOW CEILINGS", 0 },
    { AT_ENABLED, "SHOW FLOORS", 0 },

    // BBi
    { AT_ENABLED, "NO WALL HIT SOUND", 0 },
    { AT_ENABLED, "MODERN CONTROLS", 0 },
    { AT_ENABLED, "ALWAYS RUN", 0 },
    { AT_ENABLED, "HEART BEAT SOUND", 0 },
    { AT_ENABLED, "ROTATED AUTOMAP", 0 },
};


CP_itemtype NewEmenu[] = {
    { AT_ENABLED, "MISSION 1:\n"
      "STAR INSTITUTE", 0 },

    { AT_NON_SELECTABLE, "MISSION 2:\n"
      "FLOATING FORTRESS", 0 },

    { AT_NON_SELECTABLE, "MISSION 3:\n"
      "UNDERGROUND NETWORK", 0 },

    { AT_NON_SELECTABLE, "MISSION 4:\n"
      "STAR PORT", 0 },

    { AT_NON_SELECTABLE, "MISSION 5:\n"
      "HABITAT II", 0 },

    { AT_NON_SELECTABLE, "MISSION 6:\n"
      "SATELLITE DEFENSE", 0 },
};

CP_itemtype NewMenu[] = {
    { AT_ENABLED, "LEVEL 1:\n"
      "NOVICE AGENT", 0 },
    { AT_ENABLED, "LEVEL 2:\n"
      "SKILLED AGENT", 0 },
    { AT_ENABLED, "LEVEL 3:\n"
      "EXPERT AGENT", 0 },
    { AT_ENABLED, "LEVEL 4:\n"
      "VETERAN AGENT", 0 }
};

CP_itemtype LSMenu[] = {
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 }
};

CP_itemtype CusMenu[] = {
    { AT_ENABLED, "", 0 },
    { AT_DISABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_DISABLED, "", 0 },
    { AT_ENABLED, "", 0 },
    { AT_ENABLED, "", 0 }
};

// BBi
CP_itemtype video_menu[] = {
    { AT_ENABLED, "TOGGLE WIDESCREEN", nullptr },
};
// BBi


int16_t color_hlite[] = {
    HIGHLIGHT_DISABLED_COLOR,
    HIGHLIGHT_TEXT_COLOR,
    READHCOLOR,
    HIGHLIGHT_DEACTIAVED_COLOR,
};

int16_t color_norml[] = {
    DISABLED_TEXT_COLOR,
    ENABLED_TEXT_COLOR,
    READCOLOR,
    DEACTIAVED_TEXT_COLOR,
};

int16_t EpisodeSelect[6] = {
    1,
    0,
    0,
    0,
    0,
    0,
};

int16_t SaveGamesAvail[10];
int16_t StartGame;
int16_t SoundStatus = 1;
int16_t pickquick;
char SaveGameNames[10][GAME_DESCRIPTION_LEN + 1];

static uint8_t menu_background_color = 0x00;


static const std::string& get_saved_game_base_name()
{
    static auto base_name = std::string();
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        base_name = "bstone_";

        if (::is_aog_sw()) {
            base_name += "aog_sw";
        } else if (::is_aog_full()) {
            base_name += "aog_full";
        } else if (::is_ps()) {
            base_name += "ps";
        } else {
            throw std::runtime_error("Invalid game type.");
        }

        base_name += "_saved_game_";
    }

    return base_name;
}

////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////

#ifndef CACHE_KEY_DATA

using ScanCodes = std::vector<ScanCode>;
using ScanNames = std::vector<std::string>;

// Scan code names with single chars
static ScanNames scan_names = {
    "?", "?", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "?", "?",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "|", "?", "A", "S",
    "D", "F", "G", "H", "J", "K", "L", ";", "\"", "?", "?", "?", "Z", "X", "C", "V",
    "B", "N", "M", ",", ".", "/", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?", "\xF", "?", "-", "\x15", "5", "\x11", "+", "?",
    "\x13", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
}; // scan_names

// Scan codes with >1 char names
static ScanCodes ext_scan_codes = {
    ScanCode::sc_escape,
    ScanCode::sc_backspace,
    ScanCode::sc_tab,
    ScanCode::sc_control,
    ScanCode::sc_left_shift,
    ScanCode::sc_space,
    ScanCode::sc_caps_lock,
    ScanCode::sc_f1,
    ScanCode::sc_f2,
    ScanCode::sc_f3,
    ScanCode::sc_f4,
    ScanCode::sc_f5,
    ScanCode::sc_f6,
    ScanCode::sc_f7,
    ScanCode::sc_f8,
    ScanCode::sc_f9,
    ScanCode::sc_f10,
    ScanCode::sc_f11,
    ScanCode::sc_f12,
    ScanCode::sc_scroll_lock,
    ScanCode::sc_return,
    ScanCode::sc_right_shift,
    ScanCode::sc_print_screen,
    ScanCode::sc_alt,
    ScanCode::sc_home,
    ScanCode::sc_page_up,
    ScanCode::sc_end,
    ScanCode::sc_page_down,
    ScanCode::sc_insert,
    ScanCode::sc_delete,
    ScanCode::sc_num_lock,
    ScanCode::sc_up_arrow,
    ScanCode::sc_down_arrow,
    ScanCode::sc_left_arrow,
    ScanCode::sc_right_arrow,
    ScanCode::sc_none,
}; // ExtScanCodes

static ScanNames ext_scan_names = {
    "ESC", "BKSP", "TAB", "CTRL", "LSHFT", "SPACE", "CAPSLK", "F1",
    "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
    "F10", "F11", "F12", "SCRLK", "ENTER", "RSHFT", "PRTSC", "ALT",
    "HOME", "PGUP", "END", "PGDN", "INS", "DEL", "NUMLK", "UP",
    "DOWN", "LEFT", "RIGHT", "",
}; // ext_scan_names

#else

uint8_t* ScanNames, * ExtScanNames;
ScanCode* ExtScanCodes;

#endif

using SpecialKeys = std::vector<ScanCode>;

static SpecialKeys special_keys = {
    ScanCode::sc_back_quote,
    ScanCode::sc_equals,
    ScanCode::sc_minus,
    ScanCode::sc_l,
    ScanCode::sc_p,
    ScanCode::sc_m,
    ScanCode::sc_s,
    ScanCode::sc_i,
    ScanCode::sc_q,
    ScanCode::sc_w,
    ScanCode::sc_e,
    ScanCode::sc_return,
    ScanCode::sc_1,
    ScanCode::sc_2,
    ScanCode::sc_3,
    ScanCode::sc_4,
    ScanCode::sc_5,
    ScanCode::sc_tab,
}; // special_keys


// BBi
namespace {


using BindsNames = std::map<ScanCode, const char*>;


enum BindsFindDirection {
    e_bfd_forward,
    e_bfd_backward
}; // BindsFindDirection

enum BindsFindFrom {
    e_bff_current,
    e_bff_not_current
}; // BindingFindFrom


class BindsItem {
public:
    std::string name;
    int name_width;
    Binding* binding;

    BindsItem(
        std::string new_name = std::string(),
        int new_name_width = 0,
        Binding* new_binding = nullptr) :
            name(new_name),
            name_width(new_name_width),
            binding(new_binding)
    {
    }
}; // BindsItem

using BindsItems = std::vector<BindsItem>;

static BindsItems binds = {
    { "MOVEMENT", 0, nullptr, },
    { "FORWARD", 0, &in_bindings[e_bi_forward], },
    { "BACKWARD", 0, &in_bindings[e_bi_backward], },
    { "LEFT", 0, &in_bindings[e_bi_left], },
    { "RIGHT", 0, &in_bindings[e_bi_right], },
    { "STRAFE", 0, &in_bindings[e_bi_strafe], },
    { "STRAFE LEFT", 0, &in_bindings[e_bi_strafe_left], },
    { "STRAFE RIGHT", 0, &in_bindings[e_bi_strafe_right], },
    { "QUICK LEFT", 0, &in_bindings[e_bi_quick_left], },
    { "QUICK RIGHT", 0, &in_bindings[e_bi_quick_right], },
    { "TURN AROUND", 0, &in_bindings[e_bi_turn_around], },
    { "RUN", 0, &in_bindings[e_bi_run], },
    { "", 0, nullptr, },

    { "WEAPONS", 0, nullptr, },
    { "ATTACK", 0, &in_bindings[e_bi_attack], },
    { "AUTO CHARGE PISTOL", 0, &in_bindings[e_bi_weapon_1], },
    { "SLOW FIRE PROTECTOR", 0, &in_bindings[e_bi_weapon_2], },
    { "RAPID ASSAULT WEAPON", 0, &in_bindings[e_bi_weapon_3], },
    { "DUAL NEUTRON DISRUPTOR", 0, &in_bindings[e_bi_weapon_4], },
    { "PLASMA DISCHARGE UNIT", 0, &in_bindings[e_bi_weapon_5], },
    { "ANTI-PLASMA CANNON (PS)", 0, &in_bindings[e_bi_weapon_6], },
    { "FISSION DETONATOR (PS)", 0, &in_bindings[e_bi_weapon_7], },
    { "", 0, nullptr, },

    { "INTERACTION", 0, nullptr, },
    { "USE", 0, &in_bindings[e_bi_use], },
    { "", 0, nullptr, },

    { "HUD", 0, nullptr, },
    { "STATS", 0, &in_bindings[e_bi_stats], },
    { "MAGNIFY RADAR (PS)", 0, &in_bindings[e_bi_radar_magnify], },
    { "MINIFY RADAR (PS)", 0, &in_bindings[e_bi_radar_minify], },
    { "", 0, nullptr, },

    { "MENU", 0, nullptr, },
    { "HELP", 0, &in_bindings[e_bi_help], },
    { "SAVE", 0, &in_bindings[e_bi_save], },
    { "LOAD", 0, &in_bindings[e_bi_load], },
    { "SOUND OPTIONS", 0, &in_bindings[e_bi_sound], },
    { "CONTROLS", 0, &in_bindings[e_bi_controls], },
    { "END GAME", 0, &in_bindings[e_bi_end_game], },
    { "QUICK SAVE", 0, &in_bindings[e_bi_quick_save], },
    { "QUICK LOAD", 0, &in_bindings[e_bi_quick_load], },
    { "QUICK EXIT", 0, &in_bindings[e_bi_quick_exit], },
    { "", 0, nullptr, },

    { "OPTIONS", 0, nullptr, },
    { "ATTACK INFO", 0, &in_bindings[e_bi_attack_info], },
    { "LIGHTNING", 0, &in_bindings[e_bi_lightning], },
    { "SOUND", 0, &in_bindings[e_bi_sfx], },
    { "MUSIC", 0, &in_bindings[e_bi_music], },
    { "CEILING", 0, &in_bindings[e_bi_ceiling], },
    { "FLOORING", 0, &in_bindings[e_bi_flooring], },
    { "HEART BEAT (AOG)", 0, &in_bindings[e_bi_heart_beat], },
    { "", 0, nullptr, },

    { "MISC", 0, nullptr, },
    { "PAUSE", 0, &in_bindings[e_bi_pause], },
    { "(UN)GRAB MOUSE", 0, &in_bindings[e_bi_grab_mouse], },
}; // binds


const int k_binds_max_per_window = 14;
const int k_binds_text_keys_gap = 3;
const int k_binds_line_spacing = 1;
const int k_binds_top = 28;

const uint8_t k_binds_category_color = 0x4A;
const uint8_t k_binds_text_color = 0x56;
const uint8_t k_binds_key_text_color = 0x7F;
const uint8_t k_binds_key_bar_default_color = 0x03;
const uint8_t k_binds_key_bar_active_color = 0x31;
const uint8_t k_binds_key_bar_assign_color = 0x14;

int binds_count;
int binds_window;
int binds_window_offset;
int binds_key_index;
int binds_key_width;
int binds_max_text_width;
int binds_text_height;
int binds_text_x;
int binds_key_x[k_max_binding_keys];
bool binds_is_assigning = false;

BindsNames binds_names;


void binds_initialize_menu()
{
    static bool is_initialized = false;

    if (is_initialized) {
        return;
    }

    binds_count = 0;
    binds_window = 0;
    binds_window_offset = 0;
    binds_key_index = 0;
    binds_key_width = 0;
    binds_max_text_width = 0;
    binds_text_height = 0;
    binds_is_assigning = false;

    bool has_bindings = false;

    fontnumber = 2;

    for (auto& bind : binds) {
        ++binds_count;

        if (!bind.name.empty()) {
            int width = 0;
            int height = 0;
            VW_MeasurePropString(bind.name.c_str(), &width, &height);

            bind.name_width = width;

            if (width > binds_max_text_width) {
                binds_max_text_width = width;
            }

            if (height > binds_text_height) {
                binds_text_height = height;
            }

            has_bindings = true;
        }
    }


    if (!has_bindings) {
        ::Quit("No bindings.");
    }

    binds_names.clear();
    binds_names[ScanCode::sc_return] = "ENTER";
    binds_names[ScanCode::sc_space] = "SPACE";
    binds_names[ScanCode::sc_minus] = "-";
    binds_names[ScanCode::sc_equals] = "=";
    binds_names[ScanCode::sc_backspace] = "BACKSPACE";
    binds_names[ScanCode::sc_tab] = "TAB";
    binds_names[ScanCode::sc_alt] = "ALT";
    binds_names[ScanCode::sc_left_bracket] = "[";
    binds_names[ScanCode::sc_right_bracket] = "]";
    binds_names[ScanCode::sc_control] = "CTRL";
    binds_names[ScanCode::sc_caps_lock] = "CAPS LOCK";
    binds_names[ScanCode::sc_num_lock] = "NUM LOCK";
    binds_names[ScanCode::sc_scroll_lock] = "SCROLL LOCK";
    binds_names[ScanCode::sc_left_shift] = "L-SHIFT";
    binds_names[ScanCode::sc_right_shift] = "R-SHIFT";
    binds_names[ScanCode::sc_up_arrow] = "UP";
    binds_names[ScanCode::sc_down_arrow] = "DOWN";
    binds_names[ScanCode::sc_left_arrow] = "LEFT";
    binds_names[ScanCode::sc_right_arrow] = "RIGHT";
    binds_names[ScanCode::sc_insert] = "INS";
    binds_names[ScanCode::sc_delete] = "DEL";
    binds_names[ScanCode::sc_home] = "HOME";
    binds_names[ScanCode::sc_end] = "END";
    binds_names[ScanCode::sc_page_up] = "PGUP";
    binds_names[ScanCode::sc_page_down] = "PGDN";
    binds_names[ScanCode::sc_slash] = "/";
    binds_names[ScanCode::sc_f1] = "F1";
    binds_names[ScanCode::sc_f2] = "F2";
    binds_names[ScanCode::sc_f3] = "F3";
    binds_names[ScanCode::sc_f4] = "F4";
    binds_names[ScanCode::sc_f5] = "F5";
    binds_names[ScanCode::sc_f6] = "F6";
    binds_names[ScanCode::sc_f7] = "F7";
    binds_names[ScanCode::sc_f8] = "F8";
    binds_names[ScanCode::sc_f9] = "F9";
    binds_names[ScanCode::sc_f10] = "F10";
    binds_names[ScanCode::sc_f11] = "F11";
    binds_names[ScanCode::sc_f12] = "F12";
    binds_names[ScanCode::sc_print_screen] = "PRT SCR";
    binds_names[ScanCode::sc_pause] = "PAUSE";
    binds_names[ScanCode::sc_back_quote] = "BACK QUOTE";
    binds_names[ScanCode::sc_semicolon] = ";";
    binds_names[ScanCode::sc_quote] = "'";
    binds_names[ScanCode::sc_backslash] = "\\";
    binds_names[ScanCode::sc_comma] = ",";
    binds_names[ScanCode::sc_period] = ".";

    binds_names[ScanCode::sc_1] = "1";
    binds_names[ScanCode::sc_2] = "2";
    binds_names[ScanCode::sc_3] = "3";
    binds_names[ScanCode::sc_4] = "4";
    binds_names[ScanCode::sc_5] = "5";
    binds_names[ScanCode::sc_6] = "6";
    binds_names[ScanCode::sc_7] = "7";
    binds_names[ScanCode::sc_8] = "8";
    binds_names[ScanCode::sc_9] = "9";
    binds_names[ScanCode::sc_0] = "0";

    binds_names[ScanCode::sc_a] = "A";
    binds_names[ScanCode::sc_b] = "B";
    binds_names[ScanCode::sc_c] = "C";
    binds_names[ScanCode::sc_d] = "D";
    binds_names[ScanCode::sc_e] = "E";
    binds_names[ScanCode::sc_f] = "F";
    binds_names[ScanCode::sc_g] = "G";
    binds_names[ScanCode::sc_h] = "H";
    binds_names[ScanCode::sc_i] = "I";
    binds_names[ScanCode::sc_j] = "J";
    binds_names[ScanCode::sc_k] = "K";
    binds_names[ScanCode::sc_l] = "L";
    binds_names[ScanCode::sc_m] = "M";
    binds_names[ScanCode::sc_n] = "N";
    binds_names[ScanCode::sc_o] = "O";
    binds_names[ScanCode::sc_p] = "P";
    binds_names[ScanCode::sc_q] = "Q";
    binds_names[ScanCode::sc_r] = "R";
    binds_names[ScanCode::sc_s] = "S";
    binds_names[ScanCode::sc_t] = "T";
    binds_names[ScanCode::sc_u] = "U";
    binds_names[ScanCode::sc_v] = "V";
    binds_names[ScanCode::sc_w] = "W";
    binds_names[ScanCode::sc_x] = "X";
    binds_names[ScanCode::sc_y] = "Y";
    binds_names[ScanCode::sc_z] = "Z";

    binds_names[ScanCode::sc_kp_minus] = "KP MINUS";
    binds_names[ScanCode::sc_kp_plus] = "KP PLUS";

    binds_names[ScanCode::sc_mouse_left] = "MOUSE 1";
    binds_names[ScanCode::sc_mouse_middle] = "MOUSE 2";
    binds_names[ScanCode::sc_mouse_right] = "MOUSE 3";
    binds_names[ScanCode::sc_mouse_x1] = "MOUSE 4";
    binds_names[ScanCode::sc_mouse_x2] = "MOUSE 5";

    for (const auto& binds_name : binds_names) {
        int width = 0;
        int height = 0;
        VW_MeasurePropString(binds_name.second, &width, &height);

        if (width > binds_key_width) {
            binds_key_width = width;
        }
    }

    int max_keys_width = k_max_binding_keys * (binds_key_width + 1);
    int max_text_width = 2 + binds_max_text_width;
    int max_width = max_keys_width + k_binds_text_keys_gap + max_text_width;

    int text_x = (SCREEN_W - max_width) / 2;

    int base_key_x = text_x + max_text_width + k_binds_text_keys_gap;

    binds_text_x = text_x;

    for (int i = 0; i < k_max_binding_keys; ++i) {
        binds_key_x[i] = base_key_x + (i * (binds_key_width + 1));
    }

    is_initialized = true;
}

bool binds_advance_to_item(
    BindsFindDirection direction)
{
    switch (direction) {
    case e_bfd_forward:
        if ((binds_window + binds_window_offset + 1) < binds_count) {
            ++binds_window_offset;

            if (binds_window_offset == k_binds_max_per_window) {
                ++binds_window;
                --binds_window_offset;
            }

            return true;
        } else {
            return false;
        }

    case e_bfd_backward:
        if ((binds_window + binds_window_offset) > 0) {
            --binds_window_offset;

            if (binds_window_offset < 0) {
                --binds_window;
                binds_window_offset = 0;
            }

            return true;
        } else {
            return false;
        }

    default:
        return false;
    }
}

bool binds_find_item(
    BindsFindDirection direction,
    BindsFindFrom from)
{
    if (from == e_bff_not_current) {
        if (!binds_advance_to_item(direction)) {
            return false;
        }
    }

    while (true) {
        if (binds[binds_window + binds_window_offset].binding) {
            return true;
        }

        if (!binds_advance_to_item(direction)) {
            return false;
        }
    }

    return false;
}

bool binds_assign_new_key(
    ScanCode key,
    Binding& binding)
{
    auto it = binds_names.find(LastScan);

    if (it == binds_names.end()) {
        return false;
    }

    for (int b = 0; b < k_max_bindings; ++b) {
        for (int k = 0; k < k_max_binding_keys; ++k) {
            if (in_bindings[b][k] == key) {
                in_bindings[b][k] = ScanCode::sc_none;
            }
        }
    }

    binding[binds_key_index] = key;

    return true;
}

void binds_remove_binding()
{
    auto& item = binds[binds_window + binds_window_offset];
    (*item.binding)[binds_key_index] = ScanCode::sc_none;
}

void binds_draw_item_text(
    int item_index)
{
    auto& item = binds[binds_window + item_index];

    if (item.name.empty()) {
        return;
    }

    int x = SCREEN_X + binds_text_x;
    int y = SCREEN_Y + k_binds_top +
            (item_index * (binds_text_height + k_binds_line_spacing));

    int text_left_offset = 0;
    int text_width = 0;

    if (item.binding) {
        text_width = item.name_width;
        text_left_offset = binds_max_text_width - item.name_width;

        if (text_left_offset < 0) {
            text_left_offset = 0;
            text_width = binds_max_text_width;
        }
    } else {
        text_width = SCREEN_W;
    }

    PrintX = static_cast<int16_t>(x + text_left_offset);
    PrintY = static_cast<int16_t>(y);
    WindowX = PrintX;
    WindowY = PrintY;
    WindowW = static_cast<int16_t>(text_width);
    WindowH = static_cast<int16_t>(binds_text_height);

    if (item.binding) {
        fontcolor = k_binds_text_color;
        US_Print(item.name.c_str());
    } else {
        fontcolor = k_binds_category_color;
        US_PrintCentered(item.name.c_str());
    }
}

void binds_draw_keys(
    int item_index)
{
    const auto& item = binds[binds_window + item_index];

    if (!item.binding) {
        return;
    }

    int y = SCREEN_Y + k_binds_top +
        (item_index * (binds_text_height + k_binds_line_spacing));

    bool is_current = (item_index == binds_window_offset);

    for (int k = 0; k < k_max_binding_keys; ++k) {
        uint8_t color;
        ScanCode key;
        const char* key_name;

        bool is_active = is_current && (binds_key_index == k);

        if (is_active) {
            color =
                binds_is_assigning ?
                k_binds_key_bar_assign_color :
                k_binds_key_bar_active_color;
        } else {
            color = k_binds_key_bar_default_color;
        }

        int x = SCREEN_X + binds_key_x[k];

        VWB_Bar(
            x,
            y,
            binds_key_width,
            binds_text_height,
            color);

        PrintX = static_cast<int16_t>(x);
        PrintY = static_cast<int16_t>(y);
        WindowX = PrintX;
        WindowY = PrintY;
        WindowW = static_cast<int16_t>(binds_key_width);
        WindowH = static_cast<int16_t>(binds_text_height);

        if (!(is_active && binds_is_assigning)) {
            key = (*item.binding)[k];

            if (key != ScanCode::sc_none) {
                key_name = "???";

                auto name_it = binds_names.find(key);

                if (name_it != binds_names.end()) {
                    key_name = name_it->second;
                }

                fontcolor = k_binds_key_text_color;
                US_PrintCentered(key_name);
            }
        }
    }
}

void binds_draw()
{
    bool found_item = false;

    found_item = binds_find_item(e_bfd_forward, e_bff_current);

    if (!found_item) {
        found_item = binds_find_item(e_bfd_backward, e_bff_current);
    }

    ClearMScreen();
    DrawMenuTitle("CUSTOMIZE CONTROLS");

    DrawInstructions(
        binds_is_assigning ? IT_CONTROLS_ASSIGNING_KEY : IT_CONTROLS);

    fontnumber = 2;

    for (int i = 0; i < k_binds_max_per_window; ++i) {
        int item_index = binds_window + i;

        if (item_index == binds_count) {
            break;
        }

        binds_draw_item_text(i);
        binds_draw_keys(i);
    }
}

void binds_draw_menu()
{
    bool is_up_pressed = false;
    bool is_down_pressed = false;
    bool is_left_pressed = false;
    bool is_right_pressed = false;
    bool is_pgdn_pressed = false;
    bool is_pgup_pressed = false;
    bool is_enter_pressed = false;
    bool is_delete_pressed = false;
    bool is_escape_pressed = false;

    bool handle_up = false;
    bool handle_down = false;
    bool handle_left = false;
    bool handle_right = false;
    bool handle_pgdn = false;
    bool handle_pgup = false;
    bool handle_enter = false;
    bool handle_delete = false;
    bool handle_escape = false;

    bool fade_in = true;

    CA_CacheGrChunk(STARTFONT + 2);
    CA_CacheGrChunk(STARTFONT + 4);
    binds_initialize_menu();

    binds_is_assigning = false;

    while (true) {
        binds_draw();
        VW_UpdateScreen();

        if (fade_in) {
            fade_in = false;
            MenuFadeIn();
        }

        in_handle_events();

        if (binds_is_assigning) {
            LastScan = ScanCode::sc_none;
            bool quit = false;

            while (!quit) {
                LastScan = ScanCode::sc_none;
                in_handle_events();

                if (Keyboard[ScanCode::sc_escape]) {
                    quit = true;
                    sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);
                } else if (LastScan != ScanCode::sc_none) {
                    auto& item = binds[binds_window + binds_window_offset];

                    if (binds_assign_new_key(LastScan, *item.binding)) {
                        ShootSnd();
                        quit = true;
                    } else {
                        sd_play_player_sound(NOWAYSND, bstone::AC_ITEM);
                    }
                }
            }

            is_escape_pressed = true;
            binds_is_assigning = false;
        } else {
            if (Keyboard[ScanCode::sc_up_arrow]) {
                if (!is_up_pressed) {
                    handle_up = true;
                    is_up_pressed = true;
                }
            } else {
                is_up_pressed = false;
            }

            if (Keyboard[ScanCode::sc_down_arrow]) {
                if (!is_down_pressed) {
                    handle_down = true;
                    is_down_pressed = true;
                }
            } else {
                is_down_pressed = false;
            }

            if (Keyboard[ScanCode::sc_left_arrow]) {
                if (!is_left_pressed) {
                    handle_left = true;
                    is_left_pressed = true;
                }
            } else {
                is_left_pressed = false;
            }

            if (Keyboard[ScanCode::sc_right_arrow]) {
                if (!is_right_pressed) {
                    handle_right = true;
                    is_right_pressed = true;
                }
            } else {
                is_right_pressed = false;
            }

            if (Keyboard[ScanCode::sc_page_down]) {
                if (!is_pgdn_pressed) {
                    handle_pgdn = true;
                    is_pgdn_pressed = true;
                }
            } else {
                is_pgdn_pressed = false;
            }

            if (Keyboard[ScanCode::sc_page_up]) {
                if (!is_pgup_pressed) {
                    handle_pgup = true;
                    is_pgup_pressed = true;
                }
            } else {
                is_pgup_pressed = false;
            }

            if (Keyboard[ScanCode::sc_return]) {
                if (!is_enter_pressed) {
                    handle_enter = true;
                    is_enter_pressed = true;
                }
            } else {
                is_enter_pressed = false;
            }

            if (Keyboard[ScanCode::sc_delete]) {
                if (!is_delete_pressed) {
                    handle_delete = true;
                    is_delete_pressed = true;
                }
            } else {
                is_delete_pressed = false;
            }

            if (Keyboard[ScanCode::sc_escape]) {
                if (!is_escape_pressed) {
                    handle_escape = true;
                    is_escape_pressed = true;
                }
            } else {
                is_escape_pressed = false;
            }

            if (handle_up) {
                handle_up = false;
                binds_find_item(e_bfd_backward, e_bff_not_current);
            }

            if (handle_down) {
                handle_down = false;
                binds_find_item(e_bfd_forward, e_bff_not_current);
            }

            if (handle_left) {
                handle_left = false;

                if (binds_key_index == 1) {
                    binds_key_index = 0;
                }
            }

            if (handle_right) {
                handle_right = false;

                if (binds_key_index == 0) {
                    binds_key_index = 1;
                }
            }

            if (handle_pgdn) {
                handle_pgdn = false;

                for (int i = 0; i < k_binds_max_per_window; ++i) {
                    binds_find_item(e_bfd_forward, e_bff_not_current);
                }
            }

            if (handle_pgup) {
                handle_pgup = false;

                for (int i = 0; i < k_binds_max_per_window; ++i) {
                    binds_find_item(e_bfd_backward, e_bff_not_current);
                }
            }

            if (handle_enter) {
                handle_enter = false;
                binds_is_assigning = true;
            }

            if (handle_delete) {
                handle_delete = false;
                binds_remove_binding();
                ShootSnd();
            }

            if (handle_escape) {
                handle_escape = false;
                sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);
                break;
            }
        }
    }

    ::MenuFadeOut();
}


} // namespace
// BBi

void HelpScreens()
{
    HelpPresenter(nullptr, false, HELPTEXT, true);
}

void HelpPresenter(
    const char* fname,
    bool continue_keys,
    uint16_t id_cache,
    bool startmusic)
{
    const int FULL_VIEW_WIDTH = 19;

    PresenterInfo pi;
    int oldwidth;

    memset(&pi, 0, sizeof(pi));

    pi.flags = TPF_SHOW_PAGES;
    if (continue_keys) {
        pi.flags |= TPF_CONTINUE;
    }

    VW_FadeOut();

// Change view size to MAX! (scaler clips shapes on smaller views)
//
    oldwidth = viewwidth / 16;
    if (oldwidth != FULL_VIEW_WIDTH) {
        NewViewSize();
    }

// Draw help border
//
    CacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);
    VWB_DrawPic(0, 0, H_TOPWINDOWPIC);
    VWB_DrawPic(0, 8, H_LEFTWINDOWPIC);
    VWB_DrawPic(312, 8, H_RIGHTWINDOWPIC);
    VWB_DrawPic(8, 176, H_BOTTOMINFOPIC);
    UnCacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);

// Setup for text presenter
//
    pi.xl = 8;
    pi.yl = 8;
    pi.xh = 311;
    pi.yh = 175;
    pi.ltcolor = 0x7b;
    pi.bgcolor = 0x7d;
    pi.dkcolor = 0x7f;
    pi.shcolor = 0x00;
    pi.fontnumber = 4;

    if (continue_keys) {
        pi.infoline = (char*)" UP / DN - PAGES       ENTER - CONTINUES         ESC - EXITS";
    } else {
        pi.infoline = (char*)"           UP / DN - PAGES            ESC - EXITS";
    }

    if (startmusic) {
        ::StartCPMusic(static_cast<int16_t>(TEXTSONG));
    }

// Load, present, and free help text.
//
    TP_LoadScript(fname, &pi, id_cache);
    TP_Presenter(&pi);
    TP_FreeScript(&pi, id_cache);

    ::MenuFadeOut();

// Reset view size
//
    if (oldwidth != FULL_VIEW_WIDTH) {
        NewViewSize();
    }

    if (startmusic && TPscan == ScanCode::sc_escape) {
        ::StartCPMusic(MENUSONG);
    }

    IN_ClearKeysDown();
}

void US_ControlPanel(
    ScanCode scancode)
{
    // BBi
    menu_background_color = (
        (::is_aog_sw() | ::is_aog_full_v3_0()) ?
        0x04 :
        TERM_BACK_COLOR);


    int16_t which;

    if (ingame) {
        if (CP_CheckQuick(scancode)) {
            return;
        }
    }

    SetupControlPanel();

    ::StartCPMusic(MENUSONG);

    //
    // F-KEYS FROM WITHIN GAME
    //
    auto finish = false;

    switch (scancode) {
    case ScanCode::sc_f1:
        ::CleanupControlPanel();
        ::HelpScreens();
        return;

    case ScanCode::sc_f2:
        ::CP_SaveGame(0);
        finish = true;
        break;

    case ScanCode::sc_f3:
        ::CP_LoadGame(0);
        finish = true;
        break;

    case ScanCode::sc_f4:
        ::CP_Sound(0);
        finish = true;
        break;

    case ScanCode::sc_f6:
        CP_Control(0);
        finish = true;
        break;

    default:
        break;
    }

    if (finish) {
        ::CleanupControlPanel();
        return;
    }

    DrawMainMenu();
    MenuFadeIn();
    StartGame = 0;

    //
    // MAIN MENU LOOP
    //
    do {
        which = HandleMenu(&MainItems, &MainMenu[0], nullptr);

        switch (which) {
        case MM_VIEW_SCORES:
            if (!MainMenu[MM_VIEW_SCORES].routine) {
                if (CP_EndGame()) {
                    StartGame = 1;
                }
            }

            DrawMainMenu();
            MenuFadeIn();
            break;

        case -1:
        case MM_LOGOFF:
            CP_Quit();
            break;

        default:
            if (!StartGame) {
                DrawMainMenu();
                MenuFadeIn();
            }
        }

        //
        // "EXIT OPTIONS" OR "NEW GAME" EXITS
        //
    } while (!StartGame);

    //
    // DEALLOCATE EVERYTHING
    //
    CleanupControlPanel();
    if (!loadedgame) {
        StopMusic();
    }


    //
    // CHANGE MAINMENU ITEM
    //
    if (startgame || loadedgame) {
        MainMenu[MM_VIEW_SCORES].routine = nullptr;
        strcpy(MainMenu[MM_VIEW_SCORES].string, "END GAME");
    }

    if (ingame && loadedgame) {
        refresh_screen = false;
    }


#ifdef CACHE_KEY_DATA
    FREEFONT(SCANNAMES_DATA);
    FREEFONT(EXTSCANNAMES_DATA);
    FREEFONT(EXTSCANCODES_DATA);
#endif
}

void DrawMainMenu()
{
    ControlPanelFree();
    CA_CacheScreen(BACKGROUND_SCREENPIC);
    ControlPanelAlloc();

    ClearMScreen();
    DrawMenuTitle("MAIN OPTIONS");
    DrawInstructions(IT_STANDARD);

    //
    // CHANGE "MISSION" AND "DEMO"
    //
    if (ingame) {
        strcpy(&MainMenu[MM_BACK_TO_DEMO].string[8], "MISSION");
        MainMenu[MM_BACK_TO_DEMO].active = AT_READIT;
    } else {
        strcpy(&MainMenu[MM_BACK_TO_DEMO].string[8], "DEMO");
        MainMenu[MM_BACK_TO_DEMO].active = AT_ENABLED;
    }

    fontnumber = 4; // COAL

    DrawMenu(&MainItems, &MainMenu[0]);

    VW_UpdateScreen();
}

void CP_ReadThis(
    int16_t)
{
    ControlPanelFree();
    HelpScreens();
    ControlPanelAlloc();
}

void CP_OrderingInfo(
    int16_t)
{
    ControlPanelFree();
    HelpPresenter("", false, ORDERTEXT, true);
    ControlPanelAlloc();
}

void CP_BlakeStoneSaga(
    int16_t)
{
    ControlPanelFree();
    HelpPresenter("", false, SAGATEXT, true);
    ControlPanelAlloc();
}

// --------------------------------------------------------------------------
// CP_CheckQuick() - CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
// --------------------------------------------------------------------------
bool CP_CheckQuick(
    ScanCode scancode)
{
    switch (scancode) {
    // END GAME
    //
    case ScanCode::sc_f7:
// BBi
#if 0
        VW_ScreenToScreen(PAGE1START, ::bufferofs, 320, 160);
#endif

        CA_CacheGrChunk(STARTFONT + 1);

        WindowH = 160;
        if (Confirm(ENDGAMESTR)) {
            playstate = ex_died;
            pickquick = gamestate.lives = 0;
        }

        WindowH = 200;
        fontnumber = 4;
        return true;

    // QUICKSAVE
    //
    case ScanCode::sc_f8:
        if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick) {
            char string[100] = "Quick Save will overwrite:\n\"";

            CA_CacheGrChunk(STARTFONT + 1);

            strcat(string, SaveGameNames[static_cast<int>(LSItems.curpos)]);
            strcat(string, "\"?");

// BBi
#if 0
            VW_ScreenToScreen(PAGE1START, ::bufferofs, 320, 160);
#endif

            if (Confirm(string)) {
                CA_CacheGrChunk(STARTFONT + 1);
                CP_SaveGame(1);
                fontnumber = 4;
            } else {
                refresh_screen = false;
            }
        } else {
            CA_CacheGrChunk(STARTFONT + 1);

            VW_FadeOut();

            ::StartCPMusic(MENUSONG);

            pickquick = CP_SaveGame(0);

            lasttimecount = TimeCount;
            ::in_clear_mouse_deltas();
        }

        return true;

    // QUICKLOAD
    //
    case ScanCode::sc_f9:
        if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick) {
            char string[100] = "Quick Load:\n\"";

            CA_CacheGrChunk(STARTFONT + 1);

            strcat(string, SaveGameNames[static_cast<int>(LSItems.curpos)]);
            strcat(string, "\"?");

// BBi
#if 0
            VW_ScreenToScreen(PAGE1START, ::bufferofs, 320, 160);
#endif

            if (Confirm(string)) {
                CP_LoadGame(1);
            } else {
                refresh_screen = false;
                return true;
            }

            fontnumber = 4;
        } else {
            CA_CacheGrChunk(STARTFONT + 1);

            VW_FadeOut();

            ::StartCPMusic(MENUSONG);

            pickquick = CP_LoadGame(0);

            lasttimecount = TimeCount;
            ::in_clear_mouse_deltas();
        }

        if (pickquick) {
            refresh_screen = false;
        }
        return true;

    // QUIT
    //
    case ScanCode::sc_f10:
        CA_CacheGrChunk(STARTFONT + 1);

// BBi
#if 0
        VW_ScreenToScreen(PAGE1START, ::bufferofs, 320, 160);
#endif

        WindowX = WindowY = 0;
        WindowW = 320;
        WindowH = 160;
        if (Confirm(QuitToDosStr)) {
            ExitGame();
        }

        refresh_screen = false;
        WindowH = 200;
        fontnumber = 4;

        return true;

    default:
        return false;
    }
}

int16_t CP_EndGame()
{
    if (!Confirm(ENDGAMESTR)) {
        return 0;
    }

    pickquick = gamestate.lives = 0;
    playstate = ex_died;
    InstantQuit = 1;

    return 1;
}

void CP_ViewScores(
    int16_t)
{
    fontnumber = 4;

    ::StartCPMusic(static_cast<int16_t>(ROSTER_MUS));

    DrawHighScores();
    VW_UpdateScreen();
    MenuFadeIn();
    fontnumber = 1;

    IN_Ack();

    ::StartCPMusic(MENUSONG);

    ::MenuFadeOut();
}

void CP_NewGame(
    int16_t)
{
    int16_t which, episode = 0;

    DrawMenuTitle("Difficulty Level");
    DrawInstructions(IT_STANDARD);


firstpart:

    if (!::is_ps()) {
        DrawNewEpisode();
        do {
            which = HandleMenu(&NewEitems, &NewEmenu[0], DrawEpisodePic);
            switch (which) {
            case -1:
                ::MenuFadeOut();
                return;

            default:
                if (!EpisodeSelect[which]) {
                    ::sd_play_player_sound(NOWAYSND, bstone::AC_ITEM);
                    CacheMessage(READTHIS_TEXT);
                    IN_ClearKeysDown();
                    IN_Ack();
                    VL_Bar(35, 69, 250, 62, ::menu_background_color);
                    DrawNewEpisode();
                    which = 0;
                } else {
                    episode = which;
                    which = 1;
                }
                break;
            }

        } while (!which);

        ShootSnd();
    } else {
        episode = 0;
    }

    //
    // ALREADY IN A GAME?
    //
    if (!::is_ps() && ingame) {
        if (!Confirm(CURGAME)) {
            ::MenuFadeOut();
            return;
        }
    }

secondpart:

    ::MenuFadeOut();
    if (ingame) {
        CA_CacheScreen(BACKGROUND_SCREENPIC);
    }
    DrawNewGame();
    which = HandleMenu(&NewItems, &NewMenu[0], DrawNewGameDiff);

    if (which < 0) {
        ::MenuFadeOut();

        if (!::is_ps()) {
            goto firstpart;
        } else {
            return;
        }
    }

    ShootSnd();
    ::MenuFadeOut();
    ControlPanelFree();

    if (Breifing(BT_INTRO, episode)) {
        CA_CacheScreen(BACKGROUND_SCREENPIC);
        ControlPanelAlloc();
        goto secondpart;
    }

    StartGame = 1;
    NewGame(which, episode);

    //
    // CHANGE "READ THIS!" TO NORMAL COLOR
    //
    MainMenu[MM_READ_THIS].active = AT_ENABLED;
}

void DrawMenuTitle(
    const char* title)
{

    fontnumber = 3;
    CA_CacheGrChunk(STARTFONT + 3);

    PrintX = WindowX = 32;
    PrintY = WindowY = 32;
    WindowW = 244;
    WindowH = 20;

    SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(title);

    WindowX = 32 - 1;
    WindowY = 32 - 1;

    SETFONTCOLOR(ENABLED_TEXT_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(title);

    FREEFONT(STARTFONT + 3);

}

const int16_t INSTRUCTIONS_Y_POS = 154 + 10;

// ---------------------------------------------------------------------------
// DrawInstructions() - Draws instructions centered at the bottom of
//      the view screen.
//
// NOTES: Orginal font number or font color is not maintained.
// ---------------------------------------------------------------------------
void DrawInstructions(
    inst_type Type)
{
    const char* instr[MAX_INSTRUCTIONS] = {
        "UP/DN SELECTS - ENTER CHOOSES - ESC EXITS",
        "PRESS ANY KEY TO CONTINUE",
        "ENTER YOUR NAME AND PRESS ENTER",
        "RT/LF ARROW SELECTS - ENTER CHOOSES",

        // BBi
        "UP/DN SELECTS - LF/RT CHANGES - ESC EXITS",
        "ARROWS SELECTS - ENTER CHOOSES - DEL REMOVES",
        "ESC EXITS"
    };

    fontnumber = 2;

    WindowX = 48;
    WindowY = INSTRUCTIONS_Y_POS;
    WindowW = 236;
    WindowH = 8;

    VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, ::menu_background_color);

    SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(instr[Type]);

    WindowX--;
    WindowY--;

    SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(instr[Type]);
}

void DrawNewEpisode()
{
    ClearMScreen();

    DrawMenuTitle("CHOOSE A MISSION");
    DrawInstructions(IT_STANDARD);

    PrintY = 51;
    WindowX = 58;

    fontnumber = 2; // six point font
    DrawMenu(&NewEitems, &NewEmenu[0]);

    DrawEpisodePic(NewEitems.curpos);

    VW_UpdateScreen();
    MenuFadeIn();
    WaitKeyUp();

}

void DrawNewGame()
{
    ClearMScreen();
    DrawMenuTitle("DIFFICULTY LEVEL");
    DrawInstructions(IT_STANDARD);

    fontnumber = 2; // six point font
    DrawMenu(&NewItems, &NewMenu[0]);

    DrawNewGameDiff(NewItems.curpos);

    px = 48;
    py = INSTRUCTIONS_Y_POS - 24;
    ShPrint("        HIGHER DIFFICULTY LEVELS CONTAIN", TERM_SHADOW_COLOR, false);

    px = 48;
    py += 6;
    ShPrint("            MORE, STRONGER ENEMIES", TERM_SHADOW_COLOR, false);


    VW_UpdateScreen();

    MenuFadeIn();
    WaitKeyUp();
}

void DrawNewGameDiff(
    int16_t w)
{
    VWB_DrawPic(192, 77, w + C_BABYMODEPIC);
}

void DrawEpisodePic(
    int16_t w)
{
    VWB_DrawPic(176, 72, w + C_EPISODE1PIC);
}

void CP_GameOptions(
    int16_t)
{
    int16_t which;

    CA_CacheScreen(BACKGROUND_SCREENPIC);
    DrawGopMenu();
    MenuFadeIn();
    WaitKeyUp();

    do {
        which = HandleMenu(&GopItems, &GopMenu[0], nullptr);

        if (which != -1) {
            DrawGopMenu();
            MenuFadeIn();
        }

    } while (which >= 0);

    ::MenuFadeOut();
}

void DrawGopMenu()
{
    CA_CacheScreen(BACKGROUND_SCREENPIC);

    ClearMScreen();
    DrawMenuTitle("GAME OPTIONS");
    DrawInstructions(IT_STANDARD);

    fontnumber = 4; // COAL

    DrawMenu(&GopItems, &GopMenu[0]);

    VW_UpdateScreen();
}

void ChangeSwaps()
{
    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
    Message(Computing);

    PM_Shutdown();
    PM_Startup();
    ClearMemory();
    ControlPanelAlloc();

    IN_UserInput(50);
    IN_ClearKeysDown();

}

void CP_Switches(
    int16_t)
{
    int16_t which;

    CA_CacheScreen(BACKGROUND_SCREENPIC);
    DrawSwitchMenu();
    MenuFadeIn();
    WaitKeyUp();

    do {
        which = HandleMenu(&SwitchItems, &SwitchMenu[0], DrawAllSwitchLights);

        switch (which) {
        case SW_LIGHTING:
            gamestate.flags ^= GS_LIGHTING;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_REBA_ATTACK_INFO:
            gamestate.flags ^= GS_ATTACK_INFOAREA;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_CEILING:
            gamestate.flags ^= GS_DRAW_CEILING;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_FLOORS:
            gamestate.flags ^= GS_DRAW_FLOOR;
            ShootSnd();
            DrawSwitchMenu();
            break;

        // BBi
        case SW_NO_WALL_HIT_SOUND:
            g_no_wall_hit_sound = !g_no_wall_hit_sound;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_MODERN_CONTROLS:
            in_use_modern_bindings = !in_use_modern_bindings;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_ALWAYS_RUN:
            g_always_run = !g_always_run;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_HEART_BEAT_SOUND:
            g_heart_beat_sound = !g_heart_beat_sound;
            ShootSnd();
            DrawSwitchMenu();
            break;

        case SW_ROTATED_AUTOMAP:
            g_rotated_automap = !g_rotated_automap;
            ShootSnd();
            DrawSwitchMenu();
            break;
        }
    } while (which >= 0);

    ::MenuFadeOut();
}

void DrawSwitchMenu()
{
    CA_CacheScreen(BACKGROUND_SCREENPIC);

    ClearMScreen();
    DrawMenuTitle("GAME SWITCHES");
    DrawInstructions(IT_STANDARD);

    fontnumber = 2;

    DrawMenu(&SwitchItems, &SwitchMenu[0]);
    DrawAllSwitchLights(SwitchItems.curpos);

    VW_UpdateScreen();
}

void DrawAllSwitchLights(
    int16_t which)
{
    int16_t i;
    uint16_t Shape;

    for (i = 0; i < SwitchItems.amount; i++) {
        if (SwitchMenu[i].string[0]) {
            Shape = C_NOTSELECTEDPIC;

            //
            // DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
            //

            if (SwitchItems.cursor.on) {
                if (i == which) { // Is the cursor sitting on this pic?
                    Shape += 2;
                }
            }

            switch (i) {
            case SW_LIGHTING:
                if (gamestate.flags & GS_LIGHTING) {
                    Shape++;
                }
                break;

            case SW_REBA_ATTACK_INFO:
                if (gamestate.flags & GS_ATTACK_INFOAREA) {
                    Shape++;
                }
                break;

            case SW_CEILING:
                if (gamestate.flags & GS_DRAW_CEILING) {
                    Shape++;
                }
                break;

            case SW_FLOORS:
                if (gamestate.flags & GS_DRAW_FLOOR) {
                    Shape++;
                }
                break;

            // BBi
            case SW_NO_WALL_HIT_SOUND:
                if (g_no_wall_hit_sound) {
                    ++Shape;
                }
                break;

            case SW_MODERN_CONTROLS:
                if (in_use_modern_bindings) {
                    ++Shape;
                }
                break;

            case SW_ALWAYS_RUN:
                if (g_always_run) {
                    ++Shape;
                }
                break;

            case SW_HEART_BEAT_SOUND:
                if (g_heart_beat_sound) {
                    ++Shape;
                }
                break;

            case SW_ROTATED_AUTOMAP:
                if (g_rotated_automap) {
                    ++Shape;
                }
                break;
            }

            VWB_DrawPic(SwitchItems.x - 16, SwitchItems.y + i * SwitchItems.y_spacing - 1, Shape);
        }
    }

    DrawSwitchDescription(which);

}

void DrawSwitchDescription(
    int16_t which)
{
    const char* instr[] = {
        "TOGGLES LIGHT SOURCING IN HALLWAYS",
        "TOGGLES DETAILED ATTACKER INFO",
        "TOGGLES CEILING MAPPING",
        "TOGGLES FLOOR MAPPING",

        // BBi
        "TOGGLES WALL HIT SOUND",
        "TOGGLES BETWEEN CLASSIC AND MODERN CONTROLS",
        "TOGGLES ALWAYS RUN MODE",
        "TOGGLES HEART BEAT SOUND WITH EKG",
        "TOGGLES <TAB>/<SHIFT+TAB> FUNCTIONS",
    };

    fontnumber = 2;

    WindowX = 48;
    WindowY = (::is_ps() ? 134 : 144);
    WindowW = 236;
    WindowH = 8;

    VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, ::menu_background_color);

    SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(instr[which]);

    WindowX--;
    WindowY--;

    SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
    US_PrintCentered(instr[which]);
}

void CP_Sound(
    int16_t)
{
    int16_t which;

    CA_CacheScreen(BACKGROUND_SCREENPIC);
    DrawSoundMenu();
    MenuFadeIn();
    WaitKeyUp();

    do {
        which = HandleMenu(&SndItems, &SndMenu[0], DrawAllSoundLights);
        //
        // HANDLE MENU CHOICES
        //
        switch (which) {
        //
        // SOUND EFFECTS / DIGITIZED SOUND
        //
        case 0:
            if (::sd_is_sound_enabled) {
                ::SD_WaitSoundDone();
                ::SD_EnableSound(false);
                ::DrawSoundMenu();
            }
            break;

        case 1:
            if (!::sd_is_sound_enabled) {
                ::SD_WaitSoundDone();
                ::SD_EnableSound(true);
                ::CA_LoadAllSounds();
                ::DrawSoundMenu();
                ::ShootSnd();
            }
            break;

        //
        // MUSIC
        //
        case 4:
            if (::sd_is_music_enabled) {
                ::SD_EnableMusic(false);
                ::DrawSoundMenu();
                ::ShootSnd();
            }
            break;

        case 5:
            if (!::sd_is_music_enabled) {
                ::SD_EnableMusic(true);
                ::DrawSoundMenu();
                ::ShootSnd();
                ::StartCPMusic(MENUSONG);
            }
            break;
        }
    } while (which >= 0);

    ::MenuFadeOut();
}

void DrawSoundMenu()
{
    //
    // DRAW SOUND MENU
    //

    ClearMScreen();
    DrawMenuTitle("SOUND SETTINGS");
    DrawInstructions(IT_STANDARD);

    //
    // IF NO ADLIB, NON-CHOOSENESS!
    //

    if (!::sd_has_audio) {
        ::SndMenu[1].active = AT_DISABLED;
        ::SndMenu[5].active = AT_DISABLED;
    }

    fontnumber = 4;

    SETFONTCOLOR(DISABLED_TEXT_COLOR, TERM_BACK_COLOR);
    ShadowPrint("SOUND EFFECTS", 105, 72);
    ShadowPrint("BACKGROUND MUSIC", 105, 100);

    fontnumber = 2;
    DrawMenu(&SndItems, &SndMenu[0]);


    DrawAllSoundLights(SndItems.curpos);

    VW_UpdateScreen();
}

void DrawAllSoundLights(
    int16_t which)
{
    int16_t i;
    uint16_t Shape;

    for (i = 0; i < SndItems.amount; i++) {
        if (SndMenu[i].string[0]) {
            Shape = C_NOTSELECTEDPIC;

            //
            // DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
            //

            if (SndItems.cursor.on) {
                if (i == which) { // Is the cursor sitting on this pic?
                    Shape += 2;
                }
            }

            switch (i) {
            //
            // SOUND EFFECTS / DIGITIZED SOUND
            //
            case 0:
                if (!::sd_is_sound_enabled) {
                    ++Shape;
                }
                break;

            case 1:
                if (::sd_is_sound_enabled) {
                    ++Shape;
                }
                break;

            //
            // MUSIC
            //
            case 4:
                if (!::sd_is_music_enabled) {
                    ++Shape;
                }
                break;

            case 5:
                if (::sd_is_music_enabled) {
                    ++Shape;
                }
                break;
            }

            VWB_DrawPic(SndItems.x - 16, SndItems.y + i * SndItems.y_spacing - 1, Shape);
        }
    }
}

char LOADSAVE_GAME_MSG[2][25] = { "^ST1^CELoading Game\r^XX",
                                  "^ST1^CESaving Game\r^XX" };

extern int8_t LS_current, LS_total;

// --------------------------------------------------------------------------
// DrawLSAction() - DRAW LOAD/SAVE IN PROGRESS
// --------------------------------------------------------------------------
void DrawLSAction(
    int16_t which)
{
    int8_t total[] = { 19, 19 };

    VW_FadeOut();
    screenfaded = true;
    DrawTopInfo(static_cast<sp_type>(sp_loading + which));

    ::VL_Bar(
        0,
        ::ref_view_top,
        ::vga_ref_width,
        ::ref_view_height,
        BLACK);

    DisplayPrepingMsg(LOADSAVE_GAME_MSG[which]);

    if (which) {
        PreloadUpdate(1, 1); // GFX: bar is full when saving...

    }
    LS_current = 1;
    LS_total = total[which];
    WindowY = 181;
}

int16_t CP_LoadGame(
    int16_t quick)
{
    int16_t which;
    int16_t exit = 0;

    //
    // QUICKLOAD?
    //
    if (quick) {
        which = LSItems.curpos;

        if (SaveGamesAvail[which]) {
            auto name = ::get_saved_game_base_name();
            name += static_cast<char>('0' + which);

            DrawLSAction(0); // Testing...

            auto name_path = ::get_profile_dir() + name;

            loadedgame = ::LoadTheGame(name_path);

            if (!loadedgame) {
                LS_current = -1; // clean up
            }

            return loadedgame;
        }
    }

restart:

    DrawLoadSaveScreen(0);

    do {
        which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);

        if (which >= 0 && SaveGamesAvail[which]) {
            ShootSnd();

            auto name = ::get_saved_game_base_name();
            name += static_cast<char>('0' + which);

            auto name_path = ::get_profile_dir() + name;

            DrawLSAction(0);

            if (!::LoadTheGame(name_path)) {
                exit = 0;
                StartGame = 0;
                loadedgame = 0;
                LS_current = -1; // Clean up
                ::playstate = ex_abort;
                goto restart;
            }

            loadedgame = true;
            StartGame = true;

            ::ShootSnd();

            //
            // CHANGE "READ THIS!" TO NORMAL COLOR
            //
            MainMenu[MM_READ_THIS].active = AT_ENABLED;
            exit = 1;
            break;
        }
    } while (which >= 0);

    if (which == -1) {
        ::MenuFadeOut();
    }

    if (loadedgame) {
        refresh_screen = false;
    }

    return exit;
}


///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(
    int16_t w)
{
    static int16_t lastgameon = 0;

    PrintLSEntry(lastgameon, ENABLED_TEXT_COLOR);
    PrintLSEntry(w, HIGHLIGHT_TEXT_COLOR);

    lastgameon = w;
}

void DrawLoadSaveScreen(
    int16_t loadsave)
{
    int16_t i;

    CA_CacheScreen(BACKGROUND_SCREENPIC);
    ClearMScreen();

    fontnumber = 1;

    if (!loadsave) {
        DrawMenuTitle("Load Mission");
    } else {
        DrawMenuTitle("Save Mission");
    }

    DrawInstructions(IT_STANDARD);

    for (i = 0; i < 10; i++) {
        PrintLSEntry(i, ENABLED_TEXT_COLOR);
    }

    fontnumber = 4;
    DrawMenu(&LSItems, &LSMenu[0]);

    VW_UpdateScreen();
    MenuFadeIn();
    WaitKeyUp();
}

// --------------------------------------------------------------------------
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
// --------------------------------------------------------------------------
void PrintLSEntry(
    int16_t w,
    int16_t color)
{
    SETFONTCOLOR(color, BKGDCOLOR);
    DrawOutline(LSM_X + LSItems.indent, LSM_Y + w * LSItems.y_spacing - 2, LSM_W - LSItems.indent, 8, color, color);

    fontnumber = 2;

    PrintX = LSM_X + LSItems.indent + 2;
    PrintY = LSM_Y + w * LSItems.y_spacing;

    if (SaveGamesAvail[w]) {
        US_Print(SaveGameNames[w]);
    } else {
        US_Print("       ----- EMPTY -----");
    }

    fontnumber = 1;
}

int16_t CP_SaveGame(
    int16_t quick)
{
    int16_t which, exit = 0;
    char input[GAME_DESCRIPTION_LEN + 1];
    bool temp_caps = allcaps;
    US_CursorStruct TermCursor = { '@', 0, HIGHLIGHT_TEXT_COLOR, 2 };

    allcaps = true;
    use_custom_cursor = true;
    US_CustomCursor = TermCursor;

    //
    // QUICKSAVE?
    //
    if (quick) {
        which = LSItems.curpos;

        if (SaveGamesAvail[which]) {
            DrawLSAction(1); // Testing...
            auto name = ::get_saved_game_base_name();
            name += static_cast<char>('0' + which);

            auto name_path = ::get_profile_dir() + name;

            ::SaveTheGame(name_path, &SaveGameNames[which][0]);

            return 1;
        }
    }

    DrawLoadSaveScreen(1);

    do {
        which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);
        if (which >= 0) {
            //
            // OVERWRITE EXISTING SAVEGAME?
            //
            if (SaveGamesAvail[which]) {
                if (!Confirm(GAMESVD)) {
                    DrawLoadSaveScreen(1);
                    continue;
                } else {
                    DrawLoadSaveScreen(1);
                    PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
                    VW_UpdateScreen();
                }
            }

            ShootSnd();

            strcpy(input, &SaveGameNames[which][0]);

            auto name = ::get_saved_game_base_name();
            name += static_cast<char>('0' + which);

            fontnumber = 2;
            VWB_Bar(LSM_X + LSItems.indent + 1, LSM_Y + which * LSItems.y_spacing - 1, LSM_W - LSItems.indent - 1, 7, HIGHLIGHT_BOX_COLOR);
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, HIGHLIGHT_BOX_COLOR);
            VW_UpdateScreen();


            if (US_LineInput(LSM_X + LSItems.indent + 2, LSM_Y + which * LSItems.y_spacing, input, input, true, GAME_DESCRIPTION_LEN, LSM_W - LSItems.indent - 10)) {
                SaveGamesAvail[which] = 1;
                strcpy(&SaveGameNames[which][0], input);

                DrawLSAction(1);

                auto name_path = ::get_profile_dir() + name;
                ::SaveTheGame(name_path, input);

                ShootSnd();
                exit = 1;
            } else {
                VWB_Bar(
                    LSM_X + LSItems.indent + 1,
                    LSM_Y + which * LSItems.y_spacing - 1,
                    LSM_W - LSItems.indent - 1,
                    7,
                    ::menu_background_color);

                PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
                VW_UpdateScreen();
                ::sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);
                continue;
            }

            fontnumber = 1;
            break;
        }

    } while (which >= 0);

    ::MenuFadeOut();
    use_custom_cursor = false;
    allcaps = temp_caps;
    return exit;
}

void CP_ExitOptions(
    int16_t)
{
    StartGame = 1;
}

void CP_Control(
    int16_t)
{
    enum {MOUSEENABLE, JOYENABLE, USEPORT2, PADENABLE, CALIBRATEJOY, MOUSESENS, CUSTOMIZE};

    int16_t which;

    CA_CacheScreen(BACKGROUND_SCREENPIC);

    DrawCtlScreen();
    MenuFadeIn();
    WaitKeyUp();

    do {
        which = HandleMenu(&CtlItems, &CtlMenu[0], nullptr);
        switch (which) {
        case MOUSEENABLE:
            ::mouseenabled = !::mouseenabled;

            DrawCtlScreen();
            CusItems.curpos = -1;
            ShootSnd();
            break;

        case JOYENABLE:
            ::joystickenabled = !::joystickenabled;
            if (joystickenabled) {
                CalibrateJoystick();
            }
            DrawCtlScreen();
            CusItems.curpos = -1;
            ShootSnd();
            break;

        case USEPORT2:
            joystickport ^= 1;
            DrawCtlScreen();
            ShootSnd();
            break;

        case PADENABLE:
            ::joypadenabled = !::joypadenabled;
            DrawCtlScreen();
            ShootSnd();
            break;

        case CALIBRATEJOY:
            CalibrateJoystick();
            DrawCtlScreen();
            break;


        case MOUSESENS:
        case CUSTOMIZE:
            DrawCtlScreen();
            MenuFadeIn();
            WaitKeyUp();
            break;
        }
    } while (which >= 0);

    ::MenuFadeOut();
}

void DrawMousePos()
{
    const int thumb_width = 16;
    const int track_width = 160;
    const int slide_width = track_width - thumb_width;
    const int max_mouse_delta = ::max_mouse_sensitivity - ::min_mouse_sensitivity;

    ::VWB_Bar(
        74,
        92,
        track_width,
        8,
        HIGHLIGHT_BOX_COLOR);

    ::DrawOutline(
        73,
        91,
        track_width + 1,
        9,
        ENABLED_TEXT_COLOR,
        ENABLED_TEXT_COLOR);

    ::VWB_Bar(
        74 + ((slide_width * ::mouseadjustment) / max_mouse_delta),
        92,
        thumb_width,
        8,
        HIGHLIGHT_TEXT_COLOR);
}

void DrawMouseSens()
{
    ClearMScreen();
    DrawMenuTitle("MOUSE SENSITIVITY");
    DrawInstructions(IT_MOUSE_SEN);

    fontnumber = 4;

    SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
    PrintX = 36;
    PrintY = 91;
    US_Print("SLOW");
    PrintX = 242;
    US_Print("FAST");

    DrawMousePos();

    VW_UpdateScreen();
    MenuFadeIn();
}

void CalibrateJoystick()
{
    uint16_t minx, maxx, miny, maxy;

    CacheMessage(CALJOY1_TEXT);
    VW_UpdateScreen();

    while (IN_GetJoyButtonsDB(joystickport)) {
    }
    while ((LastScan != ScanCode::sc_escape) && !IN_GetJoyButtonsDB(joystickport)) {
    }
    if (LastScan == ScanCode::sc_escape) {
        return;
    }

    IN_GetJoyAbs(joystickport, &minx, &miny);
    while (IN_GetJoyButtonsDB(joystickport)) {
    }

    CacheMessage(CALJOY2_TEXT);
    VW_UpdateScreen();

    while ((LastScan != ScanCode::sc_escape) && !IN_GetJoyButtonsDB(joystickport)) {
    }
    if (LastScan == ScanCode::sc_escape) {
        return;
    }

    IN_GetJoyAbs(joystickport, &maxx, &maxy);
    if ((minx == maxx) || (miny == maxy)) {
        return;
    }

    IN_SetupJoy(joystickport, minx, maxx, miny, maxy);
    while (IN_GetJoyButtonsDB(joystickport)) {
    }

    IN_ClearKeysDown();
    JoystickCalibrated = true;
}

void MouseSensitivity(
    int16_t)
{
    ControlInfo ci;
    int16_t exit = 0, oldMA;

    oldMA = mouseadjustment;
    DrawMouseSens();
    do {
        ReadAnyControl(&ci);
        switch (ci.dir) {
        case dir_North:
        case dir_West:
            if (::mouseadjustment > 0) {
                ::mouseadjustment -= 1;
                DrawMousePos();
                VW_UpdateScreen();
                ::sd_play_player_sound(MOVEGUN1SND, bstone::AC_ITEM);

                while (Keyboard[ScanCode::sc_left_arrow]) {
                    ::in_handle_events();
                }

                WaitKeyUp();
            }
            break;

        case dir_South:
        case dir_East:
            if (::mouseadjustment < ::max_mouse_sensitivity) {
                ::mouseadjustment += 1;
                DrawMousePos();
                VW_UpdateScreen();
                ::sd_play_player_sound(MOVEGUN1SND, bstone::AC_ITEM);

                while (Keyboard[ScanCode::sc_right_arrow]) {
                    ::in_handle_events();
                }

                WaitKeyUp();
            }
            break;

        default:
            break;
        }

        if (ci.button0 || Keyboard[ScanCode::sc_space] || Keyboard[ScanCode::sc_return]) {
            exit = 1;
        } else if (ci.button1 || Keyboard[ScanCode::sc_escape]) {
            exit = 2;
        }

    } while (!exit);

    if (exit == 2) {
        mouseadjustment = oldMA;
        ::sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);
    } else {
        ::sd_play_player_sound(SHOOTSND, bstone::AC_ITEM);
    }

    WaitKeyUp();
    ::MenuFadeOut();
}

// --------------------------------------------------------------------------
// DrawCtlScreen() - DRAW CONTROL MENU SCREEN
// --------------------------------------------------------------------------
void DrawCtlScreen()
{
    const int16_t Y_CTL_PIC_OFS = 3;

    int16_t i;
    int16_t x;
    int16_t y;

    ClearMScreen();
    DrawMenuTitle("CONTROL");
    DrawInstructions(IT_STANDARD);

    WindowX = 0;
    WindowW = 320;
    SETFONTCOLOR(TEXTCOLOR, BKGDCOLOR);

    if (JoysPresent[0]) {
        CtlMenu[1].active = AT_ENABLED;
        CtlMenu[2].active = AT_ENABLED;
        CtlMenu[3].active = AT_ENABLED;
        CtlMenu[4].active = AT_ENABLED;
    }

    CtlMenu[2].active = CtlMenu[3].active = CtlMenu[4].active = static_cast<activetypes>(joystickenabled);

    if (MousePresent) {
        CtlMenu[0].active = AT_ENABLED;
        CtlMenu[5].active = AT_ENABLED;
    }

    CtlMenu[5].active = static_cast<activetypes>(mouseenabled);

    fontnumber = 4;
    DrawMenu(&CtlItems, &CtlMenu[0]);

    x = CTL_X + CtlItems.indent - 24;
    y = CTL_Y + Y_CTL_PIC_OFS;
    if (mouseenabled) {
        VWB_DrawPic(x, y, C_SELECTEDPIC);
    } else {
        VWB_DrawPic(x, y, C_NOTSELECTEDPIC);
    }

    y = CTL_Y + 9 + Y_CTL_PIC_OFS;
    if (joystickenabled) {
        VWB_DrawPic(x, y, C_SELECTEDPIC);
    } else {
        VWB_DrawPic(x, y, C_NOTSELECTEDPIC);
    }

    y = CTL_Y + 9 * 2 + Y_CTL_PIC_OFS;
    if (joystickport) {
        VWB_DrawPic(x, y, C_SELECTEDPIC);
    } else {
        VWB_DrawPic(x, y, C_NOTSELECTEDPIC);
    }

    y = CTL_Y + 9 * 3 + Y_CTL_PIC_OFS;
    if (joypadenabled) {
        VWB_DrawPic(x, y, C_SELECTEDPIC);
    } else {
        VWB_DrawPic(x, y, C_NOTSELECTEDPIC);
    }

    //
    // PICK FIRST AVAILABLE SPOT
    //

    if (CtlItems.curpos < 0 || !CtlMenu[static_cast<int>(CtlItems.curpos)].active) {
        for (i = 0; i < CtlItems.amount; ++i) {
            if (CtlMenu[i].active) {
                CtlItems.curpos = static_cast<int8_t>(i);
                break;
            }
        }
    }

    DrawMenuGun(&CtlItems);
    VW_UpdateScreen();
}

enum ControlButton1 {
    FIRE,
    STRAFE,
    RUN,
    OPEN
}; // ControlButton1

char mbarray[4][3] = { "B0", "B1", "B2", "B3" };
int order[4] = { RUN, OPEN, FIRE, STRAFE, };

void CustomControls(
    int16_t)
{
    if (in_use_modern_bindings) {
        binds_draw_menu();
        return;
    }

    int16_t which;

    DrawCustomScreen();

    do {
        which = HandleMenu(&CusItems, &CusMenu[0], FixupCustom);

        switch (which) {
        case 0:
            DefineMouseBtns();
            DrawCustMouse(1);
            break;

        case 2:
            DefineJoyBtns();
            DrawCustJoy(0);
            break;

        case 4:
            DefineKeyBtns();
            DrawCustKeybd(0);
            break;

        case 5:
            DefineKeyMove();
            DrawCustKeys(0);
        }
    } while (which >= 0);



    ::MenuFadeOut();
}

void DefineMouseBtns()
{
    CustomCtrls mouseallowed = { 1, 1, 1, 1 };
    EnterCtrlData(2, &mouseallowed, DrawCustMouse, PrintCustMouse, MOUSE);
}

void DefineJoyBtns()
{
    CustomCtrls joyallowed = { 1, 1, 1, 1 };
    EnterCtrlData(5, &joyallowed, DrawCustJoy, PrintCustJoy, JOYSTICK);
}

void DefineKeyBtns()
{
    CustomCtrls keyallowed = { 1, 1, 1, 1 };
    EnterCtrlData(8, &keyallowed, DrawCustKeybd, PrintCustKeybd, KEYBOARDBTNS);
}

void DefineKeyMove()
{
    CustomCtrls keyallowed = { 1, 1, 1, 1 };
    EnterCtrlData(10, &keyallowed, DrawCustKeys, PrintCustKeys, KEYBOARDMOVE);
}

bool TestForValidKey(
    ScanCode Scan)
{
    auto found = false;

    auto it = std::find(buttonscan.begin(), buttonscan.end(), Scan);

    if (it == buttonscan.end()) {
        it = std::find(dirscan.begin(), dirscan.end(), Scan);

        found = (it != dirscan.end());
    }

    if (found) {
        *it = ScanCode::sc_none;
        ::sd_play_player_sound(SHOOTDOORSND, bstone::AC_ITEM);
        ::DrawCustomScreen();
    }

    return !found;
}


enum ControlButton2 {
    FWRD,
    RIGHT,
    BKWD,
    LEFT
}; // ControlButton2

int16_t moveorder[4] = { LEFT, RIGHT, FWRD, BKWD };

// --------------------------------------------------------------------------
// EnterCtrlData() - ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
// --------------------------------------------------------------------------
void EnterCtrlData(
    int16_t index,
    CustomCtrls* cust,
    void (* DrawRtn)(int16_t),
    void (* PrintRtn)(int16_t),
    int16_t type)
{
    int16_t j;
    int16_t exit;
    int16_t tick;
    int16_t redraw;
    int16_t which = 0;
    int16_t x = 0;
    int16_t picked;
    ControlInfo ci;
    bool clean_display = true;

    ShootSnd();
    PrintY = CST_Y + 13 * index;
    IN_ClearKeysDown();
    exit = 0;
    redraw = 1;

    CA_CacheGrChunk(STARTFONT + fontnumber);

    //
    // FIND FIRST SPOT IN ALLOWED ARRAY
    //
    for (j = 0; j < 4; j++) {
        if (cust->allowed[j]) {
            which = j;
            break;
        }
    }

    do {
        if (redraw) {
            x = CST_START + CST_SPC * which;
            DrawRtn(1);

            VWB_Bar(x - 1, PrintY - 1, CST_SPC, 7, HIGHLIGHT_BOX_COLOR);
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, HIGHLIGHT_BOX_COLOR);
            PrintRtn(which);
            PrintX = x;
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
            VW_UpdateScreen();
            WaitKeyUp();
            redraw = 0;
        }

        ReadAnyControl(&ci);

        if (type == MOUSE || type == JOYSTICK) {
            if (IN_KeyDown(ScanCode::sc_return) || IN_KeyDown(ScanCode::sc_control) || IN_KeyDown(ScanCode::sc_alt)) {
                IN_ClearKeysDown();
                ci.button0 = ci.button1 = false;
            }
        }

        //
        // CHANGE BUTTON VALUE?
        //

        if ((ci.button0 | ci.button1 | ci.button2 | ci.button3) ||
            ((type == KEYBOARDBTNS || type == KEYBOARDMOVE) && LastScan == ScanCode::sc_return))
        {
            tick = 0;
            TimeCount = 0;
            picked = 0;
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, HIGHLIGHT_BOX_COLOR);

            do {
                int16_t button, result = 0;

                if (type == KEYBOARDBTNS || type == KEYBOARDMOVE) {
                    IN_ClearKeysDown();
                }

                // BBi
                ::in_handle_events();

                //
                // FLASH CURSOR
                //

                if (TimeCount > 10) {
                    switch (tick) {
                    case 0:
                        VWB_Bar(x - 1, PrintY - 1, CST_SPC, 7, HIGHLIGHT_BOX_COLOR);
                        break;

                    case 1:
                        PrintX = x;
                        US_Print("?");

                        ::sd_play_player_sound(
                            HITWALLSND, bstone::AC_ITEM);
                    }

                    tick ^= 1;
                    TimeCount = 0;
                    VW_UpdateScreen();
                }

                //
                // WHICH TYPE OF INPUT DO WE PROCESS?
                //

                switch (type) {
                case MOUSE:
                    button = ::IN_MouseButtons();

                    switch (button) {
                    case 1: result = 1;
                        break;
                    case 2: result = 2;
                        break;
                    case 4: result = 3;
                        break;
                    }

                    if (result) {
                        int16_t z;

                        for (z = 0; z < 4; z++) {
                            if (order[which] == buttonmouse[z]) {
                                buttonmouse[z] = bt_nobutton;
                                break;
                            }
                        }

                        buttonmouse[result - 1] = static_cast<int16_t>(order[which]);
                        picked = 1;

                        ::sd_play_player_sound(
                            SHOOTDOORSND, bstone::AC_ITEM);

                        clean_display = false;
                    }
                    break;

                case JOYSTICK:
                    if (ci.button0) {
                        result = 1;
                    } else if (ci.button1) {
                        result = 2;
                    } else if (ci.button2) {
                        result = 3;
                    } else if (ci.button3) {
                        result = 4;
                    }

                    if (result) {
                        int16_t z;

                        for (z = 0; z < 4; z++) {
                            if (order[which] == buttonjoy[z]) {
                                buttonjoy[z] = bt_nobutton;
                                break;
                            }
                        }

                        buttonjoy[result - 1] = static_cast<int16_t>(order[which]);
                        picked = 1;

                        ::sd_play_player_sound(SHOOTDOORSND, bstone::AC_ITEM);

                        clean_display = false;
                    }
                    break;

                case KEYBOARDBTNS:
                    if (LastScan != ScanCode::sc_none) {
                        if (LastScan == ScanCode::sc_escape) {
                            break;
                        }

                        auto it = std::find(
                            special_keys.cbegin(),
                            special_keys.cend(),
                            LastScan);

                        if (it != special_keys.cend()) {
                            ::sd_play_player_sound(NOWAYSND, bstone::AC_ITEM);
                        } else {
                            clean_display = TestForValidKey(LastScan);

                            if (clean_display) {
                                ShootSnd();
                            }

                            buttonscan[order[which]] = LastScan;

                            picked = 1;
                        }
                        IN_ClearKeysDown();
                    }
                    break;


                case KEYBOARDMOVE:
                    if (LastScan != ScanCode::sc_none) {
                        if (LastScan == ScanCode::sc_escape) {
                            break;
                        }

                        auto it = std::find(
                            special_keys.cbegin(),
                            special_keys.cend(),
                            LastScan);

                        if (it != special_keys.cend()) {
                            ::sd_play_player_sound(NOWAYSND, bstone::AC_ITEM);
                        } else {
                            clean_display = TestForValidKey(LastScan);

                            if (clean_display) {
                                ShootSnd();
                            }

                            dirscan[moveorder[which]] = LastScan;
                            picked = 1;
                        }
                        IN_ClearKeysDown();
                    }
                    break;
                }


                //
                // EXIT INPUT?
                //

                if (IN_KeyDown(ScanCode::sc_escape)) {
                    picked = 1;
                    continue;
                }

            } while (!picked);

            if (!clean_display) {
                DrawCustomScreen();
            }

            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
            redraw = 1;
            WaitKeyUp();
            continue;
        }

        if (ci.button1 || IN_KeyDown(ScanCode::sc_escape)) {
            exit = 1;
        }

        //
        // MOVE TO ANOTHER SPOT?
        //
        switch (ci.dir) {

        case dir_West:
            VWB_Bar(x - 1, PrintY - 1, CST_SPC, 7, ::menu_background_color);
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
            PrintRtn(which);
            do {
                which--;
                if (which < 0) {
                    which = 3;
                }
            } while (!cust->allowed[which]);

            redraw = 1;

            ::sd_play_player_sound(MOVEGUN1SND, bstone::AC_ITEM);

            while (ReadAnyControl(&ci), ci.dir != dir_None) {
            }
            IN_ClearKeysDown();
            break;



        case dir_East:
            VWB_Bar(x - 1, PrintY - 1, CST_SPC, 7, ::menu_background_color);
            SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
            PrintRtn(which);
            do {
                which++;
                if (which > 3) {
                    which = 0;
                }
            } while (!cust->allowed[which]);

            redraw = 1;

            ::sd_play_player_sound(MOVEGUN1SND, bstone::AC_ITEM);

            while (ReadAnyControl(&ci), ci.dir != dir_None) {
            }

            IN_ClearKeysDown();
            break;

        case dir_North:
        case dir_South:
            exit = 1;

        default:
            break;
        }

    } while (!exit);

    FREEFONT(STARTFONT + fontnumber);

    ::sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);

    WaitKeyUp();
}

// --------------------------------------------------------------------------
// FIXUP GUN CURSOR OVERDRAW SHIT
// --------------------------------------------------------------------------
void FixupCustom(
    int16_t w)
{
    static int16_t lastwhich = -1;

    switch (w) {
    case 0: DrawCustMouse(1);
        break;
    case 2: DrawCustJoy(1);
        break;
    case 4: DrawCustKeybd(1);
        break;
    case 5: DrawCustKeys(1);
    }


    if (lastwhich >= 0) {
        if (lastwhich != w) {
            switch (lastwhich) {
            case 0: DrawCustMouse(0);
                break;
            case 2: DrawCustJoy(0);
                break;
            case 4: DrawCustKeybd(0);
                break;
            case 5: DrawCustKeys(0);
            }
        }
    }

    lastwhich = w;
}

void DrawCustomScreen()
{
    int16_t i;

    ClearMScreen();
    DrawMenuTitle("CUSTOMIZE");
    DrawInstructions(IT_STANDARD);

    //
    // MOUSE
    //

    WindowX = 32;
    WindowW = 244;

    fontnumber = 4;

    SETFONTCOLOR(0x0C, TERM_BACK_COLOR);


    PrintY = 49;
    US_CPrint("MOUSE\n");
    PrintY = 79;
    US_CPrint("JOYSTICK/GRAVIS GAMEPAD\n");
    PrintY = 109;
    US_CPrint("KEYBOARD\n");

    fontnumber = 2;

    SETFONTCOLOR(DISABLED_TEXT_COLOR, TERM_BACK_COLOR);

    for (i = 60; i <= 120; i += 30) {
        ShadowPrint("RUN", CST_START, i);
        ShadowPrint("OPEN", CST_START + CST_SPC * 1, i);
        ShadowPrint("FIRE", CST_START + CST_SPC * 2, i);
        ShadowPrint("STRAFE", CST_START + CST_SPC * 3, i);
    }

    ShadowPrint("LEFT", CST_START, 135);
    ShadowPrint("RIGHT", CST_START + CST_SPC * 1, 135);
    ShadowPrint("FWRD", CST_START + CST_SPC * 2, 135);
    ShadowPrint("BKWRD", CST_START + CST_SPC * 3, 135);


    DrawCustMouse(0);
    DrawCustJoy(0);
    DrawCustKeybd(0);
    DrawCustKeys(0);

    //
    // PICK STARTING POINT IN MENU
    //
    if (CusItems.curpos < 0) {
        for (i = 0; i < CusItems.amount; i++) {
            if (CusMenu[i].active) {
                CusItems.curpos = static_cast<int8_t>(i);
                break;
            }
        }
    }

    VW_UpdateScreen();
    MenuFadeIn();
}

void PrintCustMouse(
    int16_t i)
{
    int16_t j;

    for (j = 0; j < 4; j++) {
        if (order[i] == buttonmouse[j]) {
            PrintX = CST_START + CST_SPC * i;
            US_Print(mbarray[j]);
            break;
        }
    }
}

void DrawCustMouse(
    int16_t hilight)
{
    int16_t i, color;

    color = ENABLED_TEXT_COLOR;

    if (hilight) {
        color = HIGHLIGHT_TEXT_COLOR;
    }

    SETFONTCOLOR(color, TERM_BACK_COLOR);

    if (!mouseenabled) {
        SETFONTCOLOR(DISABLED_TEXT_COLOR, TERM_BACK_COLOR);
        CusMenu[0].active = AT_DISABLED;
    } else {
        CusMenu[0].active = AT_ENABLED;
    }

    PrintY = CST_Y + 7;
    for (i = 0; i < 4; i++) {
        PrintCustMouse(i);
    }
}

void PrintCustJoy(
    int16_t i)
{
    int16_t j;

    for (j = 0; j < 4; j++) {
        if (order[i] == buttonjoy[j]) {
            PrintX = CST_START + CST_SPC * i;
            US_Print(mbarray[j]);
            break;
        }
    }
}

void DrawCustJoy(
    int16_t hilight)
{
    int16_t i, color;


    color = ENABLED_TEXT_COLOR;
    if (hilight) {
        color = HIGHLIGHT_TEXT_COLOR;
    }

    SETFONTCOLOR(color, TERM_BACK_COLOR);

    if (!joystickenabled) {
        SETFONTCOLOR(DISABLED_TEXT_COLOR, TERM_BACK_COLOR);
        CusMenu[2].active = AT_DISABLED;
    } else {
        CusMenu[2].active = AT_ENABLED;
    }

    PrintY = CST_Y + 37;
    for (i = 0; i < 4; i++) {
        PrintCustJoy(i);
    }
}

void PrintCustKeybd(
    int16_t i)
{
    PrintX = CST_START + CST_SPC * i;
    US_Print(IN_GetScanName(buttonscan[order[i]]).c_str());
}

void DrawCustKeybd(
    int16_t hilight)
{
    int16_t i, color;

    if (hilight) {
        color = HIGHLIGHT_TEXT_COLOR;
    } else {
        color = ENABLED_TEXT_COLOR;
    }

    SETFONTCOLOR(color, TERM_BACK_COLOR);

    PrintY = CST_Y + 67;

    for (i = 0; i < 4; i++) {
        PrintCustKeybd(i);
    }
}

void PrintCustKeys(
    int16_t i)
{
    PrintX = CST_START + CST_SPC * i;
    US_Print(IN_GetScanName(dirscan[moveorder[i]]).c_str());
}

void DrawCustKeys(
    int16_t hilight)
{
    int16_t i, color;

    color = ENABLED_TEXT_COLOR;

    if (hilight) {
        color = HIGHLIGHT_TEXT_COLOR;
    }

    SETFONTCOLOR(color, TERM_BACK_COLOR);

    PrintY = CST_Y + 82;
    for (i = 0; i < 4; i++) {
        PrintCustKeys(i);
    }
}

void CP_Quit()
{
    if (Confirm(QuitToDosStr)) {
        ExitGame();
    }

    DrawMainMenu();
}

// ---------------------------------------------------------------------------
// Clear Menu screens to dark red
// ---------------------------------------------------------------------------
void ClearMScreen()
{
    VWB_Bar(SCREEN_X, SCREEN_Y, SCREEN_W, SCREEN_H, ::menu_background_color);
}

// ---------------------------------------------------------------------------
// Un/Cache a LUMP of graphics
// ---------------------------------------------------------------------------
void CacheLump(
    int16_t lumpstart,
    int16_t lumpend)
{
    int16_t i;

    for (i = lumpstart; i <= lumpend; i++) {
        CA_CacheGrChunk(i);
    }
}

void UnCacheLump(
    int16_t lumpstart,
    int16_t lumpend)
{
    int16_t i;

    for (i = lumpstart; i <= lumpend; i++) {
        FREEFONT(i);
    }
}

void DrawWindow(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t wcolor)
{
    VWB_Bar(x, y, w, h, static_cast<uint8_t>(wcolor));
    DrawOutline(x, y, w, h, BORD2COLOR, DEACTIVE);
}

void DrawOutline(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t color1,
    int16_t color2)
{
    VWB_Hlin(x, x + w, y, static_cast<uint8_t>(color2));
    VWB_Vlin(y, y + h, x, static_cast<uint8_t>(color2));
    VWB_Hlin(x, x + w, y + h, static_cast<uint8_t>(color1));
    VWB_Vlin(y, y + h, x + w, static_cast<uint8_t>(color1));
}

void SetupControlPanel()
{
    // BBi
    SwitchItems.amount = (::is_ps() ? 7 : 9);
    SwitchItems.y = MENU_Y + (::is_ps() ? 15 : 7);
    // BBi

    ControlPanelAlloc();

    fontnumber = 2;

    WindowH = 200;

    if (!ingame) {
        CA_LoadAllSounds();
    } else {
        MainMenu[MM_SAVE_MISSION].active = AT_ENABLED;
    }

    ReadGameNames();
}

void ReadGameNames()
{
    for (int i = 0; i < 10; ++i) {
        auto name = ::get_saved_game_base_name();
        name += static_cast<char>('0' + i);

        auto name_path = ::get_profile_dir() + name;

        bstone::FileStream stream(name_path);

        if (!stream.is_open()) {
            continue;
        }

        SaveGamesAvail[i] = 1;

        int chunk_size = ::FindChunk(&stream, "DESC");

        if (chunk_size > 0) {
            char temp[GAME_DESCRIPTION_LEN + 1];

            std::uninitialized_fill_n(
                temp,
                GAME_DESCRIPTION_LEN,
                '\0');

            auto temp_size = std::min(GAME_DESCRIPTION_LEN, chunk_size);

            stream.read(temp, temp_size);

            ::strcpy(&SaveGameNames[i][0], temp);
        } else {
            ::strcpy(&SaveGameNames[i][0], "DESCRIPTION LOST");
        }
    }
}

void CleanupControlPanel()
{
    if (!loadedgame) {
        FreeMusic();
    }
    ControlPanelFree();
    fontnumber = 4;
}

// ---------------------------------------------------------------------------
// ControlPanelFree() - This FREES the control panel lump from memory
//      and REALLOCS the ScaledDirectory
// ---------------------------------------------------------------------------
void ControlPanelFree()
{
    UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
    NewViewSize();
}

// ---------------------------------------------------------------------------
// ControlPanelAlloc() - This CACHEs the control panel lump into memory
//      and FREEs the ScaledDirectory.
// ---------------------------------------------------------------------------
void ControlPanelAlloc()
{
    CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
}

// ---------------------------------------------------------------------------
// ShadowPrint() - Shadow Prints given text @ a given x & y in default font
//
// NOTE: Font MUST already be loaded
// ---------------------------------------------------------------------------
void ShadowPrint(
    const char* strng,
    int16_t x,
    int16_t y)
{
    int16_t old_bc, old_fc;

    old_fc = fontcolor;
    old_bc = backcolor;

    PrintX = x + 1;
    PrintY = y + 1;

    SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
    US_Print(strng);

    PrintX = x;
    PrintY = y;
    SETFONTCOLOR(old_fc, old_bc);
    US_Print(strng);
}

// ---------------------------------------------------------------------------
// HandleMenu() - Handle moving gun around a menu
// ---------------------------------------------------------------------------
int16_t HandleMenu(
    CP_iteminfo* item_i,
    CP_itemtype* items,
    void (* routine)(int16_t w))
{
#define box_on item_i->cursor.on
    int8_t key;
    static int16_t redrawitem = 1;

    int16_t i, x, y, basey, exit, which, flash_tics;
    ControlInfo ci;

    which = item_i->curpos;
    x = item_i->x;
    basey = item_i->y;
    y = basey + which * item_i->y_spacing;
    box_on = 1;
    DrawGun(item_i, items, x, &y, which, basey, routine);

    SetTextColor(items + which, 1);

    if (redrawitem) {
        ShadowPrint((items + which)->string, item_i->x + item_i->indent, item_i->y + which * item_i->y_spacing);
    }

    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //

    if (routine) {
        routine(which);
    }

    VW_UpdateScreen();

    flash_tics = 40;
    exit = 0;
    TimeCount = 0;
    IN_ClearKeysDown();

    do {
        CalcTics();
        flash_tics -= tics;

        CycleColors();

        //
        // CHANGE GUN SHAPE
        //

        if (flash_tics <= 0) {
            flash_tics = 40;

            box_on ^= 1;

            if (box_on) {
                DrawGun(item_i, items, x, &y, which, basey, routine);
            } else {
                EraseGun(item_i, items, x, y, which);
                if (routine) {
                    routine(which);
                }
            }


            VW_UpdateScreen();
        }

        CheckPause();


        //
        // SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
        //

        key = LastASCII;
        if (key) {
            int16_t ok = 0;

            if (key >= 'a') {
                key -= 'a' - 'A';
            }

            for (i = which + 1; i < item_i->amount; i++) {
                if ((items + i)->active && (items + i)->string[0] == key) {
                    EraseGun(item_i, items, x, y, which);
                    which = i;
                    item_i->curpos = static_cast<int8_t>(which); // jtr -testing
                    box_on = 1;
                    DrawGun(item_i, items, x, &y, which, basey, routine);
                    VW_UpdateScreen();

                    ok = 1;
                    IN_ClearKeysDown();
                    break;
                }
            }

            //
            // DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
            //

            if (!ok) {
                for (i = 0; i < which; i++) {
                    if ((items + i)->active && (items + i)->string[0] == key) {
                        EraseGun(item_i, items, x, y, which);
                        which = i;
                        item_i->curpos = static_cast<int8_t>(which); // jtr -testing
                        box_on = 1;
                        DrawGun(item_i, items, x, &y, which, basey, routine);
                        VW_UpdateScreen();

                        IN_ClearKeysDown();
                        break;
                    }
                }
            }
        }

        //
        // GET INPUT
        //

        ReadAnyControl(&ci);

        switch (ci.dir) {
        // ------------------------
        // MOVE UP
        //
        case dir_North:
            EraseGun(item_i, items, x, y, which);

            do {
                if (!which) {
                    which = item_i->amount - 1;
                } else {
                    which--;
                }

            } while (!(items + which)->active);

            item_i->curpos = static_cast<int8_t>(which); // jtr -testing

            box_on = 1;
            DrawGun(item_i, items, x, &y, which, basey, routine);

            VW_UpdateScreen();

            TicDelay(20);
            break;

        // --------------------------
        // MOVE DOWN
        //
        case dir_South:
            EraseGun(item_i, items, x, y, which);

            do {
                if (which == item_i->amount - 1) {
                    which = 0;
                } else {
                    which++;
                }
            } while (!(items + which)->active);

            item_i->curpos = static_cast<int8_t>(which); // jtr -testing

            box_on = 1;
            DrawGun(item_i, items, x, &y, which, basey, routine);

            VW_UpdateScreen();

            TicDelay(20);
            break;

        default:
            break;
        }

        if (ci.button0 || Keyboard[ScanCode::sc_space] || Keyboard[ScanCode::sc_return]) {
            exit = 1;
        }

        if (ci.button1 || Keyboard[ScanCode::sc_escape]) {
            exit = 2;
        }

    } while (!exit);

    IN_ClearKeysDown();

    //
    // ERASE EVERYTHING
    //

    box_on = 0;
    redrawitem = 1;
    EraseGun(item_i, items, x, y, which);

    if (routine) {
        routine(which);
    }

    VW_UpdateScreen();

    item_i->curpos = static_cast<int8_t>(which);

    switch (exit) {
    case 1:
        //
        // CALL THE ROUTINE
        //
        if ((items + which)->routine) {
            // Make sure there's room to save when CP_SaveGame() is called.
            //
            if (reinterpret_cast<size_t>(items[which].routine) == reinterpret_cast<size_t>(CP_SaveGame)) {
                if (!CheckDiskSpace(DISK_SPACE_NEEDED, CANT_SAVE_GAME_TXT, cds_menu_print)) {
                    return which;
                }
            }

            //
            // ALREADY IN A GAME?
            //
            if (::is_ps() && ingame && ((items + which)->routine == CP_NewGame)) {
                if (!Confirm(CURGAME)) {
                    ::MenuFadeOut();
                    return 0;
                }
            }

            ShootSnd();
            ::MenuFadeOut();
            (items + which)->routine(0);
        }
        return which;

    case 2:
        ::sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);

        return -1;
    }

    return 0;     // JUST TO SHUT UP THE ERROR MESSAGES!
}

// ---------------------------------------------------------------------------
// ERASE GUN & DE-HIGHLIGHT STRING
// ---------------------------------------------------------------------------
void EraseGun(
    CP_iteminfo* item_i,
    CP_itemtype* items,
    int16_t x,
    int16_t y,
    int16_t which)
{
    static_cast<void>(x);

    VWB_Bar(item_i->cursor.x, y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, ::menu_background_color);
    SetTextColor(items + which, 0);

    ShadowPrint((items + which)->string, item_i->x + item_i->indent, y);
}

// ---------------------------------------------------------------------------
// DrawGun() - DRAW GUN AT NEW POSITION
// ---------------------------------------------------------------------------
void DrawGun(
    CP_iteminfo* item_i,
    CP_itemtype* items,
    int16_t x,
    int16_t* y,
    int16_t which,
    int16_t basey,
    void (* routine)(int16_t w))
{
    static_cast<void>(x);

    *y = basey + which * item_i->y_spacing;

    VWB_Bar(item_i->cursor.x, *y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, HIGHLIGHT_BOX_COLOR);
    SetTextColor(items + which, 1);

    ShadowPrint((items + which)->string, item_i->x + item_i->indent, item_i->y + which * item_i->y_spacing);

    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //

    if (routine) {
        routine(which);
    }
}

// ---------------------------------------------------------------------------
// TicDelay() - DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
// ---------------------------------------------------------------------------
void TicDelay(
    int16_t count)
{
    ControlInfo ci;

    TimeCount = 0;

    do {
        ReadAnyControl(&ci);
    } while (TimeCount < static_cast<uint32_t>(count) && ci.dir != dir_None);
}

// ---------------------------------------------------------------------------
// DrawMenu() - Draw a menu
//
//       This also calculates the Y position of the current items in the
//                      CP_itemtype structures.
// ---------------------------------------------------------------------------
void DrawMenu(
    CP_iteminfo* item_i,
    CP_itemtype* items)
{
    int16_t i, which = item_i->curpos;

    WindowX = PrintX = item_i->x + item_i->indent;
    WindowY = PrintY = item_i->y;

    WindowW = 320;
    WindowH = 200;

    for (i = 0; i < item_i->amount; i++) {
        SetTextColor(items + i, which == i);
        ShadowPrint((items + i)->string, WindowX, item_i->y + i * item_i->y_spacing);
    }
}

// ---------------------------------------------------------------------------
// SET TEXT COLOR (HIGHLIGHT OR NO)
// ---------------------------------------------------------------------------
void SetTextColor(
    CP_itemtype* items,
    int16_t hlight)
{
    if (hlight) {
        SETFONTCOLOR(color_hlite[items->active], TERM_BACK_COLOR);
    } else {
        SETFONTCOLOR(color_norml[items->active], TERM_BACK_COLOR);
    }
}

// ---------------------------------------------------------------------------
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
// ---------------------------------------------------------------------------
void WaitKeyUp()
{
    for (auto quit = false; !quit; ) {
        ControlInfo ci;

        ::ReadAnyControl(&ci);

        quit = !(
            ci.button0 != 0 ||
            ci.button1 != 0 ||
            ci.button2 != 0 ||
            ci.button3 != 0 ||
            Keyboard[ScanCode::sc_space] ||
            Keyboard[ScanCode::sc_return] ||
            Keyboard[ScanCode::sc_escape]);
    }
}

// ---------------------------------------------------------------------------
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
// ---------------------------------------------------------------------------
void ReadAnyControl(
    ControlInfo* ci)
{
    bool mouseactive = false;

    ::IN_ReadControl(0, ci);

    //
    // UNDO some of the ControlInfo vars that were init
    // with IN_ReadControl() for the mouse...
    //
    if (ControlTypeUsed == ctrl_Mouse) {
        //
        // Clear directions & buttons (if enabled or not)
        //
        ci->dir = dir_None;
        ci->button0 = 0;
        ci->button1 = 0;
        ci->button2 = 0;
        ci->button3 = 0;
    }

    if (mouseenabled) {
        int mousex;
        int mousey;

        // READ MOUSE MOTION COUNTERS
        // RETURN DIRECTION
        // HOME MOUSE
        // CHECK MOUSE BUTTONS

        ::in_get_mouse_deltas(mousex, mousey);
        ::in_clear_mouse_deltas();

        const int DELTA_THRESHOLD = 10;

        if (mousey < -DELTA_THRESHOLD) {
            ci->dir = dir_North;
            mouseactive = true;
        } else if (mousey > DELTA_THRESHOLD) {
            ci->dir = dir_South;
            mouseactive = true;
        }

        if (mousex < -DELTA_THRESHOLD) {
            ci->dir = dir_West;
            mouseactive = true;
        } else if (mousex > DELTA_THRESHOLD) {
            ci->dir = dir_East;
            mouseactive = true;
        }

        int buttons = ::IN_MouseButtons();

        if (buttons != 0) {
            ci->button0 = buttons & 1;
            ci->button1 = buttons & 2;
            ci->button2 = buttons & 4;
            ci->button3 = false;
            mouseactive = true;
        }
    }

    if (joystickenabled && !mouseactive) {
        int16_t jx;
        int16_t jy;
        int16_t jb;

        ::INL_GetJoyDelta(joystickport, &jx, &jy);

        if (jy < -SENSITIVE) {
            ci->dir = dir_North;
        } else if (jy > SENSITIVE) {
            ci->dir = dir_South;
        }

        if (jx < -SENSITIVE) {
            ci->dir = dir_West;
        } else if (jx > SENSITIVE) {
            ci->dir = dir_East;
        }

        jb = ::IN_JoyButtons();

        if (jb != 0) {
            ci->button0 = jb & 1;
            ci->button1 = jb & 2;

            if (joypadenabled) {
                ci->button2 = jb & 4;
                ci->button3 = jb & 8;
            } else {
                ci->button2 = false;
                ci->button3 = false;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
int16_t Confirm(
    const char* string)
{
    int16_t xit = 0, x, y, tick = 0, whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };


    Message(string);

// Next two lines needed for flashing cursor ...
//
    SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
    CA_CacheGrChunk(STARTFONT + fontnumber);

    IN_ClearKeysDown();

    //
    // BLINK CURSOR
    //
    x = PrintX;
    y = PrintY;
    TimeCount = 0;
    do {
        if (TimeCount >= 10) {
            switch (tick) {
            case 0:
                VWB_Bar(x, y, 8, 13, BORDER_MED_COLOR);
                break;

            case 1:
                PrintX = x;
                PrintY = y;
                US_Print("_");
            }

            VW_UpdateScreen();
            tick ^= 1;
            TimeCount = 0;
        }

        // BBi
        IN_CheckAck();
    } while (!Keyboard[ScanCode::sc_y] && !Keyboard[ScanCode::sc_n] && !Keyboard[ScanCode::sc_escape]);


    if (Keyboard[ScanCode::sc_y]) {
        xit = 1;
        ShootSnd();
    }

    while (Keyboard[ScanCode::sc_y] || Keyboard[ScanCode::sc_n] || Keyboard[ScanCode::sc_escape]) {
        IN_CheckAck();
    }

    IN_ClearKeysDown();

    ::sd_play_player_sound(
        whichsnd[xit],
        bstone::AC_ITEM);

    FREEFONT(STARTFONT + fontnumber);

    return xit;
}

// ---------------------------------------------------------------------------
// PRINT A MESSAGE IN A WINDOW
// ---------------------------------------------------------------------------
void Message(
    const char* string)
{
    int16_t h = 0, w = 0, mw = 0;
    size_t i;
    fontstruct* font;
    uint16_t OldPrintX, OldPrintY;

    fontnumber = 1;
    CA_CacheGrChunk(STARTFONT + 1);

    font = static_cast<fontstruct*>(grsegs[STARTFONT + fontnumber]);

    h = font->height;
    for (i = 0; i < strlen(string); i++) {
        if (string[i] == '\n') {
            if (w > mw) {
                mw = w;
            }
            w = 0;
            h += font->height;
        } else {
            w += font->width[static_cast<int>(string[i])];
        }
    }

    if (w + 10 > mw) {
        mw = w + 10;
    }

    OldPrintY = PrintY = (WindowH / 2) - h / 2;
    OldPrintX = PrintX = WindowX = 160 - mw / 2;

    // bump down and to right for shadow

    PrintX++;
    PrintY++;
    WindowX++;

    BevelBox(WindowX - 6, PrintY - 6, mw + 10, h + 10, BORDER_HI_COLOR, BORDER_MED_COLOR, BORDER_LO_COLOR);

    SETFONTCOLOR(BORDER_LO_COLOR, BORDER_MED_COLOR);
    US_Print(string);

    PrintY = OldPrintY;
    WindowX = PrintX = OldPrintX;

    SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
    US_Print(string);

    FREEFONT(STARTFONT + 1);

    VW_UpdateScreen();
}

// --------------------------------------------------------------------------
// Searches for an "^XX" and replaces with a 0 (NULL)
// --------------------------------------------------------------------------
void TerminateStr(
    char* pos)
{
    pos = strstr(pos, "^XX");
    *pos = 0;
}

// ---------------------------------------------------------------------------
// Caches and prints a message in a window.
// ---------------------------------------------------------------------------
void CacheMessage(
    uint16_t MessageNum)
{
    char* string;

    CA_CacheGrChunk(MessageNum);
    string = (char*)grsegs[MessageNum];

    TerminateStr(string);

    Message(string);

    FREEFONT(MessageNum);
}

// ---------------------------------------------------------------------------
// CacheCompData() - Caches and Decompresses data from the VGAGRAPH
//
// NOTE: - User is responsible for freeing loaded data
//       - Returns the size of the data
//       - Does not call TerminateStr() for loaded TEXT data
//
// RETURNS: Lenght of loaded (decompressed) data
//
// ---------------------------------------------------------------------------
uint32_t CacheCompData(
    uint16_t item_number,
    void** dst_ptr)
{
    char* chunk;
    char* dst;
    CompHeader_t CompHeader {};
    uint32_t data_length;

    // Load compressed data
    CA_CacheGrChunk(item_number);
    chunk = (char*)grsegs[item_number];

    if (!::is_ps()) {
        data_length = ::ca_gr_last_expanded_size;
    } else {
        memcpy(CompHeader.NameId, &chunk[0], 4);
        CompHeader.OriginalLen = ((uint32_t*)&chunk[4])[0];
        CompHeader.CompType = (ct_TYPES)((int16_t*)&chunk[8])[0];
        CompHeader.CompressLen = ((uint32_t*)&chunk[10])[0];

        data_length = CompHeader.OriginalLen;

        chunk += 14;
    }

    // Allocate Dest Memory

    dst = new char[data_length];
    *dst_ptr = dst;

    if (!::is_ps()) {
        std::copy(
            chunk,
            &chunk[data_length],
            dst);
    } else {
        // Decompress and terminate string

        if (!LZH_Startup()) {
            Quit("out of memory");
        }

        ::LZH_Decompress(
            chunk,
            dst,
            data_length,
            CompHeader.CompressLen);

        LZH_Shutdown();
    }

    // Free compressed data
    UNCACHEGRCHUNK(item_number);

    // Return loaded size
    return data_length;
}

void StartCPMusic(
    int16_t song)
{
    int chunk;

    lastmenumusic = song;

    SD_MusicOff();
    chunk = song;
    CA_CacheAudioChunk(static_cast<int16_t>(STARTMUSIC + chunk));
    ::SD_StartMusic(chunk);
}

void FreeMusic()
{
    SD_MusicOff();
}


#ifdef CACHE_KEY_DATA
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
uint8_t far* IN_GetScanName(
    ScanCode scan)
{
    uint8_t* p;
    ScanCode* s;

    for (s = ExtScanCodes, p = ExtScanNames; *s; p += 7, s++) {
        if (*s == scan) {
            return (uint8_t*)p;
        }
    }

    return (uint8_t*)(ScanNames + (scan << 1));
}
#else
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
const std::string& IN_GetScanName(
    ScanCode scan)
{
    for (auto i = 0; ext_scan_codes[i] != ScanCode::sc_none; ++i) {
        if (ext_scan_codes[i] == scan) {
            return ext_scan_names[i];
        }
    }

    return scan_names[static_cast<int>(scan)];
}
#endif

// ---------------------------------------------------------------------------
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
// ---------------------------------------------------------------------------
void CheckPause()
{
    if (Paused) {
        switch (SoundStatus) {
        case 0:
            SD_MusicOn();
            break;

        case 1:
            SD_MusicOff();
            break;
        }

        SoundStatus ^= 1;
        VW_WaitVBL(3);
        IN_ClearKeysDown();
        Paused = false;
    }
}

// -------------------------------------------------------------------------
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
// -------------------------------------------------------------------------
void DrawMenuGun(
    CP_iteminfo* iteminfo)
{
    int16_t x, y;

    x = iteminfo->cursor.x;
    y = iteminfo->y + iteminfo->curpos * iteminfo->y_spacing + iteminfo->cursor.y_ofs;

    VWB_Bar(x, y, iteminfo->cursor.width, iteminfo->cursor.height, HIGHLIGHT_BOX_COLOR);
}

void ShootSnd()
{
    ::sd_play_player_sound(SHOOTSND, bstone::AC_ITEM);
}

void ShowPromo()
{
    const auto PROMO_MUSIC = HIDINGA_MUS;

// Load and start music
//
    ::CA_CacheAudioChunk(STARTMUSIC + PROMO_MUSIC);
    ::SD_StartMusic(PROMO_MUSIC);

// Show promo screen 1
//
    ::MenuFadeOut();
    ::CA_CacheScreen(PROMO1PIC);
    VW_UpdateScreen();
    MenuFadeIn();
    ::IN_UserInput(TickBase * 20);

// Show promo screen 2
//
    ::MenuFadeOut();
    ::CA_CacheScreen(PROMO2PIC);
    VW_UpdateScreen();
    MenuFadeIn();
    ::IN_UserInput(TickBase * 20);

// Music off and freed!
//
    ::StopMusic();
}

void ExitGame()
{
    VW_FadeOut();

    if (::is_aog_sw() && !::no_screens) {
        ::ShowPromo();
    }

    SD_MusicOff();
    SD_StopSound();
    Quit();
}

// BBi
int volume_index = 0;
int* const volumes[2] = { &sd_sfx_volume, &sd_music_volume };

void draw_volume_control(
    int index,
    int volume,
    bool is_enabled)
{
    int16_t slider_color =
        is_enabled ? ENABLED_TEXT_COLOR : DISABLED_TEXT_COLOR;

    int16_t outline_color =
        is_enabled ? HIGHLIGHT_TEXT_COLOR : DEACTIAVED_TEXT_COLOR;

    int y = 82 + (index * 40);

    VWB_Bar(74, static_cast<int16_t>(y), 160, 8, HIGHLIGHT_BOX_COLOR);
    DrawOutline(73, static_cast<int16_t>(y - 1), 161, 9,
                outline_color, outline_color);
    VWB_Bar(static_cast<int16_t>(74 + ((160 * volume) / (::sd_max_volume + 1))),
            static_cast<int16_t>(y), 16, 8, static_cast<uint8_t>(slider_color));
}

void draw_volume_controls()
{
    for (int i = 0; i < 2; ++i) {
        draw_volume_control(i, *(volumes[i]), i == volume_index);
    }
}

void cp_sound_volume(
    int16_t)
{
    ClearMScreen();
    DrawMenuTitle("SOUND VOLUME");
    DrawInstructions(IT_SOUND_VOLUME);

    fontnumber = 4;

    SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);

    PrintX = 150;
    PrintY = 60;
    US_Print("SFX");

    PrintX = 145;
    PrintY = 105;
    US_Print("MUSIC");

    for (int i = 0; i < 2; ++i) {
        PrintX = 36;
        PrintY = static_cast<uint16_t>(81 + (i * 40));
        US_Print("MUTE");

        PrintX = 242;
        US_Print("LOUD");
    }

    draw_volume_controls();

    VW_UpdateScreen();
    MenuFadeIn();

    ControlInfo ci;

    int old_volumes[2];
    for (int i = 0; i < 2; ++i) {
        old_volumes[i] = -1;
    }

    for (bool quit = false; !quit; ) {
        bool update_volumes = false;
        bool redraw_controls = false;

        ReadAnyControl(&ci);

        switch (ci.dir) {
        case dir_North:
            if (volume_index == 1) {
                redraw_controls = true;
                volume_index = 0;
                draw_volume_controls();
                VW_UpdateScreen();
            }

            while (Keyboard[ScanCode::sc_up_arrow]) {
                ::in_handle_events();
            }
            break;

        case dir_South:
            if (volume_index == 0) {
                redraw_controls = true;
                volume_index = 1;
                draw_volume_controls();
                VW_UpdateScreen();
            }

            while (Keyboard[ScanCode::sc_down_arrow]) {
                ::in_handle_events();
            }
            break;

        case dir_West:
            if (*volumes[volume_index] > ::sd_min_volume) {
                redraw_controls = true;
                update_volumes = true;
                --(*volumes[volume_index]);
                draw_volume_controls();
                VW_UpdateScreen();
            }

            while (Keyboard[ScanCode::sc_left_arrow]) {
                ::in_handle_events();
            }
            break;

        case dir_East:
            if (*volumes[volume_index] < ::sd_max_volume) {
                redraw_controls = true;
                update_volumes = true;
                ++(*volumes[volume_index]);
            }

            while (Keyboard[ScanCode::sc_right_arrow]) {
                ::in_handle_events();
            }
            break;

        default:
            break;
        }

        if (update_volumes) {
            update_volumes = false;

            if (old_volumes[0] != *volumes[0]) {
                sd_set_sfx_volume(sd_sfx_volume);
                sd_play_player_sound(MOVEGUN1SND, bstone::AC_ITEM);
            }

            if (old_volumes[1] != *volumes[1]) {
                sd_set_music_volume(sd_music_volume);
            }
        }

        if (redraw_controls) {
            redraw_controls = false;
            draw_volume_controls();
            VW_UpdateScreen();
        }

        quit = (ci.button1 || Keyboard[ScanCode::sc_escape]);
    }

    sd_play_player_sound(ESCPRESSEDSND, bstone::AC_ITEM);

    WaitKeyUp();
    MenuFadeIn();
}

///
void draw_video_descriptions(
    int16_t which)
{
    const char* instructions[] = {
        "STRETCHES RENDERED IMAGE TO THE WHOLE WINDOW",
    };

    ::fontnumber = 2;

    ::WindowX = 48;
    ::WindowY = 144;
    ::WindowW = 236;
    ::WindowH = 8;

    ::VWB_Bar(
        ::WindowX,
        ::WindowY - 1,
        ::WindowW,
        ::WindowH,
        ::menu_background_color);

    ::SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
    ::US_PrintCentered(instructions[which]);

    --::WindowX;
    --::WindowY;

    SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
    ::US_PrintCentered(instructions[which]);
}

void video_draw_menu()
{
    ::CA_CacheScreen(BACKGROUND_SCREENPIC);
    ::ClearMScreen();
    ::DrawMenuTitle("VIDEO SETTINGS");
    ::DrawInstructions(IT_STANDARD);
    ::DrawMenu(&video_items, video_menu);
    VW_UpdateScreen();
}

void video_draw_switch(
    int16_t which)
{
    uint16_t Shape;

    for (int i = 0; i < video_items.amount; i++) {
        if (video_menu[i].string[0]) {
            Shape = ::C_NOTSELECTEDPIC;

            if (video_items.cursor.on) {
                if (i == which) {
                    Shape += 2;
                }
            }

            switch (i) {
            case mvl_stretch_to_window:
                if (::vid_widescreen) {
                    Shape++;
                }
                break;

            default:
                break;
            }

            ::VWB_DrawPic(
                video_items.x - 16,
                video_items.y + i * video_items.y_spacing - 1,
                Shape);
        }
    }

    draw_video_descriptions(which);
}

void cp_video(
    int16_t)
{
    int16_t which;

    ::CA_CacheScreen(BACKGROUND_SCREENPIC);
    ::video_draw_menu();
    ::MenuFadeIn();
    ::WaitKeyUp();

    do {
        which = ::HandleMenu(&video_items, video_menu, video_draw_switch);

        switch (which) {
        case mvl_stretch_to_window:
            ::vid_widescreen = !::vid_widescreen;
            ::ShootSnd();
            ::video_draw_switch(video_items.curpos);
            ::vl_update_widescreen();
            ::SetupWalls();
            ::NewViewSize();
            ::SetPlaneViewSize();
            ::VL_RefreshScreen();
            break;

        default:
            break;
        }
    } while (which >= 0);

    ::MenuFadeOut();
}
///

void MenuFadeOut()
{
    if (::is_aog()) {
        ::VL_FadeOut(0, 255, 44, 0, 0, 10);
    } else {
        ::VL_FadeOut(0, 255, 40, 44, 44, 10);
    }
}
// BBi
