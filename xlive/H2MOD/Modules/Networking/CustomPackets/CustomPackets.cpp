#include "CustomPackets.h"

#include "H2MOD.h"
#include "Util\Hooks\Hook.h"
#include "..\Memory\bitstream.h"
#include "..\..\MapManager\MapManager.h"
#include "H2MOD\Modules\OnScreenDebug\OnscreenDebug.h"

char g_network_message_types[e_network_message_types::end * 32]; // out of band packets

void register_packet_impl(void *packetObject, int type, char* name, int a4, int size1, int size2, void* write_packet_method, void* read_packet_method, void* a9)
{
	typedef void(__thiscall *register_packet_type)(void *packetObject, int type, char* name, int a4, int size1, int size2, void* write_packet_method, void* read_packet_method, void* a9);
	auto register_packet = reinterpret_cast<register_packet_type>(h2mod->GetAddress(0x1E81D6, 0x1CA199));
	return register_packet(packetObject, type, name, a4, size1, size2, write_packet_method, read_packet_method, a9);
}

void send_packet(void *thisx, int a2, int a3, char a4, unsigned int type, unsigned int size, void* a7)
{
	typedef void(__thiscall *dynamic_packet_check_method)(void *thisx, int a2, int a3, char a4, unsigned int type, unsigned int size, void* a7);
	auto dynamic_packet_check = reinterpret_cast<dynamic_packet_check_method>(h2mod->GetAddress(0x1BED40, 0x1B8C1A));
	dynamic_packet_check(thisx, a2, a3, a4, type, size, a7);
}

void __cdecl encode_map_file_name_packet(char* buffer, int a2, s_custom_map_filename* data)
{
	bitstream::p_data_encode_bool()(buffer, "is-custom-map", data->is_custom_map);
	bitstream::p_data_encode_string()(buffer, "map-file-name", (int)&data->file_name, 32);
}
bool __cdecl decode_map_file_name_packet(char* buffer, int a2, s_custom_map_filename* data)
{
	data->is_custom_map = bitstream::p_data_decode_bool()(buffer, "is-custom-map");
	bitstream::p_data_decode_string()(buffer, "map-file-name", (int)&data->file_name, 32);
	return bitstream::p_packet_is_valid()(buffer) == 0;
}

void __cdecl encode_request_map_filename_packet(char* buffer, int a2, s_request_map_filename* data)
{
	//bitstream::p_data_encode_string()(buffer, "user-name", (int)data->username, 16);
	bitstream::p_data_encode_bits()(buffer, "user-identifier", &data->user_identifier, 64);
}
bool __cdecl decode_request_map_filename_packet(char* buffer, int a2, s_request_map_filename* data)
{
	//bitstream::p_data_decode_string()(buffer, "user-name", (int)data->username, 16);
	bitstream::p_data_decode_bits()(buffer, "user-identifier", (int)&data->user_identifier, 64);
	return bitstream::p_packet_is_valid()(buffer) == 0;
}

void register_custom_packets(void* a1)
{
	typedef void(__cdecl* register_test_packet)(void* a1);
	auto p_register_test_packet = reinterpret_cast<register_test_packet>(h2mod->GetAddress(0x1ECE05, 0x1CD7BE));
	p_register_test_packet(a1);

	register_packet_impl(g_network_message_types, e_network_message_types::map_file_name, "map-file-name", 0, sizeof(s_custom_map_filename), sizeof(s_custom_map_filename), 
		(void*)encode_map_file_name_packet, (void*)decode_map_file_name_packet, NULL);

	register_packet_impl(g_network_message_types, e_network_message_types::request_map_filename, "request-map-filename", 0, sizeof(s_request_map_filename), sizeof(s_request_map_filename),
		(void*)encode_request_map_filename_packet, (void*)decode_request_map_filename_packet, NULL);
}

typedef void(__stdcall *network_message_gateway)(void *thisx, network_address* addr, int a3, int a4, void* packet);
network_message_gateway p_network_message_gateway;

