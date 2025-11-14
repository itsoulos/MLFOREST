#include <math.h>

int classifier(double *x){

	int CLASS = 0;

	if(x[7]>=(-02.0)&&x[9]<((x[1]-(-92.32))/x[10])) CLASS=0;
	else CLASS=1;
	
	return CLASS;
}
