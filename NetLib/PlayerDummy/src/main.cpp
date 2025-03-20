#pragma comment(lib, "winmm")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCKAPI_
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <windows.h>
#include <assert.h>

#include "MessageBuilder.h"
#include "CircularQueue.h"

using namespace Common;

#define MSECOND_PER_FRAME (20)
#define BUFFER_SIZE (512)
struct Player
{
	SOCKET socket;

	int _PlayerId;

	int _Action;
	char _Direction;
	short _X;
	short _Y;

	bool _IsAlive;
	int _Hp;

	CircularQueue* _recvQueue;
	CircularQueue* _sendQueue;
};

const int X_SPEED = 3;
const int Y_SPEED = 2;

const int PLAYER_MAX_NUM = 15;
SOCKADDR_IN g_serverInfo;
USHORT g_port = 5000;
Player g_player[PLAYER_MAX_NUM] = { 0, };

void connectProc();
void recvProc();
void packetProc(int index);
void MessageProc(int index, char Type, char* message);
void InitNetwork();


void RES_create_my_chracter(int index, char* message);
void RES_create_other_chracter(int index, char* message);
void RES_create_move_start(int index, char* message);
void RES_create_move_stop(int index, char* message);

void MOVE_DUMMY(const int frame);
void SendMoveStart(const char goDirection);
void SendMoveStop_goTop(const int frame);
void SendMoveStop_goDown(const int frame);

int main()
{
	DWORD nextTick;
	int sleepTime;

	InitNetwork();
	connectProc();
	// Connect한 뒤, 바로 movestart를 보내주면, 플레이어 개체가 업데이트 되지 않고 넘어감.
	// Connect한 뒤, 플레이어에 대한 정보를 서버는 Send함. Send한 뒤, 클라측에서 Recv가 다음 혹은 다다음 프레임에 터짐. 
	Sleep(3000);
	const int TEST_FRAME = 100;
	//1번 recv 
	nextTick = timeGetTime();
	while (true)
	{
		recvProc();
		MOVE_DUMMY(TEST_FRAME);
	}
	::WSACleanup();

}


void InitNetwork()
{
	WSAData wsaData;

	int Flag = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Flag != 0)
	{
		printf("ERROR %d ", ::WSAGetLastError());
		return;
	}
	ZeroMemory(&g_serverInfo, sizeof(SOCKADDR_IN));
	g_serverInfo.sin_family = AF_INET;
	g_serverInfo.sin_port = htons(g_port);
	::InetPton(AF_INET, L"127.0.0.1", &g_serverInfo.sin_addr);

	LINGER option;
	int errorCode;
	option.l_onoff = 1;
	option.l_linger = 0;
	ULONG on = 1;
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		g_player[i].socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		int Flag = ::ioctlsocket(g_player[i].socket, FIONBIO, &on);
		if (Flag == SOCKET_ERROR)
		{
			errorCode = ::WSAGetLastError();
			continue;
		}

		Flag = ::setsockopt(g_player[i].socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&option), sizeof(option));
		if (Flag == SOCKET_ERROR)
		{
			errorCode = ::WSAGetLastError();
			printf("ioctl ERROR : %d\n", errorCode);
		}

		g_player[i]._recvQueue = new CircularQueue(1024);
		g_player[i]._sendQueue = new CircularQueue(1024);
	}


}
void connectProc()
{
	int Flag;
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
	CONNECT:
		Flag = ::connect(g_player[i].socket, reinterpret_cast<SOCKADDR*>(&g_serverInfo), sizeof(g_serverInfo));
		if (Flag == SOCKET_ERROR)
		{
			int errorCode;
			errorCode = ::WSAGetLastError();
			printf("CONNECT ERROR : %d \n", errorCode);
			if (errorCode == WSAEWOULDBLOCK)
			{
				goto CONNECT;
			}
			continue;
		}
	}

}

