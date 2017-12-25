extern "C"{
#include "csapp.h"
}

using namespace std;
#include <iostream>
#include <string>
#include <map>

/*make sure the port or secret key are integers */
int validateDigits(string userIn){
	for(unsigned int i = 0; i < userIn.length(); i+=1){
		if(!(isdigit(userIn[i]))){
			cout << userIn << " is not a valid integer " << endl;
			exit(1);/*userIn[i] was not an int, so throw error and quit the program*/
		}
	}
	return atoi(userIn.c_str());
}

/*print the details about the client's request type*/
void displayDetails(string requestType, string detail, char status){
	cout << "Request type = " << requestType << endl;
	cout << "Detail = " << detail << endl;
	string completion;
	if(status == 0){
		completion = "success";
	}else{
		completion = "failure";
	}
	cout << "Completion = " << completion << endl;
}

/*handle the ssSet program*/
void handleSet(int connfd, map<string, string> &envVariables){
	char varName[15 + 1];/*15 char's for the users varibale + 1 for the null term*/
	Rio_readn(connfd, varName, 16);
	
	/*Get the length of length of the variable values length and convert it to host order*/
	int clientVarLength[1];
	Rio_readn(connfd, clientVarLength, 4);
	int varLength = ntohl(clientVarLength[0]);
	char varValue[varLength + 1];
	Rio_readn(connfd, varValue, varLength + 1);
	
	/*convert the char arrays hold the variable and its value to strings*/
	string varStr(varName);
	string valueStr(varValue);
	envVariables[varStr] = valueStr;/*add the variable name to the map, with the value being the variables value*/
	
	/*write the successcode and padding to the client*/
	char successCode[2];
	successCode[0] = 0; /*ssSet should always be successful if we've reached this point in handling it*/
	successCode[1] = '\0';
	char padding[4] = {'0', '0', '0'};
	padding[3] = '\0';
	Rio_writen(connfd, successCode, 1);
	Rio_writen(connfd, padding, 3);

	string requestType = "set";
	displayDetails(requestType, varStr + ": " +  varValue , successCode[0]);
}

/*check whether userVar is a valid environment string*/
char checkPrintenv(string userVar, string &varValue){
	string fileName = "/tmp/get.txt";/*send the ouput to this file*/
        string command = "printenv \"" + userVar + "\" > " + fileName;/*the command to run using system()*/
        int systemCode = system((char*)command.c_str());/*run the command get its return value*/
        if(systemCode < 0){
		return -1;
	}/*system() gives negative int when there is a problem with execution*/

	FILE* outFile;/*file object for the tmp file*/
        outFile = fopen(fileName.c_str(), "rb");
        fseek(outFile, 0, SEEK_END);
        int fileSize = ftell(outFile);/*find the number of bytes in the tmp file*/
	char statusCode;
	if(fileSize != 0){/*the variable is an environment string so get its value*/
		statusCode = 0;
		fseek(outFile, 0, SEEK_SET);
		unsigned int bytesToRead;
		if(fileSize > 99){
			bytesToRead = 99;
		}else{
			bytesToRead = fileSize;
		}/*seek the beginning of the file and prep to read in 99 bytes, or the entire file if it's less than 99 bytes*/
		
		char buf[bytesToRead + 1];/*buf to hold the bytes to read from the file + 1 for the null term*/
		fread(buf, 1, bytesToRead, outFile); /*read in bytesToRead bytes from the file into buf*/
		buf[bytesToRead - 1] = '\0';/*buf[bytesToRead] is a \n, so write over that with the null term*/
		varValue = buf;/*assign buf to the string varValue*/
	}else{
		statusCode = -1;
	}/*file size was 0 so userVar was not a valid environment string*/
	fclose(outFile);
	return statusCode;
}

