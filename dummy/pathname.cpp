#include <iostream>
#include <string>
using namespace std;
int main(){
	
	string directory,pathname;
	string wholecommand;
	getline(cin,wholecommand);
    for(int i=0; i< wholecommand.length(); i++){
	if(wholecommand[i]=='c' && wholecommand[i+1]=='d'){
            for(int j = i+3; j< wholecommand.length();j++){ 
                if(isprint(wholecommand[j])!= 0 && isblank(wholecommand[j])== 0){
                    directory += wholecommand[j];
                }
            }
        }
    }
    cout << directory << endl << "length :" << directory.length() << endl;
}
