#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

struct person{
    
    char gender[50];
    char name[50];
    char surname[50];

};

struct person * handle_database(char * fname, int *line_num){
    
    FILE * f_data;
    char buffer[BUFFER_SIZE];
    char  *word;
    
    f_data = fopen(fname, "r");
    
    // counting the number of lines in file
    while (fgets(buffer, BUFFER_SIZE, f_data) != NULL) {
        
        (*line_num)++;
    }
    
    rewind(f_data);
    // allocate memory for people
    struct person* person_list = malloc((*line_num) * sizeof(struct person)); 
    
    int i = 0;
    while (fgets(buffer, BUFFER_SIZE, f_data) != NULL) {
    
        word = strtok(buffer, " ");
        if(strcmp(word, "m") == 0){
            strcpy(person_list[i].gender, "Mr.");
        }
        else{ // if gender = f
            strcpy(person_list[i].gender, "Ms.");
        }
        
        word = strtok(NULL, " ");
        strcpy(person_list[i].name, word);
        
        word = strtok(NULL, "\n");
        strcpy(person_list[i].surname, word);
        
        i++;
    }
    
    fclose(f_data);
    return person_list;
}

void correct_txt(char* txt_file, struct person * person_list, int line_num){
    
    FILE * fp;
    fp = fopen(txt_file, "r+");
    char w[50];
    
    for(int i =0; i < line_num; i++){
    
        while (fscanf(fp, "%s", w) == 1) {
            int pos = 0;
            if(strcmp(w, person_list[i].name) == 0){ // finding name
                // going back to Mr. or Ms.
                pos = -(strlen(person_list[i].name)+strlen(person_list[i].gender)+1);
                
                fseek(fp, pos , SEEK_CUR);
                
                fputs(person_list[i].gender, fp); // change Mr. or Ms
                
                pos = strlen(person_list[i].name)+1; // move to name
                fseek(fp, pos, SEEK_CUR);
                fscanf(fp, "%s", w); // get surname
                if(strcmp(w, person_list[i].surname) != 0){ // change surname if not correct
                    // go back to starting position of surname
                    pos = -strlen(person_list[i].surname);
                    fseek(fp, pos, SEEK_CUR);
                    // change surname
                    fputs(person_list[i].surname, fp);
                    
                }
            }
        }
        
        rewind(fp);
    }
    
    
    fclose(fp);
    
}

void corrector(char * dirname, struct person* person_list, int line_num){
 
    DIR *root = opendir(dirname);
    assert(root != NULL);
    struct dirent *d;
    chdir(dirname);
    while ((d = readdir(root)) != NULL) {
        
        if(d->d_type == DT_REG){ // if it is a file
            
            //printf("This is a regular file\n");
            
            if((strstr(d->d_name, ".txt") != NULL)){
                
                if((strcmp(d->d_name, "database.txt") != 0) || (strcmp(dirname, ".") != 0) ){ // if file is not database.txt file in the root
                
                    // correct the txt file
                    correct_txt(d->d_name, person_list, line_num);
                    
                }
    
            }
             
             
             // file content update 
            
        }
        else if((d->d_type == DT_DIR) && (strcmp(d->d_name,".") != 0) && (strcmp(d->d_name,"..") != 0)) { // if it is directory
        
            // recursive call
            
            corrector(d->d_name, person_list, line_num);
                 
        }
    }
    chdir("..");
    closedir(root);
}

int main(int argc, char *argv[]) {
    int line_num = 0;
    struct person *person_list = handle_database("database.txt", &line_num);
    
    corrector(".", person_list, line_num);
    return 0;
    
}