/*Get, Run, and Digest all use the same protocol to send data to the client, so 
use this function to send their successCode and return values to the client*/
void sendReturnData(int connfd, char successCode, unsigned int varLength, string var){
	/*write the successCode and 3 bytes of padding to the client*/
	char returnCode[2];
	returnCode[0] = successCode;
	returnCode[1] = '\0';
	Rio_writen(connfd, returnCode, 1);
	char padding[4] = {'0','0','0'};	
	padding[3] = '\0';
	Rio_writen(connfd, padding, 3);

	/*conver the length of the return value to network order and write that to the client*/
	unsigned int networkLength[1];
	networkLength[0] = htonl(varLength);
	Rio_writen(connfd, networkLength, 4);

	/*store the return value in a char array and write it to the client*/
	char varValue[varLength + 1];
	for(unsigned int i = 0; i < varLength; i+=1){
		varValue[i] = var[i];
	}
	varValue[varLength] = '\0';
	Rio_writen(connfd, varValue, varLength + 1);
}

/*process the Get request*/
void handleGet(int connfd, map<string, string> envVariables){
	char varName[15 + 1];/*varName is @ most 15 bytes + 1 for the null term*/
	Rio_readn(connfd, varName, 16);
        string userVar(varName);/*cast the user's variable name to a string*/
	char successCode[1]; 
	string requestType = "get";
	string varValue;

	/*check if userVar is in the map, if it is then set successCode and send the data to the client*/
	if(envVariables.count(userVar) > 0){
		successCode[0] = 0;
		displayDetails(requestType, varName, successCode[0]);
		sendReturnData(connfd, successCode[0], envVariables[userVar].length(), envVariables[userVar]);
		return;
	}else{/*userVar not in the map, so 1st make sure it's length is not 0--so it's different from: "" or '' */ 
		if(userVar.length() == 0){
			successCode[0] = -1;/*user entered "" or '' so set the successCode for failure*/
			displayDetails(requestType, varName, successCode[0]);
			sendReturnData(connfd, successCode[0], 0, "");
			return;
		}/*make sure the user entered @ least 1 non-whitespace char for the variable name*/
		unsigned int i = 0;
		while(userVar[i] == ' '){
			i+=1;/*loop over the userVar so long as we're @ a whitespace*/
		}
		if(i == userVar.length()){
			successCode[0] = -1;/*all the char's were ' ' so set successCode for failure*/
		}else{
			/*check whether userVar is an environment variable*/
			successCode[0] = checkPrintenv(userVar, varValue);
		}
	}
	displayDetails(requestType, varName, successCode[0]);
	sendReturnData(connfd, successCode[0], varValue.length(), varValue);
}

/*process the Run request*/
void handleRun(int connfd){
	char clientProgram[7 + 1];/*client program may be 7 chars + 1 for the null term*/
	Rio_readn(connfd, clientProgram, 8);
	string program(clientProgram);/*cast the char array to a string*/
	string command;
	string requestType = "run";
	char successCode[1];
	
	/*set the command based on which of the 3 program names were entered*/
	if(program == "inet"){
		command = "/sbin/ifconfig -a";
	}else if(program == "hosts"){
		command = "/bin/cat /etc/hosts";
	}else{
		command = "/usr/bin/uptime";
	}
	/*store output from running the command in /tmp/run.txt */
	string fileName = "/tmp/run.txt";
	command+=" > " + fileName;/*redirect stdout to this tmp file*/
	int systemCode = system((char*)command.c_str());/*run the command and get return value of system() */
	if(systemCode < 0 ){/*system() returns < 0 return upon failure */
		successCode[0] = -1;/*set the successCode for failure and send the return data to the client*/
		displayDetails(requestType, program, successCode[0]);
		sendReturnData(successCode[0], connfd, 0, "");
		return;
	}
	successCode[0] = 0;/*system() didn't fail so set successCode for success*/
	FILE* runFile;/*file object for the tmp file*/
	runFile = fopen(fileName.c_str(), "rb");
	fseek(runFile, 0, SEEK_END);
	int numBytes = ftell(runFile);/*get the # of bytes in the tmp file*/
	fseek(runFile, 0, SEEK_SET);
	unsigned int bytesToRead;/*return to the beginning of the file and determine how many bytes to read in*/
	if(numBytes < 99){
		bytesToRead = numBytes;/* file has < 99 bytes so read all of them in*/
	}else{
		bytesToRead = 99; //read in 99 bytes, and save 1 byte for the null terminator, so 100 total bytes
	}
	char buf[bytesToRead + 1];/*buf to hold the contents of the file + 1 byte for the null term */
	fread(buf, 1, bytesToRead, runFile);
	buf[bytesToRead] = '\0';
	fclose(runFile);
	displayDetails(requestType, program, successCode[0] );	
	sendReturnData(connfd, successCode[0], bytesToRead + 1, buf);
}

