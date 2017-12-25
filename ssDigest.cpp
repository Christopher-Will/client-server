extern "C" {
#include "csapp.h"
}

using namespace std;
#include <iostream>
#include <string>

void simpleDigest(string machineName, string TCPport, string secretKey, string value){	
	int valueLength = value.length();
	if(valueLength > 99){
		cout << value << " cannot exceed 100 characters " << endl;
		exit(1);/*the variable may only be a total of 100 bytes, where the 100th is the null term*/
	}
	
	int toserverfd;
        rio_t rio;
	int port = atoi(TCPport.c_str());
        toserverfd = Open_clientfd((char*)machineName.c_str(), port); 
	Rio_readinitb(&rio, toserverfd);

	/*convert the key to a network ordered byte and write it to the server*/
	unsigned int key = htonl(atoi(secretKey.c_str()));
	unsigned int clientKey[1] = {key};
	Rio_writen(toserverfd, clientKey, 4); 
	
	/*ssDigest is type 2--write that and the padding to the server*/
	char type[1] = {2};
	Rio_writen(toserverfd, type, 1);
	char padding[3] = {'0', '0', '0'};
	Rio_writen(toserverfd, padding, 3);
	
	/*convert the length of the variable to network order and write it to the server*/
	int networkLength = htonl(valueLength);
	int clientValueLength[1] = {networkLength};
	Rio_writen(toserverfd, clientValueLength, 4);

	/*store the value of the variable in a char array and write it to the server*/
	char clientValue[valueLength + 1];
	for(int i = 0; i < valueLength; i+=1){
		clientValue[i] = value[i];
	}
	clientValue[valueLength] = '\0';
	Rio_writen(toserverfd, clientValue, valueLength + 1);
        
	/*get the success code and padding from the server*/
	char successCode[2];
        Rio_readn(toserverfd, successCode, 1);
        successCode[1] = '\0';

        char serverPadding[4];
        Rio_readn(toserverfd, serverPadding, 3);
        serverPadding[3] = '\0';

	/*Get the length of the return value from the server and convert it to host order*/
        unsigned int serverLength[1];
        Rio_readn(toserverfd, serverLength, 4);
        unsigned hostLength = ntohl(serverLength[0]);

	/*store the result from the server in a char array and output it if successCode different from -1*/
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
		cout << "useage: ./ssDigest <host> <port> <key> <value> " << endl;
		exit(1);//ssDigest expects only 5 arguements, throw an error otherwise
	}
	string host = argv[1];
	string port = argv[2];
	string key = argv[3];
	string value = argv[4];
	simpleDigest(host, port, key, value);
	return 0;
}
