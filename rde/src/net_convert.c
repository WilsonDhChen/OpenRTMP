
#include"rde_def.h"



UINT64 MYAPI hton_ll(UINT64 n)
{
	return (((UINT64)htonl((u_long)n)) << 32) | htonl(  (u_long)(n >> 32));
}
UINT64 MYAPI ntoh_ll(UINT64 n)
{
	return (((UINT64)ntohl((u_long)n)) << 32) | ntohl( (u_long)(n >> 32) );
}
double MYAPI ntoh_double(double net_double) 
{
	UINT64 host_int64;
	host_int64 = ntoh_ll(*((UINT64 *) &net_double));
	return *((double *) &host_int64);
}

double MYAPI hton_double(double host_double) 
{
	UINT64 net_int64;
	net_int64 = hton_ll(*((UINT64 *) &host_double));
	return *((double *) &net_int64);
}
