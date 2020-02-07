#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char** argv) {

//This is where I set the initial PATH
setenv("PATH","/bin",1);

char* str = NULL;
size_t len = 5000;
char* args[10000];
char* args2[10000];
int i = 0;
int j = 0;
int index = 0;
pid_t pid = getpid();
pid_t const parent = getpid();
int ampersand = 0; //basically a boolean check for an &
int status;
FILE *fp;
char* filename;
int standardout = dup(1);
int fdout;
char colon[] = ":";

//This is the error message
char error_message[30] = "An error has occurred\n";

if(argc == 2)
	fp = fopen(argv[1],"r"); //Open the file in bath mode
else{
	fp = stdin; //Start in interactive mode
	printf("dash> "); //Display shell prompt
}

while(pid == parent){			
while(getline(&str, &len, fp) != -1){ //Gets a line from either stdin (interactive) or the file (batch)
char temp[len]; //Temporary array
memset(args,0,sizeof(args)); //Clears argument array
strcpy(temp,str); //Copy the line into a temp char array (for strtok)
str = strtok(temp,"\n"); //Gets rid of the end of line character at the end
if(strcmp(temp,"\n") != 0)
	strcpy(str,temp); //Copies the temp array into str pointer
i = 0;
str = strtok(temp," "); //Gets the first token out of the line
while(str != NULL){ //This loop handles the rest of the tokens
args[i] = str;
str = strtok(NULL," ");
i++;
}
do{ //This do while loop handles parallel commands
index = 0;
i = 0;
ampersand = 0;
j = 0;
memset(args2,0,sizeof(args2));
while(args[i] != NULL) {
if(strcmp(args[i],"&") == 0){ //Checks for the & sign, if present, ampersand = 1 and a fork will happen
	index = i;
	ampersand = 1;
	break;
	}
else
	i++;
}

if(strcmp(args[index],"&") == 0) { //Takes every word after the & and places them in another array of arguments
	args[index] = NULL;
	while(args[index+1] != NULL){
	args2[j] = args[index+1];
	args[index+1] = NULL; //Clears the array so only one command is run
	index++;
	j++;
	}
	if( (pid = fork()) == 0){ //Forks the process, and copies the commands+args to the args array
		i = 0;
		while(args2[i] != NULL) {
		args[i] = args2[i];
		i++;
		}
	}
	else
		while(wait(&status) != pid);

}
}while(ampersand == 1);// end of do while loop

if(strcmp(args[0],"cd") == 0){ //handles the cd command
	if(args[1] != NULL && args[2] == NULL)
		chdir(args[1]);
	else
		write(STDERR_FILENO, error_message, strlen(error_message));
}

if(strcmp(args[0],"path") == 0){ //Takes every argument after path, combines them with ":" in between, and sets the PATH
			   	 //environment variable to the resultant string
	i = 1;
	if (args[2] == NULL){
		setenv("PATH",args[1],1);
	}
	else
	{
	while (args[i+1] != NULL) {
		char *tempstr = (char *) malloc(1000 * sizeof(args[1]+1));
		strncpy(tempstr,args[1],100);
		args[1] = (char *) malloc(sizeof(args[1]+1));
		args[1] = tempstr;
		strcat(args[1],colon);
		strcat(args[1],args[i+1]);
		i++;
		}
		setenv("PATH",args[1],1);
	}
}

if(strcmp(args[0],"exit") == 0) //If command is exit, exits
	exit(0);

index = 0;
i = 0;
while(args[i] != NULL){ //This while loop checks for the ">" char and returns the index of the argument array it is at
if(strcmp(args[i],">") == 0){
	index = i;
	break;
	}
i++;
}
filename = NULL;
if(strcmp(args[index],">") == 0) { //Takes the file name and attempts to open the file, if successful, uses dup2 to take over stdout
	filename = args[index+1];
	args[index] = NULL;
	args[index+1] = NULL;
	if ((fdout = open(filename, O_CREAT|O_WRONLY , 0777 )) < 0) {
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	dup2(fdout, 1); 
	close(fdout);
}

if( strcmp(args[0],"exit") != 0)  { //Only executes commands if it doesn't match one of the builtins
	if( strcmp(args[0],"cd") != 0 ) {
		if( strcmp(args[0],"path") != 0 ) {
			if( (pid = fork()) == 0 ) {
				execvp(args[0],args); //This executes the commands,  assuming the path is set correctly
					
			}
			else
				while(wait(&status) != pid); //Waits for the child process to finish
} } }

//This closes the dup2 thing
if( filename != NULL ) {
	dup2(standardout, 1);
	close(standardout);
	filename = NULL;
}

if(argc != 2){ 
	if(getpid() != parent) //Ensures a child process ends once a command finishes
		exit(0);
	
	printf("dash> "); //Display the shell prompt
}

}//end getline's while

}//end pid=parent's while
return(0);
}
