#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include "Socket.h"
#include "common.h"

using namespace std;

#define BUFLEN_MAX 		10000
#define NAMELEN_MAX 	50
#define CMD_BUFLEN		30


char strErr[] = "ERROR CMD!!!";

class CP2PServer
{
public:

	CP2PServer(char *strPort)
		:m_objSocket(SOCK_DGRAM, AF_INET, INADDR_ANY, strPort, 5)
	{}
	
	virtual ~CP2PServer(){};

	// cooperate with client ReadAndSend
	int AcceptAndShow()   
	{
		char buf[BUFLEN_MAX] = {0};
		int status;
		m_objSocket.Bind();

		LOG("server receving data ......\n");
		
		while(1)
		{
			struct sockaddr_in addr;
			status = m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
			if (SOCK_SUCCESS != status)
			{
				if(SOCK_AGAIN != status)
					LOG("RecvFrom Error!\n");
				usleep(300);
				continue;
			}

			LOG("RecvFrom Success!\n");
			LOG("ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			LOG("data: %s\n\n", buf);
			usleep(300);
		}
	}

	int Start()
	{
		char buf[BUFLEN_MAX] = {0};
		int status;
		m_objSocket.Bind();

		LOG("server receving data ......\n");
		
		while(1)
		{
			char data[BUFLEN_MAX] = {0};
			char cmdData[BUFLEN_MAX] = {0};
			
			memset(data, 0, BUFLEN_MAX);
			memset(cmdData, 0, BUFLEN_MAX);
			memset(buf, 0, BUFLEN_MAX);

			struct sockaddr_in addr;
			status = m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
			if (SOCK_SUCCESS != status)
			{
				if(SOCK_AGAIN != status)
					LOG("RecvFrom Error!\n");

				usleep(300);
				continue;
			}

			int cmd = ParseCmd(buf, cmdData);
			if (cmd < 0)
			{
				LOG("ParseCmd failed!  buf = %s\n", buf);
				m_objSocket.SendTo(strErr, strlen(strErr), (struct sockaddr *)&addr);
				
				usleep(300);
				continue;
			}

			string strData;
			vector<CUser>::iterator it;

			switch(cmd)
			{
			case CMD_JOIN:
				for (it = m_vecUserList.begin(); it != m_vecUserList.end(); it++)
				{
					if (strcmp(cmdData, it->strUserName.c_str()) == 0)
						break;
				}

				if (it == m_vecUserList.end())
					m_vecUserList.push_back(CUser(cmdData, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)));
				else
				{
					strData += string("Have same user!");
					m_objSocket.SendTo(strData.c_str(), strData.size(), (struct sockaddr *)&addr);
					continue;
				}
					
				break;
			
			case CMD_LIST_USERS:
				for (it = m_vecUserList.begin(); it != m_vecUserList.end(); it++)
				{
					sprintf(data, "name: %s, ip: %s, port: %d\n", it->strUserName.c_str(), it->strUserIP.c_str(), it->nUserPort);
					strData += string(data);
				}
				break;
			case CMD_EXIT_ROOM:
				for (it = m_vecUserList.begin(); it != m_vecUserList.end(); it++)
				{
					if (strcmp(cmdData, it->strUserName.c_str()) == 0)
					{
						it = m_vecUserList.erase(it);
						if (it == m_vecUserList.end())
							break;
					}
				}
				break;

			case CMD_BEGIN_CHAT:
				{
					string::size_type pos;
					string strCmdData(cmdData);
					if ((pos = strCmdData.find("+", 0)) == string::npos)
						break;

					string strFriendName(strCmdData, pos+1);
					LOG("strFriendName: %s\n", strFriendName.c_str());

					char chatAck[CMD_BUFLEN] = {0};
					
					for (it = m_vecUserList.begin(); it != m_vecUserList.end(); it++)
					{
						if (strcmp(it->strUserName.c_str(), strFriendName.c_str()) == 0)
						{
							string strChat = string(arrCmd[CMD_BEGIN_CHAT]) + string(strCmdData, 0, pos);
							LOG("strChat: %s\n", strChat.c_str());
							m_objSocket.SendTo(strChat.c_str(), strChat.size(), it->strUserIP.c_str(), it->nUserPort);
							//m_objSocket.SendTo(strChat.c_str(), strChat.size(), it->strUserIP.c_str(), it->nUserPort);
							//m_objSocket.SendTo(strChat.c_str(), strChat.size(), it->strUserIP.c_str(), it->nUserPort);
							/*int status = 0, nCount = 0;
							do
							{
								status = m_objSocket.RecvFrom(chatAck, CMD_BUFLEN, &);
								nCount++;
							} while (status != 0 && nCount < 3);*/

							break;
						}
					}					
				}
				break;

			case CMD_QUERY_SELF_ADDR:
				strData = string(inet_ntoa(addr.sin_addr)) + string("+") + to_string(ntohs(addr.sin_port));
				break;
				
			default:
				break;
			}

			string strACK = string(arrCmd[cmd+1]) + strData;

			LOG("#### ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			LOG("#### data: %s\n\n", buf);
			LOG("cmd num = %d\n", cmd);
			LOG("data: %s\n", cmdData);

			m_objSocket.SendTo(strACK.c_str(), strACK.size(), (struct sockaddr *)&addr);
		
			usleep(300);
		}
	}

private:

	

private:
	CSocket m_objSocket;
	vector<CUser> m_vecUserList;
};


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		LOG("Usage: ./P2PServer PORT\n");
		return 0;
	}

	CP2PServer server(argv[1]);
	server.Start();
	//server.AcceptAndShow();

	return 0;
}

