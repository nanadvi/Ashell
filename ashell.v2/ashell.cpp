#define BUFFER_SIZE 1024
#include <cstdio>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <cctype>
#include <cstring>
#include <vector>
#include <sys/wait.h>
using namespace std;


string history[10];
int historySelector = 0; //use this number as index when the arrow keys are hit
int historySize = 0; // every time ENTER is pressed historySize increases and commands will be put in the array
string wholecommand;
string thepath;
string **arguments;
string *commands;
int commandNum = 0; 
int pipesNum = 0;
int leftcNum = 0;
int rightcNum = 0;
int argNum = 0;
string ** pipes;
string ** leftChevron;
string ** rightChevron;


void execParser(){
    bool leftPipe = false;
    bool leftleftChev= false;
    bool leftrightChev= false;
    commands = new string[wholecommand.length()];
    pipes = new string *[wholecommand.length()];
    arguments = new string *[wholecommand.length()];
    leftChevron = new string *[wholecommand.length()];
    rightChevron = new string *[wholecommand.length()];
    for (int i=0; i < wholecommand.length(); i++)
    {
        commands[i] = "";
        pipes[i] = new string[2];              //resizing arrays 
        leftChevron[i] = new string[2];
        rightChevron[i] = new string[2];
        arguments[i] = new string[wholecommand.length()];
        for (int j=0; j<2; j++)
        {
            
            pipes[i][j] = "";
            
            leftChevron[i][j] = "";
            rightChevron[i][j] = "";
        }
        for (int j=0; j<wholecommand.length(); j++)
            arguments[i][j] = "";

    }
    for(int i=0; i< wholecommand.length(); i++){
        argNum = 0;
        if(isspace(wholecommand[i])==0){
            for(int j = i; j<wholecommand.length(); j++){
                if(isspace(wholecommand[j])== 0){
                    commands[commandNum] += wholecommand[j];
                }
                else //whitespace
                {
                    i = j;
                    break;
                }  
                i=j;
            }
            commandNum++;
            i++;
            for(int j = i; j<wholecommand.length(); j++){
                if(isspace(wholecommand[j])==0 && wholecommand[j]!='>' && wholecommand[j]!='<' && wholecommand[j]!='|' && wholecommand[j]!='\n'){
                    arguments[commandNum-1][argNum] += wholecommand[j];
                    
                }
                else if(wholecommand[j]=='|'){
                    if(!leftPipe){
                        leftPipe = true;
                        pipes[pipesNum][0] = commands[commandNum-1]; //putting cat
                    }
                    else if(leftPipe){
                        pipes[pipesNum][1] = commands[commandNum-1]; // puts grep
                        pipesNum++;
                        pipes[pipesNum][0] = commands[commandNum-1];
                    }
                    if(leftleftChev){
                        leftChevron[leftcNum][1] = commands[commandNum-1]; // puts grep
                        leftcNum++;
                        leftleftChev = false;
                    }
                    if(leftrightChev){
                        rightChevron[rightcNum][1] = commands[commandNum-1]; // puts grep
                        rightcNum++;
                        leftrightChev = false;
                    }
                    i=j;
                    break;
                    
                }
                else if(wholecommand[j]=='>'){
                    if(!leftrightChev){
                        leftrightChev = true;
                        rightChevron[rightcNum][0] = commands[commandNum-1]; //putting cat
                    }
                    else if(leftrightChev){
                        rightChevron[rightcNum][1] = commands[commandNum-1]; // puts grep
                        rightcNum++;
                        rightChevron[rightcNum][0] = commands[commandNum-1];
                    }
                    if(leftPipe){
                        pipes[pipesNum][1] = commands[commandNum-1]; // puts grep
                        pipesNum++;
                        leftPipe = false;
                    }
                    if(leftleftChev){
                        leftChevron[leftcNum][1] = commands[commandNum-1]; // puts grep
                        leftcNum++;
                        leftleftChev = false;
                    }
                    i=j;
                    break;
                    
                }
                else if(wholecommand[j]=='<'){
                    if(!leftleftChev){
                        leftleftChev = true;
                        leftChevron[leftcNum][0] = commands[commandNum-1]; //putting cat
                    }
                    else if(leftleftChev){
                        leftChevron[leftcNum][1] = commands[commandNum-1]; // puts grep
                        leftcNum++;
                        leftChevron[leftcNum][0] = commands[commandNum-1];
                    }
                    if(leftPipe){
                        pipes[pipesNum][1] = commands[commandNum-1]; // puts grep
                        pipesNum++;
                        leftPipe = false;
                    }
                    if(leftrightChev){
                        rightChevron[rightcNum][1] = commands[commandNum-1]; // puts grep
                        rightcNum++;
                        leftrightChev = false;
                    }
                    i=j;
                    break;
                }
                else if(isspace(wholecommand[j])!=0)
                    if(arguments[commandNum-1][argNum] !=""){
                        argNum++;
                    } // dont increment argNum if previous char is whitespace
                if (!(j+1 < wholecommand.length()))
                {
                    i=j;
                    if (leftPipe)
                        pipes[pipesNum][1] = commands[commandNum-1];
                    if (leftleftChev)
                        leftChevron[leftcNum][1] = commands[commandNum-1]; // puts grep
                    if (leftrightChev)
                        rightChevron[rightcNum][1] = commands[commandNum-1];
                }
            }    
        }
    }
}

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

