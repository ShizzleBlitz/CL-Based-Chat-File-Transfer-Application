# CL-Based-Chat-File-Transfer-Application
A command line based file transfer and chatting application made in C++ for Linux.


# Setup
This application was creted in Linux and requires a c++ compiler to execute. To setup the server, you must first specify the address of the server in the Client.cpp file. It is the variable labeled "CommunityIP".

Afterwards, both files must be compiled and the Community file should be run first. Then you can add as many clients as you want to the server and have them exchange files with each other.


# Main Menu
![image](https://github.com/ShizzleBlitz/CL-Based-Chat-File-Transfer-Application/assets/100959619/b6b9d207-6fd6-469d-89a2-42df55d10393)

These are the options available to you when you setup your client. the Check Messages option lets you check received messages from other users, but only one at a time. The other options let you do the following.

1. Download files from a list. These are directly downloaded to the folder where you launnched the application, so be sure there are no conflicting filenames.

![image](https://github.com/ShizzleBlitz/CL-Based-Chat-File-Transfer-Application/assets/100959619/807a2ffe-f3da-4777-98e0-1845f40390cc)

2. Chat with another user. This will send messages to them but only one user can chat with another user at a time.

![image](https://github.com/ShizzleBlitz/CL-Based-Chat-File-Transfer-Application/assets/100959619/44b82e10-00dd-4a39-b2b4-d55a5f9954f6)

3. Update Files. This will update the Community on which files you have available for transfer. to do this, you need to put them in the same directory as the Client beforehand.

![image](https://github.com/ShizzleBlitz/CL-Based-Chat-File-Transfer-Application/assets/100959619/a64a0463-1e3b-47df-9d70-17f4a6afa7ee)


# Issues
There may yet be a few bugs that haven't been ironed out in the project. The project can easily be expanded on to include multiple mailboxes for the receiving messages, allow for custom filepaths to be shared, let users talk in a different window or even add GUI capabilities. For now though, it's a simple and acceptable Semester Project that I may revisit in the future.
