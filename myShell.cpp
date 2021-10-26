#include "myShell.h"
using namespace std;

int main (){
    //init variables
    vector<int> bgProcesses;
    vector<string> prevDirs;
    vector<string> spacedVector;
    vector<string> pipedVector;
    vector<string> signVector;
    string currentArgument;
    dup2(0,1);
    
    while (true){
        dup2(1,0);

        //this will check for completion of the bg processes
        //do it before user prmopt shows up
        for(int i = 0; i < bgProcesses.size(); i++)
        {
            if(waitpid(bgProcesses.at(i), 0, WNOHANG) == bgProcesses.at(i)) //if bg process is done
            {
                cout << "Completed process [" << bgProcesses.at(i) << "]..." << endl;

                bgProcesses.erase(bgProcesses.begin() + i);
                i-=1;
            }
        }

        userPrompt(); //outprint the command prompt first

        string inputline;
        getline (cin, inputline);   // get a line from standard input
        removeWhitespace(inputline); //remove extra white space so everything is more uniform

        bool isBackground = false;
        if(inputline.find('&') != string::npos)
        {
            inputline.erase(remove(inputline.begin(), inputline.end(), '&'), inputline.end());
            isBackground = true;
        }

        //set the piped vector...will split the string by pipes ('|')
        //now we have all the commands we will want to run separate
       // signVector = splitBySign(inputline);
        pipedVector = splitByPipe(inputline);
        
        if (inputline == string("exit")){
            cout << "Terminating shell..." << endl;
            break;
        }

        
        for(int i = 0; i < pipedVector.size(); i++)
        {  
            int fd[2];
            pipe(fd); 

            currentArgument = pipedVector[i];
            
            int pid = fork ();
            if (pid == 0) //child process
            { 
                IORedirection(currentArgument); //check for io redirection, execute if needed
                changeDirectory(prevDirs, currentArgument); //check for cd, execute if needed
               
                if(i < pipedVector.size() - 1)
                {
                    dup2(fd[1], 1);
                    close(fd[1]);
                }

                vector<string> inputVector;
                
                if(currentArgument.find("awk") != string::npos)
                    inputVector = splitForAwk(currentArgument);//special split function since awk was being dum
                else
                    inputVector = splitBySpace(currentArgument); //split line into a vector of strings, split by space. default split

                char** args = stringToChar(inputVector);

                execvp (args [0], args);
            }
            else
            {
                    if(!isBackground)
                    {
                        if(i == pipedVector.size() - 1)
                        {
                            waitpid(pid, 0, 0);
                        }
                        dup2(fd[0], 0);
                    }
                    else
                    {
                        bgProcesses.push_back(pid);
                        cout << "Started process [" << pid << "]..." << endl;
                    }
                    close(fd[1]);
            }
        }
    }
}