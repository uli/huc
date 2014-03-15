f(int got){if(got!=0xffff)abort();}
main(){signed char c=-1;unsigned u;u=/*(unsigned short)*/c;f(u);exit(0);}
