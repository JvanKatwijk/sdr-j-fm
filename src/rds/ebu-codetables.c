#include <cstdint>

// PTY id to text mapping (is maybe not the best place in this file...)
const char * const pty_table[][2] =
{
  {"--",                    "--"},
  {"News",                  "News"},
  {"Current Affairs",       "Information"},
  {"Information",           "Sports"},
  {"Sport",                 "Talk"},
  {"Education",             "Rock"},
  {"Drama",                 "Classic Rock"},
  {"Culture",               "Adult Hits"},
  {"Science",               "Soft Rock"},
  {"Varied",                "Top 40"},
  {"Pop Music",             "Country"},
  {"Rock Music",            "Oldies"},
  {"Easy Listening",        "Soft"},
  {"Light Classical",       "Nostalgia"},
  {"Serious Classical",     "Jazz"},
  {"Other Music",           "Classical"},
  {"Weather",               "Rhythm & Blues"},
  {"Finance",               "Soft Rhythm & Blues"},
  {"Children’s Progs",      "Language"},
  {"Social Affairs",        "Religious Music"},
  {"Religion",              "Religious Talk"},
  {"Phone-In",              "Personality"},
  {"Travel",                "Public"},
  {"Leisure",               "College"},
  {"Jazz Music",            "Spanish Talk"},
  {"Country Music",         "Spanish Music"},
  {"National Music",        "Hip Hop"},
  {"Oldies Music",          "Unassigned"},
  {"Folk Music",            "Unassigned"},
  {"Documentary",           "Weather"},
  {"Alarm Test",            "Emergency Test"},
  {"Alarm",                 "Emergency"}
};

constexpr uint16_t pty_locale = 0; // set to 0 for Europe (1 for USA) which will use first column instead


//	mapping from EBU code tables to 16 bit codes for QChar

const uint16_t EBU_E1[16][14] =
{
  /* 0*/ { ' ', '0', '@', 'P', ' ', 'p', 0xE1, 0xE2, 'X', 'X', 0xC1, 0xC2, 0xC3, 0xE3 },
  /* 1*/ { '!', '1', 'A', 'Q', 'a', 'q', 0xE0, 0xE4, 0x3B1, 0xB9, 0xC0, 0xC4, 0xC5, 0xE5 },
  /* 2*/ { '\"', '2', 'B', 'R', 'b', 'r', 0xE9, 0xEA, 0xA9, 0xB2, 0xC9, 0xCA, 0xC6, 0xE6 },
  /* 3*/ { '#', '3', 'C', 'S', 'c', 's', 0xE8, 0xEB, 'X', 0xB3, 0XC8, 0XCB, 0x152, 0x153 },
  /* 4*/ { 'X', '4', 'D', 'T', 'd', 't', 0xED, 0xEE, 'X', 'X', 0xCD, 0xCE, 0x176, 0x175 },
  /* 5*/ { '\%', '5', 'E', 'U', 'e', 'u', 0xEC, 0xEF, 'X', 'X', 0xCC, 0xCF, 0xDD, 0xFD },
  /* 6*/ { '&', '6', 'F', 'V', 'f', 'v', 0xF3, 0xF4, 'X', 'X', 0xD3, 0xD4, 0xD5, 0xF5 },
  /* 7*/ { '\'', '7', 'G', 'W', 'g', 'w', 0xF5, 0xF6, 'X', 'X', 0xD2, 0xD6, 0xD8, 0xF8 },
  /* 8*/ { '(', '8', 'H', 'X', 'h', 'x', 0xFA, 0xFB, 'X', 'X', 0xDA, 0xDB, 'X', 'X' },
  /* 9*/ { ')', '9', 'I', 'Y', 'i', 'y', 0xF9, 0xFC, 'X', 'X', 0xD9, 0xDC, 'X', 'X' },
  /* A*/ { '*', ':', 'J', 'Z', 'j', 'z', 0xD1, 0xF1, 0xA3, 'X', 0x158, 0x159, 0x154, 0x155 },
  /* B*/ { '+', ';', 'K', '[', 'k', '{', 0xC7, 0xE7, 0x24, 'X', 0x10C, 0x10D, 0x106, 0x107 },
  /* C*/ { ',', '<', 'L', '\\', 'l', 'X', 0x15E, 0x15F, 0x2190, 0xBC, 0x160, 0x161, 0x15A, 0x15B },
  /* D*/ { '-', '=', 'M', ']', 'm', '}', 0x3B2, 0x11D, 0x2191, 0xBD, 0x17D, 0x17E, 0x179, 0x17A },
  /* E*/ { '.', '>', 'N', 'X', 'n', 'X', 'X', 'X', 0x2192, 0xBE, 'X', 'X', 'X', 'X' },
  /* F*/ { '/', '?', 'O', 'X', 'o', ' ', 0x132, 0x133, 0x2193, 'X', 'X', 'X', 'X', 'X' }
};

uint16_t mapEBUtoUnicode(uint8_t alfabet, uint8_t character)
{
  uint8_t columnnibble = (character & 0xF0) >> 4;
  uint8_t rownibble = character & 0x0F;

  (void)alfabet;
  return EBU_E1[rownibble][columnnibble - 2];
}
