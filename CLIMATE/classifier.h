#include <math.h>

int classifier(double *x){

	int CLASS = 0;

	if(x[16]<=(-92.47)||x[2]<=((-3.341)-(-67.010))||x[0]<((-1.4)-x[7])&&x[5]<=(-59.5)) CLASS=0;
	else CLASS=1;
	
	return CLASS;
}