void recvProc()
{
	int recvLen;

	//SELECT 
	fd_set readSet;
	FD_ZERO(&readSet);
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		FD_SET(g_player[i].socket, &readSet);
	}

	TIMEVAL timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 0;
	int selectError;
	selectError = ::select(0, &readSet, nullptr, nullptr, &timeval);
	if (selectError == SOCKET_ERROR)
	{
		int errorCode = ::WSAGetLastError();
		printf("selectError ERROR : %d \n", errorCode);
		return;
	}
	
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		if (FD_ISSET(g_player[i].socket, &readSet))
		{
			int recvQLen = g_player[i]._recvQueue->GetDirect_EnqueueSize();
			//받아올거
			recvLen = ::recv(g_player[i].socket, g_player[i]._recvQueue->GetRearPtr(), recvQLen, 0);
			if (recvLen <= 0)
			{
				int errorCode;
				errorCode = ::WSAGetLastError();
				printf("recv ERROR : %d  | recvLen : %d \n", errorCode, recvLen);
				if (errorCode != WSAEWOULDBLOCK)
				{
					continue;
				}
				continue;
			}
			g_player[i]._recvQueue->MoveRear(recvLen);
			packetProc(i);
		}
		// 

	}

}

void packetProc(int index)
{
	int payloadPeekLen;
	int headerPeekLen;
	int dequeueLen;
	//받은 메시지 다 처리
	while (true)
	{
		char buffer[32] = { 0, };

		headerPeekLen = g_player[index]._recvQueue->Peek(buffer, sizeof(MESSAGE_HEADER));
		MESSAGE_HEADER* header = reinterpret_cast<MESSAGE_HEADER*>(buffer);
		//Header완성 됐는지 확인
		if (headerPeekLen < sizeof(MESSAGE_HEADER))
		{
			return;
		}
		
		_BYTE HLen = header->_PayloadLen;
		_BYTE Type = header->_MessageType;

		payloadPeekLen = g_player[index]._recvQueue->Peek(buffer, sizeof(MESSAGE_HEADER) + static_cast<size_t>(HLen));
		//PayLoad완성 됐는지 확인
		if (payloadPeekLen < sizeof(MESSAGE_HEADER) + static_cast<size_t>(HLen))
		{
			return;
		}

		dequeueLen = g_player[index]._recvQueue->Dequeue(buffer, sizeof(MESSAGE_HEADER) + static_cast<size_t>(HLen));
		assert(dequeueLen == (sizeof(MESSAGE_HEADER) + static_cast<size_t>(HLen)));
		MessageProc(index, Type, buffer);
	}
}


void MessageProc(int index, char Type, char* message)
{
	switch (Type)
	{
	case static_cast<int>(MESSAGE_DEFINE::RES_CREATE_MY_CHARACTER):
		RES_create_my_chracter(index, message);
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_DELETE_CHARACTER):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_MOVE_START):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_MOVE_STOP):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_ATTACK_RIGHT_HAND):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_ATTACK_KICK):
		break;
	case static_cast<int>(MESSAGE_DEFINE::RES_DAMAGE):
		break;

	}
}

void MOVE_DUMMY(const int frame)
{
	//MOVE START를 보낸다.
	//MOVE STOP을 보내고, 내 더미들의 위치를 업데이트 한다. 
	//중간상태는 없어도됨. 

	SendMoveStart(static_cast<char>(MOVE_DIRECTION::TOP));
	//정해진 프레임 만큼 잠들기. ex)10프레임이다. 
	Sleep(MSECOND_PER_FRAME * frame);
	SendMoveStop_goTop(frame);

	SendMoveStart(static_cast<char>(MOVE_DIRECTION::BOTTOM));
	Sleep(MSECOND_PER_FRAME * frame);
	SendMoveStop_goDown(frame);

}

void SendMoveStart(const char goDirection)
{
	MESSAGE_REQ_MOVE_START sendMsg;

	sendMsg.header._Code = 0x89;
	sendMsg.header._MessageType = static_cast<char>(MESSAGE_DEFINE::REQ_MOVE_START);
	sendMsg.header._PayloadLen = sizeof(MESSAGE_REQ_MOVE_START) - sizeof(MESSAGE_HEADER);
	printf("=======================================================================\n");
	printf("MOVE START !!! \n");
	//위쪽으로 움직이기
	sendMsg._Direction = goDirection;
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		sendMsg._X = g_player[i]._X;
		sendMsg._Y = g_player[i]._Y;

		printf("PLAYER ID : %d | PLAYER X : %hd | PLAYER_Y : %hd \n", i, g_player[i]._X, g_player[i]._Y);

		::send(g_player[i].socket, reinterpret_cast<char*>(&sendMsg), sizeof(MESSAGE_REQ_MOVE_START), 0);
	}
	printf("=======================================================================\n");
}

