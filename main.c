#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

struct frame
{
        int fresh;           //default 0, Upon first access it's set to 1.
        long long addr;          //Actual address 
        char r_w;                 //Holds r_w data
        int accesscount;            //On every access the count is increased
        int reference;               //will be set to required value based on the replacement policy
        int rflag;                     //when the reference value become 0 then now referenc should increase upto 10.
};

int max;                  //0 - max vritual space
int psize;              //page size==frame size
int flag;               //To check wether the page is already availabel or not
char debug;             //prints debug satatus

int PFAULTC;             //Page fault condition
int PWRITE;             //Page when write in to the ram

void initonevspace(struct frame **p,int i)
{
        p[i]->reference=2;              //Our reference value
        p[i]->accesscount=0;            //Every R W considered as accesscount.
        p[i]->rflag=0;                  //Resetting reference flag, which set's once the reference count become 0.
}

int checkpossible(struct frame **p,int fcount)
{
        for(int i=0;i<fcount;i++)
        {
                if(p[i]->reference==0)       //Least used Page
                {
                        initonevspace(p,i);
                        return i;
                }
                else if(p[i]->reference==10) //Since simulation, we assume all the code has been completed executing. This is free for swapping
                {
                        initonevspace(p,i);
                        return i;
                }
        }
        return -1;
}

void reduceref(struct frame**p,int fcount)   //reduce upto zero (All frame reference is going to reduce since we didn't find any one with 0)
{
        for(int i=0;i<fcount;i++)
                p[i]->reference--;
}

void swapframe(unsigned long long addr,char r_w,struct frame **p,int fnumber)
{
        if(p[fnumber]->fresh==0)   //Will be true Only once in it's execution (at the beginning stage/first Swap)
        {
                p[fnumber]->fresh=1;     // from now onwards it's a used page, Not fresh.
				if(debug=='d')
                printf("Page NULL replaced with %lld\n",addr/psize);  //execute only once during first time loading in to purticular frame
        }
        else
        {
				PFAULTC++;      //Requirement only to count the page fault which caused by already existing page not with NULL frame at the time of beginning.
				if(debug=='d')
                printf("Page %lld replaced with %lld\n",p[fnumber]->addr/psize,addr/psize);
        }
        if(p[fnumber]->r_w=='w' && debug=='d')
		{
                PWRITE++;
                printf("Page %lld was dirty\n\n",p[fnumber]->addr/psize);    //When W replaces with r It is dirty bit.
		}
	else if (debug=='d')
	printf("Page %lld was not dirty\n\n",p[fnumber]->addr/psize);
        //Actual swapping, Since we don't have virtual backup, we only updates which consider as swap here.
        initonevspace(p,fnumber);   //Resetting
        p[fnumber]->addr=addr;     //Adding new address
        p[fnumber]->r_w=r_w;	//Adding new r w status
}

void swap(unsigned long long addr,char r_w,struct frame **p,int fcount)
{
        //Replacement algorithm
        int x;
	L1:
        if((x=checkpossible(p,fcount))!=-1)//returns indexx number with which swapping need to be done, possible if 0 or 10. if 10 reset and send that index.
        {
                swapframe(addr,r_w,p,x);   //swap with the INDEX NUMBER. x contains the index of the frame which need to replace
                goto L2;
        }
        else
        {
                reduceref(p,fcount);   //reduce reference number of all node by 1.
	}
        goto L1;
	L2:
        return ;
}

int add(unsigned long long addr,char r_w,struct frame **p,int fcount)
{
        if(addr<max)
        {
                int pg=addr/psize;
                for(int i=0;i<fcount;i++)
                {
                        //////////////////////////////////////////////////////////////////
                        //The FOLLOWING 'IF' ACT AS TRANLSATIONAL LOOKASIDE BUFFER(TLB))//
                        //////////////////////////////////////////////////////////////////
                        if((pg==((p[i]->addr)/psize)) && (r_w==p[i]->r_w))    //Checking if the given offset already avaialable or not
                        {
//                              printf("already available:%llu %c at %d\n",addr,r_w,i);
                                (p[i]->accesscount)++;                       //Even if avaiable need to increase the access count
                                if((p[i]->accesscount%4)==0)
                                {
                                                if(p[i]->rflag==0)
                                                p[i]->reference--;      //based on the updated access count, need to reduce the referece count
						if(p[i]->reference==0)
                                                {    
							p[i]->rflag=1;  //Once the reference value become zero, We'll start to increase the reference upto 10
				        	}
				}
                                if(p[i]->rflag)
                                        p[i]->reference++;
                                flag=1;    //no need to swap beacuase already available in the frame.
                                break;
                        }
                }
                if(flag==0)  //Page fault condition didn't find the address in the frame range
                {
                        //printf("page fault addre:%llu\n",addr);
                        swap(addr,r_w,p,fcount);                        //need to swap the file. ACTUAL PAGE FAULT
                }
                flag=0;
        }
        else
        {
                printf("Segmentation fault::Outside virtual space\n");  //NEED TO PURPOSFULLY HACK THE USER INPUT ADDR FROM VIRTUAL RAM
                exit(0);
        }
}



int main(int argc,char **argv)
{
	int TMEMACCESS=0;
        if(argc!=6)
        {
                printf("Use:filename.c Vspace Pagesize Totalframe inputfile d/n\n");
                return 0;
        }
        int fnumber=atoi(argv[3]);
        struct frame  **p;
        p=malloc(sizeof(char *)*fnumber);
        for(int i=0;i<fnumber;i++)
        p[i]=calloc(1,sizeof(struct frame));
        FILE *fp=fopen(argv[4],"r");
        if(fp==0)
        {
                printf("Failed opening the file\n");
                return 0;
        }
        debug=argv[5][0];
        max=pow(2,atoi(argv[1]));
        psize=pow(2,atoi(argv[2]));
        printf("-------------INFO-----------\nCONFIRM:\nvirtaul space- %d \nPage size-     %d(maximum %d pages)\nTota Frames-   %d\n-------------START------------\n\n",max,psize,max/psize,fnumber);
        char a[20];
        unsigned long long addr;
        char r_w;
        int i=0;
        while(1)
        {
                bzero(a,20);
                if(fgets(a,20,fp))
                {
                        addr=atoi(a);
                        r_w=a[strlen(a)-2];
                }
                else
                break;
				TMEMACCESS++;
                add(addr,r_w,p,fnumber);
        }
		if(debug!='d')
		printf("    - DEBUG IS DISABLED -\n\n");
                printf("-------------STOP-------------\n");
		printf("FINAL STATUS:\n");
		printf("Total M.Access-          %d\n",TMEMACCESS);
		printf("Total PageFault-         %d\n",PFAULTC);
                printf("Number of pages Written- %d\n",PWRITE);	
		printf("--------------END-------------\n");
}                       
