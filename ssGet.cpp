extern "C" {
#include "csapp.h"
}

using namespace std;
#include <iostream>
#include <string>


void simpleGet(string machineName, string TCPport, string secretKey, string varName){
	int varLength = varName.length();
	if(varLength > 15){//the 16th byte will be the null term.
        	cerr << "variable name must be no more than 15 characters " << endl;
                exit(1);
        }

	int toserverfd;
        rio_t rio;
	int port = atoi(TCPport.c_str());/*convert the port to an int*/
        toserverfd = Open_clientfd((char*)machineName.c_str(), port); 
	Rio_readinitb(&rio, toserverfd);

	unsigned int key = htonl(atoi(secretKey.c_str()));/*convert the key to an int in network byte order*/
	unsigned int clientKey[1] = {key};/*add this networked byte ordered key to an int array to send to the server*/
	Rio_writen(toserverfd, clientKey, 4); /*write the key to the sever*/
	
	char type[1] = {1};/*ssGet has type value 1*/
	Rio_writen(toserverfd, type, 1);
	char padding[3] = {'0', '0', '0'};
	Rio_writen(toserverfd, padding, 3);

	char clientVar[15 + 1];/*holds the name of the client's variable, which is @ most 15 char's + 1 null term*/
	for(int i = 0; i < varLength; i+=1){
		clientVar[i] = varName[i];
	}
	clientVar[varLength] = '\0';
	Rio_writen(toserverfd, clientVar, 16);
	
	/*Reead in the success code and padding from ther server*/
	char successCode[2];
	Rio_readn(toserverfd, successCode, 1);
	successCode[1] = '\0';

	char serverPadding[4];
	Rio_readn(toserverfd, serverPadding, 3);
	serverPadding[3] = '\0';

	/*get the length of value asscoiated with the variable name*/
	unsigned int serverLength[1];
	Rio_readn(toserverfd, serverLength, 4);
	unsigned hostLength = ntohl(serverLength[0]);

	/*have char array with enough space for the variable value, + 1 byte for the null term*/
	char serverValue[hostLength + 1];
	Rio_readn(toserverfd, serverValue, hostLength + 1);
	if(successCode[0] == -1){
		cout << "failure" << endl;
	}else{
		cout << serverValue << endl;
	}/*output failure or the value of the variable, depending on successCode*/
	Close(toserverfd);
}

int main(int argc, char* argv[]){
	if(argc != 5){
		cout << "useage: ./ssGet <host> <port> <key> <variable> " << endl;
		exit(1); //thow error and return if # of params is not 5
	}
	/*assign the host, port, key, and variable name based on their loaction on argv*/
	string host = argv[1];
	string port = argv[2];
	string key = argv[3];
	string varName = argv[4];
	simpleGet(host, port, key, varName);
	return 0;
}
