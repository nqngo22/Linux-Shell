//Nguyen Ngo
//Shyam Pinnipati

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <termios.h>
#include <cctype>
#include <pwd.h>
#include <sstream>
#include <algorithm>

void modeToLetters( int mode, char str[] )
{
  strcpy( str, "----------" );           /* default=no perms */

  if ( S_ISDIR(mode) )  str[0] = 'd';    /* directory?       */

  if ( mode & S_IRUSR ) str[1] = 'r';    /* 3 bits for user  */
  if ( mode & S_IWUSR ) str[2] = 'w';
  if ( mode & S_IXUSR ) str[3] = 'x';

  if ( mode & S_IRGRP ) str[4] = 'r';    /* 3 bits for group */
  if ( mode & S_IWGRP ) str[5] = 'w';
  if ( mode & S_IXGRP ) str[6] = 'x';

  if ( mode & S_IROTH ) str[7] = 'r';    /* 3 bits for other */
  if ( mode & S_IWOTH ) str[8] = 'w';
  if ( mode & S_IXOTH ) str[9] = 'x';
}

void ls(){
  DIR *pdir = NULL;
  struct dirent *pent = NULL;
  struct stat perms;
  pdir = opendir(".");
  while((pent = readdir(pdir))){
    std::string pwd = get_current_dir_name();
    char permis[11]; //read, write, execute permissions
    memset(permis, '-', 10);
    pwd.push_back('/');
    pwd += pent->d_name;
    stat(pwd.c_str(), &perms);
    modeToLetters(perms.st_mode, permis);
    write(STDOUT_FILENO, permis, strlen(permis));
    write(STDOUT_FILENO, " ", 1);
    write(STDOUT_FILENO, pent->d_name, strlen(pent->d_name));
    write(STDOUT_FILENO, "\n", 1);
  }
}

void ls(const char* dir) {
  DIR *pdir = NULL;
  struct dirent *pent = NULL;
  struct stat perms;
  pdir = opendir(dir);
  while((pent = readdir(pdir))){
    std::string pwd = get_current_dir_name();
  	char permis[11]; //read, write, execute permissions
  	memset(permis, '-', 10);
  	//std::cout << permis << std::endl;
  	pwd.push_back('/');
  	pwd+=dir;
  	pwd.push_back('/');
  	pwd += pent->d_name;
  	stat(pwd.c_str(), &perms);
  	modeToLetters(perms.st_mode, permis);
  	write(STDOUT_FILENO, permis, strlen(permis));
  	write(STDOUT_FILENO, " ", 1);
    write(STDOUT_FILENO, pent->d_name, strlen(pent->d_name));
    write(STDOUT_FILENO, "\n", 1);
  }
}

void cd(const char* dir) {
  DIR *pdir = NULL;
  char error[] = "Error changing directory.";

  pdir = opendir(dir);

  if(pdir == NULL){
    write(STDOUT_FILENO, error, 25);
  }

  //change into dir
  chdir(dir);
}

void cd()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	DIR *pdir = NULL;
  char error[] = "Error changing directory.";

  pdir = opendir(homedir);

  if(pdir == NULL){
    write(STDOUT_FILENO, error, 25);
  }

  //change into dir
  chdir(homedir);
}

void pwd() {
  char* working_dir = NULL;
  char newline = 0x0A;
  working_dir = get_current_dir_name();

  //count the length of the current directory path in order to use with write
  int wd_length = 0;
  int i = 0;
  while(working_dir[i] != '\0') {
    wd_length++;
    i++;
  }
  write(STDOUT_FILENO, working_dir, wd_length);
  write(STDOUT_FILENO, &newline, 1);
}

int display_dir() {
  //displays the current working directory at each command line
  //similar to pwd() but w/o newline

  char* working_dir = NULL;
  working_dir = get_current_dir_name();

  //count the length of the current directory path in order to use with write
  int wd_length = 0;
  int i = 0;
  int j = 0;
  while(working_dir[i] != '\0') {
    wd_length++;
    i++;
  }
  if (wd_length>16){
  	for (j = wd_length-1; j >= 0; j--){
  		if(working_dir[j] == '/')
  			break;
  	}
  	std::string machine = "/...";
  	std::string machine2(working_dir);
  	machine.append(machine2.substr(j));

  	write(STDOUT_FILENO, machine.c_str(), machine.length());
    wd_length = (int)(machine.length());
  }
  else
  	write(STDOUT_FILENO, working_dir, wd_length);
  write(STDOUT_FILENO, "> ", 2);
  return wd_length+2;
}

