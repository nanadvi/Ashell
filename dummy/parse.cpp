#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

int main(){
	string wholecommand;
	getline(cin,wholecommand);
	string command;
	bool validCommand = true;
	cout << wholecommand << endl; 
	for (int i = 0; i < wholecommand.length(); i++)
	{
		if(wholecommand[i] =='c')
		{
			if(wholecommand[i+1] == 'd'){
				if(i+2 <= wholecommand.length()-1){
					if(wholecommand[i+2] == ' '){
						for(int j = 0; j<i; j++){
							if(wholecommand[j] != ' '){
								validCommand = false;
								break;
							}
						}
					}
					else
						validCommand = false;
				}
			}
			else
				validCommand = false;
			if(validCommand)
				command = "cd";
				break;
		}
		else if(wholecommand[i] =='l')
		{

			if(wholecommand[i+1] == 's'){
				if(i+2 <= wholecommand.length()-1){
					if(wholecommand[i+2] == ' '){
						for(int j = 0; j<i; j++){
							if(wholecommand[j] != ' '){
								validCommand = false;
								break;
							}
						}
					}
					else
						validCommand = false;
				}

			}
			else
				validCommand = false;
			if(validCommand)
				command = "ls";
				break;
		}
		else if(wholecommand[i] =='f')
		{
			validCommand = true;
			if(wholecommand[i+1] == 'f'){
				if(i+2 <= wholecommand.length()-1){
					if(wholecommand[i+2] == ' '){
						for(int j = 0; j<i; j++){
							if(wholecommand[j] != ' '){
								validCommand = false;
								break;
							}
						}
					}
					else
						validCommand = false;
				}
			}
			else
				validCommand = false;
			if(validCommand)
				command = "ff";
				break;
		}
		else if(wholecommand[i] =='e')
		{
			validCommand = true;
			if(wholecommand[i+1] == 'x' && wholecommand[i+2] == 'i' &&
			wholecommand[i+3] == 't'){
				if(i+4 <= wholecommand.length()-1){
					if(wholecommand[i+4] == ' '){
						for(int j = 0; j<i; j++){
							if(wholecommand[j] != ' '){
								validCommand = false;
								break;
							}
						}
					}
					else
					 validCommand = false;
				}
			}
			else
				validCommand = false;
			if(validCommand)
				command = "exit";
				break;
		}
		else if(wholecommand[i] == ' ')
			continue;
		else
			validCommand = false;
		
	}
	if(!validCommand)
		command = "ERROR";
        cout << command << endl;
}


