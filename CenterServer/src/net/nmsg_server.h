#ifndef net_gen_nmsg_server_h
#define net_gen_nmsg_server_h
#include <list>
#include <vector>
#include <net/nmodule.h>
#include "genum/module_enum.h"
class nmsg_server: public clib::nmodule
{
public:
    nmsg_server():nmodule(nmsg_server_start_id, nmsg_server_end_id){};
    virtual ~nmsg_server(){};
public:
     virtual bool dispatch(clib::psession c, clib::pnpackage p);
     virtual bool async_dispatch(clib::psession c, clib::pnpackage p);
public:
     virtual bool cg_login(clib::psession __c, clib::pnpackage __p, const clib::string& key)=0;
     virtual bool cg_ping(clib::psession __c, clib::pnpackage __p)=0;
public:
     static bool gc_login_rst(clib::psession __pc, s_int32 rst, const clib::string& time);
     static bool gc_pong(clib::psession __pc);
     static bool gc_account_invalid(clib::psession __pc);
};
#endif
