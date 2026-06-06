#include <math.h>

int classifier(double *x){

	int CLASS = 0;

	if(x[7]+x[0]+x[3]<=x[2]||!(x[3]<(-568.690)||x[2]<=x[3]&&x[4]<=x[1]&&x[3]<=x[2]||x[6]>(((x[5]*x[2]-x[6])+x[6])*(x[4]/(-52.17))))) CLASS=0;
	else CLASS=1;
	
	return CLASS;
}
