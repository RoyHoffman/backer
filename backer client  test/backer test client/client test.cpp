#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <sdkddkver.h>
#include <conio.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <Mswsock.h>

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>

#include <experimental/filesystem>




// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512

using namespace std;
using namespace std::experimental::filesystem;


void socketError(string);
int dirtyConnect();

void send(int, const string&);

string nameProtocol(int);
int sizeProtocol(int);

string recvUntil(int, char);
string recvUntil(int, int);
void intostr(string&, char *, int);

void sendfile(int, string);
void recivefile(int,string);

void reciveDirectory(int, string);
void sendDirectory(int, string);

string fileNameFromPath(const string&);





void hold();
void hold(string);


int main(int argc, char **argv)
{

	int socket;
	socket = dirtyConnect();

	char recvBuf[DEFAULT_BUFLEN];
	int recvBufLen = DEFAULT_BUFLEN;

	memset(recvBuf, 0, sizeof(recvBuf));
	if (recv(socket, recvBuf, recvBufLen, NULL) == SOCKET_ERROR)
		socketError("recv()");
	cout << recvBuf << "\n-------" << endl;
	//*
	sendfile(socket, "C:\\Users\\Roey Hofman\\Pictures\\Camera Roll\\wow.PNG");
	sendfile(socket, "C:\\Users\\Roey Hofman\\Documents\\strategy homework conclusions.txt");
	//*/

    /*
	recivefile(socket, "C:\\Users\\Roey Hofman\\Desktop\\here");
	recivefile(socket, "C:\\Users\\Roey Hofman\\Desktop\\here");
	recivefile(socket, "C:\\Users\\Roey Hofman\\Desktop\\here");
	//*/

	//reciveDirectory(socket, "C:\\Users\\Roey Hofman\\Desktop\\here");
	
	hold();
	
}

void sendfile(int socket, string path)
{

	string name = fileNameFromPath(path);

	HANDLE hFile;
	hFile = CreateFile(path.c_str(),               // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
		NULL);                 // no attr. template


	LARGE_INTEGER lisize;
	GetFileSizeEx(hFile, &lisize);
	LONGLONG filesize = lisize.QuadPart;

	string format = to_string(name.length()) + 'a' + name + to_string(filesize)+ 'a';

	cout << format << endl;

	send(socket, format);

	//_TRANSMIT_FILE_BUFFERS head(); at a later time you can be cool and use this shit to transfer the protocol

	if (TransmitFile(socket, hFile, 0, 0, NULL, NULL, NULL) == FALSE)
		socketError("TransmitFile()");

	cout << "sent file successfully";
	CloseHandle(hFile);

}



int dirtyConnect()
{
	cout << "here" << endl;

	WSADATA wsaData;

	int udpSocket, listener, tcpSocket;

	char *sendbuf = "this is a test";

	string buf;

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	int iResult;

	char *ip = "255.255.255.255";
	int udpPort = 2001;
	int	tcpPort = 2002;

	SOCKADDR_IN brodcastAddr, myAddr;

	cout << "here1" << endl;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char broadcastEnable = 1;
	int ret = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)); //enable broadcast

	memset((char *)&brodcastAddr, 0, sizeof(brodcastAddr));
	brodcastAddr.sin_port = htons(udpPort);
	inet_pton(AF_INET, ip, &brodcastAddr.sin_addr);
	brodcastAddr.sin_family = AF_INET;

	cout << "here" << endl;

	int i;

	if (sendto(udpSocket, sendbuf, strlen(sendbuf), 0, (struct sockaddr *) &brodcastAddr, sizeof(brodcastAddr)) == SOCKET_ERROR)
		socketError("sendto()");

	cout << "sent" << endl;


	tcpSocket = socket(AF_INET, SOCK_STREAM, NULL);
	listener = socket(AF_INET, SOCK_STREAM, NULL);

	memset((char *)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_addr.s_addr = INADDR_ANY;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(tcpPort);
	//inet_pton(AF_INET, "10.0.0.14", &myAddr.sin_addr);



	if (bind(listener, (struct sockaddr *) &myAddr, sizeof(myAddr)) == SOCKET_ERROR)
		socketError("bind()");

	cout << "listening" << endl;

	listen(listener, 5);

	tcpSocket = accept(listener, NULL, NULL);
	if (tcpSocket == INVALID_SOCKET)
		socketError("accept()");

	cout << "connected---" << endl;


	memset(recvbuf, 0, sizeof(recvbuf));
	if (recv(tcpSocket, recvbuf, recvbuflen, NULL) == SOCKET_ERROR)
		socketError("recv()");
	cout << recvbuf << "\n-------" << endl;


	buf = "testing";
	if (send(tcpSocket, buf.c_str(), strlen(buf.c_str()), NULL) == SOCKET_ERROR)
		socketError("send()");


	closesocket(udpSocket);

	//cin >> i;
	//WSACleanup();

	return tcpSocket;
}