/*process the Digest request*/
void handleDigest(int connfd){
	/*read in the length of the clients value */
	int clientValueLength[1];
	Rio_readn(connfd, clientValueLength, 4);
	int valueLength = ntohl(clientValueLength[0]);

	/*char array to hold the clients value + 1 byte for the null term*/
	char clientValue[valueLength + 1];
	Rio_readn(connfd, clientValue, valueLength + 1);
	string value(clientValue);
	string requestType = "digest";
	char successCode[1];
	
	/*store the output of the command in /tmp/digest.txt */
	string fileName = "/tmp/digest.txt";
	string command = "sh -c 'echo `/bin/hostname` " +  value + " | /usr/bin/md5sum' > " + fileName;
	int systemCode = system((char*)command.c_str()); /*run the command and store its return value from system() */
	if(systemCode < 0){
		successCode[0] = -1;/* < 0 return means system() failed*/
		displayDetails(requestType, value, successCode[0]);
		sendReturnData(connfd, successCode[0], 0, "");
		return;
	}
	/*system() didn't fail so read in the contents from the tmp file */
	FILE* digestFile;
	digestFile = fopen(fileName.c_str(), "rb");
	fseek(digestFile, 0, SEEK_END);
	int fileSize = ftell(digestFile);/*get the # of bytes in the tmp file--guarenteed to be less than 100 */
	fseek(digestFile, 0, SEEK_SET);
	char buf[fileSize + 1];/*buf to hold the # of bytes in the file + 1 for the null term*/
	fread(buf, 1, fileSize, digestFile);/*read the file into buf*/
	fclose(digestFile);
	buf[fileSize] = '\0';

	successCode[0] = 0;/*set the successCode for success*/
	displayDetails(requestType, value, successCode[0]);
	sendReturnData(connfd, successCode[0], fileSize + 1, buf);
}

int main(int argc, char* argv[]){
	if(argc != 3){
		cerr << "useage: ./simpled <port> <key>" << endl;
		exit(1); /*simpled expects only 3 arguements, throw error otherwise*/
	}
	int listenfd, connfd, listenPort;	
	struct sockaddr_in clientAddr;
	listenPort = validateDigits(argv[1]);/*set the port to the 2nd param*/
	listenfd = open_listenfd(listenPort);/*listen on that port*/
	socklen_t addrLength = sizeof(clientAddr);
	map<string, string> envVariables;/*map to hold only variables given using ssSet*/
	unsigned int serverKey = validateDigits(argv[2]);

	while(1){
		clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);/*use the IP address INADDR_ANY*/
		bind(listenfd, (SA*)&clientAddr, addrLength);/*bind this INADDR_ANY to the socket*/
		connfd = Accept(listenfd, (SA *)&clientAddr, &addrLength);/*start accepting connections*/
		if(connfd < 1){
			perror("accept error");
		}

		unsigned int clientKey[1];
		Rio_readn(connfd, clientKey, 4); 
		unsigned int key = ntohl(clientKey[0]);/*convert the client's key from network to host byte order*/

		/*get the type of the client's request and their padding*/
		char type[2];
		Rio_readn(connfd, type, 1);
		type[1] = '\0';
		char padding[4];
		Rio_readn(connfd, padding, 3);
		padding[3] = '\0';

		cout << "Secret key = " << key << endl;
		if(serverKey != key){/*if they gave the wrong key then set the successCode to -1 and end their connection*/
			sendReturnData(connfd, -1, 0, "");
		        cout << "--------------------------" << endl;
			Close(connfd);
			continue;
		}
		/*call relevant function to handle each of the 4 possible client program*/
		if(type[0] == 0){
			handleSet(connfd, envVariables);
                }else if(type[0] == 1){
                        handleGet(connfd, envVariables); 
		}else if(type[0] == 2){
                        handleDigest(connfd);
                }else if (type[0] == 3){
                        handleRun(connfd);
                }
	        cout << "--------------------------" << endl;
		Close(connfd);
	}
	return 0;
}