void __stdcall message_gateway_hook(void *thisx, network_address* addr, int message_type, int a4, void* packet)
{
	switch (message_type)
	{
	case e_network_message_types::request_map_filename:
	{
		s_request_map_filename* pak = (s_request_map_filename*)packet;
		TRACE_GAME_NETWORK_N("[H2MOD-CustomPackets] received on hook1 request-map-filename from XUID: %lld", pak->user_identifier);
		network_session* current_session = NetworkSession::getCurrentNetworkSession();
		signed int peer_index = NetworkSession::getPeerIndexFromNetworkAddress(addr);
		if (peer_index != -1 && peer_index != current_session->local_peer_index)
		{
			s_custom_map_filename buffer;
			memset(&buffer, NULL, sizeof(s_custom_map_filename));
			std::wstring map_filename;
			mapManager->getMapFilename(map_filename);
			buffer.is_custom_map = false;
			if (map_filename.size())
			{
				buffer.is_custom_map = true;
				wcsncpy_s(buffer.file_name, map_filename.c_str(), 32);
			}

			send_packet(current_session->field_8, *(DWORD*)current_session->field_14, current_session->unk_needs_reversing[peer_index].index_unk, 1,
				e_network_message_types::map_file_name, sizeof(s_custom_map_filename), &buffer);
		}

		return;
	}

	case e_network_message_types::map_file_name:
	{
		s_custom_map_filename* pak = (s_custom_map_filename*)packet;
		if (pak->is_custom_map)
		{
			std::wstring filename_wstr(pak->file_name);
			std::string filename_str(filename_wstr.begin(), filename_wstr.end());
			mapManager->setMapFileNameToDownload(filename_str);
		}

		return;
	}

	default:
			break;
	}

	p_network_message_gateway(thisx, addr, message_type, a4, packet);
}

typedef void(__stdcall *network_message_gateway_2)(void *thisx, int a2, int message_type, int a4, void* packet);
network_message_gateway_2 p_network_message_gateway_2;

void __stdcall message_gateway_hook_2(void *thisx, int a2, int message_type, int a4, void* packet)
{
	switch (message_type)
	{
	case e_network_message_types::request_map_filename:
	{
		s_request_map_filename* pak = (s_request_map_filename*)packet;
		TRACE_GAME_NETWORK_N("[H2MOD-CustomPackets] received on hook2 request-map-filename from XUID: 0x%016X", pak->user_identifier);
		return;
	}
	case e_network_message_types::map_file_name:
		return;

	default:
		break;
	}

	p_network_message_gateway_2(thisx, a2, message_type, a4, packet);
}

extern XUID xFakeXuid[4];
void CustomPackets::send_request_map_filename(network_session* session)
{
	if (session->session_state == _network_session_state_peer_established)
	{
		//extern char g_szUserName[4][16];
		s_request_map_filename buffer;
		memset(&buffer, NULL, sizeof(s_request_map_filename));
		memcpy(&buffer.user_identifier, &xFakeXuid[0], sizeof(XUID));

		if (session->unk_needs_reversing[session->host_index].field_0) {
			send_packet(session->field_8, *(DWORD*)session->field_14, session->unk_needs_reversing[session->host_index].index_unk, 1,
				e_network_message_types::request_map_filename, sizeof(s_request_map_filename), (void*)&buffer);
		}
	}
}

void CustomPackets::ApplyGamePatches()
{
	WritePointer((DWORD)h2mod->GetAddress(0x1AC733, 0x1AC901), g_network_message_types);
	WritePointer((DWORD)h2mod->GetAddress(0x1AC8F8, 0x1ACAC6), g_network_message_types);
	WriteValue<BYTE>((DWORD)h2mod->GetAddress(0x1E825E,0x1CA221), e_network_message_types::end);
	WriteValue<int>((DWORD)h2mod->GetAddress(0x1E81C6, 0x1CA189), e_network_message_types::end * 32);

	PatchCall(h2mod->GetAddress(0x1B5196, 0x1A8EF4), register_custom_packets);

	DWORD dwBack;
	p_network_message_gateway = (network_message_gateway)DetourClassFunc((BYTE*)h2mod->GetAddress(0x1E907B, 0x1CB03B), (BYTE*)message_gateway_hook, 8);
	VirtualProtect(p_network_message_gateway, 4, PAGE_EXECUTE_READWRITE, &dwBack);

	p_network_message_gateway_2 = (network_message_gateway_2)DetourClassFunc((BYTE*)h2mod->GetAddress(0x1E929C, 0x1CB25C), (BYTE*)message_gateway_hook_2, 8);
	VirtualProtect(p_network_message_gateway_2, 4, PAGE_EXECUTE_READWRITE, &dwBack);
}