#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <wait.h>

/// FOR REDIRECTION
int fd1;
int fd2;

///	///	///	/// read the line

bool is_empty(char *s){
	int len = strlen(s);
	for(int i = 0; i < len; i++){
		if(!isspace(s[i]))
			return false;
	}	
	return true;
}

bool is_comment(char *s){
	int len = strlen(s);
	int i = 0;
	for(; i < len; i++){
		if(!isspace(s[i]))
			break;
	}
	return i < len && s[i] == '#';
}

#define MAX_TOKENS 128
char* tokens[MAX_TOKENS];
int token_count = 0;

// returns number of tokens
int tokenize(char* line){
	int len = strlen(line);
	token_count = 0; // the length of table tokens
	int i = 0; 	// idx for going through the line

	while(i < len){
		char* cur_token = malloc(sizeof(char) * 1024);
		int cur_token_len = 0; // the lenth of the current token

		char ch = line[i];
		if(ch == '"'){
			i++;
			while(line[i] != '"'){
				ch = line[i];
				cur_token[cur_token_len ++] = ch;
				i++;
			}
			cur_token[cur_token_len ++] = '\0';
			cur_token_len = 0;
			tokens[token_count] = malloc(sizeof(char)*strlen(cur_token));
			strcpy(tokens[token_count], cur_token);
			token_count ++;
			i++;
		} else {
			while(i < len && !isspace(line[i])){
				ch = line[i];
				cur_token[cur_token_len ++] = ch;
				i++;
			}

			if(cur_token_len != 0){
				cur_token[cur_token_len ++] = '\0';
				cur_token_len = 0;
				tokens[token_count] = malloc(sizeof(char)*strlen(cur_token));
				strcpy(tokens[token_count], cur_token);
				token_count ++;
			}

			i++;
		}
		free(cur_token);
	}

	return token_count;
}

///	///	///	/// simple built-in commands
int cur_status = 0;
char name_shell[64]  = {'m', 'y', 's', 'h', '\0'};

void name(){
	if(token_count == 1){
		printf("%s\n", name_shell);
	} else {
		strcpy(name_shell, tokens[1]);
	}
}

void help(){
	printf("Preprosti vgrajeni ukazi:\n");
	printf("name ime - nastavi ime lupine, ce imena ne podamo, izpise ime lupine (privzeto ime je mysh)\nhelp - izpise spisek podprtih ukazov,\nstatus - izpise izhodni status zadnjega (v ospredju) izvedenega ukaza");
	printf("exit status - konca lupino s podanim izhodnim statusom,\nprint args... - izpise podane argumente na standardni izhod (brez koncnega skoka v novo vrstico),\necho args... - kot print, le da izpise se skok v novo vrstico,\n");
    printf("pid - izpise pid procesa,\nppid - izpise pid starsa.\n");
    printf("\n\n\n");
    printf("Vgrajeni ukazi za delo z imeniki:\n");
    printf("dirchange imenik - zamenjava trenutnega delovnega imenika, ce imenika ne podamo, skoci na /,\ndirwhere - izpis trenutnega delovnega imenika,\ndirmake imenik - ustvarjanje podanega imenika,\ndirremove imenik - brisanje podanega imenika,\n");
    printf("dirlist imenik - preprost izpis vsebine imenika (le imena datotek, locena z dvema presledkoma), ce imena ne podamo, se privzame trenutni delovni imenik.\n");
}

void status(){
	printf("%d\n", cur_status);
}

void e_xit(){
	int status = atoi(tokens[1]);
	exit(status);
}

void print(){
	for(int i = 1; i < token_count; i++){
		if(i != token_count - 1)
			printf("%s ", tokens[i]);
		else printf("%s", tokens[i]);
	}
	cur_status = 0;
}

void echo(){
	print();
	printf("\n");
	cur_status = 0;
}

void pid(){
	printf("%d\n", getpid());
	cur_status = 0;
}

void ppid(){
	printf("%d\n", getppid());
	cur_status = 0;
}


/// /// /// /// built-in commands for working with directories
void dirchange(){
	cur_status = 0;
	if(token_count == 1){
		// chdir("getenv("HOME")");
		chdir("/");
	} else if(chdir(tokens[1]) != 0){
		cur_status = errno;
		perror("dirchange");
	}
}

