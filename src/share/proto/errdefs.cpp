#include "errdefs.h"

typedef struct _CodeDescMap
{
	int nErrcode;
	const char* pszDesc;
}CodeDescMap;

static const char* DESC_LGERR_NO_ERR                       = "成功";
static const char* DESC_LGERR_GameGate_Unavailable         = "无法连接到服务器";
static const char* DESC_LGERR_SourceSvr_Unavailable        = "无法连接到下载服务器";
static const char* DESC_LSERR_NO_ERR                       = "成功";
static const char* DESC_LSERR_Sys_Failure                  = "系统出错";
static const char* DESC_LSERR_User_Lock                    = "您的密码连续三次输入错误，此帐号将被暂时封闭！\n请在稍候重新登陆";
static const char* DESC_LSERR_State_Wrong                  = "这个帐号正在使用，或者被异常的终止锁定了，\n请稍后再试！";
static const char* DESC_LSERR_Already_Login                = "这个帐号正在使用，或者被异常的终止锁定了，\n请稍后再试！";
static const char* DESC_LSERR_Login_Timeout                = "无法连接到服务器";
static const char* DESC_LSERR_Auth_Failure                 = "密码不正确，请重新输入密码";
static const char* DESC_LSERR_PTID_Not_Exist               = "该帐号不存在，请重新输入帐号。";
static const char* DESC_LSERR_Old_Client                   = "您的当前客户端版本过旧。";
static const char* DESC_LSERR_Reserve_Overdue              = "您的密码即将到期";
static const char* DESC_LSERR_Other_Area                   = "此账号在其它区已登录";
static const char* DESC_LSERR_Ekey_Warp                    = "您输入的验证码错误";
static const char* DESC_LSERR_Has_Verfity                  = "密码不正确，请重新输入密码";
static const char* DESC_LSERR_Refuse                       = "封测期间仅限“封闭测试账号”登陆。";
static const char* DESC_LSERR_Only_For_TestAccount         = "本区仅“封闭测试账号”登陆，请选择其它区登录。";
static const char* DESC_LSERR_Only_For_CreateRole    	   = "本服务器目前尚未开放游戏功能。敬请耐心等待！";

static const char* DESC_GGERR_NO_ERR                       = "成功";
static const char* DESC_GGERR_LoginServer_Unreachable      = "无法连接到服务器";
static const char* DESC_GGERR_DBServer_Unreachable         = "无法连接到服务器";
static const char* DESC_GGERR_MServer_Unreachable          = "无法连接到服务器";
static const char* DESC_GGERR_GameServer_Unreachable       = "无法连接到服务器";
static const char* DESC_GGERR_Internal_Error               = "无法连接到服务器";
static const char* DESC_GGERR_EnterGs_Failed               = "无法连接到服务器";
static const char* DESC_GGERR_IsLocked_forawhile           = "您的密码连续三次输入错误，此帐号将被暂时封闭！\n请在稍候重新登陆";
static const char* DESC_GGERR_SelRole_Invalid              = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_RoleNum_Full                 = "帐号角色个数已满";
static const char* DESC_GGERR_Not_InGame                   = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_In_Processing                = "请求处理中，请稍候";
static const char* DESC_GGERR_GameGate_Shutdown            = "无法连接到服务器";
static const char* DESC_GGERR_Kickoff_InIdle               = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_Kickoff_VerifyFailed         = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_NameDuplicated               = "输入的角色名已存在，请您重新输入名字";
static const char* DESC_GGERR_RoleStatusInvalid            = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_Kickoff_DecodeFailed         = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_Kickoff_AreURobot            = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_GGERR_RoleInviteInvalid			   = "邀请码错误";
static const char* DESC_GSERR_NO_ERR                       = "成功";
static const char* DESC_GSERR_ServerBusy                   = "服务器正忙";
static const char* DESC_DBERR_NO_ERR                       = "成功";
static const char* DESC_DBERR_Cannot_Find_Player           = "找不到玩家";
static const char* DESC_DBERR_PlayerFSM_Timeout            = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_DBERR_Account_Not_Exist            = "服务器正忙";
static const char* DESC_DBERR_RoleName_Duplicated          = "输入的角色名已存在，请您重新输入名字";
static const char* DESC_DBERR_Role_Not_Exist               = "服务器正忙";
static const char* DESC_DBERR_RoleName_Invalid             = "输入的角色名存在非法字符，\n请您重新输入名字";
static const char* DESC_DBERR_Role_Not_Online              = "服务器正忙";
static const char* DESC_DBERR_Role_Already_Online          = "服务器正忙";
static const char* DESC_DBERR_Account_Not_Online           = "服务器正忙";
static const char* DESC_DBERR_Account_Already_Online       = "服务器正忙";
static const char* DESC_DBERR_RoleNum_Full                 = "帐号角色个数已满。";
static const char* DESC_DBERR_Excute_Mysql_Failed          = "服务器正忙";
static const char* DESC_DBERR_Internal_Failure             = "服务器正忙";
static const char* DESC_DBERR_DB_Kickoff_ByReLogin         = "与服务器的连接被强行中断！\n连接时间可能超过限制，\n或者用户请求重新连接。";
static const char* DESC_DBERR_DelRole_Full                 = "每天只能删除一个角色";
static const char* DESC_DBERR_JumpGS_DestIsFull            = "无法连接到服务器";
static const char* DESC_MSERR_NO_ERR                       = "成功";

