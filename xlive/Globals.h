#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "stdafx.h"
#include <unordered_map>
#include <stdlib.h>
#include <mutex>
#include "h2mod.h"
#include <windows.h>
#include "xliveless.h"
#include "Hook.h"
#include "ReadIniArguments.h"

#include "TSClient.h"
#include "TSServer.h"
#include "H2MOD_Players.h"
#include "H2MOD_GameType.h"
#include "H2MOD_Halo2Final.h"
#include "H2MOD_Infection.h"
#include "H2MOD_GunGame.h"
#include "H2ConsoleCommands.h"
#include "H2MOD_MapManager.h"
#include "H2MOD_Network.h"
#include "H2MOD_GameManager.h"
#include "H2MOD_AdvLobbySettings.h"

extern XNADDR join_game_xn;
extern MapManager* mapManager;
extern AdvLobbySettings* advLobbySettings;
extern volatile bool isLobby;
extern bool overrideUnicodeMessage;
extern ConsoleCommands* commands; 
extern GameManager* gameManager;
extern Players* players;
extern CustomNetwork *network;
extern char* replacedNetworkNormalTextWidget;
extern char* replacedNetworkNormalTextWidget2;

extern std::unordered_map<XUID, BOOL> xuidIsTalkingMap;
extern bool microphoneEnabled;
extern IN_ADDR clientMachineAddress;
extern TSClient* client;
extern TSServer* server;

//some utility functions below
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
int stripWhitespace(wchar_t *inputStr);

void patchBYTEs(BYTE* orig, BYTE* values, int size);
