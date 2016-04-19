
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

void findFile(string file, string dirname);
void printWorkingDir();
void execParser();

string history[10];
int historySelector = 0; //use this number as index when the arrow keys are hit
int historySize = 0; // every time ENTER is pressed historySize increases and commands will be put in the array
string wholecommand;
string thepath;
string **arguments;
string *commands;
int commandNum = 0; 
int pipesNum = 0;
int cNum = 0;
int argNum = 0;
string ** pipes;
//string ** leftChevron;
string ** chevron;
char * chevronT;
int redirCount = 0;


void execParser(){
    bool leftPipe = false;
    bool leftOfChev= false;
    commands = new string[wholecommand.length()];
    pipes = new string *[wholecommand.length()];
    arguments = new string *[wholecommand.length()];
    chevron = new string *[wholecommand.length()];
    chevronT = new char[wholecommand.length()];
    for (int i=0; i < wholecommand.length(); i++)
    {
        commands[i] = "";
        pipes[i] = new string[2];              //resizing arrays 
        chevron[i] = new string[2];
        arguments[i] = new string[wholecommand.length()];
        for (int j=0; j<2; j++)
        {
            
            pipes[i][j] = "";
            chevron[i][j] = "";
            
        }
        for (int j=0; j<wholecommand.length(); j++)
            arguments[i][j] = "";

    }
    for(int i=0; i< wholecommand.length(); i++){
        argNum = 0;
        if(isspace(wholecommand[i])==0){
            for(int j = i; j<wholecommand.length(); j++){
                if(isspace(wholecommand[j])== 0 && wholecommand[j] != 92 && wholecommand[j] != '|' && wholecommand[j] != '<' && wholecommand[j] != '>'){
                    commands[commandNum] += wholecommand[j];
                }
                else if(wholecommand[j] == 92){
                    commands[commandNum] += wholecommand[j+1];
                    j++;
                }
                else if(wholecommand[j] == '|' || wholecommand[j] == '<' || wholecommand[j] == '>' || wholecommand[j] == ' ')
                {
                    i = j;
                    break;
                }  
                i=j;
            }
            commandNum++;
            if (wholecommand[i] != '>' && wholecommand[i] != '<' && wholecommand[i] != '|')
                i++;
            for(int j = i; j<wholecommand.length(); j++){
                if(isspace(wholecommand[j])==0 && wholecommand[j]!='>' && wholecommand[j]!='<' && wholecommand[j]!='|' && wholecommand[j]!='\n' && wholecommand[j] != 92){
                    arguments[commandNum-1][argNum] += wholecommand[j];
                    
                }
                else if(wholecommand[j] == 92){
                    arguments[commandNum-1][argNum] += wholecommand[j+1];
                    j++;
                }
                else if(wholecommand[j]=='|'){
                    if(leftOfChev){
                        chevron[cNum][1] = commands[commandNum-1]; // puts grep
                        cNum++;
                        leftOfChev = false;
                        commands[commandNum-1] = "";
                        commandNum--;
                    }
                    if(!leftPipe){
                        leftPipe = true;
                        pipes[pipesNum][0] = commands[commandNum-1]; //putting cat
                    }
                    else if(leftPipe){
                        pipes[pipesNum][1] = commands[commandNum-1]; // puts grep
                        pipesNum++;
                        pipes[pipesNum][0] = commands[commandNum-1];
                    }
                    i=j;
                    break;
                    
                }
                else if(wholecommand[j]=='>' || wholecommand[j] == '<'){
                    if(!leftOfChev){
                        leftOfChev = true;
                        if (wholecommand[j]=='>')
                            chevronT[cNum] = 'r';
                        else
                            chevronT[cNum] = 'l';
                        chevron[cNum][0] = commands[commandNum-1]; //putting cat
                    }
                    else if(leftOfChev){
                        chevron[cNum][1] = commands[commandNum-1]; // puts grep
                        cNum++;
                        if (wholecommand[j]=='>')
                            chevronT[cNum] = 'r';
                        else
                            chevronT[cNum] = 'l';
                        chevron[cNum][0] = commands[commandNum-1];
                        commands[commandNum-1] = "";
                        commandNum--;
                    }
                    if(leftPipe){
                        pipes[pipesNum][1] = commands[commandNum-1]; // puts grep
                        pipesNum++;
                        leftPipe = false;
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
                    //if (leftleftChev)
                    //    leftChevron[leftcNum][1] = commands[commandNum-1]; // puts grep
                    if (leftOfChev)
                    {
                        chevron[cNum][1] = commands[commandNum-1];
                        commands[commandNum-1] = ""; //erasing this file "command"
                        commandNum--;                       
                    }
                }
            }    
        }
    }
    if (leftPipe)
        pipes[pipesNum][1] = commands[commandNum-1];
    if (leftOfChev)
    {
        chevron[cNum][1] = commands[commandNum-1];
        commands[commandNum-1] = ""; //erasing this file "command"
        commandNum--;
    }
    
    if (pipes[pipesNum][0] =="")
        pipesNum--;
//    for (int i = 0; i < wholecommand.length(); i++)
//    {
//        cNum = 0;
//        if (chevron[0][0]== "")
//        {
//            break;
//        }
//        if (chevron[i][0] !="")
//        {
//            cNum++;
//        }
//        else
//        {
//            cNum--;
//            break;
//        }
//    }
    
    
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
                    if(wholecommand[i+2] == ' ' || wholecommand[i+2] == '>' || wholecommand[i+2] == '<'  || wholecommand[i+2] == '|'){
                        for(int j = 0; j<i; j++){
                            if(wholecommand[j] != ' ' || wholecommand[j] != '>' || wholecommand[j] != '<'  || wholecommand[j] != '|' ){
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
                    if (wholecommand[i+2] == ' ' || wholecommand[i+2] == '>' || wholecommand[i+2] == '<'  || wholecommand[i+2] == '|'){
                        for(int j = 0; j<i; j++){
                            if(wholecommand[j] != ' ' || wholecommand[j] != '>' || wholecommand[j] != '<'  || wholecommand[j] != '|'){
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
                    if(wholecommand[i+4] == ' ' || wholecommand[i+4] == '>' || wholecommand[i+4] == '<'  || wholecommand[i+4] == '|'){
                        for(int j = 0; j<i; j++){
                            if(wholecommand[j] != ' ' || wholecommand[j] != '>' || wholecommand[j] != '<'  || wholecommand[j] != '|'){
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
        
        else if(wholecommand[i] =='p')
        {
            validCommand = true;
            if(wholecommand[i+1] == 'w' && wholecommand[i+2] == 'd'){
                if(i+3 <= wholecommand.length()-1){
                    if(wholecommand[i+3] == ' ' || wholecommand[i+3] == '>' || wholecommand[i+3] == '<'  || wholecommand[i+3] == '|'){
                        for(int j = 0; j<i; j++){
                            if(wholecommand[j] != ' ' || wholecommand[j] != '>' || wholecommand[j] != '<'  || wholecommand[j] != '|'){
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
                command = "pwd";
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
    bool lastdown = false;
    bool lastup = false;
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
                        if (historySelector == historySize)
                        {
                            for(int i = 0; i < wholecommand.length(); i++){
                                write(1,backspace.c_str(), backspace.length());
                            }
                            wholecommand = history[historySelector]; 
                            write(1,wholecommand.c_str(),wholecommand.length());
                            lastup = true;
                            lastdown = false;
                            write(1,"\a",1);
                        }
                        else if (!lastdown && !lastup && history[historySelector]!="")
                        {
                            for(int i = 0; i < wholecommand.length(); i++){
                                write(1,backspace.c_str(), backspace.length());
                            }
                            wholecommand = history[historySelector]; 
                            write(1,wholecommand.c_str(),wholecommand.length());
                            lastup = true;
                            lastdown = false;
                            //cerr << "History Selector Position: " << historySelector << endl;
                        }
                        else if (history[historySelector+1]!="")
                        {
//                            cerr << "History Size: " << historySize << endl;
//                            cerr << "History Selector Position before adding: " << historySelector << endl;
                            historySelector++;
                            for(int i = 0; i < wholecommand.length(); i++){
                                write(1,backspace.c_str(), backspace.length());
                            }
                            wholecommand = history[historySelector];
                            write(1,wholecommand.c_str(),wholecommand.length());
                            lastup = true;
                            lastdown = false;
//                            cerr << "History Selector Position after adding: " << historySelector << endl;
                        }
                        else
                            write(1,"\a",1);
                    }
                    if(0x42 == RXChar){ // when the down arrow is hit
                        // try backspace
                        if(historySelector == 0){
                            lastdown = false;
                            lastup = false;
                            if(wholecommand == history[0] && wholecommand.length() != 0)
                            {
                                for(int i = 0; i < wholecommand.length(); i++){
                                    write(1,backspace.c_str(), backspace.length());
                                }
                                wholecommand = "";
                            }
                            else if(wholecommand.length() == 0)
                                write(1,"\a",1);
                            else
                            {
                                 for(int i = 0; i < wholecommand.length(); i++){
                                    write(1,backspace.c_str(), backspace.length());
                                }
                                wholecommand = "";	
                            }
                        }
                        else
                        {
                            historySelector--;
                            for(int i = 0; i < wholecommand.length(); i++){
                               write(1,backspace.c_str(), backspace.length());
                            }
                            wholecommand = history[historySelector];
                            write(1,wholecommand.c_str(),wholecommand.length());
                            lastup = false;
                            lastdown = true;
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
                lastup = false;
                lastdown = false;
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
                else if (command.compare("ff") == 0)
                    commandcode = 6;
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
    char cwd[PATH_MAX];
    string cwdPath;
    thepath = getcwd (cwd, 256); 
    int substr = thepath.find_last_of('/');
    if (substr > 16)
        cwdPath = "/..."+thepath.substr(substr)+"% ";
    else
    cwdPath = thepath+"% ";
    write(1, cwdPath.c_str(), cwdPath.length());
    
}

// Reference 3: http://www.johnloomis.org/ece537/notes/Files/Examples/ls2.html
//http://stackoverflow.com/questions/3828192/checking-if-a-directory-exists-in-unix-system-call
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
    if (dup2(fd[1],1) == -1)
        write(1,"Dupe error in left pipe", 23);
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
    else if(command.compare("pwd") == 0)
    {
        printWorkingDir();
        exit(1);
    }
    else if(command.compare("ff")==0){
        if(arguments[commNum][1]=="")
        {
            

            findFile(arguments[commNum][0],".");
            chdir(thepath.c_str());
            exit(1);
        }
        else
        {
            if (chdir(arguments[commNum][1].c_str()) != -1)
            {
            findFile(arguments[commNum][0], arguments[commNum][1]);
            chdir(thepath.c_str());
            exit(1);
            }
            else
            {
                string temp = "Failed to open directory ";
                temp += "\"";
                temp += arguments[commNum][1];
                temp += "\"";
                temp += "/\n";
                write(1,temp.c_str(), temp.length());
                exit(1);
            }
        }
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
        if (execvp(commands[commNum].c_str(), args.data()) == -1) // piazza
        {
            string err = "Failed to execute ";
            err += commands[commNum];
            err += '\n';
            write(1, err.c_str(),err.length());
        }
        exit(1);
    }
}

void rightpipe(int fd[], string command, int commNum)
{
    pid_t pid;
    int argCount = 0;
    if (dup2(fd[0],0) == -1)  
        write(1,"Dupe error in right pipe", 24);
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
    else if(command.compare("pwd") == 0)
    {
        printWorkingDir();
        exit(1);
    }
    else if(command.compare("ff")==0){
        if(arguments[commNum][1]=="")
        {
            findFile(arguments[commNum][0],".");
            chdir(thepath.c_str());
            exit(1);
        }
        else
        {
            if (chdir(arguments[commNum][1].c_str()) != -1)
            {
            findFile(arguments[commNum][0], arguments[commNum][1]);
            chdir(thepath.c_str());
            exit(1);
            }
            else
            {
                string temp = "Failed to open directory ";
                temp += "\"";
                temp += arguments[commNum][1];
                temp += "\"";
                temp += "/\n";
                write(1,temp.c_str(), temp.length());
                exit(1);
            }
        }
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
        if(execvp(commands[commNum].c_str(), args.data())== -1)
        {
            string err = "Failed to execute ";
            err += commands[commNum];
            err += '\n';
            write(1, err.c_str(),err.length());
        }
        exit(1);
    }
}

void middlepipe(int oldfd[], int curfd[], string command, int commNum)
{
    pid_t pid;
    int argCount = 0;
    if (dup2(oldfd[0],0) == -1)
        write(1,"Dupe error in middle pipe", 25);

    close(oldfd[1]);

    if (dup2(curfd[1],1) == -1)
        write(1,"Dupe error in middle pipe", 25);
    //    close(curfd[0]);

    if (command.compare("ls") == 0)
    {
        if(arguments[commNum][0]=="")
            fileStat(".");
        else
            fileStat(arguments[commNum][0]);
        exit(1);
    }
    else if(command.compare("pwd")==0)
    {
        printWorkingDir();
        exit(1);
    }
    else if(command.compare("ff")==0){
        if(arguments[commNum][1]=="")
        {
            findFile(arguments[commNum][0],".");
            chdir(thepath.c_str());
            exit(1);
        }
        else
        {
            if (chdir(arguments[commNum][1].c_str()) != -1)
            {
            findFile(arguments[commNum][0], arguments[commNum][1]);
            chdir(thepath.c_str());
            exit(1);
            }
            else
            {
                string temp = "Failed to open directory ";
                temp += "\"";
                temp += arguments[commNum][1];
                temp += "\"";
                temp += "/\n";
                write(1,temp.c_str(), temp.length());
                exit(1);
            }
        }
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
        if(execvp(commands[commNum].c_str(), args.data()) == -1)
        {
            string err = "Failed to execute ";
            err += commands[commNum];
            err += '\n';
            write(1, err.c_str(),err.length());
        }
        exit(1);
    }
}

void performRedirects(int commNum){
    string command = commands[commNum];
    int out,in;
    int argCount = 0;
    pid_t pid;
    bool found = false;
    for (int i = redirCount; i < cNum+1; i++)
    {
        if (chevron[i][0].compare(command) == 0)
        {
            redirCount = i;
            found = true;
            break;
        }   
    }
    if (!found)
        return;
    for (int i = redirCount; i < cNum+1; i++)
    {
        if (chevronT[i]=='r')
        {
            out = open(chevron[i][1].c_str(),O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            if (dup2(out,1) == -1)
                write(1, "Dupe error in right chevron.\n", 29);
            close(out);
        } //if right chevron
        else // left chevron
        {
            if (in = open(chevron[i][1].c_str(), O_RDONLY) != -1)
            {
            if (dup2(in,0) == -1)
                write(1, "Dupe error in left chevron.\n", 28);
            close(in);
            }
            else
            {
                string error = "File \"";
                error += chevron[i][1];
                error += "\" does not exist!\n";
                write(1, error.c_str(), error.length());
            }
        } //if left chevron
        if (chevron[i][1].compare(chevron[i+1][0])==0)
            continue;
        else
        {
            redirCount = i+1;
            break;
        }
    } //end for  loop
}

void ls(){
    int pipeCom = 0;
    int rightpipeCom = 0;
    int fd[pipesNum+1][2];
    int fdsize = 0;
    execParser();
    pid_t pid;

    int save_in, save_out;
    
    if(pipes[0][0]=="" && chevron[0][0]=="" 
            && arguments[0][0] == ""){ //no |,<,> or args
        fileStat(".");
    }
    else if(pipes[0][0]=="" && chevron[0][0]=="" ) //no |,<,> 
        fileStat(arguments[0][0]);

    else if(pipes[0][0]!="") //pipes to do
    {
        save_in = dup(STDIN_FILENO); //save stdin for restoration later
        save_out = dup(STDOUT_FILENO); //save stdout for restoration later
        for (int i = 0; i < pipesNum+1; i++) //generate pipes
        {
            pipe(fd[i]);
            fdsize++;
        }

        for (int i = 0; i < pipesNum+2; i++) //for each child
        {
            pid = fork();
            if (pid == 0) //child
            {
                //performRedirects()//do redirects
                if (i == 0){ //first child, the very most left pipe.
                    performRedirects(0);

                    leftpipe(fd[0],pipes[0][0],0);
                    
                }

                else if (i == pipesNum+1)// the last child, the very most right pipe
                {
                    for (int j = commandNum; j >= 0; j--)
                    {
                        if (pipes[pipesNum][1].compare(commands[j]) == 0)//mapping pipe to command.
                        {
                            rightpipeCom = j;
                            break;
                        }
                    }
                    performRedirects(commandNum-1);
                    rightpipe(fd[fdsize-1],pipes[pipesNum][1],commandNum-1);
                }
                else//middle pipe.
                {
                    performRedirects(pipeCom);                
                    middlepipe(fd[i-1],fd[i],pipes[i][0],i);
                }

            }
            else{
                wait(NULL);
                close(fd[i][1]);
            }
            
        }
        dup2(save_in, STDIN_FILENO); //restore stdin
        dup2(save_out, STDOUT_FILENO); //restore stdout
    }
    else if(pipes[0][0]=="") //no pipes
    {
        pid = fork();
        if (pid == 0)
        {
            performRedirects(0);
            if(arguments[0][0]=="")
                fileStat(".");
            else
                fileStat(arguments[0][0]);
            exit(1);
        }
        else
        {
            wait(NULL);
        }

      
       // close(out);
    }
    

}

void cd(){
    string directory,pathname;
    for(int i=0; i< wholecommand.length(); i++){
	if(wholecommand[i]=='c' && wholecommand[i+1]=='d'){
            for(int j = i+3; j< wholecommand.length();j++){
                if(isprint(wholecommand[j])!= 0 && isblank(wholecommand[j])== 0 && wholecommand[j] != 92){
                    directory += wholecommand[j];
                }
                else if(wholecommand[j] == 92){
                    directory += wholecommand[j+1];
                    j++;
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
    int rightpipeCom = 0;
    int fd[pipesNum+1][2];
    int fdsize = 0;
    if (wholecommand=="")
        return;
    execParser();
    pid_t pid;
    int save_in, save_out;
    int argCount=0;
    
    
    if (pipes[0][0]==""&& chevron[0][0]=="") //no pipes, or chevrons
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
            if(execvp(commands[0].c_str(), args.data()) == -1)
            {
                string err = "Failed to execute ";
                err += commands[0];
                err += '\n';
                write(1, err.c_str(),err.length());
            }
            exit(1);
        }
        else{
            //int waitstat = 0;
            wait(NULL);
            return;
        }
    }
    else if(pipes[0][0]!="") //pipes to do
    {
        save_in = dup(STDIN_FILENO); //save stdin for restoration later
        save_out = dup(STDOUT_FILENO); //save stdout for restoration later
        for (int i = 0; i < pipesNum+1; i++) //generate pipes
        {
            pipe(fd[i]);
            fdsize++;
        }

        for (int i = 0; i < pipesNum+2; i++) //for each child
        {
            pid = fork();
            if (pid == 0) //child
            {
                //performRedirects()//do redirects
                if (i == 0){ //first child, the very most left pipe.
                    performRedirects(0);
                    leftpipe(fd[0],pipes[0][0],0);
                    
                }

                else if (i == pipesNum+1)// the last child, the very most right pipe
                {
                    for (int j = commandNum; j >= 0; j--)
                    {
                        if (pipes[pipesNum][1].compare(commands[j]) == 0)//mapping pipe to command.
                        {
                            rightpipeCom = j;
                            break;
                        }
                    }
//               
                    performRedirects(commandNum-1);
                    rightpipe(fd[fdsize-1],pipes[pipesNum][1],commandNum-1);
                }
                else//middle pipe.
                {
   
                    performRedirects(i);
                    middlepipe(fd[i-1],fd[i],pipes[i][0],i);
                }

            }
            else{
                wait(NULL);
                close(fd[i][1]);
            }
            
        }
        dup2(save_in, STDIN_FILENO); //restore stdin
        dup2(save_out, STDOUT_FILENO); //restore stdout
    }
    else if(pipes[0][0]=="") //no pipes
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
        pid = fork();
        if (pid == 0)
        {
            performRedirects(0);
            if(execvp(commands[0].c_str(), args.data()) == -1)
            {
                string err = "Failed to execute ";
                err += commands[0];
                err += '\n';
                write(1, err.c_str(),err.length());
            }
            exit(1);
            
        }
        else
        {
            wait(NULL);
        }
    }
}

void findFile(string file, string dirname)
{ 
    string pathName;
    DIR *dirptr; //the whole directorys
    struct dirent *eadir; //each directory
    struct stat dirinfo;
    struct stat *dirinfop = &dirinfo;
    int numDir = 0;
    int maxDir = 50;
    char ** directories = new char*[maxDir];
    if ( ( dirptr = opendir(".") ) != NULL )
    {
        while ( ( eadir = readdir(dirptr)) != NULL)
        { //need to stat each dir
            pathName = dirname;
            pathName += "/";
            string curDir(eadir->d_name);
            stat(eadir->d_name, &dirinfo);
            if ( S_ISDIR(dirinfop->st_mode) && strcmp(eadir->d_name,".") != 0 && strcmp(eadir->d_name,"..") != 0 && strcmp(eadir->d_name,"") != 0)
            {
                char * temp = new char[strlen(eadir->d_name)+1];
                strcpy(temp,eadir->d_name);
                directories[numDir] = temp;
                numDir++;
                if (numDir+2 >= maxDir) //resize needed
                {
                    maxDir *= 2;
                    char ** temp = new char*[maxDir];
                    for (int i = 0; i < numDir; i++)
                    {
                        temp[i] = directories[i];
                    }
                    delete(directories);
                    directories = temp;
                }
            }
            if (S_ISREG(dirinfop->st_mode) && strcmp(eadir->d_name,file.c_str()) == 0)
            {

                string temp = pathName;
                temp += file;
                temp += "\n";
                write(1,temp.c_str(),temp.length());
            }
        }
        closedir(dirptr);
        for (int i = 0; i < numDir;i++)//for each directory
        {
            string temp = "./";
            string temp2(directories[i]);
            temp += temp2;
            if (chdir(temp.c_str()) != -1)
                findFile(file,pathName+temp2);
            else
            {
                string temp = "Failed to open directory ";
                temp += "\"";
                temp += temp;
                temp += "\"";
                temp += "/\n";
                write(1,temp.c_str(), temp.length());
            }
        }
    }
    //delete(directories);  
    chdir("..");
    return;
 }

void ff()
{
    int fd[pipesNum+1][2];
    int fdsize = 0;
    execParser();
    pid_t pid;
    int save_in, save_out;
    int rightpipeCom = 0;
    
    if(pipes[0][0]=="" && chevron[0][0]=="" 
            && arguments[0][1] == ""){ //no |,<,> or one arg

        findFile(arguments[0][0],".");
        chdir(thepath.c_str());
        
    }
    else if(pipes[0][0]=="" && chevron[0][0]=="" ){ //no |,<,> 
        if (chdir(arguments[0][1].c_str()) != -1)
        {
        findFile(arguments[0][0], arguments[0][1]);
        chdir(thepath.c_str());
        }
        else
        {
            string temp = "Failed to open directory ";
            temp += "\"";
            temp += arguments[0][1];
            temp += "\"";
            temp += "/\n";
            write(1,temp.c_str(), temp.length());
        }
    }
    else if(pipes[0][0]!="") //pipes to do
    {
        save_in = dup(STDIN_FILENO); //save stdin for restoration later
        save_out = dup(STDOUT_FILENO); //save stdout for restoration later
        for (int i = 0; i < pipesNum+1; i++) //generate pipes
        {
            pipe(fd[i]);
            fdsize++;
        }
        for (int i = 0; i < pipesNum+2; i++) //for each child
        {
            pid = fork();
            if (pid == 0) //child
            {
                //performRedirects()//do redirects
                if (i == 0){ //first child, the very most left pipe.
                    performRedirects(0);
                    leftpipe(fd[0],pipes[0][0],0);
                    
                }

                else if (i == pipesNum+1)// the last child, the very most right pipe
                {
                    for (int j = commandNum; j >= 0; j--)
                    {
                        if (pipes[pipesNum][1].compare(commands[j]) == 0)//mapping pipe to command.
                        {
                            rightpipeCom = j;
                            break;
                        }
                    }
     

                    performRedirects(commandNum-1);                      
                    rightpipe(fd[fdsize-1],pipes[pipesNum][1],commandNum-1);
                }
                else//middle pipe.
                {
                    performRedirects(i);               
                    middlepipe(fd[i-1],fd[i],pipes[i][0],i);
                }

            }
            else{
                close(fd[i][1]);
                //close(fd[i][0]);
                wait(NULL);
            }
            
        }
        dup2(save_in, STDIN_FILENO); //restore stdin
        dup2(save_out, STDOUT_FILENO); //restore stdout
    }
    else if(pipes[0][0]=="") //no pipes
    {
        pid = fork();
        if (pid == 0)
        {
            performRedirects(0);
            if(arguments[0][1]=="")
            {
                findFile(arguments[0][0],".");
                chdir(thepath.c_str());
            }
            else
            {
                if (chdir(arguments[0][1].c_str()) != -1)
                {
                findFile(arguments[0][0], arguments[0][1]);
                chdir(thepath.c_str());
                }
                else
                {
                    string temp = "Failed to open directory ";
                    temp += "\"";
                    temp += arguments[0][1];
                    temp += "\"";
                    temp += "/\n";
                    write(1,temp.c_str(), temp.length());
                }
            }
        exit(1);
        }
        else
        {
            wait(NULL);
        }

      
       // close(out);
    }
}
void printWorkingDir(){
    string temp = thepath;
    temp += "\n";
    write(1,temp.c_str(),temp.length());
}

void pwd(){
    string currPath;
    int pipeCom = 0;
    int fd[pipesNum+1][2];
    int fdsize = 0;
    execParser();
    pid_t pid;
    int rightpipeCom = 0;
    int save_in, save_out;
    
    
    if(pipes[0][0]=="" && chevron[0][0]=="" ) //no |,<,> 
        printWorkingDir();
    else if(pipes[0][0]!="") //pipes to do
    {
        save_in = dup(STDIN_FILENO); //save stdin for restoration later
        save_out = dup(STDOUT_FILENO); //save stdout for restoration later
        for (int i = 0; i < pipesNum+1; i++) //generate pipes
        {
            pipe(fd[i]);
            fdsize++;
        }

        for (int i = 0; i < pipesNum+2; i++) //for each child
        {
            pid = fork();
            if (pid == 0) //child
            {
                //performRedirects()//do redirects
                if (i == 0){ //first child, the very most left pipe.
                    performRedirects(0);

                    leftpipe(fd[0],pipes[0][0],0);
                    
                }

                else if (i == pipesNum+1)// the last child, the very most right pipe
                {
                    for (int j = commandNum; j >= 0; j--)
                    {
                        if (pipes[pipesNum][1].compare(commands[j]) == 0)//mapping pipe to command.
                        {
                            rightpipeCom = j;
                            break;
                        }
                    }
   
                    performRedirects(commandNum-1);               
                    rightpipe(fd[fdsize-1],pipes[pipesNum][1],commandNum-1);
                }
                else//middle pipe.
                {
                    performRedirects(pipeCom);        
                    middlepipe(fd[i-1],fd[i],pipes[i][0],i);
                }

            }
            else{
                wait(NULL);
                close(fd[i][1]);
            }
            
        }
        dup2(save_in, STDIN_FILENO); //restore stdin
        dup2(save_out, STDOUT_FILENO); //restore stdout
    }
    else if(pipes[0][0]=="") //no pipes
    {
        pid = fork();
        if (pid == 0)
        {
            performRedirects(0);
            printWorkingDir();         
            exit(1);
        }
        else
        {
            wait(NULL);
        }

      
       // close(out);
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
            case 6:
                ff();
                break;
            case 4:
                pwd();
                break;
                
        }
        commandNum = 0; 
        pipesNum = 0;
        redirCount = 0;
        cNum = 0;
        argNum = 0;
    }   
        return 0;
}
