#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
struct frame
{
	int fnumber;
	char r_w;
	int accesscount;
	int reference;
	int rflag;
};
int max;
int psize;
int flag;
int dirty;


void reduceref()
void swap(unsigned long long addr,char r_w,struct frame **p,int fname)
{
	int x;
	L1:
	if(x=checkpossible());
	{
		//swap
		printf("POSSIBLE\n");
		goto L2;
	}
	else
	{
		reduceref();
	}
	goto L1;
	L2:
	return ;
}


int add(unsigned long long addr,char r_w,struct frame **p,int fname)
{
	if(addr<max)
	{
		int pg=addr/psize;
		for(int i=0;i<fname;i++)
		{
			if(pg==p[i]->fnumber && r_w==p[i]->r_w)
			{
				(p[i]->accesscount)++;
				if((p[i]->accesscount%4)==0)
				{
					if(rflag==0)
						p[i]->reference--;
					if(p[i]->reference==0)
						rflag=1;
				}
				if(rflag)
					p[i]->reference++;


				flag=1;    //no need to swap beacuase already available in the frame.
				break;
			}
		}
		if(flag==0)  //Page fault condition
		{
			swap(addr,r_w,p,pg);
		}
		flag=0;
		//check the possiblity of adding. ELse return swap;
		//reset teh reference count based on the replacing algorithm
		printf("addre:%llu\n",addr);

	}
	else
	{
		printf("Segmentation fault\n");
		exit(0);
	}
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
	char a[20];
	unsigned long long addr;
	char r_w;
	int i=0;
	//function to initialize alll = referece with 3;
	while(1)
	{
		bzero(a,20);
		if(fgets(a,20,fp))
		{
			addr=atoi(a);
			r_w=a[strlen(a)-2];
			printf("%2d)Test R & E :%lld %c\n",++i,addr,r_w);
		}
		else
			break;
		add(addr,r_w,p,fnumber);
	}
	printf("Completed\n");
}
