#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <filesystem>

using namespace std;

char CommunityIP[] = "127.0.0.1";

struct Values{
	int fd;
	int* status;
	struct sockaddr_in* s_addr;
};
	
void updateFiles(int fd, struct sockaddr_in s_addr, string files[], string name, int i)
{	
	//Start sending filenames to the server.
	sendto(fd, "Update", 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
	sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
			
	for (int j = 0; j < i; j++)
	{
		sendto(fd, files[j].c_str(), 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
		//cout << "in updatefiles: " << files[j] << endl;
	}
	
	//End sending files to the server.
	sendto(fd, "UpdateEND", 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
}

void* recvMessage(void* argument)
{
	char buffer[1000];
	char secbuffer[1000];
	char *token;
	
	//cout << "In the recvMessage func: " << inet_ntoa((*((* ((struct Values*)argument)).s_addr)).sin_addr) << htons((*((* ((struct Values*)argument)).s_addr)).sin_port) << endl;
	
	struct sockaddr_in c_addr;
	socklen_t cli_len = sizeof(c_addr);
	
	struct sockaddr_in server = *(*((struct Values*) argument)).s_addr;
	socklen_t ser_len = sizeof(*(*((struct Values*) argument)).s_addr);
	
	while (1) {
		bzero(buffer, sizeof(buffer));
		bzero(secbuffer,sizeof(secbuffer));
		
		if (recvfrom((*((struct Values*) argument)).fd, buffer, 1000, 0, (struct sockaddr*) &c_addr, &cli_len) > 0) 
		{
			
			strcpy(secbuffer,buffer);
			token = strtok(secbuffer,":");
			token = strtok(NULL, ":");
			if (strcmp(token,"Exit") == 0)
				break;
				
			cout << endl << buffer << endl;
			cout << "Enter your message(Type 'Exit' to leave): " << endl;
		}
	}

	return argument;
	
}

void ChatSetup(int fd, struct sockaddr_in s_addr, int* status, string name)
{
	char buffer[1000];
	string Users[1000];
	socklen_t ser_len = sizeof(s_addr);
	
	int i = 0;
	int choice = 1;
	
	while (1)
	{
		bzero(buffer, sizeof(buffer));
		if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&s_addr, &ser_len) > 0)
		{
			if (strcmp(buffer,"ListEnd") == 0)
				break;
			
			Users[i] = buffer;
			cout << i+ 1 << "." << Users[ i] << endl;
			i++;
		}
	}
	
	cout << "Which user do you want to chat with (Enter a number, -1 to quit): ";
	cin >> choice;
	while (choice < 1 || choice > i)
	{
		if (choice == -1)
			return;
		cin >> choice;
	}
	
	i = choice - 1;
	
	char TalkTo[1000] = "";
	strcpy(TalkTo,Users[i].c_str());
	
	
	char* token;
	token = strtok(TalkTo,":");
	token = strtok(NULL, ":");
	
	struct sockaddr_in client;
	client.sin_family	= AF_INET;
	inet_aton(token, &client.sin_addr);
	token = strtok(NULL, ":"); //There will be a second port sent to the user, that is the port we want to connect to
	client.sin_port	= htons(atoi(token));	
	
	Values ServerArgs;
	ServerArgs.fd = fd;
	ServerArgs.status = status;
	ServerArgs.s_addr = &client;
	Values *sendtoServer = &ServerArgs;
	
	//cout << "In the startup func: " << inet_ntoa((*(ServerArgs.s_addr)).sin_addr) << htons((*(ServerArgs.s_addr)).sin_port) << endl;
	
	pthread_t th_id;
	if (pthread_create(&th_id, NULL, recvMessage, sendtoServer) != 0) {
			//on success, pthread_create returns 0
			perror("thread creation failed");
		}
	
	
	sendto(fd, (name).c_str(), 1000, 0, (struct sockaddr*)&client, sizeof(client));
	system("clear");
	cin.ignore();
	while (1)
	{
		bzero(buffer, sizeof(buffer));
		cout << "Enter your message(Type 'Exit' to leave): ";
		cin.getline(buffer,1000);
		//cout << "Your message was: " << buffer;
		sendto(fd, (name + ":" + string(buffer)).c_str(), 1000, 0, (struct sockaddr*)&client, sizeof(client)); //Sending name along confirms to receiver that its us.
		
		if (strcmp(buffer,"Exit") == 0)
			break;	
	}
}


void* recvMessageServer(void* argument)
{
	char buffer[1000];
	char secbuffer[1000];
	int *status = (*(struct Values*) argument).status;
	struct sockaddr_in client;
	socklen_t ser_len = sizeof(client); //previously ((struct Values*) argument)->s_addr
	string nameofSender;
	char* token;
	
	while (1)
	{
		bzero(buffer, sizeof(buffer));
		if (recvfrom((*((struct Values*) argument)).fd, buffer, 1000, 0, (struct sockaddr*) &client, &ser_len) > 0) 
		{
			if (strcmp(buffer,"Download") == 0)
			{
				//cout << "User wants to download something" << endl;
				while (1) {
						bzero(buffer, sizeof(buffer));
						if (recvfrom((*((struct Values*) argument)).fd, buffer, 1000, 0, (struct sockaddr*) &client, &ser_len) > 0)
							break;
						
					}
			
				
				//cout << "User wants to download the file: " << buffer << endl;
				ifstream uploadFile(buffer);
				int fileSize = filesystem::file_size(buffer);
				
				//cout << "File has this no. of bytes: " << filesystem::file_size(buffer) << endl;
				if (uploadFile.fail())
					sendto((*((struct Values*) argument)).fd, "Failure", 1000, 0, (struct sockaddr*) &client, ser_len);
				
				
				while (1) {
					//Going to be sending bits and waiting for acknowledgement				
					bzero(buffer, sizeof(buffer));
					if (fileSize >= sizeof(buffer))
					{
						uploadFile.read(buffer,sizeof(buffer));
						fileSize = fileSize - sizeof(buffer);
						sendto((*((struct Values*) argument)).fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &client, ser_len);
					}
					else if (fileSize >= 0)
					{
						uploadFile.read(buffer,fileSize);
						
						sendto((*((struct Values*) argument)).fd, "Final", sizeof(buffer), 0, (struct sockaddr*) &client, ser_len);
						sendto((*((struct Values*) argument)).fd, (to_string(fileSize)).c_str(), sizeof(buffer), 0, (struct sockaddr*) &client, ser_len);
						sendto((*((struct Values*) argument)).fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &client, ser_len);
						
						fileSize = 0;
					}
					
					
					while (1) {
						bzero(buffer, sizeof(buffer));
						if (recvfrom((*((struct Values*) argument)).fd, buffer, 1000, 0, (struct sockaddr*) &client, &ser_len) > 0)
							if (strcmp(buffer,"Received") == 0)
								break;
					}
					
					if (fileSize == 0)
					{
						sendto((*((struct Values*) argument)).fd, "Sent", 1000, 0, (struct sockaddr*) &client, ser_len);
						uploadFile.close();
						break;
					}
				}
			}
			else
			{
				nameofSender = string(buffer);
				*(((struct Values*) argument)->s_addr) = client; //to make sure main thread sends messages to the correct place.
				cout << "(You've got messages. You cannot download files without looking at them)" << endl;
				
			while (1) {
			if (*status != 1)
				continue;
				
			bzero(buffer, sizeof(buffer));
			bzero(secbuffer, sizeof(secbuffer));
		
			if (recvfrom((*((struct Values*) argument)).fd, buffer, 1000, 0, (struct sockaddr*) &client, &ser_len) > 0) 
			{
				
				strcpy(secbuffer,buffer);
				token = strtok(secbuffer,":");
				if (strcmp(token,nameofSender.c_str()) != 0)
					continue;
	
				token = strtok(NULL, ":");
				//cout << "Token is: " << token << endl;
				if (strcmp(token,"Exit") == 0)
					break;
			
			//cout << "\r                                                 \r";
				cout <<  endl << buffer << endl;
				cout << "Enter your message(Type 'Exit' to leave):" << endl;
			}
			}
			
				*status = 0;
	
			}
			
		}	
	}
	return argument;
}
void Download_file(int fd, struct sockaddr_in s_addr)
{
	char buffer[1000];
	string Files[1000];
	socklen_t ser_len = sizeof(s_addr);
	
	int i = 0;
	int choice = 0;
	
	cout << endl;
	while (1)
	{
		bzero(buffer, sizeof(buffer));
		if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&s_addr, &ser_len) > 0)
		{
			if (strcmp(buffer,"ListEnd") == 0)
				break;
			
			Files[i] = buffer;
			cout << i+ 1 << "." << Files[ i] << endl;
			i++;
		}
	}
	
	cout << "Which file do you want to download? (Enter a number, -1 to quit): ";
	
	cin >> choice;
	while (choice < 1 || choice > i)
	{
		if (choice == -1)
		{
			sendto(fd, "Cancel", 1000, 0, (struct sockaddr*)&s_addr, ser_len);	
			return;	
		}
		cin >> choice;
	}
	
	i = choice - 1;
	
	char TalkTo[1000] = "";
	strcpy(TalkTo,Files[i].c_str());
	
	char* token;
	token = strtok(TalkTo,":");
	sendto(fd, token, 1000, 0, (struct sockaddr*)&s_addr, ser_len);	
	while (1) {
		bzero(buffer, sizeof(buffer));
		if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&s_addr, &ser_len) > 0)
		{
			cout << "You want to talk to: " << buffer << endl;
			break;
		}
	}
	
	token = strtok(NULL,":");
	strcpy(TalkTo,token);
	
	token = strtok(buffer,":");
	struct sockaddr_in client;
	client.sin_family	= AF_INET;
	inet_aton(token, &client.sin_addr);
	token = strtok(NULL, ":"); //There will be a second port sent to the user, that is the port we want to connect to
	client.sin_port	= htons(atoi(token));
	
	sendto(fd, "Download", 1000, 0, (struct sockaddr*)&client, sizeof(client));
	sendto(fd, TalkTo, 1000, 0, (struct sockaddr*)&client, sizeof(client));
	ofstream newFile(TalkTo);
	int finalLength = 0;
	while (1)
	{
		bzero(buffer, sizeof(buffer));
		if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&client, &ser_len) > 0)
		{
			if (strcmp(buffer,"Sent") == 0)
				break;
			else if (strcmp(buffer,"Failure") == 0)
			{
				cout << "The file does not exist or permission is not granted" << endl;
				break;
			}
			
			else if (strcmp(buffer,"Final") == 0)
			{
				bzero(buffer, sizeof(buffer));
				if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&client, &ser_len) > 0)
					finalLength = atoi(buffer);
				
				bzero(buffer,sizeof(buffer));
				if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&client, &ser_len) > 0)
					newFile.write(&buffer[0],finalLength);
			}
			
			else
				newFile.write(&buffer[0],sizeof(buffer));
			
			sendto(fd, "Received", 1000, 0, (struct sockaddr*)&client, sizeof(client));
		}
	}
	
	//newFile.write("",1);
	newFile.close();
}

