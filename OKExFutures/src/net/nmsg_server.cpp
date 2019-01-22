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
                 case nmsg_server_gc_login_rst:
                 {
                    clib::sio_data_buf rst_sdata ;
                    rst_sdata.unserializ(__p);
                    clib::sio_data_buf time_sdata ;
                    time_sdata.unserializ(__p);
                    s_int32 rst= rst_sdata.get_int();
                    clib::string time( time_sdata.get_string());
                    return gc_login_rst(__c,__p, rst, time);
                 }
                 case nmsg_server_gc_pong:
                 {
                    return gc_pong(__c,__p);
                 }
                 case nmsg_server_gc_account_invalid:
                 {
                    return gc_account_invalid(__c,__p);
                 }
              };
      }
      return false;
}
bool nmsg_server::async_dispatch(clib::psession __c, clib::pnpackage __p)
{
      return false;
}
bool nmsg_server::cg_login(clib::psession __pc,const clib::string& key)
{
   if(__pc!=NULL){
      clib::pnpackage p = __pc->create_package();
      clib::pnpackage_header ph = __pc->create_package_header();
      p->set_header(ph);
      ph->set_id(nmsg_server_cg_login);
	  clib::sio_data::serializ_ex(p,key);
      __pc->send(p);
      p->del_ref();
      ph->del_ref();
   }
  return false;
}
bool nmsg_server::cg_ping(clib::psession __pc)
{
   if(__pc!=NULL){
      clib::pnpackage p = __pc->create_package();
      clib::pnpackage_header ph = __pc->create_package_header();
      p->set_header(ph);
      ph->set_id(nmsg_server_cg_ping);
      __pc->send(p);
      p->del_ref();
      ph->del_ref();
   }
  return false;
}
