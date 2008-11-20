#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    setuid(0);
    system("rm -rf `find /etc/sysconfig -name ifcfg-eth?.bak`");
    system("/usr/sbin/setup");
    printf("Stopping network: eth0"); fflush(stdout);
    system("/sbin/ifdown eth0");
    printf(" eth1"); fflush(stdout);
    system("/sbin/ifdown eth1");
    printf("\nStarting network: eth0"); fflush(stdout);
    system("/sbin/ifup eth0");
    printf(" eth1"); fflush(stdout);
    system("/sbin/ifup eth1");
    printf("\n");
}