void dirwhere(){
	cur_status = 0;
	char cur_path[1024];
	getcwd(cur_path, 1024);
	printf("%s\n", cur_path);
}

void dirmake(){
	cur_status = 0;
	if(mkdir(tokens[1], S_IRWXU) != 0){
		cur_status = errno;
		perror("dirmake");
	}
}

void dirremove(){
	cur_status = 0;
	if(rmdir(tokens[1]) != 0){
		cur_status = errno;
		perror("dirremove");
	}
}

void dirlist(){
	cur_status = 0;	
	char path[1024];
	if(token_count == 1){
		getcwd(path, 1024);
	} else {
		strcpy(path, tokens[1]);
	}	

	DIR *dir;
    struct dirent *content_dir;

    dir = opendir(path);
    if(dir == NULL){
        cur_status = errno;
    	perror("dirlist");
    } else {
    	content_dir = readdir(dir);
	 	while(content_dir != NULL){ 
    	    printf("%s  ", content_dir->d_name);
        	content_dir = readdir(dir);
    	}
    	printf("\n");
    	closedir(dir);
    }
}

/// /// /// /// other built-in commands for working with directories
void linkhard(){
	cur_status = 0;
	if(link(tokens[1], tokens[2]) == -1){
		cur_status = errno;
		perror("linkhard");
	}
} 

void linksoft(){
	cur_status = 0;
	if(symlink(tokens[1], tokens[2]) == -1){
		cur_status = errno;
		perror("linksoft");
	}
}

void linkread(){
	cur_status = 0;
	char value_symlink [128];
	size_t len = 128;
	int s = readlink(tokens[1], value_symlink, len);
	if(s == -1){
		cur_status = errno;
		perror("linkread");
	} else {
		value_symlink[s] = '\0';
		printf("%s\n", value_symlink);
	}
}

void linklist(){
	cur_status = 0;
	struct stat s;
	DIR *dir = opendir(".");
    struct dirent *content;

    long wanted;

    if(stat(tokens[1], &s) == 0){
    	wanted = s.st_ino;
   			
    	content = readdir(dir);
	 	while(content != NULL){ 
	 		stat(content->d_name, &s);
	 		if(s.st_ino == wanted)
	    	    printf("%s  ", content->d_name);
        	content = readdir(dir);
    	}
    	printf("\n");
    	closedir(dir);
    }
    else cur_status = errno;
   
}

void u_nlink(){
	cur_status = 0;
	if(unlink(tokens[1]) != 0){
		cur_status = errno;
		perror("unlink");
	}
}

void r_ename(){
	cur_status = 0;
	if(rename(tokens[1], tokens[2]) != 0){
		cur_status = errno;
		perror("rename");
	}
}

void cpcat(){
	cur_status = 0;
	int input_file;
	int output_file;
	if(token_count == 1){
		input_file = 0;
		output_file = 1;
	} else if(strcmp(tokens[1], "-") == 0)
		input_file = 0;
    else input_file = open(tokens[1], O_RDONLY);
    
    if(input_file < 0){
    	cur_status = errno;
    	perror("cpcat");
    } else {
    	if(token_count <= 2)
    		output_file = 1;
    	else if(strcmp(tokens[2], "-") == 0)
    		output_file = 1;
	    else output_file = open(tokens[2], O_CREAT | O_TRUNC | O_WRONLY, 0644);
    	
    	if(output_file < 0){
    		cur_status = errno;
    		perror("cpcat");
    		close(input_file);
    	} else {
    		char inp; 
 		   	while(true){
    			if(read(input_file, &inp, 1) == 0)
 	        	   break;
    			else write(output_file, &inp, 1);
    		}
    		if(input_file != 0)
		    	close(input_file);
		    if(output_file != 1)
		    	close(output_file);
    	}
    }
}

