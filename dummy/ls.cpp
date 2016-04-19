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
#include <iostream>
#include <cstring>

using namespace std;

//http://www.johnloomis.org/ece537/notes/Files/Examples/ls2.html
void ls(string dirname)
{
	DIR *dirptr; //the whole directory
	struct dirent *eadir; //each directory
	struct stat dirinfo;
	struct stat *dirinfop = &dirinfo;

	if ( ( dirptr = opendir( dirname.c_str() ) ) != NULL )
	{
		while ( ( eadir = readdir(dirptr)) != NULL)
			{ //need to stat each dir
				stat(eadir->d_name, &dirinfo);
				char perm[11];
				strcpy(perm,"----------");
				if ( S_ISDIR(dirinfop->st_mode) )  perm[0] = 'd';
				if ( dirinfop->st_mode & S_IRUSR ) perm[1] = 'r';    /* 3 bits for user  */
				if ( dirinfop->st_mode & S_IWUSR ) perm[2] = 'w';
				if ( dirinfop->st_mode & S_IXUSR ) perm[3] = 'x';

				if ( dirinfop->st_mode & S_IRGRP ) perm[4] = 'r';    /* 3 bits for group */
				if ( dirinfop->st_mode & S_IWGRP ) perm[5] = 'w';
				if ( dirinfop->st_mode & S_IXGRP ) perm[6] = 'x';

				if ( dirinfop->st_mode & S_IROTH ) perm[7] = 'r';    /* 3 bits for other */
				if ( dirinfop->st_mode & S_IWOTH ) perm[8] = 'w';
				if ( dirinfop->st_mode & S_IXOTH ) perm[9] = 'x';
				cout << perm << " " << eadir->d_name << endl;
			}
		closedir(dirptr);
	}
}

int main()
{
	ls(".");
}

