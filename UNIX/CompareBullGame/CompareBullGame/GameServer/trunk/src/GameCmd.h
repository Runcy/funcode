#ifndef GAMECMD_H
#define GAMECMD_H

//=================私人房=============================//
const int CLIENT_CREATE_PRIVATE			= 0x0011;
const int CLIENT_GET_PRIVATELIST		= 0x0012;
const int CLIENT_ENTER_PRIVATE			= 0x0013;

const int SERVER_MSG_ROBOT				= 0x0244; //给机器人发送详细信息

//================客户端命令=======================//
const int USER_HEART_BEAT			    = 0x0101;  //用户心跳，设置过期时间

const int TRANS_GAME_SERVER_START		= 0x0200;

const int CLIENT_MSG_LOGINCOMING		= 0x0200; //用户登录并且进入房间
const int CLIENT_MSG_START_GAME 		= 0x0201;  //用户准备
const int GMSERVER_GAME_START			= 0x0202; //游戏开始发前面三张牌
const int CLIENT_MSG_CALL_BANKER        = 0x0203; //抢庄
const int GMSERVER_GAME_CONFIRM			= 0x0204; //确定荘家推送
const int CLIENT_MSG_SET_NUL			= 0x0205; //玩家翻倍
const int GMSERVER_MSG_OPEN				= 0x0206; //通知玩家要开牌了
const int CLIENT_MSG_OPEN_CARD			= 0x0207; //玩家开牌
const int GMSERVER_MSG_GAMEOVER			= 0x0208; //游戏结束
const int CLIENT_MSG_CHAT				= 0x0209; //用户聊天
const int CLIENT_MSG_HELP				= 0x020A; //提醒

const int GMSERVER_WARN_KICK			= 0x020D; //游戏服务器通知用户快要踢出他了
const int CLIENT_GET_TASK				= 0x020E; //用户拉取任务信息
const int GMSERVER_COM_TASK				= 0x020F; //通知用户有任务完成了

const int CLIENT_MSG_LEAVE              = 0x0250; //用户离开
const int CLIENT_MSG_TABLEDET			= 0x0251; //用户获取牌局详细信息
const int CLIENT_MSG_LOGINCHANGE		= 0x0252; //登录并进入换桌房间
const int SERVER_MSG_KICKOUT			= 0x0253; //游戏服务器踢出用户命令
const int CLIENT_MSG_LOGOUT             = 0x020C; //用户离开

const int UPDATE_MONEY					= 0x0462;//更新金币信息
const int UPDATE_SAFEMONEY				= 0x0463;//更新保险箱信息
const int UPDATE_ROLL					= 0x0464;//更新乐劵
const int UPDATE_ROLL1					= 0x0465;//更新过期乐劵
const int UPDATE_COIN					= 0x0466;//更新乐币
const int UPDATE_EXP					= 0x0467;//更新乐币
const int UPDATE_LEAVE_GAME_SERVER		= 0x0468;//用户离开游戏服务器


#endif