string commandParser()
{
    string command;
    bool validCommand = true;
    for (unsigned i = 0; i < wholecommand.length(); i++)
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
    return command;
}

int noncanonmode()
{
    struct termios SavedTermAttributes;
    char RXChar;
    
    string command;
    string backspace = "\b \b";
    int commandcode = 0;
    
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    
    while(1)
    {
        read(STDIN_FILENO, &RXChar, 1);
        if(0x04 == RXChar){ // C-d
            break;
        }
        else{
            if(isprint(RXChar)){
                write(1, &RXChar, 1);
                wholecommand += RXChar; 
            }
            else if(0x1B == RXChar) // when the up or down arrow is hit
            {
                read(STDIN_FILENO, &RXChar, 1);
                if(0x5B == RXChar){
                    read(STDIN_FILENO, &RXChar, 1);
                    if(0x41 == RXChar){ //when the up arrow is hit
                        // try backspace
                     
                      if(history[historySelector]!="")
                      {
                       
                        for(int i = 0; i < wholecommand.length(); i++){
                            write(1,backspace.c_str(), backspace.length());
                        }
                            
                        wholecommand = history[historySelector];
                        if(historySelector < historySize)
                            historySelector++;                                                     
                        write(1,wholecommand.c_str(),wholecommand.length());                            
                        
                    }
                    }
                    if(0x42 == RXChar){ // when the down arrow is hit
                        // try backspace
                        if(historySelector == 0){
                            if(wholecommand == history[0] && wholecommand.length() != 0)
                            {
                                for(int i = 0; i < wholecommand.length(); i++){
                                    write(1,backspace.c_str(), backspace.length());
                                }
                                wholecommand = "";
                            }
                        }
                        if (historySelector != 0 )
                        {
                            for(int i = 0; i < wholecommand.length(); i++){
                                write(1,backspace.c_str(), backspace.length());
                            }
                            wholecommand = history[historySelector-1];
                            if(historySelector > 0)
                                historySelector--;
                            write(1,wholecommand.c_str(),wholecommand.length());
                        }
                            
                    }
                }
            }
            
            
            
            else if (0x7f == RXChar) //backspace
            {
                if(wholecommand.length() == 0){
                    
                    write(1,"\a",1);
                }
                else{
                    write(1,backspace.c_str(), backspace.length());
                    if (wholecommand.length() > 0)
                        wholecommand = wholecommand.substr(0,wholecommand.length()-1);
                }
            }
            
            else if (0x0A == RXChar) //enter
            {
                if(wholecommand.length()!=0)
                {
                    for(int i = historySize; i > 0; i--)
                    {
                        history[i] = history[i-1];
                    }
                
                history[0] = wholecommand;
                
                if(historySize < 9)
                    historySize++;
                historySelector = 0;
                }
                command = commandParser();
                if (command.compare("pwd") == 0)
                    commandcode = 4;
                else if (command.compare("ls") == 0)
                    commandcode = 2;
                else if (command.compare("cd") == 0)
                    commandcode = 3;
                else if (command.compare("exit") == 0)
                    commandcode = 1;
                else
                    commandcode = 5; //error or external program
                write(1,"\n",1);
                break;    
            }

            else{
                write(1,"",1);
            }
        }
    }
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return commandcode;
}

        /*ref: http://pubs.opengroup.org/onlinepubs/9699919799/toc.htm*/
               /*get the current working director*/
void command_line_interface(){
    char cwd[BUFFER_SIZE];
    string cwdPath;
    thepath = getcwd (cwd, 256); 
    int substr = thepath.find_last_of('/');
    if (substr > 16)
        cwdPath = "/..."+thepath.substr(substr)+"% ";
    else
    cwdPath = thepath+"% ";
    write(1, cwdPath.c_str(), cwdPath.length());
    
}


