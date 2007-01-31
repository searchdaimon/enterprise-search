typedef string suggestion<3>;
typedef suggestion suggestions<4>;

typedef int itest<3>;

struct ypmap_parms {
        char testc<>;
        string tests<>;
	/*char *testma<>; */
};


/* msg.x: Remote message printing protocol */
program MESSAGEPROG
{
    version MESSAGEVERS
    {
        suggestions PRINTMESSAGE(string user, string query, stringtablerrrr) = 1;
    } = 1;
} = 99;

