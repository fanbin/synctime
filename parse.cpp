#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <string>
#include "sync.h"

using namespace std;



string Parse( char* m, const char *h){
    string head = string(h);
    string message = string(m);
    string content="0";
    int i, j, k;
    i = j = k = 0; // i marks head, j marks ':', k marks ';'
    for (j=0; j<message.size(); j++)
    {
        if (message[j]==':'){
            for (k=j+1; k<message.size(); k++)
            {
                if (message[k]==';')
                {
                    if (message.substr(i,j-i) == head)
                    {
                        content = message.substr(j+1, k-j-1);
                    }
                }
            }
        }
    }
    return content;
};