void fileStat(string dirname)
{
    DIR *dirptr; //the whole directorys
    struct dirent *eadir; //each directory
    struct stat dirinfo;
    struct stat *dirinfop = &dirinfo;
    if ( ( dirptr = opendir(dirname.c_str()) ) != NULL )
    {
        while ( ( eadir = readdir(dirptr)) != NULL)
        { //need to stat each dir
            string pathName(eadir->d_name);
            pathName += "/";
            pathName += dirname;
//            if (dirname != ".")
//                stat(pathName.c_str(), &dirinfo);
//            else
            stat(eadir->d_name, &dirinfo);  
            char perm[11];
            strcpy(perm,"----------");
            if ( S_ISDIR(dirinfop->st_mode) )  perm[0] = 'd';
            if ( dirinfop->st_mode & S_IRUSR ) perm[1] = 'r';    
            if ( dirinfop->st_mode & S_IWUSR ) perm[2] = 'w';
            if ( dirinfop->st_mode & S_IXUSR ) perm[3] = 'x';
            if ( dirinfop->st_mode & S_IRGRP ) perm[4] = 'r';    
            if ( dirinfop->st_mode & S_IWGRP ) perm[5] = 'w';
            if ( dirinfop->st_mode & S_IXGRP ) perm[6] = 'x';
            if ( dirinfop->st_mode & S_IROTH ) perm[7] = 'r'; 
            if ( dirinfop->st_mode & S_IWOTH ) perm[8] = 'w';
            if ( dirinfop->st_mode & S_IXOTH ) perm[9] = 'x';
            //string temp = perm+" "+eadir->d_name+"\n"; 
            string temp(perm);
            temp += " ";
            temp += eadir->d_name;
            temp += "\n";
            write(1,temp.c_str(),temp.length());
        }
        closedir(dirptr);
    }
    else
    {
        //string temp = "Failed to open directory" +"\""+dirname+"\"" +"/"+"\n";
        string temp = "Failed to open directory ";
        temp += "\"";
        temp += dirname;
        temp += "\"";
        temp += "/\n";
        write(1,temp.c_str(), temp.length());
    }    
}

void leftpipe(int fd[], string command, int commNum)
{
    pid_t pid;
    int argCount = 0;
    pid = fork();
    if (pid == -1)
    {
        string err = "Could not fork.\n";
        write(1,err.c_str(),err.length());
    }
    if (pid == 0)
    {
        if (dup2(fd[1],1) == -1)
            write(1,"Dupe error", 10);
        close(fd[0]);
        //close(fd[1]);
        if (command.compare("ls") == 0)
        {
            if(arguments[commNum][0]=="")
                fileStat(".");
            else
                fileStat(arguments[commNum][0]);
            exit(1);
        }
        else
        {
            for (int i = 0; i < wholecommand.length(); i++)
            {
                if (arguments[commNum][i] != "")
                    argCount++;
                else
                    break;
            }
            vector<char *> args;
            char *temp2 = new char[commands[commNum].length()+1];
            strcpy(temp2, commands[commNum].c_str());
            args.push_back(temp2);
            for(int i=0; i<argCount;i++){
                char * temp = new char[arguments[commNum][i].length()+1];
                strcpy(temp,arguments[commNum][i].c_str());
                strcat(args[0]," ");
                strcat(args[0],arguments[commNum][i].c_str());
                args.push_back(temp);
            }
            args.push_back(NULL);
            execvp(commands[commNum].c_str(), args.data());
            exit(1);
        }
    }
}
void rightpipe(int fd[], string command, int commNum)
{
    pid_t pid;
    int argCount = 0;
    pid = fork();
    if (pid == -1)
    {
        string err = "Could not fork.\n";
        write(1,err.c_str(),err.length());
    }
    else if (pid == 0)
    {
        if (dup2(fd[0],0) == -1)
            write(1,"Dupe error", 10);
        //close(fd[0]);
        close(fd[1]);
        if (command.compare("ls") == 0)
        {
            if(arguments[commNum][0]=="")
                fileStat(".");
            else
                fileStat(arguments[commNum][0]);
            exit(1);
        }
        else
        {
            for (int i = 0; i < wholecommand.length(); i++)
            {
                if (arguments[commNum][i] != "")
                    argCount++;
                else
                    break;
            }
            vector<char *> args;
            char *temp2 = new char[commands[commNum].length()+1];
            strcpy(temp2, commands[commNum].c_str());
            args.push_back(temp2);
            for(int i=0; i<argCount;i++){
                char * temp = new char[arguments[commNum][i].length()+1];
                strcpy(temp,arguments[commNum][i].c_str());
                strcat(args[0]," ");
                strcat(args[0],arguments[commNum][i].c_str());
                args.push_back(temp);
            }
            args.push_back(NULL);
            execvp(commands[commNum].c_str(), args.data());
            exit(1);
        }
    }
}