CodeDescMap gaMapArray[] = {
	{LGERR_NO_ERR, DESC_LGERR_NO_ERR},
	{LGERR_GameGate_Unavailable, DESC_LGERR_GameGate_Unavailable},
	{LGERR_SourceSvr_Unavailable, DESC_LGERR_SourceSvr_Unavailable},

	{LSERR_NO_ERR, DESC_LSERR_NO_ERR},
	{LSERR_Sys_Failure, DESC_LSERR_Sys_Failure},
	{LSERR_User_Lock, DESC_LSERR_User_Lock},
	{LSERR_State_Wrong, DESC_LSERR_State_Wrong},
	{LSERR_Already_Login, DESC_LSERR_Already_Login},
	{LSERR_Login_Timeout, DESC_LSERR_Login_Timeout},
	{LSERR_Auth_Failure, DESC_LSERR_Auth_Failure},
	{LSERR_PTID_Not_Exist, DESC_LSERR_PTID_Not_Exist},
	{LSERR_Old_Client, DESC_LSERR_Old_Client},
	{LSERR_Reserve_Overdue, DESC_LSERR_Reserve_Overdue},
	{LSERR_Other_Area, DESC_LSERR_Other_Area},
	{LSERR_Ekey_Warp, DESC_LSERR_Ekey_Warp},
	{LSERR_Has_Verfity, DESC_LSERR_Has_Verfity},
	{LSERR_Refuse, DESC_LSERR_Refuse},
	{LSERR_Only_For_TestAccount, DESC_LSERR_Only_For_TestAccount},
	{LSERR_Only_For_CreateRole, DESC_LSERR_Only_For_CreateRole},
	

	{GGERR_NO_ERR, DESC_GGERR_NO_ERR},
	{GGERR_LoginServer_Unreachable, DESC_GGERR_LoginServer_Unreachable},
	{GGERR_DBServer_Unreachable, DESC_GGERR_DBServer_Unreachable},
	{GGERR_MServer_Unreachable, DESC_GGERR_MServer_Unreachable},
	{GGERR_GameServer_Unreachable, DESC_GGERR_GameServer_Unreachable},
	{GGERR_Internal_Error, DESC_GGERR_Internal_Error},
	{GGERR_EnterGs_Failed, DESC_GGERR_EnterGs_Failed},
	{GGERR_IsLocked_forawhile, DESC_GGERR_IsLocked_forawhile},
	{GGERR_SelRole_Invalid, DESC_GGERR_SelRole_Invalid},
	{GGERR_RoleNum_Full, DESC_GGERR_RoleNum_Full},
	{GGERR_Not_InGame, DESC_GGERR_Not_InGame},
	{GGERR_In_Processing, DESC_GGERR_In_Processing},
	{GGERR_GameGate_Shutdown, DESC_GGERR_GameGate_Shutdown},
	{GGERR_Kickoff_InIdle, DESC_GGERR_Kickoff_InIdle},
	{GGERR_Kickoff_VerifyFailed, DESC_GGERR_Kickoff_VerifyFailed},
	{GGERR_NameDuplicated, DESC_GGERR_NameDuplicated},
	{GGERR_RoleStatusInvalid, DESC_GGERR_RoleStatusInvalid},
	{GGERR_Kickoff_DecodeFailed, DESC_GGERR_Kickoff_DecodeFailed},
	{GGERR_Kickoff_AreURobot, DESC_GGERR_Kickoff_AreURobot},
	{GGERR_RoleInviteInvalid, DESC_GGERR_RoleInviteInvalid},

	{GSERR_NO_ERR, DESC_GSERR_NO_ERR},
	{GSERR_ServerBusy, DESC_GSERR_ServerBusy},

	{DBERR_NO_ERR, DESC_DBERR_NO_ERR},
	{DBERR_Cannot_Find_Player, DESC_DBERR_Cannot_Find_Player},
	{DBERR_PlayerFSM_Timeout, DESC_DBERR_PlayerFSM_Timeout},
	{DBERR_Account_Not_Exist, DESC_DBERR_Account_Not_Exist},
	{DBERR_RoleName_Duplicated, DESC_DBERR_RoleName_Duplicated},
	{DBERR_Role_Not_Exist, DESC_DBERR_Role_Not_Exist},
	{DBERR_RoleName_Invalid, DESC_DBERR_RoleName_Invalid},
	{DBERR_Role_Not_Online, DESC_DBERR_Role_Not_Online},
	{DBERR_Role_Already_Online, DESC_DBERR_Role_Already_Online},
	{DBERR_Account_Not_Online, DESC_DBERR_Account_Not_Online},
	{DBERR_Account_Already_Online, DESC_DBERR_Account_Already_Online},
	{DBERR_RoleNum_Full, DESC_DBERR_RoleNum_Full},
	{DBERR_Excute_Mysql_Failed, DESC_DBERR_Excute_Mysql_Failed},
	{DBERR_Internal_Failure, DESC_DBERR_Internal_Failure},
	{DBERR_DB_Kickoff_ByReLogin, DESC_DBERR_DB_Kickoff_ByReLogin},
	{DBERR_DelRole_Full, DESC_DBERR_DelRole_Full},
	{DBERR_JumpGS_DestIsFull, DESC_DBERR_JumpGS_DestIsFull},

	{MSERR_NO_ERR, DESC_MSERR_NO_ERR},

};



const char* GetErrDesc(int nErrcode)
{
	int nSize = sizeof(gaMapArray)/sizeof(CodeDescMap);

	for(int i = 0; i < nSize; i++)
	{
		if (gaMapArray[i].nErrcode == nErrcode)
		{
			 return gaMapArray[i].pszDesc;
		}
	}
	return "Unknown Error";
}
