#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from struct import pack, unpack, unpack_from
import string

import md5
import time
import json
import inspect
sys.path.append(".")
import xbxxz_pb2
from google.protobuf.json_format import MessageToJson, MessageToDict

incommingmsg = {
0:["NONE",None],
1:["MAINUSERDATA",xbxxz_pb2.t_MainUserProto],
18:["OPTION",xbxxz_pb2.t_OptionProto],
19:["ALLSYSOPEN",xbxxz_pb2.t_AllSysFuncOpenMessage_XBXXZ],
22:["SINGLESYSOPEN",xbxxz_pb2.t_SingleSysSettingDataMessage_XBXXZProto],
25:["CREATEUSEROK",xbxxz_pb2.t_CreateUserOKMessage_XBXXZ],
101:["OBJS",xbxxz_pb2.t_stAddObjectListUnityMessage_XBXXZProto],
102:["DELOBJS",xbxxz_pb2.t_DelObjMessage_XBXXZ],
103:["REFRESHOBJS",xbxxz_pb2.t_RefreshObjMessage_XBXXZ],
104:["MOVEOBJ",xbxxz_pb2.t_MoveObjMessage_XBXXZ],
105:["SPLITOBJ",None],
106:["PACKNUM",xbxxz_pb2.t_MainPackNumProto],
201:["CHAT",xbxxz_pb2.t_stMobileChannelChatMessage_XBXXZProto],
202:["ADDMAILLIST",xbxxz_pb2.t_MailAddListProto],
203:["MAILCONTENT",xbxxz_pb2.t_MailContentProto],
301:["SELECTUSERINFO",xbxxz_pb2.t_stUserInfoMessage_XBXXZProto],
302:["RANDNAME",xbxxz_pb2.t_stRandNameMessage_XBXXZ],
303:["ReturnLoginFailed",xbxxz_pb2.stServerReturnLoginFailedCmd],
401:["CLEARMAIL",None],
402:["DELMAIL",xbxxz_pb2.t_MailOpenMessage_XBXXZ],
501:["MAIN_DATAUPDATE",None],
502:["SORTLIST_REQUEST",None],
503:["COMMON_REWARD",None],
504:["POP_REWARD",None],
505:["ADDEXP",xbxxz_pb2.t_AddExpMessage_XBXXZ],
613:["LEVELUPNINE",xbxxz_pb2.t_LevelUPMessage_XBXXZ],
10001:["XIULIAN3MIN",xbxxz_pb2.t_XiuLianLeftTimeMessage_XBXXZ],
10002:["RETURNDOOM",xbxxz_pb2.t_ReturnDoomResultMessage_XBXXZ],
10003:["RETURN5SEC",xbxxz_pb2.t_Return5SecMessage_XBXXZ],
10004:["SPIRITNUM",xbxxz_pb2.t_SpiritNumMessage_XBXXZ],
10005:["CAVEINFO",xbxxz_pb2.t_CaveInfoMessage_XBXXZ],
10006:["CAVEVALUE5SEC",xbxxz_pb2.t_CaveValue5SecMessage_XBXXZ],
10007:["MAPLIST",xbxxz_pb2.t_MapListMessage_XBXXZ],
10008:["ONEMAPINFO",xbxxz_pb2.t_OneMapInfoMessage_XBXXZ],
10009:["NPCATTRESULT",xbxxz_pb2.t_NpcAttResultMessage_XBXXZ],
10010:["SCHOOLINFO",xbxxz_pb2.t_SchoolInfoMessage_XBXXZ],
10011:["SCHOOL5MIN",xbxxz_pb2.t_SchoolValue5MinMessage_XBXXZ],
10012:["BROADCAST",xbxxz_pb2.t_SaveBroadcastMessage_XBXXZ],
10013:["OFFLINEREWARD",xbxxz_pb2.t_OfflineRewardMessage_XBXXZ],
10014:["SHOPINFO",xbxxz_pb2.t_ShopInfoMessage_XBXXZ],
10015:["RECHARGEINFO",xbxxz_pb2.t_RechargeInfoMessage_XBXXZ],
10016:["RECHARGSANDBOX",xbxxz_pb2.t_RechargeSandBoxMessage_XBXXZ],
10017:["SCHOOLEXCHANGEINFO",xbxxz_pb2.t_SchoolExchangeInfoMessage_XBXXZ],
10018:["SCHOOLDATA",xbxxz_pb2.t_SchoolDataMessage_XBXXZ],
10019:["SCHOOLTASK",xbxxz_pb2.t_SchoolTaskInfoMessage_XBXXZ],
10020:["RECOMMENDLIST",xbxxz_pb2.t_RecommendListMessage_XBXXZ],
10021:["SCHOOLDANINFO",xbxxz_pb2.t_SchoolDanInfoMessage_XBXXZ],
10022:["SPEEDUPINFO",xbxxz_pb2.t_SchoolSpeedUpInfoMessage_XBXXZ],
10023:["AUTOMAPRESULT",xbxxz_pb2.t_AutoMapResultMessage_XBXXZ],
10024:["ALLAUTOMAP",xbxxz_pb2.t_AllAutoMapMessage_XBXXZ],
10025:["DRAGONINFO",xbxxz_pb2.t_DragonInfoMessage_XBXXZ],
10026:["DRAGONEVENT",xbxxz_pb2.t_DragonEventMessage_XBXXZ],
10027:["OTHERDRAGONLOG",xbxxz_pb2.t_OtherDragonEventMessage_XBXXZ],
10028:["LINGSHANENTERINFO",xbxxz_pb2.t_MapLingShanEnterInfoProto],
10029:["MAPENTERINFO",xbxxz_pb2.t_MapEnterInfoMessage_XBXXZ],
10030:["SCHOOLQIINFO",xbxxz_pb2.t_SchoolQiInfoMessage_XBXXZ],
10031:["UPSKILL",xbxxz_pb2.t_ReturnUpSkillMessage_XBXXZ],
10032:["REMOVESKILL",xbxxz_pb2.t_ReturnRemoveSkillMessage_XBXXZ],
10033:["USERPROPERTY",xbxxz_pb2.t_UserPropertyMessage_XBXXZ],
10034:["DAOTONGWASHINFO",xbxxz_pb2.t_DaoTongWashInfoMessage_XBXXZ],
10035:["DAOTONGALLINFO",xbxxz_pb2.t_DaoTongAllInfoMessage_XBXXZ],
10036:["ZHENFAINFO",xbxxz_pb2.t_ZhenFaInfoMessage_XBXXZ],
10037:["ROBSHOPINFO",xbxxz_pb2.t_RobShopInfoMessage_XBXXZ],
10038:["SINGLEROBPKINFO",xbxxz_pb2.t_SingleRobPKInfoMessage_XBXXZ],
10039:["FAIRYLANDINFO",xbxxz_pb2.t_FairyLandInfoMessage_XBXXZ],
10040:["SINGLEFAIRTLANDPKINFO",xbxxz_pb2.t_SinglePKFairyLandMessage_XBXXZ],
10041:["FAIRYLANDSHOPINFO",xbxxz_pb2.t_FairyLandShopInfoMessage_XBXXZ],
10042:["SHOPEXTRALREWARD",xbxxz_pb2.t_ShopExtralRewardInfoMessage_XBXXZ],
10043:["ALLPROPERTYOBJUSETIMES",xbxxz_pb2.t_PropertyObjUseTimesMessage_XBXXZ],
10044:["KINGINFO",xbxxz_pb2.t_KingManagerInfoMessage_XBXXZ],
10045:["GONGDETASKINFO",xbxxz_pb2.t_GongDeTaskInfoMessage_XBXXZ],
10046:["GONGDETASKPK",xbxxz_pb2.t_GongDeTaskPKMessage_XBXXZ],
10047:["FAIRYLANDOFFICERINFO",xbxxz_pb2.t_FairyLandOfficerInfoMessage_XBXXZ],
10048:["OFFICERAPLYLIST",xbxxz_pb2.t_OfficerApplyListMessage_XBXXZ],
10049:["AUTOMAPONLINEEND",None],
10050:["YAOTIANAUTOMAPONLINEEND",None],
10051:["PUREBODYINFO",xbxxz_pb2.t_PureBodyInfoMessage_XBXXZ],
10052:["ENLIGHTENMENTINFO",xbxxz_pb2.t_EnlightenmentInfoMessage_XBXXZ],
10053:["ENLIGHTENMENTRESULT",xbxxz_pb2.t_EnlightenmentResultMessage_XBXXZ],
10054:["YEARGIFTREWARDINFO",xbxxz_pb2.t_YearGiftRewardInfoMessage_XBXXZ],
10055:["STAROFFICERINFO",xbxxz_pb2.t_StarOfficerInfoMessage_XBXXZ],
10056:["PKRESULTSTAROFFICER",xbxxz_pb2.t_PKResultStarOfficerInfoMessage_XBXXZ],
10057:["GUIDFAIRYLANDINFO",xbxxz_pb2.t_GuidFairyLandInfoMessage_XBXXZ],
10058:["PKRESULTGUID",xbxxz_pb2.t_PKResultGuidInfoProto],
10059:["GUIDREQUEST",xbxxz_pb2.t_GuidRequstInfoProto],
10060:["COMBINEXCHANGEINFO",xbxxz_pb2.t_CombinExchangeInfoMessage_XBXXZ],
10061:["WORLDTRAVERINFO",xbxxz_pb2.t_worldTraverInfoMessage_XBXXZ],
10062:["PKRESULTWORLDTRAVER",xbxxz_pb2.t_PKResultWorldTraverInfoMessage_XBXXZ],
10063:["WORLDEVENTLOGINFO",xbxxz_pb2.t_WorldEventLogInfoMessage_XBXXZ],
10064:["FABAOINFO",xbxxz_pb2.t_fabaoInfoMessage_XBXXZ],
10065:["REFINEFABAOINFO",xbxxz_pb2.t_refineFaBaoInfoMessage_XBXXZ],
10066:["FAIRYPETINFO",xbxxz_pb2.t_FairyPetInfoMessage_XBXXZ],
10067:["HATCHFAIRYPETINFO",xbxxz_pb2.t_hatchFairyPetInfoMessage_XBXXZ],
10068:["CHRISTMASGIFTINFO",xbxxz_pb2.t_christmasGiftInfoMessage_XBXXZ],
10069:["GENERALINFO",xbxxz_pb2.t_generalInfoMessage_XBXXZ],
10070:["KUNLUNSTOREINFO",xbxxz_pb2.t_kunlunStoreInfoMessage_XBXXZ],
10071:["XIANMOINFO",xbxxz_pb2.t_XianMoInfoMessage_XBXXZ],
10072:["XIANMOLEFTTIME",xbxxz_pb2.t_XianMoLeftTimeMessage_XBXXZ],
10073:["NEWYEARGIFTTIME",xbxxz_pb2.t_NewYearGiftTimeMessage_XBXXZ],
10074:["NEWYEARGIFTINFO",xbxxz_pb2.t_NewYearGiftInfoMessage_XBXXZ],
10075:["NEWYEARSTOREINFO",xbxxz_pb2.t_NewYearStoreInfoMessage_XBXXZ],
10076:["IMMORTALGIFTINFO",xbxxz_pb2.t_ImmortalGiftInfoMessage_XBXXZ],
10077:["PKRESULTACTIVEMAP",xbxxz_pb2.t_PKResultActiveMapMessage_XBXXZ],
10078:["AUTOSPECIALMAPEND",None]
}

