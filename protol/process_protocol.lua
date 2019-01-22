-- 1. 生成协议
-- 2. 检测错误信息。
-- 3. 分类拷贝协议， 部分协议文件是要根据服务器的不同拷贝的， 请注意维护特殊文件的列表 game_server_as_client_msg_files 
-- kevin @ 2015.10.23


-- local lfs = require("lfs")

function getCurDir()
	local obj=io.popen("cd")
	path=obj:read("*all"):sub(1,-2)
	obj:close()
	return path;
end

function getFileContent(file_path_name)
	print(file_path_name)
	local f = io.open(file_path_name,"r")
	local content = f:read("*all")
	f:close()
	return content
end

function checkOpSucc(op_rst)
	local failed_words = {
		"not find another",
		"init function error",
		"init failed",
	}

	for k, v in pairs(failed_words) do
		if string.find(op_rst, v, 1, true) then
			return false
		end
	end

	return true;
end

function getCodeRootDir(curDir)
	local pos = string.find(curDir, "\\protol\\", 1, true)
	if pos == nil then
		return nil
	end 

	return curDir.sub(curDir, 1, pos)
end


function getFileInFolder(rootDir, check_func, file_list)
    for file in lfs.dir(rootDir) do
        if file ~= '.' and file ~= '..' then
            local path = rootDir .. '\\' .. file

            local attr = lfs.attributes(path)
            assert(type(attr) == 'table')

            if attr.mode == 'directory' then
                getFileInFolder(path, check_func, file_list)
            else
            	if check_func(path, file) then
	            	table.insert(file_list, path);
	            end
            end
        end
    end
end


function addPCH(rootDir)
	local cpp_files = {}
	getFileInFolder(rootDir, function(fullpath, file)  
			do return true end
			if nil == fullpath then
				print ("fkfkfkfkf")
				return false;
			end
			-- print(fullpath)
			if string.find(fullpath, ".cpp") then
				return true
			else
				return false;
			end
		end,
		cpp_files
	)

	for k, v in pairs(cpp_files) do
		print("add pch for ".. v)
	end
end

----------------------------------------------------------------------------------------------------------------
print("start generate protocol files...")

local curDir = getCurDir();
local code_namespace = "game"
local oprst_file = "last_trans_rst.log"

--wtee.exe 同时向控制台和文件写入信息。

local gen_protocol_cmd = string.format("%s\\net_protol.exe -p %s\\net_protol\\ -t cplus | %s\\wtee.exe %s\\%s", 
										curDir, curDir, curDir, curDir, oprst_file);
print(gen_protocol_cmd)

--删除旧协议文件
os.execute("del /F /Q lua")
os.execute("del /F /Q cplus")
-- os.execute("del /F /Q java")

--生成协议
os.execute(gen_protocol_cmd)


if not checkOpSucc(getFileContent(curDir.."\\"..oprst_file)) then
	for i = 1, 3 do
		print("\n ERROR !!! 有错误信息，请注意！！！ 协议已经生成， 但是不会被拷贝。")
	end
	return
end
os.execute("python add_stdafx.py")
local code_root_dir = "F:\\code\\Coin"
local server_msg_header_copy_cmd = {
		string.format("xcopy %s\\cplus\\genum   %s\\OKExFutures\\src\\net\\genum /s /y", curDir, code_root_dir),
		string.format("xcopy %s\\cplus\\gstruct   %s\\OKExFutures\\src\\net\\gstruct /s /y", curDir, code_root_dir),
		string.format("xcopy %s\\cplus\\genum   %s\\CenterServer\\src\\net\\genum /s /y", curDir, code_root_dir),
		string.format("xcopy %s\\cplus\\gstruct   %s\\CenterServer\\src\\net\\gstruct /s /y", curDir, code_root_dir),
	}
for k, v in pairs (server_msg_header_copy_cmd) do
	os.execute(v)
end

local copy_file = {
	"nmsg_server.cpp",
	"nmsg_server.h",
	}
for k, v in pairs(copy_file) do
	local _cmd = string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\OKExFutures\\src\\net /y", curDir, v, code_root_dir)
	os.execute(_cmd)
	_cmd = string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\CenterServer\\src\\net /y", curDir, v, code_root_dir)
	os.execute(_cmd)
