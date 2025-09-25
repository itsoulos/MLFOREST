# include <stdio.h>
# include <stdlib.h>
# include <vector>
using namespace std;
int main(int argc,char **argv)
{
	vector<double> min;
	vector<double> max;
	double ymin,ymax;
	FILE *fp=fopen(argv[1],"r");
	if(!fp) return 1;
	int d,count;
	fscanf(fp,"%d",&d);
	fscanf(fp,"%d",&count);
	min.resize(d);
	max.resize(d);
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<d;j++)
		{
			double x;
			fscanf(fp,"%lf",&x);
			if(i==0) 
			{
				min[j]=x;
				max[j]=x;
			}	
			if(x<min[j]) min[j]=x;
			if(x>max[j]) max[j]=x;
		}
		double y;
		fscanf(fp,"%lf",&y);
		if(i==0) {ymin=y;ymax=y;}
		if(y<ymin) ymin=y;
		if(y>ymax) ymax=y;
	}
	fclose(fp);

	
	fp=fopen(argv[1],"r");
	fscanf(fp,"%d",&d);
	fscanf(fp,"%d",&count);
	printf("%d\n%d\n",d,count);
	for(int i=0;i<count;i++)
	{
		double xx[d];
		for(int j=0;j<d;j++)
		{
			double x;
			fscanf(fp,"%lf",&xx[j]);
			//x=(x-min[j])/(max[j]-min[j]);
		}
		double y;
		fscanf(fp,"%lf",&y);
		if(y<=2) continue;
		if(y>3) y=1; else y=0;
		for(int j=0;j<d;j++) printf("%lf ",j==0?xx[j]-1970:xx[j]);
		printf("%lf\n",y);
	}
	fclose(fp);

	return 0;
}

