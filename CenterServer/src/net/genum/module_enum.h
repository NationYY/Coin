#ifndef genum_module_enum_h
#define genum_module_enum_h
typedef enum nmsg_server_enum
{
	nmsg_server_start_id = 101 ,
	nmsg_server_cg_login = 102,
	nmsg_server_cg_ping = 103,
	nmsg_server_gc_login_rst = 104,
	nmsg_server_gc_pong = 105,
	nmsg_server_gc_account_invalid = 106,
	nmsg_server_end_id = 200 ,
}nmsg_server_enum;
#endif
