extern "C" {
#include "csapp.h"
}

using namespace std;
#include <iostream>
#include <string>

void simpleSet(string machineName, string TCPport, string secretKey, string varName, string varValue){
	int varLength = varName.length();
	if(varLength > 15){
        	cerr << "variable name must be no more than 15 characters " << endl;
                exit(1);/*variable name may only be 15 bytes long + 1 byte for the null term*/
        }
	int valueLength = varValue.length();
	if(valueLength > 99){ /*valueLength can be a max of 100 bytes, where the 100th byte would be the null term*/
		cerr << "length of the variables value may not exceed 100 " << endl;
		exit(1);
	}

	int toserverfd;
        rio_t rio;
	int port = atoi(TCPport.c_str());
        toserverfd = Open_clientfd((char*)machineName.c_str(), port); 
	Rio_readinitb(&rio, toserverfd);

	/*convert the key to a network byte ordered int and write it to the server fd*/
	unsigned int key = htonl(atoi(secretKey.c_str()));
	unsigned int clientKey[1] = {key};
	Rio_writen(toserverfd, clientKey, 4); 
	
	/*ssSet has type of 0, so write that and 3 bytes of padding to the server fd*/
	char type[1] = {0};
	Rio_writen(toserverfd, type, 1);
	char padding[3] = {'0', '0', '0'};
	Rio_writen(toserverfd, padding, 3);
	
	/*put the value of varName in a char array, which is to be written to the server fd*/
	char clientVar[15 + 1]; //15 bytes for the var Name and 1 byte for the null term
	for(int i = 0; i < varLength; i+=1){
		clientVar[i] = varName[i];
	}
	clientVar[varLength] = '\0';
	Rio_writen(toserverfd, clientVar, 16);
	
	/*convert the length of variables value and network byte order and write it to the server fd*/
	int networkLength = htonl(valueLength);
	int length[1] = {networkLength};
	Rio_writen(toserverfd, length, 4);
	
	/*store the variables value in a char array and write it to the server*/
	char clientValue[valueLength + 1];
	for(int i = 0; i < valueLength; i+=1){
		clientValue[i] = varValue[i];
	}
	clientValue[valueLength] = '\0';
	Rio_writen(toserverfd, clientValue, valueLength + 1);
	
	/*get the success code from the server and output failure if ssSet failed*/
	char successCode[2];
	char serverPadding[3];
	Rio_readn(toserverfd, successCode, 1);
	successCode[1] = '\0';
	Rio_readn(toserverfd, serverPadding, 3);
	serverPadding[3] = '\0';
	if(successCode[0] == -1){
		cout << "failure" << endl;
	}

	Close(toserverfd);
}

int main(int argc, char* argv[]){
	if(argc != 6){
		cout << "useage: ./ssSet <host> <port> <key> <variable> <value> " << endl;
		exit(1);/*ssSet expects only 6 params, throw error otherwise*/
	}
	string host = argv[1];
	string port = argv[2];
	string key = argv[3];
	string varName = argv[4];
	string varValue = argv[5];
	simpleSet(host, port, key, varName, varValue);
	return 0;
}