end
--[[
print("开始执行协议拷贝...")
local code_root_dir = getCodeRootDir(curDir) 
print("开始执行协议拷贝........"..tostring(code_root_dir))

--拷贝client
local client_copy_cmd = string.format("xcopy %s\\lua %s\\client\\logic\\net /s /y",curDir, code_root_dir)
print(client_copy_cmd)
os.execute(client_copy_cmd)


--拷贝game_server, 注意某些文件需要特殊拷贝
local game_server_client_file_list = {

}

local game_server_msg_header_copy_cmd = {
		string.format("xcopy %s\\cplus\\genum   %s\\server\\public\\net_module\\generated\\genum /s /y", curDir, code_root_dir),
		string.format("xcopy %s\\cplus\\gstruct   %s\\server\\public\\net_module\\generated\\gstruct /s /y", curDir, code_root_dir),
	}
for k, v in pairs (game_server_msg_header_copy_cmd) do
	os.execute(v)
end

--gameserver作为客户端连接的相关消息
local game_server_as_client_msg_files = {
	"nserver_account.h",
	"nserver_account.cpp",

	"nserver_log.h",
	"nserver_log.cpp",	
	"npay.h",
	"npay.cpp",
	"nmsg_center_server.h",
	"nmsg_center_server.cpp",
	"nmsg_pay.h",
	"nmsg_pay.cpp",
	"ngm_game_operate.h",
	"ngm_game_operate.cpp",
	"ngame_2_fmanager.cpp",
	"ngame_2_fmanager.h",
	"nmsg_fight_verify.h",
	"nmsg_fight_verify.cpp",
	"nmsg_player_info_cache.h",
	"nmsg_player_info_cache.cpp",
	
	"nmsg_game_2_info.h",
	"nmsg_game_2_info.cpp",
	"ngame_2_cross.h",
	"ngame_2_cross.cpp",
	
	"ngame_2_chat.cpp",
	"ngame_2_chat.h",
}

--gameserver作为服务器连接的相关消息
local game_server_as_server_msg_files = {
	-- 注意只包含和其他服务器共用的文件，然后其他服务器不要的文件就是 gameserver需要的文件 参考：game_server_ignore_files
	"protol_name.h"
}

--忽略文件列表
local game_server_ignore_files = {
	"nmsg_gm_account.h",
	"nmsg_gm_account.cpp",
	"nmsg_pay_agent.h",
	"nmsg_pay_agent.cpp",
	"nweb_gm_operate.h",
	"nweb_gm_operate.cpp",
	"ngm_center_operate.h",
	"ngm_center_operate.cpp",
	"ngm_account_operate.h",
	"ngm_account_operate.cpp",
	"nfight_2_fmanager.cpp",
	"nfight_2_fmanager.h",
	"nworld_msg.h",
	"nworld_msg.cpp",
	"nmsg_fight.h",
	"nmsg_fight.cpp",
	"nproxy_account.h",
	"nproxy_account.cpp",
	"nclient_proxy.h",
	"nclient_proxy.cpp",
	"nmsg_move.h",
	"nmsg_move.cpp",
	"nproxy_fight.h",
	"nproxy_fight.cpp",
	"proxy_dtree.h",
	"proxy_dtree.cpp",
}

--gm后台作为客户端需要的文件
local gm_server_as_client_msg_files = 
{
	"ngm_center_operate.h",
	"ngm_center_operate.cpp",
}

--gm后台作为服务器需要的文件
local gm_server_as_server_msg_files = 
{
	"protol_name.h",
	"nweb_gm_operate.h",
	"nweb_gm_operate.cpp",
	"ngm_game_operate.h",
	"ngm_game_operate.cpp",
	"ngm_account_operate.h",
	"ngm_account_operate.cpp",
}

local fight_manager_server_as_server_msg_files = 
{
	"nfight_2_fmanager.h",
	"nfight_2_fmanager.cpp",
	"ngame_2_fmanager.h",
	"ngame_2_fmanager.cpp",
	"ncross_2_fmanager.h",
	"ncross_2_fmanager.cpp"
}

local fight_server_as_client_msg_files = 
{
	"nfight_2_fmanager.h",
	"nfight_2_fmanager.cpp",
	"nfight_2_cross.h",
	"nfight_2_cross.cpp",
}

local fight_server_as_server_msg_files = 
{
	"nworld_msg.h",
	"nworld_msg.cpp",
	"nmsg_fight.h",
	"nmsg_fight.cpp",
	"nmsg_move.h",
	"nmsg_move.cpp",
	"nproxy_fight.h",
	"nproxy_fight.cpp",
}
local proxy_server_as_client_msg_files = 
{
	"nproxy_game.h",
	"nproxy_game.cpp",
	"nproxy_account.h",
	"nproxy_account.cpp",
	"nproxy_fight.h",
	"nproxy_fight.cpp",
	"nproxy_dtree.h",
	"nproxy_dtree.cpp",
	"nproxy_center.h",
	"nproxy_center.cpp",
}

local proxy_server_as_server_msg_files = 
{
	"protol_name.h",
	"nclient_proxy.h",
	"nclient_proxy.cpp",
}

local fight_verify_server_as_server_msg_files = 
{
	"nmsg_fight_verify.h",
	"nmsg_fight_verify.cpp",
}

local account_server_as_client_msg_files =
{
	"nserver_log.h",
	"nserver_log.cpp",
	"npay.h",
	"npay.cpp",	
	"ngm_account_operate.h",
	"ngm_account_operate.cpp",
}

local account_server_as_server_msg_files =
{
	"protol_name.h",
	--"nlogin.h",
	--"nlogin.cpp",
	"nmsg_gm_account.h",
	"nmsg_gm_account.cpp",
	"nmsg_pay_agent.h",
	"nmsg_pay_agent.cpp",
	"nmsg_web_account.h",
	"nmsg_web_account.cpp",
	"nproxy_account.h",
	"nproxy_account.cpp",
	"nserver_account.h",
	"nserver_account.cpp",
	"nmsg_pay.h",
	"nmsg_pay.cpp",	
}

local account_log_server_as_server_msg_files = 
{
	"protol_name.h",
	"nserver_log.h",
	"nserver_log.cpp",
}

log_server_as_server_msg_files = 
{
	"protol_name.h",
	"nserver_log.h",
	"nserver_log.cpp",
}

central_server_as_client_msg_files = {
	"nserver_log.h",
	"nserver_log.cpp",
}

central_server_as_server_msg_files =
{
	"protol_name.h",
	"nmsg_center_server.h",
	"nmsg_center_server.cpp",
	"nmsg_gm_center_server.h",
	"nmsg_gm_center_server.cpp",
	"ngm_center_operate.h",
	"ngm_center_operate.cpp",
	"nproxy_center.h",
	"nproxy_center.cpp",
	"nlogin.h",
	"nlogin.cpp",
}

info_server_as_client_msg_files = 
{
}

info_server_as_server_msg_files = 
{
	"protol_name.h",
	"nmsg_web_2_info.h",
	"nmsg_web_2_info.cpp",
	"nmsg_game_2_info.h",
	"nmsg_game_2_info.cpp",
}

cross_logic_server_as_client_msg_files = 
{
	"ncross_2_fmanager.h",
	"ncross_2_fmanager.cpp"
}

cross_logic_server_as_server_msg_files =
{
	"protol_name.h",
	"ngame_2_cross.h",
	"ngame_2_cross.cpp",
	"nfight_2_cross.h",
	"nfight_2_cross.cpp",
}

chat_server_as_server_msg_files = 
{
	"protol_name.h",
	"ngame_2_chat.h",
	"ngame_2_chat.cpp",
}

function table.merge_indexed(dst, src)
	for i = 1, #src do
		dst[#dst+1] = src[i]
	end
end


table.merge_indexed(game_server_ignore_files, gm_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, proxy_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, account_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, account_log_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, log_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, central_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, info_server_as_server_msg_files)
table.merge_indexed(game_server_ignore_files, cross_logic_server_as_client_msg_files)
table.merge_indexed(game_server_ignore_files, cross_logic_server_as_server_msg_files)



for k, v in pairs(game_server_as_server_msg_files) do
	for kk, vv in pairs(game_server_ignore_files) do
		if vv == v then
			game_server_ignore_files[kk] = nil
		end
	end
end

local servers_copy_cmd = {
	[1] = string.format("xcopy %s\\cplus\\server\\gnet   %s\\server\\game_server\\src\\gnet /s /y", curDir, code_root_dir),
}

for k, v in pairs(game_server_ignore_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("del %s\\server\\game_server\\src\\gnet\\%s", code_root_dir, v);
end

for k, v in pairs(game_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\game_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(game_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\game_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(gm_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\gm_agency_server\\src\\gnet\\ /y", curDir, v, code_root_dir);
end

for k, v in pairs(gm_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\gm_agency_server\\src\\gnet\\ /y", curDir, v, code_root_dir);
end

for k, v in pairs(fight_manager_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\fight_manager_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(fight_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\fight_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(fight_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\fight_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(proxy_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\proxy_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(proxy_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\proxy_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(log_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\log_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\account_log_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(fight_verify_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\fight_verify_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(account_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\account\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(account_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\account\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(account_log_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\account_log_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(central_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\center_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end


for k, v in pairs(central_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\center_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end


for k, v in pairs(info_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\info_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(cross_logic_server_as_client_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\client\\gnet\\%s   %s\\server\\cross_logic_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(cross_logic_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\cross_logic_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end

for k, v in pairs(chat_server_as_server_msg_files) do
	servers_copy_cmd[#servers_copy_cmd + 1] = 
		string.format("copy %s\\cplus\\server\\gnet\\%s   %s\\server\\chat_server\\src\\gnet\\ /y", curDir, v, code_root_dir)
end



for k, v in ipairs(servers_copy_cmd) do
	print(v)
	os.execute(v)
end
--]]