void ls(){
    int leftpipeCom = 0;
    int rightpipeCom = 0;
    int count = 0;
    int fd[2];
    execParser();
    pid_t pid;
    int status, out, in;
    
    //write(1,commands[0].c_str(),commands[0].length());
    
    if(pipes[0][0]=="" && leftChevron[0][0] == "" && rightChevron[0][0]=="" 
            && arguments[0][0] == ""){ //no |,<,> or args
        fileStat(".");
    }
    else if(pipes[0][0]=="" && leftChevron[0][0] == "" && rightChevron[0][0]=="" ) //no |,<,> 
        fileStat(arguments[0][0]);

    else if(pipes[0][0]!="" && leftChevron[0][0] == "" && rightChevron[0][0]=="" ) //one |, no <,>
    {
        for(int i = 0; i < pipesNum+1; i++)
        {
            pipe(fd);
            for (int j = 0; j < commandNum; j++)
            {
                if (pipes[i][0].compare(commands[j]) == 0) // maps left pipe  to the command
                {
                    leftpipeCom = j;
                    count = j;
                    break;
                }
            }
            for (int j = count+1; j < commandNum; j++) // maps right of pipe to the next command
            {
                if (pipes[i][1].compare(commands[j]) == 0)
                {
                    rightpipeCom = j;
                    break;
                }
            }
            leftpipe(fd,pipes[i][0],leftpipeCom);
            rightpipe(fd,pipes[i][1],rightpipeCom);
            
        }
        close(fd[0]);
        close(fd[1]);
        for(int i = 0; i <pipesNum+1; i++){
            wait(NULL);
            wait(NULL);
        }
    }
    
    else if(pipes[0][0]!="" && leftChevron[0][0] == ""){
        out = open(rightChevron[0][1].c_str(), 
                O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(out,1);
       // close(out);
        if(arguments[0][0]=="")
            fileStat(".");
        else
            fileStat(arguments[0][0]);
    }
}

void cd(){
    char buffer[BUFFER_SIZE];
    string directory,pathname;
    for(int i=0; i< wholecommand.length(); i++){
	if(wholecommand[i]=='c' && wholecommand[i+1]=='d'){
            for(int j = i+3; j< wholecommand.length();j++){
                if(isprint(wholecommand[j])!= 0 && isblank(wholecommand[j])== 0){
                    directory += wholecommand[j];
                }
            }
        }
    }
    wholecommand="";
    struct stat dir;
    pathname = thepath+'/'+directory;
    if(directory.length() == 0) // no dir specified changing to HOME
    {
        const char *name = "HOME";
        char *value;
        value = getenv(name);
        chdir(value);
    }
    else if(directory.compare("..") == 0){
        chdir("..");
    }
    else if(directory.compare(".") ==0){
        return;
    }
    else{
        if(stat(pathname.c_str(), &dir) == 0 && S_ISDIR(dir.st_mode))
        {
            chdir(pathname.c_str());
        }
        else
        {
            string error = "Error changing directory.\n";
            write(1,error.c_str(),error.length());
        }
    }
}

void external()
{
    int argCount=0;
    execParser();
    if (wholecommand=="")
        return;
    if (pipes[0][0]=="" && leftChevron[0][0] == "" && rightChevron[0][0]=="") //no pipes, or chevrons
    {
        for (int i = 0; i < wholecommand.length(); i++)
        {
            if (arguments[0][i] != "")
                argCount++;
            else
                break;
        }
        
        vector<char *> args;
        char *temp2 = new char[commands[0].length()+1];
        strcpy(temp2, commands[0].c_str());
        args.push_back(temp2);
        for(int i=0; i<argCount;i++){
            char * temp = new char[arguments[0][i].length()+1];
            strcpy(temp,arguments[0][i].c_str());
            strcat(args[0]," ");
            strcat(args[0],arguments[0][i].c_str());
            args.push_back(temp);
        }
        args.push_back(NULL);
      
        pid_t pid = fork();
        if (pid == 0){ // child's PID
            execvp(commands[0].c_str(), args.data());
            exit(1);
        }
        else{
            //int waitstat = 0;
            wait(NULL);
            return;
        }
    }
    
}


int main (int argc, char *argv[])
{
    int commandstatus = 0;
    while(commandstatus != 1)
    {
        wholecommand ="";
        command_line_interface();
        commandstatus = noncanonmode();
        switch(commandstatus){
            case 3: 
                cd();
                break;
            case 2: 
                ls();
                break;
            case 5: 
                external();
                break;
                
        }
        commandNum = 0; 
        pipesNum = 0;
        leftcNum = 0;
        rightcNum = 0;
        argNum = 0;
    }   
        return 0;
}