outgoingmsg={
0:["NONE",None],
5:["OPTION",xbxxz_pb2.t_OptionProto],
9:["GAMECENTER",xbxxz_pb2.t_AppleGameCenterMessage_XBXXZ],
101:["USEOBJS",xbxxz_pb2.t_UserItemMessage_XBXXZ],
102:["MOVEOBJ",xbxxz_pb2.t_MoveObjMessage_XBXXZ],
103:["SPLITOBJ",None],
104:["SORTOBJS",None],
201:["CHAT",xbxxz_pb2.t_stMobileChannelChatMessage_XBXXZProto],
202:["GETMAILLIST",None],
203:["OPENMAIL",xbxxz_pb2.t_MailOpenMessage_XBXXZ],
204:["GETMAILITEM",xbxxz_pb2.t_MailOpenMessage_XBXXZ],
205:["DELMAIL",xbxxz_pb2.t_MailOpenMessage_XBXXZ],
206:["CLEARMAIL",None],
207:["SENDMAIL",None],
301:["GETRANDNAME",xbxxz_pb2.t_stGetRandNameMessage_XBXXZ],
302:["CREATEUSER",xbxxz_pb2.t_stCreateSelectMessage_XBXXZProto],
303:["LOGINSELECT",xbxxz_pb2.t_LoginSelectMessage_XBXXZ],
401:["SORTLIST_REQUEST",None],
10001:["JINGJIEUPLEVEL",None],
10002:["ROUSHENUPLEVEL",None],
10003:["DUJIEUPLEVEL",None],
10004:["XIULIAN3MIN",None],
10005:["REQ5SEC",xbxxz_pb2.t_ReqFiveSecMessage_XBXXZ],
10006:["JULINUPLEVEL",None],
10007:["LINGENUPLEVEL",xbxxz_pb2.t_UpLinGenMessage_XBXXZ],
10008:["CAVELEVEL",xbxxz_pb2.t_UpLevelCaveMessage_XBXXZ],
10009:["BUYPERSON",None],
10010:["CAVEPUTPERSON",xbxxz_pb2.t_PutPersonCaveMessage_XBXXZ],
10011:["ENTERMAP",xbxxz_pb2.t_EnterMapMessage_XBXXZ],
10012:["MOVEMAP",xbxxz_pb2.t_MoveMapMessage_XBXXZ],
10013:["NPCATT",None],
10014:["CREATESCHOOL",None],
10015:["LEAVESCHOOL",None],
10016:["UPSKILL",xbxxz_pb2.t_UpSkillMessage_XBXXZ],
10017:["BUYSKILLPOINT",None],
10018:["STUDYSKILL",None],
10019:["REMOVESKILL",xbxxz_pb2.t_RemoveSkillMessage_XBXXZ],
10020:["RETREAT",None],
10021:["SELLOBJ",xbxxz_pb2.t_SellObjMessage_XBXXZ],
10022:["BUYOBJ",xbxxz_pb2.t_BuyObjMessage_XBXXZ],
10023:["REQSHOPINFO",xbxxz_pb2.t_ReqOpenShopMessage_XBXXZ],
10024:["REQSCHOOLINFO",None],
10025:["SCHOOLEXCHANGE",xbxxz_pb2.t_ReqSchoolExchangeMessage_XBXXZ],
10026:["SCHOOLDUTYUP",xbxxz_pb2.t_ReqSchoolUpLevelMessage_XBXXZ],
10027:["SCHOOLSKILLBOOK",None],
10028:["SCHOOLDAILYREWARD",None],
10029:["SCHOOLTAKETASK",xbxxz_pb2.t_ReqSchoolTaskMessage_XBXXZ],
10030:["REQRECOMMENDLIST",None],
10031:["RECOMMENDCODE",xbxxz_pb2.t_RecommendCodeMessage_XBXXZ],
10032:["DANSTUDY",xbxxz_pb2.t_SchoolDanStudyMessage_XBXXZ],
10033:["STARTDAN",xbxxz_pb2.t_SchoolStartDanMessage_XBXXZ],
10034:["GETDAN",None],
10035:["STARTSPEEDUP",xbxxz_pb2.t_SchoolStartSpeedUpMessage_XBXXZ],
10036:["AUTOMAP",xbxxz_pb2.t_ReqAutoMapMessage_XBXXZ],
10037:["REQNEXTAUTO",None],
10038:["REQENTERDRAGON",xbxxz_pb2.t_ReqEnterDragonMessage_XBXXZ],
10039:["REQBUYLINGSHAN",None],
10040:["BUYAUTOBOOK",xbxxz_pb2.t_BuyAutoBookMessage_XBXXZ],
10041:["QISTUSY",xbxxz_pb2.t_SchoolQiStudyMessage_XBXXZ],
10042:["STARTQI",xbxxz_pb2.t_SchoolStartQiMessage_XBXXZ],
10043:["GETQI",None],
10044:["GETUSERPROPERTY",None],
10045:["BUYAUTOSEL",None],
10046:["DAOTONGWASHUPLEVEL",None],
10047:["DAOTONGSTARTWASH",xbxxz_pb2.t_DaoTongStartWashMessage_XBXXZ],
10048:["DAOTONGREMOVE",xbxxz_pb2.t_DaoTongRemoveMessage_XBXXZ],
10049:["DAOTONGSTARTWORK",xbxxz_pb2.t_DaoTongStartWorkMessage_XBXXZ],
10050:["DAOTONGGETREWARD",xbxxz_pb2.t_DaoTongGetRewardMessage_XBXXZ],
10051:["DAOTONGWASHGETREWARD",None],
10052:["CHANGECAVENAME",xbxxz_pb2.t_ChangeCaveNameMessage_XBXXZ],
10053:["SOULUPLEVEL",None],
10054:["ZHENFASTUDY",xbxxz_pb2.t_ZhenFaStudyMessage_XBXXZ],
10055:["REQROBSHOPINFO",xbxxz_pb2.t_ReqRobShopInfoMessage_XBXXZ],
10056:["SINGLEROBSHOP",xbxxz_pb2.t_SingleRobShopMessage_XBXXZ],
10057:["GETSINGLEROBREWARD",xbxxz_pb2.t_GetSigleRobRewardMessage_XBXXZ],
10058:["REQFAIRYLADNINFO",xbxxz_pb2.t_ReqFairyLandInfoMessage_XBXXZ],
10059:["SINGLEFAIRYLADN",xbxxz_pb2.t_SingleFairyLandMessage_XBXXZ],
10060:["FAIRYLANDBUYOBJ",xbxxz_pb2.t_FairyLandBuyObjMessage_XBXXZ],
10061:["FAIRYLANDREQSHOPINFO",xbxxz_pb2.t_ReqOpenFairyLandShopMessage_XBXXZ],
10062:["SHOPGETEXTERREWARD",xbxxz_pb2.t_GetShopExteralRewardMessage_XBXXZ],
10063:["REQPROPERTYOBJUSETIMES",None],
10064:["DRAGONEXCHANGE",xbxxz_pb2.t_DragonExchangeMessage_XBXXZ],
10065:["YAOTIANREQNEXTAUTO",None],
10066:["NEXTDAYDRAGON",None],
10067:["REQKINGMANAGER",xbxxz_pb2.t_ReqKingManagerMessage_XBXXZ],
10068:["GETKINGDAILYREWARD",xbxxz_pb2.t_ReqKingDailyRewardMessage_XBXXZ],
10069:["REQGONGDETASK",xbxxz_pb2.t_ReqGongDeTaskMessage_XBXXZ],
10070:["FIRSTPASSGONGDETASK",xbxxz_pb2.t_ReqFirstPassGongDeTaskMessage_XBXXZ],
10071:["STARTGONGDETASK",xbxxz_pb2.t_StartGongDeTaskMessage_XBXXZ],
10072:["REQFAIRYLANDOFFICER",xbxxz_pb2.t_ReqFairyLandOfficerMessage_XBXXZ],
10073:["APPLYFAIRYLANDOFFICER",xbxxz_pb2.t_ApplyFairyLandOfficerMessage_XBXXZ],
10074:["CANCELFAIRYLANDOFFICER",xbxxz_pb2.t_CancelFairyLandOfficerMessage_XBXXZ],
10075:["YESORNOFAIRYLANDOFFICER",xbxxz_pb2.t_YesORNoFairyLandOfficerMessage_XBXXZ],
10076:["REQOFFICERAPPLYLIST",xbxxz_pb2.t_ReqOfficerApplyListMessage_XBXXZ],
10077:["BUYPUREBODYSTUDY",None],
10078:["STARTPUREBODY",None],
10079:["EXCHANGEPUREBODY",xbxxz_pb2.t_ExchangePureBodyMessage_XBXXZ],
10080:["STARTENLIGHTENMENT",xbxxz_pb2.t_StartEnlightenmentMessage_XBXXZ],
10081:["GETYEARGIFT",xbxxz_pb2.t_GetYearGiftMessage_XBXXZ],
10082:["REQSTAROFFICER",xbxxz_pb2.t_reqStarOfficerMessage_XBXXZ],
10083:["PKSTAROFFICER",xbxxz_pb2.t_reqPKStarOfficerMessage_XBXXZ],
10084:["CANCELSTAROFFICER",xbxxz_pb2.t_CancelStarOfficerMessage_XBXXZ],
10085:["PASSGUIDFAIRYLAND",xbxxz_pb2.t_reqPassGuidFairyLandMessage_XBXXZ],
10086:["SELECTGUIDFAIRYLAND",xbxxz_pb2.t_selectGuidFairyLandMessage_XBXXZ],
10087:["STARTGUIDFAIRYLAND",xbxxz_pb2.t_startGuidFairyLandMessage_XBXXZ],
10088:["YESORNOGUIDFAIRYLAND",xbxxz_pb2.t_yesOrNoGuidFairyLandMessage_XBXXZ],
10089:["PKGUIDFAIRYLAND",xbxxz_pb2.t_reqPKGuidFairyLandMessage_XBXXZ],
10090:["REQCHANGEFACE",xbxxz_pb2.t_reqChangeFaceMessage_XBXXZ],
10091:["REQCOMBINEXCHANGE",None],
10092:["STARTCOMBINEXCHANGE",xbxxz_pb2.t_startCombinExchangeMessage_XBXXZ],
10093:["REQWORLDTRAVERINFO",xbxxz_pb2.t_reqWorldTraverInfoMessage_XBXXZ],
10094:["STARTWORLDTRAVER",xbxxz_pb2.t_startWorldTraverMessage_XBXXZ],
10095:["PASSWORLDTRAVER",xbxxz_pb2.t_passWorldTraverMessage_XBXXZ],
10096:["GETREWARDWORLDTRAVER",xbxxz_pb2.t_getrewardWorldTraverMessage_XBXXZ],
10097:["REQOPENFABAOINFO",None],
10098:["LOCKFABAO",xbxxz_pb2.t_lockFaBaoMessage_XBXXZ],
10099:["UNLOCKFABAO",xbxxz_pb2.t_unLockFaBaoMessage_XBXXZ],
10100:["FEEDFABAO",xbxxz_pb2.t_feedFaBaoMessage_XBXXZ],
10101:["OPENFABAOPOS",None],
10102:["FABAOIDENTIFY",None],
10103:["GETIDENTIFYFABAO",None],
10104:["REFINEFABAO",xbxxz_pb2.t_refineFaBaoMessage_XBXXZ],
10105:["GETREFINEFABAO",None],
10106:["REQFABAOIDENTIFY",None],
10107:["ONEKEYALLDAOTONG",None],
10108:["REQFAIRYPETINFO",None],
10109:["HATCHFAIRYPET",xbxxz_pb2.t_hatchFairyPetMessage_XBXXZ],
10110:["GETHATCHFAIRYPET",None],
10111:["FEEDFAIRYPET",xbxxz_pb2.t_feedFairyPetMessage_XBXXZ],
10112:["SITFAIRYPET",xbxxz_pb2.t_sitFairyPetMessage_XBXXZ],
10113:["FIGHTFAIRYPET",xbxxz_pb2.t_fightFairyPetMessage_XBXXZ],
10114:["DELFAIRYPET",xbxxz_pb2.t_delFairyPetMessage_XBXXZ],
10115:["RENAMEFAIRYPET",xbxxz_pb2.t_renameFairyPetMessage_XBXXZ],
10116:["UPLEVELPETPOOL",None],
10117:["EMPTYFAIRYPET",xbxxz_pb2.t_emptyFairyPetMessage_XBXXZ],
10118:["REQBUYFAIRYPETLAND",None],
10119:["GETCHRISTMASGIFT",xbxxz_pb2.t_getChristmasGiftMessage_XBXXZ],
10120:["REQBUYDEADPOOL",None],
10121:["REQBUYMAGICHOUSE",None],
10122:["REQGENERAL",xbxxz_pb2.t_reqGeneralMessage_XBXXZ],
10123:["REQKUNLUNSTORE",None],
10124:["EXCHANGEKUNLUNSTORE",xbxxz_pb2.t_exchangeKunLunStoreMessage_XBXXZ],
10125:["ONEKEYGETREWARDDAOTONG",None],
10126:["XIANMOCHOOSE",xbxxz_pb2.t_chooseXianMoMessage_XBXXZ],
10127:["XIANMOSKILL",xbxxz_pb2.t_learnSkillXianMoMessage_XBXXZ],
10128:["XIANMOUPLEVEL",None],
10129:["CANCELLIANDAN",None],
10130:["CANCELLIANQI",None],
10131:["REQNEWYEARGIFTINFO",None],
10132:["GETNEWYEARGIFTONE",None],
10133:["GETNEWYEARGIFTTWO",xbxxz_pb2.t_GetNewYearGiftTwoMessage_XBXXZ],
10134:["GETNEWYEARGIFTTHREE",None],
10135:["GETNEWYEARGIFTFOUR",None],
10136:["GETNEWYEARGIFTFIVE",xbxxz_pb2.t_GetNewYearGiftFiveMessage_XBXXZ],
10137:["REQNEWYEARSTORE",None],
10138:["EXCHANGNEWYEARSTORE",xbxxz_pb2.t_exchangeNewYearStoreMessage_XBXXZ],
10139:["GETNEWYEARGIFTSIX",None],
10140:["CANCELTIANJIANG",None],
10141:["REQIMMORTALGIFT",None],
10142:["CANCELKING",None],
10143:["REQPKACTIVEMAP",xbxxz_pb2.t_reqPKActiveMapMessage_XBXXZ],
10444:["AUTOACTIVEMAP",xbxxz_pb2.t_autoActiveMapMessage_XBXXZ],
}

