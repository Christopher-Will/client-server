extern "C" {
#include "csapp.h"
}

using namespace std;
#include <iostream>
#include <string>

void simpleRun(string machineName, string TCPport, string secretKey, string program){
	if(program != "inet" && program != "hosts" && program != "uptime"){
		cerr << program << " is not an acceptable program name" << endl;
		exit(1);
	}/*only run ssRun if the <program> is 1 of the 3 acceptable options*/

	int toserverfd;
        rio_t rio;
	int port = atoi(TCPport.c_str());
        toserverfd = Open_clientfd((char*)machineName.c_str(), port); 
	Rio_readinitb(&rio, toserverfd);


	/*convert the key to a network byte ordered int and write it to the server*/
	unsigned int key = htonl(atoi(secretKey.c_str()));
	unsigned int clientKey[1] = {key};
	Rio_writen(toserverfd, clientKey, 4); 
	
	/*ssRun is type 3, so write that and 3 bytes of padding*/
	char type[1] = {3};
	Rio_writen(toserverfd, type, 1);
	char padding[3] = {'0', '0', '0'};
	Rio_writen(toserverfd, padding, 3);
	
	/*program name is @ most 7 char's, plus 1 for the null term*/
	/*assign the program name to a char array and write that to the server*/
	char programName[7 + 1];
	for(unsigned int i = 0; i < program.length(); i+=1){
		programName[i] = program[i];
	}
	programName[7] = '\0';
	Rio_writen(toserverfd, programName, 8);
	

	/*get the success code and padding from the server*/
        char successCode[2];
        Rio_readn(toserverfd, successCode, 1);
        successCode[1] = '\0';

        char serverPadding[4];
        Rio_readn(toserverfd, serverPadding, 3);
        serverPadding[3] = '\0';

	/*get the length of return value in host order*/
        unsigned int serverLength[1];
        Rio_readn(toserverfd, serverLength, 4);
        unsigned hostLength = ntohl(serverLength[0]);

	/*output the 1st 100 bytes from ssRun, or failure*/
        char serverValue[hostLength + 1];
        Rio_readn(toserverfd, serverValue, hostLength + 1);
        if(successCode[0] == -1){
                cout << "failure" << endl;
        }else{
                cout << serverValue << endl;
        }
	Close(toserverfd);
}

int main(int argc, char* argv[]){
	if(argc != 5){
		cout << "useage: ./ssRun <host> <port> <key> <program> " << endl;
		exit(1);/*ssRun expects only 5 arguements, throw error otherwise*/
	}
	string host = argv[1];
	string port = argv[2];
	string key = argv[3];
	string program = argv[4];
	simpleRun(host, port, key, program);
	return 0;
}
