#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
struct frame
{
	int fnumber;
    long long addr;
	char r_w;
	int accesscount;
	int reference;
	int rflag;
};


int max;
int psize;
int flag;
int dirty;

void initonevspace(struct frame **p,int i)
{
    p[i]->fnumber=-1;
    p[i]->r_w=-1;
    p[i]->reference=3;
    p[i]->accesscount=0;
    p[i]->rflag=0;
}

int checkpossible(struct frame **p,int fcount)
{
    //printf("fcount %d\n",fcount);
    for(int i=0;i<fcount;i++)
    {
    if(p[i]->reference==0)
    {  
        initonevspace(p,i);
        return i;
    }
    else if(p[i]->reference==10)
    {
         //printf("TEST2\n");
        initonevspace(p,i);
        return i;
    }
    }
    return -1;
}
void reduceref(struct frame**p,int fcount)   //reduce upto zero
{
    for(int i=0;i<fcount;i++)  
    {   
       // printf("going to decrease %d\n",p[i]->reference);
        p[i]->reference--;
    }
}



void swapframe(unsigned long long addr,char r_w,struct frame **p,int fnumber)
{
  printf("Page in\n");
  initonevspace(p,fnumber);
  p[fnumber]->addr=addr;
  p[fnumber]->r_w=r_w;
}





void swap(unsigned long long addr,char r_w,struct frame **p,int fcount)
{
	int x;
	L1:
    //printf("check possible output%d\n",checkpossible(p,fcount));
    if((x=checkpossible(p,fcount))!=-1)//returns indexx number with which swapping need to be done, possible if 0 or 10. if 10 reset and send that index.
	{ 
        printf("Swapping :%lld %c at %d\n",addr,r_w,x);
		swapframe(addr,r_w,p,x);   //swap with the index number
		//printf("POSSIBLE\n");
		goto L2;
	}
	else
	{
        printf("Going to reduce\n");
		reduceref(p,fcount);   //reduce reference number of all node
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
        printf("%lld %c\n",addr,r_w);
		for(int i=0;i<fcount;i++)
		{
           // printf("TEST :%c\n",r_w);
			if((pg==((p[i]->addr)/psize)) && (r_w==p[i]->r_w))
			{
                printf("already available:%llu %c at %d\n",addr,r_w,i);
				(p[i]->accesscount)++;
				if((p[i]->accesscount%4)==0)
				{
					if(p[i]->rflag==0)
						p[i]->reference--;
					if(p[i]->reference==0)
						p[i]->rflag=1;
				}
				if(p[i]->rflag)
					p[i]->reference++;
				flag=1;    //no need to swap beacuase already available in the frame.
				break;
			}
		}
		if(flag==0)  //Page fault condition didn't find the address in the frame range
		{
           // printf("page fault addre:%llu\n",addr);
			swap(addr,r_w,p,fcount);    //need to swap or add the file.
		}
		flag=0;
		//printf("nothing addre:%llu\n",addr);
	}
	else
	{
		printf("Segmentation fault::Outside virtual space\n");
		exit(0);
	}
}



void initvspace(struct frame **p,int fcount)
{
    for(int i=0;i<fcount;i++)
            p[i]->r_w=-1;
}

int main(int argc,char **argv)
{
	if(argc!=6)
	{
		printf("Use:filename.c Vspace Pspace Tframe inputfile d/n\n");
		return 0;
	}
	int debug=0;
	if(strcmp("d",argv[5]))
		debug=1;
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
	max=pow(2,atoi(argv[1]));
	psize=pow(2,atoi(argv[2]));
    printf("CONFIRM virtaul space:%d page size:%d maximum pages that can hold:%d\n",max,psize,max/psize);
	char a[20];
	unsigned long long addr;
	char r_w;
	int i=0;
    initvspace(p,fnumber);   //function to initialize alll = referece with 3;
	while(1)
	{
		bzero(a,20);
		if(fgets(a,20,fp))
		{
			addr=atoi(a);
			r_w=a[strlen(a)-2];
			//printf("%2d)Test R & E :%lld %c\n",++i,addr,r_w);
		}
		else
			break;
		add(addr,r_w,p,fnumber);
	}
	printf("Completed\n");
}