void hold()
{
	string i;
	cin >> i;
}

void hold(string message)
{
	cout << message << endl;
	string i;
	cin >> i;
}


void recivefile(int socket,string path)
{

	string name = nameProtocol(socket);
	int size = sizeProtocol(socket);
	path += "\\" + name;

	//u_long size;
	//ioctlsocket(socket, FIONREAD, &size);
	cout << size << endl;

	//hold("2");

	string fileData =  recvUntil(socket, size);

	//hold("file got");
	//cout << filebuffer << "\n-------" << endl;

	HANDLE hFile;

	hFile = CreateFile(path.c_str(),                // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		CREATE_NEW,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Terminal failure: Unable to open file  for write" << endl;;
		cout << name << " failed with error code : " << GetLastError() << endl;
		hold();
		exit(EXIT_FAILURE);
	}

	DWORD dwBytesWritten = 0;
	WriteFile(
		hFile,           // open file handle
		fileData.c_str(),      // start of data to write
		size,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);            // no overlapped structure

	cout << "recieve file finished" << endl;

	//hold("still alive");
	CloseHandle(hFile);
}

void sendDirectory(int socket, string path)
{
	string name = fileNameFromPath(path);
	string format = to_string(name.length()) + 'a' + name;
	cout << format << endl;
	send(socket, format);

	for (auto & de : directory_iterator(path))
	{
		//string b = de.path().string();
		//cout << name << " name: " << fileNameFromPath(b) << "\nis directory: " << boolalpha << is_directory(de) << endl;
		if (is_directory(de))
		{
			send(socket, "d"); //directory
			cout << 'd' << endl;
			sendDirectory(socket, de.path().string());
		}
		else
		{
			send(socket, "r"); //regular file
			cout << 'r' << endl;
			sendfile(socket, de.path().string());
		}
	}
	send(socket, "f"); //finsih
}

void reciveDirectory(int socket, string path)
{
	path += "\\" + nameProtocol(socket);
	CreateDirectory(path.c_str(), NULL);
	string nextRecv;
	while (true)
	{
		nextRecv = recvUntil(socket, 1);
		if (nextRecv == "d") //directory
			reciveDirectory(socket, path);
		if (nextRecv == "r") //regular file
			recivefile(socket,path);
		if (nextRecv == "f")
			return;
	}
}

void socketError(string name)
{
	//printf("sendto() failed with error code : %d", WSAGetLastError());
	cout << name << " failed with error code : " << WSAGetLastError() << endl;
	hold();
	exit(EXIT_FAILURE);
}

int sizeProtocol(int socket)
{
	string slen = recvUntil(socket, 'a');
	return stoi(slen);
}

string nameProtocol(int socket)
{
	int len = sizeProtocol(socket);
	return recvUntil(socket, len);
}


string recvUntil(int socket, char stop)
{
	char  temp[1];
	string buffer = "";
	while (true)
	{
		if (recv(socket, temp, 1, NULL) == SOCKET_ERROR)
			socketError("recv()");
		if (*temp == stop)
			return buffer;
		buffer += *temp;
	}
	
}

string recvUntil(int socket, int lenght)
{
	char *temp = new char[lenght];
	string buffer = "";
	long bytesRecv;
	while (true)
	{
		bytesRecv = recv(socket, temp, lenght, NULL);
		if (bytesRecv == SOCKET_ERROR)
			socketError("recv()");
		//temp[bytesRecv] = '\0';
		//buffer += temp;
		intostr(buffer, temp, bytesRecv);
		lenght -= bytesRecv;
		if (lenght == 0)
		{
			delete [] temp;
			return buffer;
		}
	}
}

void send(int socket, const string& m)
{
	if (send(socket, m.c_str(), strlen(m.c_str()), 0) == -1)
		socketError("send()");
	cout << "sent" << endl;
}


void intostr(string & out, char * in, int len)
{
	for (int i = 0; i < len; i++)
		out += in[i];
}

string fileNameFromPath(const string& path)
{
	int counter = 0;
	int len = 0;
	for (char c : path)
	{
		counter++;
		if (c == '\\')
			len = counter;
	}
	return path.substr(len);
}