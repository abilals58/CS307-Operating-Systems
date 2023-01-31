
// Ahmet Bilal Yildiz
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<sys/types.h>
#include <thread>
#include <mutex>
#include <cstdio>

using namespace std;

void print_to_console(int* fd, mutex& m){
    
    
    FILE* stream = fdopen(fd[0], "r");

    char l[2];
    m.lock(); // lock the console 
    
    cout << "---- "<< this_thread::get_id() << endl; // print tid
    while(fgets(&l[0], 2, stream)){
        
        cout << l;
        
    }
    
    cout << "---- "<< this_thread::get_id() << endl; // print tid
    fflush(stdout);
    m.unlock(); //unlock the console
    
    //cout << "while loop terminates" << endl;
    //m.unlock();
}

int main()
{
    
    ifstream file("commands.txt"); // open the commands.txt file
    ofstream ofile("parse.txt"); // open the output file
    vector<vector<string>> parse;
    if(!file.is_open()){ // give error if file cannot be opened
        
        cout << "File cannot be opened" << endl;
    }
    else{
        string line;
        while (getline(file, line)){ // reading the lines of the files
        
            vector<string> line_parse;
            istringstream ss(line);
            string command, w2, file_name, inputs, options, redirection = "-", backround_job = "n";
            
            ss >> command;
            while(ss >> w2){ // checking the next  word
            
                if(w2[0] == '-'){ 
                    options = w2;
                }
                else if(w2[0] == '<' || w2[0] == '>'){
                    
                    redirection = w2;
                    ss >> file_name; // get the file_name which is not important
                 
                }
                else if(w2[0] == '&'){
                    
                    backround_job = "y";
                }
                else {
                    
                    inputs = w2;
                    
                }
            }
            
            line_parse.push_back(command);    
            line_parse.push_back(inputs); 
            line_parse.push_back(options); 
            line_parse.push_back(redirection); 
            line_parse.push_back(file_name); 
            line_parse.push_back(backround_job);
            
            parse.push_back(line_parse);
            
        }
        
        for(int i = 0; i < parse.size(); i++){
            ofile << "----------" << endl;
            ofile<< "Command: " << parse[i][0] << endl;
            ofile << "Inputs: " << parse[i][1] << endl;
            ofile << "Options: " << parse[i][2] << endl;
            ofile << "Redirection: " << parse[i][3] << endl;
            ofile << "Background Job: " << parse[i][5] << endl;
            ofile << "----------" << endl;
         }
    }
    
    vector<int> background_processes;
    vector<thread> thread_list;
    mutex m;
    
    vector<int*> pipe_list(parse.size());
    for(int i = 0; i < parse.size(); i++){
        
        if(parse[i][3] != ">"){
            
            pipe(pipe_list[i]); // declare
        }
    }
    
    
    for(int i = 0; i < parse.size(); i++){ // line by line reading the command file
    
        if(parse[i][0] != "wait" && parse[i][3] == ">"){ // hanlding the output redirector operator
            
             int c1 = fork(); // creating a child process called command
             
             if(c1 == 0){ // child process
                 
                vector<char*> args_vec; // forming a vector and pushback the arguments
                for(int j =0; j < 3; j++){
                    
                    if(parse[i][j] != ""){
                        
                        args_vec.push_back(parse[i][j].data());
                    }
                    
                }
                
                args_vec.push_back(NULL); // adding NULL at the end
                
                char** args = args_vec.data(); // convert vector to array in order to use in execvp
                
                // execution and redirection
                close(STDOUT_FILENO);
                open(parse[i][4].data(), O_WRONLY | O_CREAT, 0666);
                
                execvp(args[0], args); // execution 
                
                 
             }
             else if (c1 < 0){ // if error occurs exit
                 
                 exit(1);
             }
             else{ // shell process goes on 
                 
                 if(parse[i][5] == "n"){ // if it is not a background process shell waits for this process to terminate
                     
                     waitpid(c1, NULL, 0);
                 }
                 else{ // if the operator is a background operator, shell add this process id to the list
                     
                     background_processes.push_back(c1);
                 }
                 
                 if(i == parse.size()-1){ // before terminates shell process joins all the background threads // and all the background processes
                    
                    
                    for(int b = 0; b < background_processes.size(); b++){
                        
                        waitpid(background_processes[b], NULL, 0);
                    }
                    background_processes.clear();
                    
                    for(int p =0; p < thread_list.size(); p++){
                        
                        thread_list[p].join();
                        
                    }
                    thread_list.clear(); // delete all the waited background processes
                }
                 
             }
        }
    
        else if(parse[i][0] != "wait"){ // if there is not an output operator
            
            int* fd = new int[2];
            pipe(fd);
            
            int c = fork(); // creating a child process called command
    
            if(c == 0) { // child process goes on 
                    
                if(parse[i][3] == "-"){ // if there is no redirection or background operator
                    
                    // just add all the list to the array and execute
                    
                    vector<char*> args_vec; // forming a vector and pushback the arguments 
                    
                    for(int j =0; j < 3; j++){
                        
                        if(parse[i][j] != ""){
                            
                            args_vec.push_back(parse[i][j].data());
                        }
                        
                    }
                    args_vec.push_back(NULL); // pushback NULL at the end
                    
                    char** args = args_vec.data(); // convert vector to array in order to use in execvp
                    dup2(fd[1], STDOUT_FILENO); // writing output to the write end of the pipe
                    execvp(args[0], args); // execution 
                }
                
                else if (parse[i][3] != "-"){ // if there is redirectioning
                    
                    // redirect input or output file direction to the file.txt instead of console
                    vector<char*> args_vec; // forming a vector and pushback the arguments
                    for(int j =0; j < 3; j++){
                        
                        if(parse[i][j] != ""){
                            
                            args_vec.push_back(parse[i][j].data());
                        }
                        
                    }
                    
                    args_vec.push_back(NULL); // adding NULL at the end
                    
                    char** args = args_vec.data(); // convert vector to array in order to use in execvp
                    
                    // input redirection and writing the output to the pipe
                    close(STDIN_FILENO);
                    open(parse[i][4].data(), O_RDONLY);
                    
                    dup2(fd[1], STDOUT_FILENO); // writing output to the write end of the pipe
                    execvp(args[0], args); // execution   
                    
                }
                
            }
            else if (c < 0){ // if an error occurs
                
                exit(1); 
            }
            else{ // shell process goes on 
            
                if(parse[i][5] != "y"){ // if the job is not a background job, join the thread
                
                    close(fd[1]); // close the write end of the pipe
                    thread t1(print_to_console, fd, ref(m)); // forming the listener thread
                    
                    t1.join();
                    
                }
                else{ //if the job is a background job, shell process do not wait to child process
                
                    close(fd[1]); // close the write end of the pipe
                    thread_list.push_back(thread(print_to_console, fd, ref(m))); // when the wait command comes, all these threads will be joined 
                    
                }
                
                if(i == parse.size()-1){ // before terminates shell process joins all the background threads // and all the background processes
                    
                    for(int b = 0; b < background_processes.size(); b++){
                        
                        waitpid(background_processes[b], NULL, 0);
                    }
                    background_processes.clear();
                    
                    for(int p =0; p < thread_list.size(); p++){
                        
                        thread_list[p].join();
                        
                    }
                    thread_list.clear(); // delete all the waited background processes
                }
            }
        }
        else { // if the command is wait, shell process waits all the background processes
            
            for(int b = 0; b < background_processes.size(); b++){
                        
                waitpid(background_processes[b], NULL, 0);
            }
            background_processes.clear();
            
             for(int p =0; p < thread_list.size(); p++){
                        
                thread_list[p].join();
            }
            
            thread_list.clear(); // delete all the waited background processes
        }
        
    }
    return 0;
}

// what is & ?
// How can we deal with wait ?
// how can we provide concurrency
















