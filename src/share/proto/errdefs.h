#pragma once

//Login Gate  ErrCode 1000-1999 LGERR
enum LoginGateErrCode
{
	LGERR_NO_ERR                  	= 1000      ,	//成功
	LGERR_GameGate_Unavailable    	/*1001    */,	//无法连接到服务器
	LGERR_SourceSvr_Unavailable   	/*1002    */,	//无法连接到下载服务器
};

//Login Server ErrCode 2000-2999 LSERR
enum LoginServerErrCode
{
	LSERR_NO_ERR                  	= 2000      ,	//成功
	LSERR_Sys_Failure             	/*2001    */,	//系统出错
	LSERR_User_Lock               	/*2002    */,	//您的密码连续三次输入错误，此帐号将被暂时封闭！\n请在稍候重新登陆
	LSERR_State_Wrong             	/*2003    */,	//这个帐号正在使用，或者被异常的终止锁定了，\n请稍后再试！
	LSERR_Already_Login           	/*2004    */,	//这个帐号正在使用，或者被异常的终止锁定了，\n请稍后再试！
	LSERR_Login_Timeout           	/*2005    */,	//无法连接到服务器
	LSERR_Auth_Failure            	/*2006    */,	//密码不正确，请重新输入密码
	LSERR_PTID_Not_Exist          	/*2007    */,	//该帐号不存在，请重新输入帐号。
	LSERR_Old_Client              	/*2008    */,	//您的当前客户端版本过旧。
	LSERR_Reserve_Overdue         	/*2009    */,	//您的密码即将到期
	LSERR_Other_Area              	/*2010    */,	//此账号在其它区已登录
	LSERR_Ekey_Warp               	/*2011    */,	//您输入的验证码错误
	LSERR_Has_Verfity             	/*2012    */,	//密码不正确，请重新输入密码
	LSERR_Refuse                  	/*2013    */,	//封测期间仅限“封闭测试账号”登陆
	LSERR_Only_For_TestAccount    	/*2014    */,	//本区仅“封闭测试账号”登陆，请选择其它区登录。
	LSERR_Only_For_CreateRole    	/*2015    */,	//本服务器目前尚未开放游戏功能。敬请耐心等待！
};

//Game Gate ErrCode 3000-3999 GGERR
enum GameGateErrCode
{
	GGERR_NO_ERR                  	= 3000      ,	//成功
	GGERR_LoginServer_Unreachable 	/*3001    */,	//无法连接到服务器
	GGERR_DBServer_Unreachable    	/*3002    */,	//无法连接到服务器
	GGERR_MServer_Unreachable     	/*3003    */,	//无法连接到服务器
	GGERR_GameServer_Unreachable  	/*3004    */,	//无法连接到服务器
	GGERR_Internal_Error          	/*3005    */,	//无法连接到服务器
	GGERR_EnterGs_Failed          	/*3006    */,	//无法连接到服务器
	GGERR_IsLocked_forawhile      	/*3007    */,	//您的密码连续三次输入错误，此帐号将被暂时封闭！\n请在稍候重新登陆
	GGERR_SelRole_Invalid         	/*3008    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_RoleNum_Full            	/*3009    */,	//帐号角色个数已满
	GGERR_Not_InGame              	/*3010    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_In_Processing           	/*3011    */,	//请求处理中，请稍候
	GGERR_GameGate_Shutdown       	/*3012    */,	//无法连接到服务器
	GGERR_Kickoff_InIdle          	/*3013    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_Kickoff_VerifyFailed    	/*3014    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_NameDuplicated          	/*3015    */,	//输入的角色名已存在，请您重新输入名字
	GGERR_RoleStatusInvalid       	/*3016    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_Kickoff_DecodeFailed    	/*3017    */, 	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_Kickoff_AreURobot       	/*3018    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	GGERR_RoleInviteInvalid         /*3019    */,	//邀请码错误
	GGERR_CheckSpeedFailed			/*3020    */,	//加速检测失败
};

//Game Server ErrCode 4000-5999 GSERR
enum GSERR
{
	GSERR_NO_ERR                  	= 4000      ,	//成功
	GSERR_ServerBusy              	/*4001    */,	//服务器正忙
};

//DBServer ErrCode 6000-6999 DBERR
enum DBServerErrCode
{
	DBERR_NO_ERR                  	= 6000      ,	//成功
	DBERR_Cannot_Find_Player      	/*6001    */,	//找不到玩家
	DBERR_PlayerFSM_Timeout       	/*6002    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	DBERR_Account_Not_Exist       	/*6003    */,	//服务器正忙
	DBERR_RoleName_Duplicated     	/*6004    */,	//输入的角色名已存在，请您重新输入名字
	DBERR_Role_Not_Exist          	/*6005    */,	//服务器正忙
	DBERR_RoleName_Invalid        	/*6006    */,	//输入的角色名存在非法字符，\n请您重新输入名字
	DBERR_Role_Not_Online         	/*6007    */,	//服务器正忙
	DBERR_Role_Already_Online     	/*6008    */,	//服务器正忙
	DBERR_Account_Not_Online      	/*6009    */,	//服务器正忙
	DBERR_Account_Already_Online  	/*6010    */,	//服务器正忙
	DBERR_RoleNum_Full            	/*6011    */,	//帐号角色个数已满。
	DBERR_Excute_Mysql_Failed     	/*6012    */,	//服务器正忙
	DBERR_Internal_Failure        	/*6013    */,	//服务器正忙
	DBERR_DB_Kickoff_ByReLogin    	/*6014    */,	//与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。
	DBERR_DelRole_Full            	/*6015    */,	//每天只能删除一个角色
	DBERR_Is_Seal_Role            	/*6016    */,	//此角色已被封禁
	DBERR_JumpGS_DestIsFull       	= 6100      ,	//无法连接到服务器
};

//MServer ErrCode 7000-7999 MSERR
enum MServerErrCode
{
	MSERR_NO_ERR                  	= 7000      ,	//成功
};

extern const char* GetErrDesc(int nErrcode);

