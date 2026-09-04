#pragma once

#include <cstdint>

// Console组件的固定区域行数
constexpr int32_t Default_Console_Fixed_Lines = 20;

/// console固定区域输出起始行位置
constexpr int32_t CONSOLE_FIXED_LINE_START_INDEX = 5;

/// PT返回数据存活时间
constexpr int32_t Default_Packet_In_Queue_Time_Long = 10000;

/// Message池初始大小
constexpr int32_t Default_Message_Init_Size = 300;

/// Message池增量大小
constexpr int32_t Default_Message_Grow_Size = 300;

// 默认Player池的增长大小为128
constexpr int32_t Default_PlayerPool_Grow_Size = 128;

// 默认登录错误限制次数为5次
constexpr int32_t Default_Login_Failed_Limited_Times = 5;

// 默认限制登录时间
constexpr int32_t Default_Limit_Login_Times = 300;

// session超时定时器命中次数限制：30次 
constexpr int32_t Default_Session_Check_Timer_HitCnt = 30;

// session超时定时器检测间隔：60秒
constexpr int32_t Default_Session_Check_Timer_Interval = 60000;

constexpr int32_t TIMER_UPDATE_CONSOLE = 3000;

const char gl_csHandle[] = "Handle";
const char gl_csPTID[] = "PTID";
const char gl_csAuthenID[] = "AuthenID";
const char gl_csGameId[] = "GameId";
const char gl_csReservePwd[] = "ReservePwd";
const char gl_csClientIP[] = "ClientIP";
const char gl_csAPType[] = "APType";
const char gl_csPlatType[] = "PlatType";
const char gl_csInviteCode[] = "InviteCode";
const char gl_csData[] = "Data";
const char gl_csTime[] = "Time";

enum
{
	PTErr_no_user = 1,
	PTErr_pwd_error,
	PTErr_refuse,
	PTErr_other_area,
	PTErr_reserve_overdue,

	COMMErr_old_version,
	COMMErr_forbidden,
	COMMErr_system_error,
};


enum
{
	TimerID_CheckSession_InIdle_1 = 1,
	TimerID_SendBeatTimer_2 = 2,
	TimerID_UpdateConsole_3 = 3,
};


enum
{
	EMSG_PTID_DIGIT_NAME = 1,
	EMSG_CARD_STAT_LOST = 2,
	EMSG_OPEN_FCM = 3,
	EMSG_PTID_BINDIDCARD = 4,
	EMSG_PTID_NOTADULT = 5,
	EMSG_PTID_BINDNOTHING = 6,
};
