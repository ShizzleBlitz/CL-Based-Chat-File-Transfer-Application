#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
using namespace std;


int main() {
	multimap<string,string> Files; //First element is the name, second is filename.
	map<string,string> Users; //first element is the name, second is the IP and port of Client's personal server.
	string nameofUser; //Used for a bunch of things. Can't explain right now.
	bool repeatUser = false;

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("Socket creation failed : ");
		exit (-1);	
	}
	
	struct sockaddr_in s_addr;
	s_addr.sin_addr.s_addr	= INADDR_ANY;
	s_addr.sin_family	= AF_INET;
	s_addr.sin_port		= htons(80);

	if (bind(fd, (struct sockaddr*)(&s_addr), sizeof(s_addr) ) == -1) {
		perror("Bind failed on socket : ");
		exit(-1);
	}	

	struct sockaddr_in c_addr;
	socklen_t cli_len = sizeof(c_addr);
	
	char buffer[1000];
	
	char options[5][1000];
	strcpy(options[0],"Join");
	strcpy(options[1],"Exit");
	strcpy(options[2],"Update");
	strcpy(options[3],"Download");
	strcpy(options[4],"Chat");
	
	while (1) {
		bzero(buffer, sizeof(buffer));
		
		if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) 
		{
			//In case of the user joining the server.
			if (strcmp(buffer,options[0]) == 0)
				while (1)
				{
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) 
					{
						map<string,string>::iterator it = Users.begin();
						repeatUser = false;
						while (it != Users.end())
						{
							if (strcmp((it->first).c_str(),buffer) == 0)
							{
								cout << "repeat user has been found. Sending invalid username response." << endl;
								sendto(fd, "Invalid", 1000, 0, (struct sockaddr*)&c_addr, cli_len); 
								repeatUser = true;
								break;
							}
							it++;
						}
						
						if (repeatUser)
							break;
						
						sendto(fd, "Valid", 1000, 0, (struct sockaddr*)&c_addr, cli_len);
						
						if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) 
							Users[buffer] = string(inet_ntoa(c_addr.sin_addr)) + ":" + string(buffer); //Other users will use this port to connect to and start chats or download files.
						
						cout << "New user has joined: " << buffer << " - " << Users[buffer] << endl;
						break;
					}
				}
			
			//In case of user leaving the server.
			if (strcmp(buffer,options[1]) == 0)
			{
				while (1)
				{
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) {nameofUser = buffer;  break;}
				}
				
				Files.erase(nameofUser);
				
				map<string,string>::iterator it = Users.begin();
				while (it != Users.end())
				{
					if (it->first == nameofUser)
					{
						Users.erase(it->first);
						cout<<"User has left the server: "<< it->first << " - " << it->second << endl;
						break;
					}
					it++;
				}
			}
			
			//In case of user Updating the list.
			if (strcmp(buffer,options[2]) == 0)
			{
				while (1)
				{
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) {nameofUser = buffer;  break;}
				}
				
				Files.erase(nameofUser);
				//cout <<"updating files..." << endl;
				
				while (1)
				{
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) 
					{
						//cout << buffer << endl;
						if (strcmp(buffer,"UpdateEND") == 0)
							break;
						Files.insert(pair<string,string> (nameofUser, buffer));

					}
				}
			}
			
			//In case  of user Downloading a file.
			if (strcmp(buffer,options[3]) == 0)
			{
				while (1) {
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) {nameofUser = buffer;  break;}
				}
				
				cout << "User wants to download: " << buffer << endl;
				multimap<string,string>::iterator it = Files.begin();
				string singleFile;
				
				while (it != Files.end())
				{	
					if (nameofUser == it->first)
					{
						it++;
						continue;
					}
					
					singleFile = it->first + ":" + it->second;
					sendto(fd, singleFile.c_str(), strlen(singleFile.c_str()), 0, (struct sockaddr*)(&c_addr), cli_len);
					it++;
				}
				
				sendto(fd, "ListEnd", 1000, 0, (struct sockaddr*)(&c_addr), cli_len);
				repeatUser = false; //This isn't actually used for repeat users, but to check whether the user has cancelled out of 
				while (1) {
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) 
					{
						if (strcmp(buffer,"Cancel") == 0)
						{
							repeatUser = true;
							break;
						}
						nameofUser = buffer;  
						break;
					
					}
				}
				
				if (repeatUser)
					continue;
				
				sendto(fd, Users[string(nameofUser)].c_str(), 1000, 0, (struct sockaddr*)(&c_addr), cli_len);
			}
			
			//In case of user wanting to chat with another user.
			if (strcmp(buffer,options[4]) == 0)
			{
				while (1)
				{
					bzero(buffer, sizeof(buffer));
					if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)(&c_addr), &cli_len) > 0) {nameofUser = buffer;  break;}
				}
				
				cout<<"User wants to chat:"<<buffer << endl;
				map<string,string>::iterator it = Users.begin();
				string singleUser;
				
				while (it != Users.end())
				{	
					if (nameofUser == it->first)
					{
						it++;
						continue;
					}
					
					singleUser = it->first + ":" + it->second;
					sendto(fd, singleUser.c_str(), strlen(singleUser.c_str()), 0, (struct sockaddr*)(&c_addr), cli_len);
					it++;
				}
				
				sendto(fd, "ListEnd", 1000, 0, (struct sockaddr*)(&c_addr), cli_len);
			}
		}
		
		else {
			perror("Receive failed on socket : ");
			break;
		}
	}

	return 0;
}
