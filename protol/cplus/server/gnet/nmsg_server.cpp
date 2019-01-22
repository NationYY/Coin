#include "stdafx.h"
#include <sio/sdata_map.h>
#include <sio/sio_list.h>
#include <sio/sio_data_buf.h>
#include <sio/sdata_map_reader.h>
#include <sio/sdata_map_writer.h>
#include <sio/sio_list_reader.h>
#include <sio/sio_list_writer.h>
#include <net/client.h>
#include <net/server.h>
#include "nmsg_server.h"
bool nmsg_server::dispatch(clib::psession __c, clib::pnpackage __p)
{
     if(__p!=NULL&& __c!=NULL){
				if (async_dispatch(__c, __p) == true)
					return true; 
         unsigned int messageid = __p->get_header()->get_id();
             switch(messageid)
              {
                 case nmsg_server_cg_login:
                 {
                    clib::sio_data_buf key_sdata ;
                    key_sdata.unserializ(__p);
                    clib::string key( key_sdata.get_string());
                    return cg_login(__c,__p, key);
                 }
                 case nmsg_server_cg_ping:
                 {
                    return cg_ping(__c,__p);
                 }
              };
      }
      return false;
}
bool nmsg_server::async_dispatch(clib::psession __c, clib::pnpackage __p)
{
      return false;
}
bool nmsg_server::gc_login_rst(clib::psession __pc,s_int32 rst,const clib::string& time)
{
   if(__pc!=NULL){
      clib::pnpackage p = __pc->create_package();
      clib::pnpackage_header ph = __pc->create_package_header();
      p->set_header(ph);
      ph->set_id(nmsg_server_gc_login_rst);
	  clib::sio_data::serializ_ex(p,rst);
	  clib::sio_data::serializ_ex(p,time);
      __pc->send(p);
      p->del_ref();
      ph->del_ref();
   }
  return false;
}
bool nmsg_server::gc_pong(clib::psession __pc)
{
   if(__pc!=NULL){
      clib::pnpackage p = __pc->create_package();
      clib::pnpackage_header ph = __pc->create_package_header();
      p->set_header(ph);
      ph->set_id(nmsg_server_gc_pong);
      __pc->send(p);
      p->del_ref();
      ph->del_ref();
   }
  return false;
}
bool nmsg_server::gc_account_invalid(clib::psession __pc)
{
   if(__pc!=NULL){
      clib::pnpackage p = __pc->create_package();
      clib::pnpackage_header ph = __pc->create_package_header();
      p->set_header(ph);
      ph->set_id(nmsg_server_gc_account_invalid);
      __pc->send(p);
      p->del_ref();
      ph->del_ref();
   }
  return false;
}
