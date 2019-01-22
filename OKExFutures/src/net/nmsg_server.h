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
     virtual bool gc_login_rst(clib::psession __c, clib::pnpackage __p, s_int32 rst, const clib::string& time)=0;
     virtual bool gc_pong(clib::psession __c, clib::pnpackage __p)=0;
     virtual bool gc_account_invalid(clib::psession __c, clib::pnpackage __p)=0;
public:
     static bool cg_login(clib::psession __pc, const clib::string& key);
     static bool cg_ping(clib::psession __pc);
};
#endif