import socket
class Game():
    def loadtsv(self, filename):
        retobj = {}
        fd = open(filename)
        headers = fd.readline().strip().split("\t")
        for line in fd.readlines():
            seps = line.strip().split("\t")
            seps = [int(a) if a.isdigit() else a for a in seps]
            retobj[seps[0]] = dict(zip(headers, seps))
        return retobj
    
    def __init__(self):
        self.ItemBase = self.loadtsv("ab\\ItemBase.txt")
        
    def sendpkt(self, s, cmd, param, payload):
        size = 6 + len(payload)
        buf = pack("<IBBI", size, cmd, param, 0) + payload
        s.send(buf)
    
    def readbytes(self, s, size):
        buffer=""
        toread = size
        while toread > 0:
            data = s.recv(toread)
            if len(data) == 0:
                raise Exception("close by peer")
            toread -= len(data)
            buffer += data
        return buffer
        
    
    def readpkt(self, s):
        buf = self.readbytes(s, 4)
        size = unpack("<I", buf)[0]
        buf = self.readbytes(s, 6)
        cmd, param, ts = unpack("<BBI", buf)
        payload = self.readbytes(s, size-6)
        return cmd, param, payload
        
    def login(self):
        self.state = "init"
        self.timeremain = 60
        
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect(("193.112.170.89",15069))
        
        payload = pack("<II", 0, 1999)
        self.sendpkt(s, 2, 1, payload)
        
        sid = "appid=&openid=a991bf5c45eeee943e00bcc4bd43daa7&timestamp=%d&platform=guest&server_id=11&access_token="%(int(time.time()))
        sid = sid + "&sign=" + md5.new("5720902420ca230e3ed4767a06890bdf&" + sid).hexdigest().upper()
        payload = pack("<H", len(sid)) + sid
        self.sendpkt(s, 2, 2, payload)
        
        cmd, param, payload = self.readpkt(s)
        if cmd == 2 and param == 4:
            userid, loginid, domain, port = unpack_from("<QI65sH", payload, 0)
            self.userid = userid
            self.loginid = loginid
            self.serveraddr = (domain.strip("\0"), port)
            print "Login Success"
        s.close()
    
    def sendomsg(self, msgid, msg):
        data = msg.SerializeToString()
        msgsize = len(data)
        self.sendpkt(self.conn, 53, 2, pack("<II", msgid, msgsize) + data)
    
    def SELECTUSERINFO(self, d):
        p = xbxxz_pb2.t_LoginSelectMessage_XBXXZ()
        p.isReconected = 0
        self.sendomsg(303, p)

    def OBJS(self, d):
        for item in d.userset:
            self.items[item.obj.qwThisID] = item.obj
            #print("%d %s*%d objectid:%d position:%d,%d"%(item.obj.qwThisID, item.obj.strName, item.obj.dwNum, item.obj.dwObjectID, item.obj.pos.dwLocation, item.obj.pos.x))

    def MAPENTERINFO(self, d):
        self.enterinfo = d
        print "MAPENTERINFO totalentertimes", d.totalentertimes
        
    def AUTOMAPONLINEEND(self, d):
        print "AUTOMAPONLINEEND" 
        
    def MAINUSERDATA(self, d):
        self.userdata = d
    
    def AUTOMAPRESULT(self, d):
        if d.mapid >= 2000:
            return
        types = [0]*32
        for step in d.allsteps:
            types[step.type] += 1
        print "automapresult %d/%d"%(d.curcount, d.totalcount), json.dumps(types, ensure_ascii=False)
        print "automapresult endtime =", d.endtime 
        if d.endtime == 0:
            return
        self.timeremain = d.endtime + 5
        raise Exception("breakexception")
    
    def CHAT(self, d):
        print d.pstrChat
    
    def DoAutoMap(self):
        totalentertimes = self.enterinfo.totalentertimes
        maxturntimes = self.enterinfo.maxturntimes
        maxautotimes = self.enterinfo.maxautotimes
        remainingturntimes = maxturntimes - totalentertimes
        #1035 玄明天小世界 玄晶石+蓝晶石
        #1047 太极天小世界 黑曜石
        
        mapid1 = 1035
        mapid2 = 1035
        
        if remainingturntimes <= 0:
            self.timeremain = 86400 - (int(time.time()+28800)%86400) + 60
            raise Exception("breakexception")
        
        maxautotimes = min(maxautotimes, remainingturntimes)
        if totalentertimes >= maxturntimes/2:
            p = xbxxz_pb2.t_ReqAutoMapMessage_XBXXZ()
            p.mapid = mapid2
            p.count = maxautotimes
            self.sendomsg(10036, p)
            self.doautomap = True
            print "Automap(%d,%d)"%(mapid2, maxautotimes)
        else:
            p = xbxxz_pb2.t_ReqAutoMapMessage_XBXXZ()
            p.mapid = mapid1
            p.count = maxautotimes
            self.sendomsg(10036, p)            
            self.doautomap = True
            print "Automap(%d,%d)"%(mapid2, maxautotimes)
    
    def DoSellDummy(self):
        for qwThisID in self.items:
            item = self.items[qwThisID]
            itemid = item.dwObjectID
            itemnum = item.dwNum
            
            if itemid >=12 and itemid <=42:
                p = xbxxz_pb2.t_SellObjMessage_XBXXZ()
                p.objthisid = qwThisID
                p.num = itemnum
                self.sendomsg(10021, p)
                print "Sell %s*%d"%(item.strName, itemnum) 
            
    def DoAI(self):
        if self.state == "init":
            self.starttime = time.time()
            self.state = "waitinittime"
        elif self.state == "waitinittime":
            if time.time() - self.starttime > 10:
                self.DoSellDummy()
                time.sleep(1)
                self.DoAutoMap()
                self.state = "waitfinishtime"
        elif self.state == "waitfinishtime":
            if time.time() - self.starttime > 20:
                raise Exception("break exception")
    
    def decodeimsg(self, payload):
        msgid, size = unpack_from("<II", payload, 0)
        data = payload[8 : 8 + size]
        if msgid in incommingmsg:
            cls = incommingmsg[msgid][1]
            msgname = incommingmsg[msgid][0]
            d = None
            if cls != None:
                d = incommingmsg[msgid][1]()
                d.ParseFromString(data)
                if hasattr(self, msgname):
                    getattr(self, msgname)(d)
                #print "%f %s %s"%(time.time(), msgname, json.dumps(json.loads(MessageToJson(d)), ensure_ascii=False))
        else:
            #print "%f %s(%d:%d)"%(time.time(), "unknown", msgid, size)
            pass
                        
    def mainloop(self):
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn.settimeout(10)
        self.conn.connect(self.serveraddr)
        self.sendpkt(self.conn, 2, 1, pack("<II", 0, 1999))
        self.sendpkt(self.conn, 2, 5, pack("<IQ4", self.loginid, self.userid) + "\0"*24)
        self.items = {}
        while True:
            self.DoAI()
            cmd, param, payload = self.readpkt(self.conn)
            if cmd == 2:
                print cmd, param, payload
            else:
                self.decodeimsg(payload)

game = Game()
while True:
    print time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), "Connecting..."
    try:
        game.login()
        game.mainloop()
    except Exception as e:
        print str(e)
    print "Sleep %d seconds..."%(game.timeremain)
    time.sleep(game.timeremain)
    

