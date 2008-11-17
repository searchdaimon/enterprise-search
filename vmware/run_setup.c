#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    setuid(0);
    system("/usr/sbin/setup");
    system("/etc/init.d/network restart");
    system("/home/setup/make_issue.sh");
}