void SendMoveStop_goDown(const int frame)
{
	MESSAGE_REQ_MOVE_STOP sendMsg;

	sendMsg.header._Code = 0x89;
	sendMsg.header._MessageType = static_cast<char>(MESSAGE_DEFINE::REQ_MOVE_STOP);
	sendMsg.header._PayloadLen = sizeof(MESSAGE_REQ_MOVE_STOP) - sizeof(MESSAGE_HEADER);
	printf("=======================================================================\n");
	printf("MOVE STOP !!! \n");
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		g_player[i]._Y += 2 * frame;
		if (g_player[i]._Y > static_cast<short>(MOVE_BOUNDARY::BOTTOM))
		{
			g_player[i]._Y = static_cast<short>(MOVE_BOUNDARY::BOTTOM);
		}

		sendMsg._X = g_player[i]._X;
		sendMsg._Y = g_player[i]._Y;
		sendMsg._Direction = g_player[i]._Direction;


		printf("PLAYER ID : %d | PLAYER X : %hd | PLAYER_Y : %hd \n", i, g_player[i]._X, g_player[i]._Y);

		::send(g_player[i].socket, reinterpret_cast<char*>(&sendMsg), sizeof(MESSAGE_REQ_MOVE_STOP), 0);
	}
	printf("=======================================================================\n");
}

void SendMoveStop_goTop(const int frame)
{
	MESSAGE_REQ_MOVE_STOP sendMsg;

	sendMsg.header._Code = 0x89;
	sendMsg.header._MessageType = static_cast<char>(MESSAGE_DEFINE::REQ_MOVE_STOP);
	sendMsg.header._PayloadLen = sizeof(MESSAGE_REQ_MOVE_STOP) - sizeof(MESSAGE_HEADER);

	printf("=======================================================================\n");
	printf("MOVE STOP !!! \n");
	for (int i = 0; i < PLAYER_MAX_NUM; i++)
	{
		g_player[i]._Y -= 2 * frame;
		if (g_player[i]._Y < static_cast<short>(MOVE_BOUNDARY::TOP))
		{
			g_player[i]._Y = static_cast<short>(MOVE_BOUNDARY::TOP);
		}
		printf("PLAYER ID : %d | PLAYER X : %hd | PLAYER_Y : %hd \n", i, g_player[i]._X, g_player[i]._Y);
		sendMsg._X = g_player[i]._X;
		sendMsg._Y = g_player[i]._Y;
		sendMsg._Direction = g_player[i]._Direction;

		::send(g_player[i].socket, reinterpret_cast<char*>(&sendMsg), sizeof(MESSAGE_REQ_MOVE_STOP), 0);
	}
	printf("=======================================================================\n");
}

void RES_create_my_chracter(int index, char* message)
{
	res_createMyCharacter_t* recvMsg = reinterpret_cast<res_createMyCharacter_t*>(message);

	g_player[index]._PlayerId = recvMsg->_Id;
	g_player[index]._Hp = recvMsg->_HP;
	g_player[index]._X = recvMsg->_X;
	g_player[index]._Y = recvMsg->_Y;
	g_player[index]._Direction = recvMsg->_Direction;

	g_player[index]._Action = -1;
	g_player[index]._IsAlive = true;

	printf("=========================================================\n");
	printf("INDEX : %d | character X : %hd  | character Y : %hd  | \n", index, g_player[index]._X, g_player[index]._Y);
	printf("========================================================\n");
}

void RES_create_other_chracter(int index, char* message)
{

}

void RES_create_move_start(int index, char* message)
{

}

void RES_create_move_stop(int index, char* message)
{

}
