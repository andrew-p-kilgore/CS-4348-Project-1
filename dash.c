#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

void redirect(char* file){
	int fdout;
	if ((fdout = open(file, O_CREAT|O_WRONLY , 0777 )) < 0) {
	perror(file);	/* open failed */
	}
	dup2(fdout, 1); 
	close(fdout);
}

int main(int argc, char** argv) {

//This is where I set the initial PATH
setenv("PATH","/bin",1);

char* str = NULL;
size_t len = 500;
char* args[1000];
char* args2[1000];
int i = 0;
int j = 0;
int index = 0;
pid_t pid;
int status;
FILE *fp;
char* filename;
int standardout = dup(1);
int fdout;

//This is the error message
char error_message[30] = "An error has occurred\n";

if(argc == 2)
	fp = fopen(argv[1],"r");
else{
	fp = stdin;
	printf("dash> "); //Display shell prompt
}


while(getline(&str, &len, fp) != -1){
char temp[len];
memset(args,0,sizeof(args));
strcpy(temp,str); //Copy the line into a temp char array (for strtok)
str = strtok(temp,"\n"); //Gets rid of the end of line character at the end
strcpy(str,temp); //Copies the temp array into str pointer
i = 0;
str = strtok(temp," "); //Gets the first token out of the line
while(str != NULL){ //This loop handles the rest of the tokens
args[i] = str;
str = strtok(NULL," ");
i++;
}

index = 0;
i = 0;

while(args[i] != NULL) {
if(strcmp(args[i],"&") == 0){
	break;
	}
i++;
}
if(strcmp(args[i],"&") == 0) {
	args[i] = NULL;
	while(args[i+1] != NULL){
	args2[j] = args[i+1];
	args[i+1] = NULL;
	i++;
	j++;
	}
	if( (pid = fork()) == 0){
		i = 0;
		while(args2[i] != NULL) {
		args[i] = args2[i];

		}
	}
	else
		while(wait(&status) != pid);

}

if(strcmp(args[0],"cd") == 0){
	if(args[1] != NULL && args[2] == NULL)
		chdir(args[1]);
	else
		write(STDERR_FILENO, error_message, strlen(error_message));
}

if(strcmp(args[0],"path") == 0){
	setenv("PATH",args[1],1);
}

if(strcmp(args[0],"exit") == 0)
	exit(0);

index = 0;
i = 0;
while(args[i] != NULL){
if(strcmp(args[i],">") == 0){
	index = i;
	break;
	}
i++;
}
filename = NULL;
if(strcmp(args[index],">") == 0) {
	filename = args[index+1];
	args[index] = NULL;
	args[index+1] = NULL;
	if ((fdout = open(filename, O_CREAT|O_WRONLY , 0777 )) < 0) {
		perror(filename);	/* open failed */
	}
	dup2(fdout, 1); 
	close(fdout);
}

if( strcmp(args[0],"exit") != 0)  {
	if( strcmp(args[0],"cd") != 0 ) {
		if( strcmp(args[0],"path") != 0 ) {
			if( (pid = fork()) == 0 ) {
				execvp(args[0],args); //This executes the commands,  assuming the path is set correctly
			}
			else
				while(wait(&status) != pid);
} } }

//This closes the dup2 thing
if( filename != NULL ) {
	dup2(standardout, 1);
	close(standardout);
	filename = NULL;
}

if(argc != 2){
	printf("dash> "); //Display the shell prompt
}
}//endwhile

return(0);
}
