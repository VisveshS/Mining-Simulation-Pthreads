#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "helper/hash.h"
#include <cmath>
#include <fstream>
#define MAX 1048576.0
bool mineAttempt;// if someone claims to have solved it
int nonce;// solution to puzzle
int n_verify,n_reject;// number of verifivations, rejections
int miner_pubkey;
string prevhash;
int N, block_num;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;// ensure only one thread attempts to solve puzzle at a time
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;// ensure only one thread attempts to solve puzzle at a time

using namespace std;

struct nodeArgs
{
   int index;
   float puzzle_threshold;
};

struct block
{
	unsigned char prevHash[SHA256_DIGEST_LENGTH];
	int minerIndex;
	struct block* next;
} *head;

float RandomFloat(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

/* This ‘sum’ is shared by the thread(s) */
void *runner(void *param);

float fx(float n, int iter=29)
{
   if(iter==0)
      return n;
   float lambda = 3.8;
   float f = lambda*(1-n)*n;
   return fx(f,iter-1);
}

/* threads call this function */
int main(int argc, char *argv[]){
   srand(time(0));
   cout<<"_______________________________\n";
   cout<<"Demo of the chaotic function:\n";
   cout<<"_______________________________\n";
   float x=RandomFloat(0.1,0.9);
   for(float i=0.0;i<0.00001;i+=0.000001)
      cout<<"fx("<<x+i<<") = "<<fx(x+i)<<endl;
   cout<<"_______________________________\n";
   ofstream ofs;
   ofs.open("outputs/blockchain.txt", std::ofstream::out | std::ofstream::trunc);
   ofs.close();
   ofs.open("outputs/useful_information.txt", std::ofstream::out | std::ofstream::trunc);
   ofs.close();
   prevhash = "*";
   /* the thread identifier */
   pthread_attr_t attr;
   /* set of thread attributes */
   if (argc != 3){
      fprintf(stderr,"usage: a.out #miner #block\n");
      return -1;
   }
   if (atoi(argv[1]) < 0){
      fprintf(stderr,"%d must be >= 0\n",atoi(argv[1])); return -1;
   }
   if (atoi(argv[2]) < 0){
      fprintf(stderr,"%d must be >= 0\n",atoi(argv[1])); return -1;
   }
   N = atoi(argv[1]);
   int num_blocks = atoi(argv[2]);
   pthread_t tid[N];
   /* get the default attributes */
   pthread_attr_init(&attr);
   /* create the thread */
   for(int block=0;block<num_blocks;block++)
   {
      mineAttempt=false;
      n_verify=0;
      n_reject=0;
      nonce=-1;
      block_num = block;
      struct nodeArgs *Args[N];
      float puzzle_threshold=RandomFloat(0.2,0.9999);
      for(int i=0;i<N;i++)
      {
      	Args[i] = (struct nodeArgs*)malloc(sizeof(struct nodeArgs));
         Args[i]->index=i;
      	Args[i]->puzzle_threshold=puzzle_threshold;
      }
      // args1->n=atoi((char*)argv[1]);
      for(int i=0;i<N;i++)
		pthread_create(&tid[i],&attr,runner,Args[i]);
      /* wait for the thread to exit */
      for(int i=0;i<N;i++)
      pthread_join(tid[i],NULL);
   }
}

void *runner(void *param){
   int size = 100000;
   float y0 = ((struct nodeArgs*)param)->puzzle_threshold;
   bool solved=false;
   int index=((struct nodeArgs*)param)->index;
   // printf("Index %d\n",index);
   int mr = 0;
   string hash;

   //solving puzzle, only one thread at a time because of mutex
   for(int nonce_ = 0; nonce_ < size and mineAttempt == false; nonce_++)
   {
      hash = gethash(nonce_, prevhash, mr, index);
      float guess = (float)std::stoi(hash.substr(0,5), nullptr, 16)/MAX;
      float f = fx(guess);
      pthread_mutex_lock(&mutex);
      usleep(20000);
		if(f < y0 and not mineAttempt)
		{
    		printf("problem solved by thread %d\n", index);
    		mineAttempt = true;
    		solved = true;
    		nonce = nonce_;
         miner_pubkey = index;
    	}
      pthread_mutex_unlock(&mutex);
   }

   //if yo solved it, wait for enough verifications and add to blockchain
   if(solved)
   {
      while(n_verify+n_reject<N-1);
      if(n_verify>=N/2) 
      {
         printf("block mined by %d\n", index);
         // prevhash = sha256(prevhash + hash);
         ofstream ofs;
         ofs.open("outputs/blockchain.txt", std::ofstream::out|std::ofstream::app);
         ofs << "block number:" << block_num << endl;
         ofs << "miner id:"<< miner_pubkey << endl;
         ofs << "prevhash:" << prevhash << endl;
         ofs << "nonce:" <<nonce<<endl;
         ofs << "hash:" << hash << endl;
         ofs.close();
   
         string hash1 = gethash(nonce, prevhash, mr, miner_pubkey);
         float number = (float)std::stoi(hash1.substr(0,5), nullptr, 16)/MAX;
         ofs.open("outputs/useful_information.txt", std::ofstream::out|std::ofstream::app);
         ofs<<"fx("<<number<<") = "<<fx(number)<<" < "<<y0<<endl;
         ofs.close();

         prevhash=hash;
      }
      else printf("verification error in %d\n", index);
   }

   // perform verification if someone else has solved 
   if(not solved)
   {
      while(nonce == -1);
      string hash = gethash(nonce, prevhash, mr, miner_pubkey);
      float guess = (float)std::stoi(hash.substr(0,5), nullptr, 16)/MAX;
      float f = fx(guess);
      // if (hash.compare(hash_pub) == 0)
      //    cout << "Hash matched" << endl;
      // else 
      //    cout << "Hash did not match" << endl;
      // cout << "fval:: " << f << endl;
      if(f<y0)
      {
         pthread_mutex_lock(&mutex);
         n_verify++;
         pthread_mutex_unlock(&mutex);
      }
      else 
      {
         pthread_mutex_lock(&mutex);
         n_reject++;
         pthread_mutex_unlock(&mutex);
      }
   }
   pthread_exit(0);
}