int main() {
	char buffer[1000];
	string name;
	string files[40];
	int i = 0;
	
	int* status;
	*status = 0; //0 means keep the main thread running, 1 user is talking to another user server and 2 means file is being downloaded.
	pthread_t th_handler;
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("Socket creation failed : ");
		exit (-1);	
	}
	
	struct sockaddr_in s_addr;
	s_addr.sin_family	= AF_INET;
	s_addr.sin_port		= htons(80);
	inet_aton(CommunityIP, &s_addr.sin_addr);

	
	cout<<"Enter your display name: ";
	cin >> name;
	
	
	//Joining the server.
	socklen_t ser_len = sizeof(s_addr);
	sendto(fd, "Join", 1000, 0, (struct sockaddr*)&s_addr, ser_len);	
	sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, ser_len);
	
	//confirming that the name isn't taken.
	while (1)
	if (recvfrom(fd, buffer, 1000, 0, (struct sockaddr*)&s_addr, &ser_len) > 0)
		{
			if (strcmp(buffer,"Valid") == 0)
				break;
			cout << "That name is already taken. Enter again: " << endl;
			cin >> name;
			sendto(fd, "Join", 1000, 0, (struct sockaddr*)&s_addr, ser_len);	
			sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, ser_len);
		}
		
	//Creating a second server to chat with other users.
	int chatfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (chatfd == -1) {
		perror("Socket creation failed : ");
		exit (-1);	
	}
	
	struct sockaddr_in chat_addr;
	chat_addr.sin_addr.s_addr = INADDR_ANY;
	chat_addr.sin_family	= AF_INET;
	chat_addr.sin_port	= htons(0);
	socklen_t chat_len = sizeof(chat_addr);

	if (bind(chatfd, (struct sockaddr*)(&chat_addr), sizeof(chat_addr) ) == -1) {
		perror("Bind failed on socket : ");
		exit(-1);
	}	
	getsockname(chatfd, (struct sockaddr*)(&chat_addr), &chat_len);
	cout << "Your server port is:" << htons(chat_addr.sin_port) << endl;
	sendto(fd, (to_string(htons(chat_addr.sin_port))).c_str(), 1000, 0, (struct sockaddr*)&s_addr, ser_len);
	struct sockaddr_in c_addr2;	

	//Creating a thread for receiving messages on the chat server
	Values ServerArgs;
	ServerArgs.fd = chatfd;
	ServerArgs.status = status;
	ServerArgs.s_addr = &c_addr2;
	
	Values *sendtoServer = &ServerArgs;
	
	if (pthread_create(&th_handler, NULL, recvMessageServer, sendtoServer) != 0) {
			//on success, pthread_create returns 0
			perror("thread creation failed");
		}
	

	//Setting buffer[0] to '3' so that it always updates the server upon joining.
	buffer[0] = '3';
	
	//Using do-while so it always updates at least once.
	do {
		switch(buffer[0])
		{
			case '1':
			sendto(fd, "Download", 1000, 0, (struct sockaddr*)&s_addr, ser_len);
			sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, ser_len);
			Download_file(fd, s_addr);
			break;
			
			case '2':
			sendto(fd, "Chat", 1000, 0, (struct sockaddr*)&s_addr, ser_len);
			sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, ser_len);
			ChatSetup(fd,s_addr,status,name);
			break;
			
			case '3':
			i = 0;
			for (const auto & entry : filesystem::directory_iterator("."))
			{	
				if (entry.is_directory())
					continue;
			
				if (i == sizeof(files))
					break;
	     
			    	files[i] = entry.path().filename();
       				//cout << files[i] << endl;
       				i++;
        	
			}
			
			updateFiles(fd,s_addr,files,name,i);
			break;
			
			case '4':
			*status = 1; 
			cin.ignore(); //just for clearing buffer
			break;
			
			case '5':
			sendto(fd, "Exit", 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));	
			sendto(fd, name.c_str(), 1000, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));	
			break;
	
		}
		
		if (buffer[0] == '5')
		{
			cout << "socket being closed" << endl;
			break;
		}
		
		
		while (*status == 1) {
			//cout << "In the main func: " << inet_ntoa(c_addr2.sin_addr) << htons(c_addr2.sin_port) << endl;
			cout << "Enter your message(Type 'Exit' to leave): " << endl;
			bzero(buffer, sizeof(buffer));
			cin.getline(buffer,1000);
			
			sendto(chatfd, (name + ":" + string(buffer)).c_str(), 1000, 0, (struct sockaddr*)&c_addr2, sizeof(c_addr2));
			
			if (strcmp(buffer,"Exit") == 0)
			{
				*status = 0;
				break;
			}
		}
		
		system("clear");
		bzero(buffer, sizeof(buffer));
		cout<<"\nChoices:\n1.Download Files\n2.Chat with another User\n3.Update Files\n4.Check Messages\n5.Exit";	
		cout << "\nEnter your choice: ";
		cin >> buffer;	
			
	} while (1);

	close(fd);
	close(chatfd);
	
	return 0;
}
