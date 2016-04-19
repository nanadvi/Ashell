#define BUFFER_SIZE 1024
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <iostream>
#include <vector>

using namespace std;



void findFile(string NameOftheFile, string directory){
	string pathName;
	string cwdPath;
	string path;
	char cwd[BUFFER_SIZE];
	vector <string> fileNames;
	vector <string> dirnames;

	DIR *dir;
	struct dirent *file;
	struct stat dirinfo;
	dir = opendir(directory.c_str());
	if(dir)
	{
		while((file = readdir(dir)) != NULL)
		{
			// if(directory == ".")
				path = file->d_name;
			// else 
			// 	path = directory+ "/" +file->d_name;
			 stat(path.c_str(),&dirinfo);
			if( strcmp(file->d_name,".") == 0 || strcmp(file->d_name,"..") == 0)
			 	continue;

			if (S_ISDIR(dirinfo.st_mode))
				dirnames.push_back(file->d_name);
			 else
				fileNames.push_back(file->d_name);
		}
		closedir(dir);
		for(unsigned int i = 0 ; i < fileNames.size() ; i++)
		{

			if(fileNames[i].compare(NameOftheFile) == 0)
			{
				pathName += getcwd (cwd, 256);
				pathName += "/" ;
				pathName += fileNames[i];
				cout << pathName << endl;
				break;	

			}
		}
			
	}
	else{
		fprintf(stderr, "Cannot read directory '%s': \n", cwdPath.c_str());
		perror("");
	}
	for(unsigned int j = 0; j < dirnames.size(); j++){
		pathName = "";
		findFile(NameOftheFile,dirnames[j]);
	}
}

int main(int argc, char *argv[]){
	if(argc > 2){
		string NameOftheFile(argv[1]);
		string directory(argv[2]);
		findFile(NameOftheFile,directory);
	}
	else{
		string NameOftheFile(argv[1]);
		findFile(NameOftheFile,".");
	}

	return 0;
}