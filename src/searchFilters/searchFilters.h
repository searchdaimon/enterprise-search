#include "../common/define.h"

int filterAdultWeight(char AdultWeight,int adultpages,int noadultpages);
int filterResponse(int responscode);
int filterSameIp(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterSameDomain(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterDescription(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterSameCrc32(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterTLDs(char domain[]);
int filterSameUrl(int showabal,char url[], struct SiderFormat *Sider);
int filterDomainLength(char domain[]);
int filterDomainNrOfLines(char domain[]);
