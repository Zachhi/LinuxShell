#include <stdio.h>
#include<iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdint>
#include <time.h>
#include <sstream>
#include <algorithm>
using namespace std;

//prints out the user: date time year $: for the command prompt
void userPrompt() 
{
        string user = getenv("USER"); //fetch user
        time_t currTime = time(0); //fetch time
        string currDateTime = ctime(&currTime); //formats the time
        currDateTime.insert(20, "CDT "); //insert CDT after the timestamp and before the year

        cout << user << ": "<<currDateTime.substr(0,currDateTime.length()-1) << "$ "; //print it
}

//remove all whitespace .... //'    d  f  ' -> 'df'
void removeAllWhitespace(string &arguments)
{
    string newArguments;
    for(int i = 0; i < arguments.size(); i++)
    {
        if(arguments.at(i) != ' ')
            newArguments+=arguments.at(i);
    }
    arguments = newArguments;
}

//remove extra whitespace... 'ps   aux  > a' -> 'ps aux > a'
void removeWhitespace(string &arguments)
{
    string newArguments;
    bool spaceFound = false;

    //loop through string. Every time there is a first ecounter with a space after a letter, add it to the new string
    //add every word seen to the string
    //ignore every other space that isn't the first ecounter after a letter

    int numSingleQuote = 0;
    int numDoubleQuote = 0;

    for(int i = 0; i < arguments.length(); i++)
    {
        if(arguments.at(i)=='\'')
            numSingleQuote += 1;
        else if(arguments.at(i)=='\"')
            numDoubleQuote += 1;

        if(arguments.at(i)==' ' && !spaceFound)
        {
            newArguments += ' '; //add space if no space has been found
            spaceFound = true; //space found is true
        }
        else if(arguments.at(i) == ' ' && (numSingleQuote%2==1 || numDoubleQuote%2==1))
        {
            newArguments += ' ';
        }
        else if (arguments.at(i)!=' ')
        {
            newArguments += arguments.at(i); //add the letter
            spaceFound = false; //space found is now false
        }

    }

    //trim the leading and trailing spaces
    if(newArguments.at(0)==' ')
        newArguments = newArguments.substr(1, newArguments.length());
    if(newArguments.at(newArguments.length()-1)==' ')
        newArguments = newArguments.substr(0, newArguments.length()-1);

    //and now set the arguments to the correct value
    arguments = newArguments;
}

//split string into vec of strings by space
vector<string> splitBySpace(string &arguments)
{
    removeWhitespace(arguments);

    //if arguments = "ps aux >; a"
    //then vector = {"ps", "aux", ">", "a"}
    vector<string> argumentVector;
    string tempStr;
    int startingIndex = 0;
    int length = 0;
    int numSingleQuote = 0;
    int numDoubleQuote = 0;

    for(int i = 0; i < arguments.length(); i++)
    {
        length += 1;
        if((arguments.at(i) == ' ' || i == arguments.length()-1))
        {
            if(i == arguments.length()-1)
                tempStr = arguments.substr(startingIndex, length);
            else
                tempStr = arguments.substr(startingIndex, length-1);
            
            tempStr.erase(remove(tempStr.begin(), tempStr.end(), '\''), tempStr.end());
            tempStr.erase(remove(tempStr.begin(), tempStr.end(), '\"'), tempStr.end());
            argumentVector.push_back(tempStr);
            startingIndex = i+1;
            length = 0;
        }
    }

    for(int i = 0; i < argumentVector.size(); i++)
       // cout << "spacedArg: " << argumentVector[i] << endl;

    return argumentVector;
}

//help to split for awk
vector<string> splitForAwk(string &arguments)
{
    removeWhitespace(arguments);

    //if arguments = "ps aux >; a"
    //then vector = {"ps", "aux", ">", "a"}
    vector<string> argumentVector;
    string tempStr;
    int startingIndex = 0;
    int length = 0;
    int numSingleQuote = 0;
    int numDoubleQuote = 0;

    for(int i = 0; i < arguments.length(); i++)
    {
        if(arguments.at(i) == '\'')
            numSingleQuote += 1;
        else if(arguments.at(i) == '\"')
            numDoubleQuote += 1;

        length += 1;

        if((arguments.at(i) == ' ' || i == arguments.length()-1) && (numSingleQuote%2 ==0  && numDoubleQuote%2 == 0))
        {
            if(i == arguments.length()-1)
            {
                tempStr = arguments.substr(startingIndex);
            }
            else
            {
                tempStr = arguments.substr(startingIndex, length-1);
            }
            tempStr.erase(remove(tempStr.begin(), tempStr.end(), '\''), tempStr.end());
            tempStr.erase(remove(tempStr.begin(), tempStr.end(), '\"'), tempStr.end());
            tempStr.erase(remove(tempStr.begin(), tempStr.end(), ' '), tempStr.end());
            argumentVector.push_back(tempStr);
            startingIndex = i+1;
            length = 0;
        }
    }

    vector <string> awkVector;

    for(int i = 0; i < argumentVector.size(); i++)
       // cout << "spacedArg: " << argumentVector[i] << endl;

    return argumentVector;
}

