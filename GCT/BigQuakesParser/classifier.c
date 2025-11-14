#include <math.h>

int classifier(double *x){

	int CLASS = 0;

	if(!(x[2]>=x[8]&&x[1]>=(x[3]*x[2]/x[10])||!(x[2]>((x[7]+(-4.0))*(x[5]+x[5]-x[6]*x[9]))&&!(x[9]>=((-0.91)*x[0])&&x[1]<=x[9]&&(-11.64)*x[8]*x[1]+x[2]<=(-96.925))))) CLASS=0;
	else CLASS=1;
	
	return CLASS;
}
