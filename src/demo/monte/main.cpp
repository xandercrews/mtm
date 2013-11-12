#include <stdio.h>
#include <cmath>
#include <cstdlib>

double norminv(double q) {
     if(q == .5)
          return 0;

     q = 1.0 - q;

     double p = (q > 0.0 && q < 0.5) ? q : (1.0 - q);
     double t = sqrt(log(1.0 / pow(p, 2.0)));

     double c0 = 2.515517;
     double c1 = 0.802853;
     double c2 = 0.010328;

     double d1 = 1.432788;
     double d2 = 0.189269;
     double d3 = 0.001308;

     double x = t - (c0 + c1 * t + c2 * pow(t, 2.0)) /
                    (1.0 + d1 * t + d2 * pow(t, 2.0) + d3 * pow(t, 3.0));

     if(q > .5)
          x *= -1.0;

     return x;
}

int main(int argc, char ** argv) {
	char const * arg = (argc > 1) ? argv[1] : "0.3";
	printf("norminv(0.3) == %f\n", norminv(strtod(arg, nullptr)));
}
