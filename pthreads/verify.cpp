#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "helper/hash.h"
using namespace std;
struct block
{
   string blocknumber;
   string minerID;
   string prevhash;
   string nonce;
   string hash;
};
int main () {
   vector<block> blocks;
   fstream newfile;
   string tp;
   newfile.open("outputs/blockchain.txt",ios::in);
   int i=0,size=-1;
   while(getline(newfile, tp))
   {
      const char delim[4] = ":";
      char *token;
      const char *cstr = tp.c_str();
      token = strtok((char*)cstr, delim);
      token = strtok(NULL, delim);
      if(i==0)
      {
         struct block x;
         blocks.push_back(x);
         blocks[++size].blocknumber=token;
      }
      else if(i==1)
         blocks[size].minerID=token;
      else if(i==2)
         blocks[size].prevhash=token;
      else if(i==3)
         blocks[size].nonce=token;
      else if(i==4)
         blocks[size].hash=token;
      i=(i+1)%5;
   }
   newfile.close();
   bool corrupted=false;
   for(i=1;i<=size and not corrupted;i++)
   {
      int mr=0;
      string hash = gethash(stoi(blocks[i-1].nonce), blocks[i-1].prevhash, mr, stoi(blocks[i-1].minerID));
      if (hash.compare(blocks[i].prevhash) != 0)
         cout<<"block "<<i<<" is corrupted\n",corrupted=true;
      else
         cout<<"block "<<i<<" is verified\n";
   }
   return(0);
}