void ResetCanonicalMode(int fd, struct termios *savedattributes){          //noncanmode.c
  tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){         //noncanmode.c
  struct termios TermAttributes;
  //char *name;

  // Make sure stdin is a terminal.
  if(!isatty(fd)){
      fprintf (stderr, "Not a terminal.\n");
      exit(0);
  }

  // Save the terminal attributes so we can restore them later.
  tcgetattr(fd, savedattributes);

  // Set the funny terminal modes.
  tcgetattr (fd, &TermAttributes);
  TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
  TermAttributes.c_cc[VMIN] = 1;
  TermAttributes.c_cc[VTIME] = 0;
  tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

void call_fork(std::vector<std::string> args, std::vector<std::string> history, bool temp_hist_bool) {
  pid_t pid = fork();

  if (pid == -1){
    // error, failed to fork()
    write(STDOUT_FILENO, "Error\n", 6);
    exit(-1);
  } else if (pid > 0){
      if (!temp_hist_bool){
        int status;
        wait(&status);
      }
  } else {
    // we are the child
    if(args[0] == "pwd") {
			pwd();
			exit(0);
    }else if(args[0] == "ls") {
      if(args.size() < 2){
        ls();
        exit(0);
      }else{
        ls(args[1].c_str());
        exit(0);
      }
    }else if(args[0] == "history"){
      for(size_t i = 0; i < history.size(); i++){
        write(STDOUT_FILENO, std::to_string(i).c_str(), strlen(std::to_string(i).c_str()) );
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, history[history.size()-i-1].c_str(), strlen(history[history.size()-i-1].c_str()));
        write(STDOUT_FILENO, "\n", 1);
      }
      exit(0);
    }
  }
}