//split string into vec of strings by pipe ('|')
vector<string> splitByPipe(string &arguments)
{
    removeWhitespace(arguments);

    vector <string> argumentVector;
    string tempStr;
    int startingIndex = 0;
    int length = 0;
    int numSingleQuote = 0;
    int numDoubleQuote = 0;

    for(int i = 0; i < arguments.length(); i++)
    {
        if(arguments.at(i) == '\'')
            numSingleQuote += 1;
        else if(arguments.at(i) == '\"')
            numDoubleQuote += 1;
        length += 1;
        if((arguments.at(i)=='|') && (numSingleQuote%2 ==0  && numDoubleQuote%2 == 0))
        {
            if(i == arguments.length()-1)
                tempStr = arguments.substr(startingIndex);
            else
                tempStr = arguments.substr(startingIndex, length-1);

            startingIndex = i+1;
            length = 0; 
            argumentVector.push_back(tempStr);
        }
        else if((i == arguments.length()-1) && (numSingleQuote%2 ==0  && numDoubleQuote%2 == 0))
        {
            tempStr = arguments.substr(startingIndex);
            argumentVector.push_back(tempStr);
        }
             
    }

    return argumentVector;
}

//split string into vec of strings by sign 
vector<string> splitBySign(string &arguments)
{
    removeWhitespace(arguments);

    vector <string> argumentVector;
    string tempStr;
    int startingIndex = 0;
    int length = 0;

    for(int i = 0; i < arguments.length(); i++)
    {
        length += 1;
        if(arguments.at(i)=='|')
        {
            if(i == arguments.length()-1)
                tempStr = arguments.substr(startingIndex);
            else
                tempStr = arguments.substr(startingIndex, length-1);

            startingIndex = i+1;
            length = 0; 
            argumentVector.push_back(tempStr);
        }
        else if(i == arguments.length()-1)
        {
            tempStr = arguments.substr(startingIndex);
            argumentVector.push_back(tempStr);
        }
             
    }

    for(int i = 0; i < argumentVector.size(); i++)
        //cout << "pipeArg: " << argumentVector[i] << endl;

    return argumentVector;
}

//change string vec to chars
char** stringToChar(vector<string> arguments)
{   
    char ** converted = new char * [arguments.size() + 1]; //create the char arr pointer with the same size of arguments, but add 1 for null character at end
    converted[arguments.size()] = NULL;                 //set the last index to nullptr 

    //loop through vector<string> and convert and store in converted
    for(int i = 0; i < arguments.size(); i++)
        converted[i] = (char*) arguments[i].c_str();

    return converted;
}

//help with I/O redirection
void IORedirection(string &arguments)
{
            int fd1;
            string fileName1, fileName2;
            int index1 = -1;
            int index2 = -1;

            int numSingle = 0;
            int numDouble = 0;
            for(int i = 0; i < arguments.size(); i++)
            {
                if(arguments.at(i) == '\'')
                    numSingle += 1;
                else if(arguments.at(i) == '\"')
                    numDouble += 1;

                if((arguments.at(i)=='<') && (numSingle%2 ==0  && numDouble%2 == 0))
                    index1 = i;
                else if((arguments.at(i)=='>') && (numSingle%2 ==0  && numDouble%2 == 0))
                    index2 = i;
            }

            //case of both > and <, like grep /init < a > b
            if(index2 > 0 && index1 > 0)
            {
                fileName1 = arguments.substr(index1 + 1, (index2 - index1 - 1));
                fileName2 = arguments.substr(index2 + 1);

                removeAllWhitespace(fileName1);
                removeAllWhitespace(fileName2);

                arguments = arguments.substr(0, index1);

                fd1 = open(fileName1.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
                dup2(fd1, 0);
                close(fd1);
                fd1 = open(fileName2.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
                dup2(fd1, 1); 
                close(fd1);  
            }
            //case of >, like ps aux > a
            else if(index2 > 0)
            {
                fileName1 = arguments.substr(index2 + 1);
                removeAllWhitespace(fileName1);
                arguments = arguments.substr(0, index2); 
                
                fd1 = open(fileName1.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
                dup2(fd1, 1);
                close(fd1);  
            }
            //case of <, like grep /init < a
            else if(index1 > 0)
            {
                fileName1 = arguments.substr(index1 + 1);
                removeAllWhitespace(fileName1);
                arguments = arguments.substr(0, index1);

                fd1 = open(fileName1.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
                dup2(fd1, 0);
                close(fd1);        
            }
}

//help with cd
void changeDirectory(vector<string>& previousDir, string nextDir)
{
    
    if(nextDir.substr(0,2) == "cd")
    {
        if(nextDir.length() > 3)
            nextDir = nextDir.substr(3);

        if(nextDir != "-")
        {
            char currDir[256];
            getcwd(currDir, 256);

            char* nextDirMod = (char*) nextDir.c_str();
            chdir(nextDirMod);

            string currDirString = currDir;
            previousDir.push_back(currDirString); 
        }
        else
        {   
            if(previousDir.size() > 0)
            {
                string back = previousDir.back();
                char* nextDirMod = (char*)back.c_str();
                chdir(nextDirMod);
                previousDir.pop_back();
            }
            else
                cout << "Failed...no previous directory" << endl;
        }
    }
    
}

//single pipe