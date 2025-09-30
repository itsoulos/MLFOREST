# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include <math.h>
#include <vector>
using namespace std;
int main(int argc,char **argv)
{
	FILE *fp=fopen(argv[1],"r");
	if(!fp) return 1;
	int d,count;
	fscanf(fp,"%d",&d);
	fscanf(fp,"%d",&count);
	printf("@Relation test\n\n");
	vector<double> nclass;
	for(int i=1;i<=d;i++)
	{
		printf("@ATTRIBUTE x%d NUMERIC\n",i);
	}
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<d;j++)
		{
			double x;
			fscanf(fp,"%lf",&x);
		}
		double y;
		fscanf(fp,"%lf",&y);
		bool found=false;
		for(int j=0;j<nclass.size();j++)
		{	
			if(fabs(nclass[j]-y)<1e-5) found=true;
		}
		if(!found) nclass.push_back(y);
	}
	fclose(fp);
	fp=fopen(argv[1],"r");
	fscanf(fp,"%d",&d);
	fscanf(fp,"%d",&count);
	printf("@ATTRIBUTE class {");
	//sort nclass here
	for(int i=0;i<nclass.size();i++)
	{
		for(int j=0;j<nclass.size()-1;j++)
		{
			if(nclass[j+1]>nclass[j])
			{
				double t=nclass[j];
				nclass[j]=nclass[j+1];
				nclass[j+1]=t;
			}
		}
	}
	for(int i=0;i<nclass.size();i++)
		if(i!=nclass.size()-1) printf("%.2lf,",nclass[i]);
		else printf("%.2lf}\n\n",nclass[i]);
	printf("@DATA\n");
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<d;j++)
		{
			double x;
			fscanf(fp,"%lf",&x);
			printf("%lf,",x);
		}
		double y;
		fscanf(fp,"%lf",&y);
		printf("%.2lf\n",y);
	}
	fclose(fp);
	return 0;
	
}
