# include <stdio.h>
# include <stdlib.h>


/** arg1: file
 *  arg2: noise
 *  arg3: index to be disturbed **/
int main(int argc,char **argv)
{
	int d,c;
	double x,y;
	FILE *fp=fopen(argv[1],"r");
	if(!fp) return 1;
	double noise = atof(argv[2]);

	if(noise<=0 || noise>=1) 
	{
		printf("error noise %lf \n",noise);
		fclose(fp);
		return 1;
	}

	fscanf(fp,"%d",&d);
	fscanf(fp,"%d",&c);
	int index = atoi(argv[3]);
	if(index<0 || index>=d)
	{
		printf("error index %d \n",index);
		fclose(fp);
		return 1;
	}
	printf("%d\n%d\n",d,c);
	for(int i=0;i<c;i++)
	{
		for(int j=0;j<d;j++)
		{
			fscanf(fp,"%lf",&x);
			if(j==index)
			{
				int direction=rand()%2==1?1:-1;
				double randNumber = rand()*1.0/RAND_MAX;
				double addValue = randNumber * x * noise;
				x = x + direction*addValue;
				if(x<0) x=0;
			}
			printf("%lf ",x);
		}
		fscanf(fp,"%lf",&y);
		printf("%lf\n",y);
	}
	fclose(fp);
	return 0;
}
