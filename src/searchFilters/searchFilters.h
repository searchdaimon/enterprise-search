#include "../common/define.h"

int filterAdultWeight(char AdultWeight,int adultpages,int noadultpages);
int filterResponse(int responscode);
int filterSameIp(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterSameDomain(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterDescription(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int find_domain_no_www (char url[],char domain[],int sizeofdomain);
int filterSameCrc32(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int find_domain_no_subname (char url[],char domain[], int sizeofdomain);
int filterTLDs(char domain[]);
int filterSameUrl(int showabal,char url[], struct SiderFormat *Sider);