int pipeFunction(std::vector<std::string> args, int pipes, std::vector<std::string> history, bool temp_hist_bool) {

  const int commands = pipes + 1;
  int pipefds[pipes*2];
  int status;
  int pid;
  std::vector<int> commandStarts;
  std::vector<char *> arguments(args.size()+1);

  //need to use vector<char *> instead of vector<std:: string> for execvp()
  for(int i = 0; i < (int)args.size(); i++) {
    arguments[i] = &args[i][0];
  }

  //create pipes
  for(int i = 0; i < pipes; i++) {
    if(pipe(pipefds + i*2) < 0){
      perror("pipe error");
      exit(EXIT_FAILURE);
    }
  }

  //first command is always at index 0
  commandStarts.push_back(0);
  for(int i = 1; arguments[i] != NULL; i++) {
    if(strcmp(arguments[i], "|") == 0) {
      arguments[i] = NULL;
      commandStarts.push_back(i+1);
    }
  }

  for(int i = 0; i < commands; i++) {
    pid = fork();

    if(pid == 0) {
      //not last command
      if(i < (commands - 1)) {
        if(dup2(pipefds[2*i + 1], STDOUT_FILENO) < 0) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
      }
      //not first command
      if(i != 0) {
        if(dup2(pipefds[2*i -2], STDIN_FILENO) < 0) {
          perror("dup2") ;
          exit(EXIT_FAILURE);
        }
      }

      for(int j = 0; j < pipes * 2; j++) {
        close(pipefds[j]);
      }

      //execute processes
      if(strcmp(arguments[commandStarts[i]], "pwd") == 0){
        pwd();
        exit(0);
      }else if(strcmp(arguments[commandStarts[i]], "ls") == 0){
        if(arguments[commandStarts[i]+1] == NULL){
          ls();
          exit(0);
        }else{
          ls(arguments[commandStarts[i] + 1]);
          exit(0);
        }
      }else if(strcmp(arguments[commandStarts[i]],"history") == 0){
        for(size_t i = 0; i < history.size(); i++)
        {
          write(STDOUT_FILENO, std::to_string(i).c_str(), strlen(std::to_string(i).c_str()) );
          write(STDOUT_FILENO, " ", 1);
          write(STDOUT_FILENO, history[history.size()-i-1].c_str(), strlen(history[history.size()-i-1].c_str()));
          write(STDOUT_FILENO, "\n", 1);
        }
        exit(0);
      }else if( execvp(arguments[commandStarts[i]], arguments.data() + commandStarts[i]) < 0 ){
        perror(*(arguments.data()));
        exit(EXIT_FAILURE);
      }//end pid == 0
    } else if (pid < 0) {
      perror("error");
      exit(EXIT_FAILURE);
      break;
    }
  }

  //last close of pipes
  for(int i = 0; i < 2 * pipes; i++){
    close(pipefds[i]);
  }

  //wait for all child processes
  for(int i = 0; i < pipes + 1; i++){
    wait(&status);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> history;
  int fd = STDIN_FILENO;
  struct termios savedAttribtes;
  int dir_len;
  char RXChar;
  std::string temp_hist;
  int curr_vec = -1;
  char back = 0x7F;

 //set non-canonical mode
  SetNonCanonicalMode(fd, &savedAttribtes);

  while(1){
    //shows the current directory at each command line
    dir_len = display_dir();

    while(1){
      read(fd, &RXChar, 1);
      if(0x0A == RXChar){ // newline
        write(STDOUT_FILENO, "\n", 1);
        break;
      }else if(0x04 == RXChar){
        return 0;
      }else if(0x7F == RXChar){
        if (temp_hist.length() == 0){
          write(STDOUT_FILENO, "\a", 1); //output bell
        }else{
          write(STDOUT_FILENO, &RXChar, 1);
          temp_hist.pop_back();
        }
      }else if(0x1B == RXChar){//first for up/down/delete
        read(fd, &RXChar, 1);
        if (0x5B == RXChar){ //2nd check for up/down/delete
          read(fd, &RXChar, 1);
          if(0x33 == RXChar){ //delete
            read(fd, &RXChar, 1);
            if (0x7E == RXChar){ //delete part 2
              if (temp_hist.length() == 0){
                write(STDOUT_FILENO, "\a", 1); //output bell
              }else{
                write(STDOUT_FILENO, &back, 1);
                temp_hist.pop_back();
              }
            }
          }

          if (0x41 == RXChar){ //up arrow
            if(curr_vec == 9 || (int)(history.size()) == 0 || curr_vec == (int)(history.size())-1){ //out of bounds
              write(STDOUT_FILENO, "\a", 1); //output bell
            }else{
              write(STDOUT_FILENO, "\r", 1); //return to beginning
              for (int i = 0; i < (int)(temp_hist.length())+dir_len; ++i){
                write(STDOUT_FILENO, " ", 1); //blank out what was there before
              }
              write(STDOUT_FILENO, "\r", 1); //return to beginning
              display_dir();
              curr_vec++;
              write(STDOUT_FILENO, history[curr_vec].c_str(), (int)(history[curr_vec].length()));
              temp_hist = history[curr_vec];
            }
          }else if (0x42 == RXChar){ //down arrow
            if(curr_vec == -1 || (int)(history.size()) == 0){ //out of bounds
              write(STDOUT_FILENO, "\a", 1); //output bell
            }
            else if(curr_vec == 0){ //wiping to blank
              write(STDOUT_FILENO, "\r", 1); //return to beginning
              for (int i = 0; i < (int)(temp_hist.length())+dir_len; ++i){
                write(STDOUT_FILENO, " ", 1); //blank out what was there before
              }
              write(STDOUT_FILENO, "\r", 1); //return to beginning
              display_dir();
              temp_hist = "";
              curr_vec --;
            }else{
              write(STDOUT_FILENO, "\r", 1); //return to beginning
              for (int i = 0; i < (int)(temp_hist.length()) + dir_len; ++i){
                write(STDOUT_FILENO, " ", 1); //blank out what was there before
              }
              write(STDOUT_FILENO, "\r", 1);
              display_dir();
              curr_vec--;
              write(STDOUT_FILENO, history[curr_vec].c_str(), (int)(history[curr_vec].length()));
              temp_hist = history[curr_vec];

            }
          }
        }
      }else{
        write(STDOUT_FILENO, &RXChar, 1);
        temp_hist.push_back(RXChar);
      }
    }

    if (temp_hist == "")
      continue;

    if(history.size() == 10)
      history.pop_back();

    history.insert(history.begin(), temp_hist); //load last entry into vector
    curr_vec = -1; //reset curr_vec

    bool temp_hist_bool;

    if(temp_hist.substr(temp_hist.size()- 1) == "&"){
      temp_hist_bool = true;
      temp_hist.pop_back();
    }else{
      temp_hist_bool = false;
    }

    std::stringstream ss(temp_hist);
    std::string item;
    std::vector<std::string> args;
    while (std::getline(ss, item, ' ')) {
      args.push_back(item);
    }
    temp_hist = ""; //reset temp_hist

    //check if command contains piping
    int pipecount = std::count(args.begin(), args.end(), "|");

    if(pipecount >0 || std::count(args.begin(), args.end(), "<") > 0 || std::count(args.begin(), args.end(), ">") > 0){
      pipeFunction(args, pipecount, history, temp_hist_bool);
      continue;
    }else if(args[0] == "pwd" || args[0] == "ls" || args[0] == "history") {
      call_fork(args, history, temp_hist_bool);
    }else if(args[0] == "cd") {
      if(args.size() > 1)
        cd(args[1].c_str());
      else
      	cd();
    }else if(args[0] == "exit") {
      break;
    }else{
      std::string command = "";
      command.append(args[0]);
      std::vector<char *> arguments(args.size() + 1);
      for(int i = 0; i < (int)args.size(); i++) {
        arguments[i] = &args[i][0];
      }

      //pid_t parent = getpid();
      pid_t pid = fork();

      if (pid == -1)
      {
          // error, failed to fork()
        write(STDOUT_FILENO, "Error\n", 6);
        exit(-1);
      }else if (pid > 0){
      	if (!temp_hist_bool)
      	{
      		int status;
        	wait(&status);
      	}
      }else{
        // we are the child
        execvp(command.c_str(), arguments.data());
        std::string error = "Failed to execute ";
        error += args[0] + "\n";
        //std::cout << error << std::endl;
        write(STDOUT_FILENO, error.c_str(), error.length());
        exit(-1);
      }
    }
  }
  ResetCanonicalMode(fd, &savedAttribtes);
  return 0;
}
