
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/sendfile.h>

#include <experimental/filesystem>


using namespace std;
using namespace std::experimental::filesystem;


int dirtyConnect();

void send(int,const string&);

void socketError(string);
void sendfile(int,string);

void recvfile(int);

string nameProtocol(int);
int sizeProtocol(int);

string recvUntil(int, char);
string recvUntil(int, int);

void intostr(string&, char[], int);

string fileNameFromPath(const string&);
string fileNameFromPath(const path&);

void sendDirectory(int,string);

int main()
{
    int socket;
   
    
    socket = dirtyConnect();
    
    if(send(socket, "outTest",7,0)==-1)
        socketError("send()");
    cout << "sent" << endl;
    //*
    recvfile(socket);
    recvfile(socket);
    //*/
    /*
    sendfile(socket,"/home/roy/log/connection log");
    sendfile(socket,"/home/roy/Pictures/testdownload.jpg");
    sendfile(socket,"/home/roy/Pictures/PNG_transparency_demonstration_1.png");
    //*/
    //sendDirectory(socket,"/home/roy/Desktop/test send");
    
    
    
    return 0;
}

void recvfile(int socket)
{
    string name = nameProtocol(socket);
    int size = sizeProtocol(socket);
    
    cout << size << "\n" <<name <<endl;
    
    string fileData = recvUntil(socket,size);
    
    int file = creat(name.c_str(),S_IRWXU);
    
    write(file,fileData.c_str(),size);
    
    close(file);
    
}

void sendDirectory(int socket,string path)
{
    string name = fileNameFromPath(path);
    string format = to_string(name.length())+'a'+name;
    
    cout << format << endl;
    
    send(socket,format);
    
    for (auto & de : directory_iterator(path))
    {
        
        //string b = de.path().string();
        //cout << name << " name: " << fileNameFromPath(b) << "\nis directory: " << boolalpha << is_directory(de) << endl;
        
        if(is_directory(de))
        {
            send(socket,"d"); //directory
            cout << 'd' << endl;
            sendDirectory(socket,de.path().string());
        }
        else
        {
            send(socket,"r"); //regular file
            cout << 'r' << endl;
            sendfile(socket,de.path().string());
        }
    }
    send(socket,"f"); //finsih
}

void sendfile(int socket,string path)
{
    string name = fileNameFromPath(path);
    
    int file = open(path.c_str(),0);
    
    if(file==-1)
        socketError("open()");
    
    struct stat filestat;
    fstat(file,&filestat);
    size_t size = filestat.st_size;  //size of file
    
    string format = to_string(name.length())+'a'+name+to_string(size)+'a';
    
    cout << format << endl;
    
    send(socket,format);
    
    
    
    sleep(1);
    /*
    char recvbuf[90];
    read(file,recvbuf,90);
    cout << recvbuf << endl;
    */
    if(sendfile(socket,file,NULL,size)==-1)
        socketError("sendfile()");
    
    /*
     0 read
     * 1 write
     * 2 read and write
     */
}

int dirtyConnect()
{
    /* ---------- INITIALIZING VARIABLES ---------- */

    int udpSocket, tcpSocket;
    int udpPortNum = 2001;
    int tcpPortNum = 2002;
    
    
    int bufsize = 1024;
    char buffer[bufsize];
    
    string sendbuf = "";

    struct sockaddr_in server_addr,client_addr,t;
    socklen_t udp_size = sizeof(client_addr);

    /* ---------- ESTABLISHING SOCKET CONNECTION ----------*/
    /* --------------- socket() function ------------------*/

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket< 0) 
        socketError("establishing socket");

    cout << "\n=> Socket server has been created..." << endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(udpPortNum);
    
    

    if ((bind(udpSocket, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
        socketError("bind()");
    
    cout << "listening" << endl;
    
    if(recvfrom(udpSocket, buffer, bufsize,0, (struct sockaddr*)&client_addr, (unsigned int*)&udp_size)<0)
        socketError("recvfrom()");
    
    cout << buffer << endl << "----------------------------"<<endl;;
    
    
    char h[20];
    inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr),h,20);
    cout << "client address is: "<< h << endl;
    
    client_addr.sin_port = htons(tcpPortNum);
    //client_addr.sin_family = AF_INET;
    //inet_pton(AF_INET, "10.0.0.14", &client_addr.sin_addr);
    
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket< 0) 
        socketError("establishing tcp socket");
    
    
    if (connect(tcpSocket,(struct sockaddr *)&client_addr, sizeof(client_addr)) == 0)
        cout << "=> Connection to the server port number: " << tcpPortNum << endl;
    else 
        socketError("connect()");
    
    
    sendbuf = "hi";
    if(send(tcpSocket, sendbuf.c_str(),strlen(sendbuf.c_str()),0)==-1)
        socketError("send()");
    
    cout << "sent and listening" << endl;
    
    memset(buffer, 0, sizeof(buffer));
    if(recv(tcpSocket, buffer,bufsize,0)<0)
        socketError("recv()");
    cout << "client sent: "<<buffer<<endl;
    
        
    
    return tcpSocket;
}

void socketError(string name)
{
    cout << name << " failed with error code : " << errno << endl;
    exit(-1);
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
		if (recv(socket, temp, 1, NULL) == -1)
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
		if (bytesRecv == -1)
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


void intostr(string & out, char * in, int len)
{
	for (int i = 0; i < len; i++)
		out += in[i];
}
  
string fileNameFromPath(const string& path)
{
    int counter = 0;
    int len = 0;
    for(char c: path)
    {
        counter++;
        if (c == '/')
            len = counter;
    }
    return path.substr(len);
}

string fileNameFromPath(const path& path)
{
    string s= path.string();
    return fileNameFromPath(s);
}

void send(int socket,const string& m)
{
    if(send(socket, m.c_str(),strlen(m.c_str()),0)==-1)
        socketError("send()");
    cout << "sent" << endl;
}