void cpcat2(char** tokens2, int num_tokens){
	cur_status = 0;
	int input_file;
	int output_file;
	if(num_tokens == 1){
		input_file = 0;
		output_file = 1;
	} else if(strcmp(tokens2[1], "-") == 0)
		input_file = 0;
    else input_file = open(tokens2[1], O_RDONLY);
    
    if(input_file < 0){
    	cur_status = errno;
    	perror("cpcat");
    } else {
    	if(num_tokens <= 2)
    		output_file = 1;
    	else if(strcmp(tokens2[2], "-") == 0)
    		output_file = 1;
	    else output_file = open(tokens2[2], O_CREAT | O_TRUNC | O_WRONLY, 0644);
    	
    	if(output_file < 0){
    		cur_status = errno;
    		perror("cpcat");
    		close(input_file);
    	} else {
    		char inp; 
 		   	while(true){
    			if(read(input_file, &inp, 1) == 0)
 	        	   break;
    			else write(output_file, &inp, 1);
    		}
    		if(input_file != 0)
		    	close(input_file);
		    if(output_file != 1)
		    	close(output_file);
    	}
    }
}


/// /// /// /// Pipeline

void pipes(){ 
	int fdsc1[2];
	int fdsc2[2];
	bool opened_first = false;
	for(int i = 1; i < token_count; i++){
		// PREPARING ARRAY OF STRINGS FOR EXEC
		int num_words = 0;
		for(int ii = 0; ii < strlen(tokens[i]); ii++){
			if(tokens[i][ii] == ' ')
				num_words ++;
		}

		//number of spaces is for 1 smaller than number of words and we need one more place for NULL for execvp
		num_words += 2; 	
		char* tokens_commands[64]; 

		char* token;
		char* rest = tokens[i];

		int j = 0;
		while((token = strtok_r(rest, " ", &rest))){
			tokens_commands[j] = malloc(sizeof(char)*strlen(token));
			strcpy(tokens_commands[j], token);
        	j++;
		}

		tokens_commands[j] = NULL;

		/// ///	///	
		if(i == 1){
			fflush(stdout);
			pipe(fdsc1);
			opened_first = true;
	    	if (!fork()) {
	    		fflush(stdout);
    	    	dup2(fdsc1[1], 1);
        		close(fdsc1[0]);
        		close(fdsc1[1]);
        		if(strcmp(tokens_commands[0], "cpcat") == 0){
        			cpcat2(tokens_commands, num_words - 1);
        			exit(0);
        		} else execvp(tokens_commands[0], tokens_commands);
    		}
		} else if (i != (token_count - 1)){
			if(opened_first){
			opened_first = false;
			pipe(fdsc2);
	    	if(!fork()) {
    	    	dup2(fdsc1[0], 0);
        		dup2(fdsc2[1], 1);
     	   		close(fdsc1[0]);
        		close(fdsc1[1]);
        		close(fdsc2[0]);
        		close(fdsc2[1]);
        		if(strcmp(tokens_commands[0], "cpcat") == 0){
        			cpcat2(tokens_commands, num_words - 1);
        			exit(0);        		
        		} else execvp(tokens_commands[0], tokens_commands);
    		}
    		close(fdsc1[0]);
    		close(fdsc1[1]);
    		} else {
    		opened_first = true;
    		pipe(fdsc1);
    		if(!fork()) {
    	    	dup2(fdsc2[0], 0);
        		dup2(fdsc1[1], 1);
     	   		close(fdsc2[0]);
        		close(fdsc2[1]);
        		close(fdsc1[0]);
        		close(fdsc1[1]);
        		if(strcmp(tokens_commands[0], "cpcat") == 0){
        			cpcat2(tokens_commands, num_words - 1);
        			exit(0);
        		}  else execvp(tokens_commands[0], tokens_commands);
    		}
    		close(fdsc2[0]);
    		close(fdsc2[1]);	
    		}
		} else {
			if(!opened_first){ 
			if(!fork()) {
				fflush(stdout);
        		dup2(fdsc2[0], 0);
       			close(fdsc2[0]);
        		close(fdsc2[1]);
        		if(strcmp(tokens_commands[0], "cpcat") == 0){
        			cpcat2(tokens_commands, num_words - 1);
       				exit(0);
        		} else execvp(tokens_commands[0], tokens_commands);
    		}
 		   	close(fdsc2[0]);
    		close(fdsc2[1]);
    	    } else {
    	    if(!fork()) {
				fflush(stdout);
        		dup2(fdsc1[0], 0);
       			close(fdsc1[0]);
        		close(fdsc1[1]);
        		if(strcmp(tokens_commands[0], "cpcat") == 0){
        			cpcat2(tokens_commands, num_words - 1);
        			exit(0);
        		} else execvp(tokens_commands[0], tokens_commands);
    		}
    		close(fdsc1[0]);
    		close(fdsc1[1]);		
    	    }
		}
	}

	for(int i = 1; i < token_count; i++)
		wait(NULL);
}

