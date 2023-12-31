#pragma once
class LoginSession;

class PacketHandler
{
public:
	static void HandlePacket(LoginSession* session, BYTE* packet, int32 packetSize);
private:
	static void HandlePacket_C2S_LOGIN(LoginSession* session, BYTE* packet, int32 packetSize);
	static void HandlePacket_C2S_CREATECHARACTER(LoginSession* session, BYTE* packet, int32 packetSize);
	static void HandlePacket_C2S_CHARACTERLIST(LoginSession* session, BYTE* packet, int32 packetSize);
	static void HandlePacket_C2S_DELETECHARACTER(LoginSession* session, BYTE* packet, int32 packetSize);
	static void HandlePacket_C2S_GAMEPLAY(LoginSession* session, BYTE* packet, int32 packetSize);
};

