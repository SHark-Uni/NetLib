#include "NetLib.h"

NetLib::NetWorkLib::~NetWorkLib()
{
}

bool NetLib::NetWorkLib::InitForTCP()
{
    return false;
}

void NetLib::NetWorkLib::Process()
{
}

void NetLib::NetWorkLib::SendUniCast(int sessionKey, char* message)
{
}

void NetLib::NetWorkLib::SendBroadCast(char* message)
{
}

void NetLib::NetWorkLib::SendBroadCast(int exceptSession, char* message)
{
}

void NetLib::NetWorkLib::RecvProc()
{
}

void NetLib::NetWorkLib::AcceptProc()
{
}

void NetLib::NetWorkLib::SendProc()
{
}