int main(int argc, char* argv[])
{

    while(true){
    	if(isatty(1))
    		printf("%s> ", name_shell);

    	bool child = false;

    	char* line = malloc(1024*sizeof(char));
		size_t len = 0;
		int end = getline(&line, &len, stdin);

		// checking whether we reached end
		if(end == -1)
			break;

		// checking whether its a line of spaces
		if(is_empty(line))
			continue;
	
		// checking whether its a comment
		if(is_comment(line))
			continue;

		token_count = tokenize(line);	

		// RUNNING IN THE BACKGROUND
		bool run_in_background = false;
		if(strcmp(tokens[token_count - 1], "&") == 0){
			run_in_background = true;

			token_count --;
			int pid = fork();
    		if(pid != 0)
    		//	exit(0);
    		//	waitpid(pid, &cur_status, '\0');
	    		continue;
	    	else{
	    		child = true;
	    	}
		}

		// REDIRECTIONING 
		char redir_file_in[128];
		char redir_file_out[128];

		bool redir_in = false;
		bool redir_out = false;	

		int stdin_copy = dup(0);
		int stdout_copy = dup(1);

		fflush(stdout);
		// REDIRECTIONING OUT
		if(tokens[token_count - 1][0] == '>'){
			strcpy(redir_file_out, tokens[token_count - 1] + 1);

			token_count --;
			redir_out = true;

			fd2 = open(redir_file_out, O_CREAT | O_TRUNC | O_WRONLY, 0644);
			dup2(fd2, 1);
		} 	

		// REDIRECTIONING IN
		if(tokens[token_count - 1][0] == '<'){
			strcpy(redir_file_in, tokens[token_count - 1] + 1);

			token_count --;
			redir_in = true;

			fd1 = open(redir_file_in, O_RDONLY);
			dup2(fd1, 0);
		}

		/// /// ///
		if(strcmp(tokens[0], "name") == 0){
			name(); 
		} else if(strcmp(tokens[0], "help") == 0){
			help();
		} else if(strcmp(tokens[0], "status") == 0){
			status();
		} else if(strcmp(tokens[0], "exit") == 0){
			e_xit();
		} else if(strcmp(tokens[0], "print") == 0){
			print();
		} else if(strcmp(tokens[0], "echo") == 0){
			echo();
		} else if(strcmp(tokens[0], "pid") == 0){
			pid();
		} else if(strcmp(tokens[0], "ppid") == 0){
			ppid();
		} else if(strcmp(tokens[0], "dirchange") == 0){
			dirchange();
		} else if(strcmp(tokens[0], "dirwhere") == 0){
			dirwhere();
		} else if(strcmp(tokens[0], "dirmake") == 0){
			dirmake();
		} else if(strcmp(tokens[0], "dirremove") == 0){
			dirremove();
		} else if(strcmp(tokens[0], "dirlist") == 0){
			dirlist();
		} else if(strcmp(tokens[0], "linkhard") == 0){
			linkhard();
		} else if(strcmp(tokens[0], "linksoft") == 0){
			linksoft();
		} else if(strcmp(tokens[0], "linkread") == 0){
			linkread();
		} else if(strcmp(tokens[0], "linklist") == 0){
			linklist();
		} else if(strcmp(tokens[0], "unlink") == 0){
			u_nlink();
		} else if(strcmp(tokens[0], "rename") == 0){
			r_ename();
		} else if(strcmp(tokens[0], "cpcat") == 0){
			cpcat();
		} else if(strcmp(tokens[0], "pipes") == 0){
			pipes();
		} else {
			int pid = fork();
    		if(pid == 0){ 
    			tokens[token_count] = NULL;
				return execvp(tokens[0], tokens);
    		}
        	else if (!run_in_background)
        		waitpid(pid, &cur_status, '\0');
		}
		
		free(line);
		
		fflush(stdout);
		if(redir_in){
			dup2(stdin_copy, 0);
			close(fd1);
		}
		
		if(redir_out){
			dup2(stdout_copy, 1);
			close(fd2);
		}

		if(child)
			break;
    }	

 	exit(0);
}
 
