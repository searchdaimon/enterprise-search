#include "../common/define.h"

int filterAdultWeight_bool(char AdultWeight, int noadultpages);
int filterAdultWeight_value(int AdultWeight, int noadultpages);
int filterResponse(int responscode);
int filterSameIp(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterSameDomain(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterDescription(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterSameCrc32(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
int filterTLDs(char domain[]);
int filterSameUrl(int showabal,char url[], struct SiderFormat *Sider);
int filterDomainLength(char domain[]);
int filterDomainNrOfLines(char domain[]);
int filterSameDomainID(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
void filtersTrapedReset(struct filtersTrapedFormat *filtersTraped);
int filterResponseCode(struct SiderFormat *CurentSider);
int filterTitle(char title[]);
int filterSummery(char summery[]);
int pi_switch(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider);
