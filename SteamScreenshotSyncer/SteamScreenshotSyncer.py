import os
import json
import re
import threading
import time
import getpass
import base64
try:
    import requests
    from lxml import etree
    import rsa
    import vdf
except:
    print("缺失组件，正在尝试安装。。。")
    print("如果安装失败，请手动安装以下包：\nrequests\nlxml\nvdf\nrsa")
    os.system("pip install requests")
    os.system("pip install lxml")
    os.system("pip install rsa")
    os.system("pip install vdf")
    print("请重启脚本")
    os.system("pause>nul")
    exit()

#配置
MAX_THREAD=32    #并发线程数
MAX_TIMEOUT=5    #弱网适用，最大连接时间(秒)，超时会重新连接
MAX_DOWNLOADTIME=10    #弱网适用，每张截图的最大下载时间(秒)，超时会重新下载

def login():
    global username,password,twofactor,emailauth,screenshotsURL,isLogin
    rsaInfo = steamSession.request("get","https://steamcommunity.com/login/getrsakey/?username="+username,verify=False)
    rsajson=json.loads(rsaInfo.text)
    if rsajson==None:
        print("登陆失败，无法连接到Steam")
        os.system("pause>nul")
        exit(-1)
    if rsajson["success"]!=True:
        print("登陆失败，未知错误。")
        os.system("pause>nul")
        exit()
    pkey_mod=rsajson["publickey_mod"]
    pkey_exp=rsajson["publickey_exp"]
    timestamp=rsajson["timestamp"]
    pkey= rsa.PublicKey(int(pkey_mod,16),int(pkey_exp,16))
    encryptedpass=base64.b64encode(rsa.encrypt(bytes(password,encoding="utf8"),pkey))
    loginParams={
        "username":username,
        "password":encryptedpass,
        "twofactorcode":twofactor,
        "emailauth":emailauth,
        "loginfriendlyname":"",
        "captchagid":"",
        "captcha_text":"",
        "emailsteamid":"",
        "rsatimestamp":timestamp,
        "remember_login":"false",
    }
    result = steamSession.request("post","https://steamcommunity.com/login/dologin/",data=loginParams,verify=False)
    resultJson=json.loads(result.text)
    if resultJson==None:
        print("登陆失败，无法连接到Steam")
        os.system("pause>nul")
        exit(-1)
    if resultJson["success"]!=True:
        if resultJson["requires_twofactor"]!=None and resultJson["requires_twofactor"]==True:
            print("登陆失败，二步验证码不正确。")
            twofactor=input("请输入二步验证码:")
            login()
            return
        #elif resultJson["requires_emailauth"]!=None and resultJson["requires_emailauth"]==True:
        #    print("登陆失败，需要邮箱验证码")
        #    twofactor=input("请输入邮箱验证码:")
        #    login()
        #    return
        else:
            print(resultJson["message"])
            print("登陆失败")
            exit()
        #登陆成功
    print("登陆成功!")
    isLogin=True
    steamID=resultJson["transfer_parameters"]["steamid"]
    screenshotsURL="https://steamcommunity.com/profiles/"+steamID+"/screenshots/"

def getScreenshotFolder():
    import winreg
    reg_steam = winreg.OpenKeyEx(winreg.HKEY_CURRENT_USER,"SOFTWARE\\Valve\\Steam")
    if reg_steam==None:
        print("你的电脑看起来并没有安装Steam")
        os.system("pause>nul")
        exit()
    SteamPath = winreg.QueryValueEx(reg_steam,"SteamPath")[0]
    reg_users=winreg.OpenKeyEx(winreg.HKEY_CURRENT_USER,"SOFTWARE\\Valve\\Steam\\Users")
    userlist=[]
    try:
        i=0
        while True:
            userlist.append(winreg.EnumKey(reg_users,i))
            i+=1
    except: pass
    if len(userlist)==0:
        print("你的Steam看起来没有登陆过任何账号")
        print("找不到截图文件夹")
        os.system("pause>nul")
        exit()
    elif len(userlist)==1:
        return SteamPath+"\\userdata\\"+userlist[0]+"\\760\\remote"
    elif len(userlist)>1:
        defultAccountName = winreg.QueryValueEx(reg_steam,"LastGameNameUsed")[0]
        userdic={}
        for userIndex in range(len(userlist)):
            cfgPath=SteamPath+"\\userdata\\"+userlist[userIndex]+"\\config\\localconfig.vdf"
            if os.path.exists(cfgPath):
                fp=open(cfgPath,"r",encoding="utf8")
                cfgVdf = vdf.load(fp)
                fp.close()
                userdic[userIndex+1]=[cfgVdf["UserLocalConfigStore"]["friends"]["PersonaName"],userlist[userIndex]]
                del cfgVdf
            else:
                userdic[userIndex+1]=[userlist[userIndex],userlist[userIndex]]
        print("你的Steam登录过不止一个账户，你要把截图同步到哪一个账户里？")
        defaultIndex=1
        for userIndex in range(len(userlist)):
            if userdic[userIndex+1][0]==defultAccountName:
                print(str(userIndex+1)+". "+userdic[userIndex+1][0]+"(默认)")
                defaultIndex=userIndex+1
                defultAccountName=""
            else:
                print(str(userIndex+1)+". "+userdic[userIndex+1][0])
        selectedIndex=input()
        if selectedIndex.isnumeric()==False or (1>int(selectedIndex) or len(userlist)<=int(selectedIndex)):
            return SteamPath+"\\userdata\\"+userdic[defaultIndex][1]+"\\760\\remote"
        else:
            return SteamPath+"\\userdata\\"+userdic[int(selectedIndex)][1]+"\\760\\remote"

def collectExistedScreenshot():
    global existedScreenshot,syncFolder
    if os.path.exists(syncFolder+"\\..\\screenshots.vdf")==False:
        return
    fp=open(syncFolder+"\\..\\screenshots.vdf","r",encoding="utf8")
    screenshotCfg=vdf.load(fp)
    fp.close()
    for (app,app_content) in screenshotCfg["Screenshots"].items():
        if app.isnumeric()==False:
            continue
        for (screenshotIndex,screenshotItem) in app_content.items():
            if screenshotItem["hscreenshot"]!="18446744073709551615":  #图片已被上传
                existedScreenshot.append(screenshotItem["hscreenshot"])

def analyzePage(curPage):
    global screenshotDownloadList
    imgList_page = curPage.xpath("//a[contains(@class,\"profile_media_item\")]")
    for img_page in imgList_page:
        gameAppID = img_page.get("data-appid")
        publishedfileid=img_page.get("data-publishedfileid")
        if screenshotDownloadList.__contains__(gameAppID)==False:
            screenshotDownloadList[gameAppID]=[]
        screenshotDownloadList[gameAppID].append(publishedfileid)

class downloadWorkingThread(threading.Thread):
    def run(self):
        global screenshotDownloadList,cur_ss
        while True:
            threadlock.acquire()
            if len(screenshotDownloadList)==0:
                threadlock.release()
                return
            else:
                curKey=list(screenshotDownloadList.keys())[0]
                fileID = screenshotDownloadList[curKey].pop()
                if len(screenshotDownloadList[curKey])==0:
                    screenshotDownloadList.pop(curKey)
                threadlock.release()
            while(True):
                try:
                    filepage = steamSession.request("get","https://steamcommunity.com/sharedfiles/filedetails/?id="+str(fileID)+"&l=schinese",verify=False,timeout=MAX_TIMEOUT) 
                except:
                    continue
                if filepage.ok==True:
                    break
            filepage_html = etree.HTML(filepage.text)
            fileurl= filepage_html.xpath("//div[@class=\"actualmediactn\"]/a")[0].get("href")
            description_page=filepage_html.xpath("//div[@class=\"screenshotDescription\"]")
            description=""
            if len(description_page)!=0:
                description=description_page[0].text
            #判断本地是否已有这个截图
            match_result = gethscreenshotRe.search(fileurl)
            hscreenshot="0"
            if match_result!=None:
                hscreenshot=match_result.group(1)
            if existedScreenshot.__contains__(hscreenshot):
                continue
            dataPage=filepage_html.xpath("//div[@class=\"detailsStatsContainerRight\"]")[0]
            match_result=getresRe.search(dataPage[2].text)
            width = match_result.group(1)
            height=match_result.group(2)
            dateStr = dataPage[1].text
            if dateStr.find("年")==-1:
                dateStr=str(time.localtime(time.time()).tm_year) +"年"+dateStr
            match_result = gettimeRe.search(dateStr)
            year=int(match_result.group(1))
            month=int(match_result.group(2))
            day=int(match_result.group(3))
            ismorning=match_result.group(4)=="上"
            hour=int(match_result.group(5))+(12 if ismorning==False else 0)
            minute=int(match_result.group(5))
            while True:
                try:
                    screenshotData = steamSession.request("get",fileurl,verify=False,timeout=MAX_DOWNLOADTIME)
                except:
                    continue
                if screenshotData.ok==True:
                    break
            if syncMode==False:
                with open(syncFolder+"\\"+hscreenshot+".jpg","wb+") as f:
                    f.write(screenshotData.content)
                cur_ss+=1
            else:
                timestr = str(year)+str(month)+str(day)+str(hour)+str(minute)
                ss_index=1
                while os.path.exists(syncFolder+"\\"+curKey+"\\screenshots\\"+timestr+"00_"+str(ss_index)+".jpg"):
                    ss_index+=1
                os.makedirs(syncFolder+"\\"+curKey+"\\screenshots",exist_ok=True)
                with open(syncFolder+"\\"+curKey+"\\screenshots\\"+timestr+"00_"+str(ss_index)+".jpg","wb+") as f:
                    f.write(screenshotData.content)
                threadlock.acquire()
                if os.path.exists(syncFolder+"\\..\\screenshots.vdf"):
                    fp=open(syncFolder+"\\..\\screenshots.vdf","r",encoding="utf8")
                    screenshotCfg=vdf.load(fp)
                    fp.close()
                else:
                    screenshotCfg={"Screenshots":{}}
                if curKey not in screenshotCfg["Screenshots"]:
                    screenshotCfg["Screenshots"][curKey]={}
                targetIndex=len(screenshotCfg["Screenshots"][curKey].keys())
                screenshotCfg["Screenshots"][curKey][str(targetIndex)]={
                    "type":"1",
			        "filename":curKey+ "/screenshots/"+timestr+"00_"+str(ss_index)+".jpg",
                    "thumbnail":curKey+ "/screenshots/thumbnails/"+timestr+"00_"+str(ss_index)+".jpg",
                    "vrfilename":"",
    	    		"imported":"1",
	    	    	"width":width,
		    	    "height":height,
    	    		"gameid"	:curKey,
	    	    	"creation":str(int(time.mktime(time.strptime(str(year)+
                                                    "/"+str(month)+"/"+str(day)+"/"+str(hour)+"/"+str(minute),
                                    "%Y/%m/%d/%H/%M")))),
		    	    "caption":description,
			        "Permissions":"8",
			        "hscreenshot"	:hscreenshot,
                }
                with open(syncFolder+"\\..\\screenshots.vdf","w+",encoding="utf8") as f:
                    f.write(vdf.dumps(screenshotCfg))
                cur_ss+=1
                threadlock.release()

os.system("title 蛇牌Steam截图同步工具 by JackMyth")
requests.packages.urllib3.disable_warnings()
isLogin=False
screenshotsURL=""
syncMode=False
steamSession=requests.Session()
existedScreenshot=[]
print("请输入下载文件夹")
print("若留空，则会直接同步至Steam的截图文件夹")
syncFolder=input()
if syncFolder=="" or os.path.exists(syncFolder)!=True:
    print("将直接将截图同步至下载文件夹")
    syncMode=True
    syncFolder=getScreenshotFolder()
    os.makedirs(syncFolder,exist_ok=True)
    collectExistedScreenshot()
username=input("请输入用户名(留空则不登陆):")
if len(username)!=0:
    #登录进程
    password=getpass.getpass("请输入密码(密码不显示，直接输入即可):")
    twofactor=input("请输入二步验证码(无则留空):")
    emailauth=""
    print("尝试登陆...")
    login()
if screenshotsURL=="":
    steamID=input("请输入SteamID64或你的主页id(二者均可):")
    steamID=steamID.lstrip().rstrip()
    fullNumber=False
    if len(steamID)==17 and steamID.isnumeric():
        screenshotsURL="https://steamcommunity.com/profiles/"+steamID+"/screenshots/"
    else:
        screenshotsURL="https://steamcommunity.com/id/"+steamID+"/screenshots/"
privacy=30
if isLogin==True:
    privacyList={
        "1":30,
        "2":8,
        "3":4,
        "4":2,
        "5":16
    }
    print("你想要同步哪些截图？")
    print("1.所有的截图(默认)")
    print("2.公开的截图")
    print("3.仅好友可见的截图")
    print("4.私密的截图")
    print("5.非公开的截图")
    privacySelect=input()
    if privacyList.__contains__(privacySelect) == False:
        print("输入有误，将同步所有截图")
        privacy=privacyList["1"]
    else:
        privacy=privacyList[privacySelect]
print("选择要同步的游戏：")
print("1. 同步所有游戏(默认)")
print("2. 列出列表以便选择游戏")
syncGameSelect=input("如果你有游戏的AppID,也可以直接输入:")
if syncGameSelect=="" or syncGameSelect.isnumeric()==False or int(syncGameSelect)==1:
    targetAppID=0
elif int(syncGameSelect)==2:
    print("正在获取游戏列表...")
    while True:
        try:
            ss_page = steamSession.request("get",screenshotsURL,verify=False,timeout=MAX_TIMEOUT)
        except:
            continue
        if ss_page.ok==False:
            print("获取游戏列表失败，正在重新获取...")
        else:
            gamelist_re=re.compile("'appid'\\s*:\\s*'(\\d*)'")
            ss_html = etree.HTML(ss_page.text)
            gamelist_page = ss_html.xpath("//div[@id=\"sharedfiles_filterselect_app_filterable\"]/div[@class=\"option ellipsis\"]")
            gamelist=[]
            for gameitem_page in gamelist_page:
                gamename = gameitem_page.text
                gameAppID_js= gameitem_page.get("onclick")
                gameappid_match = gamelist_re.search(gameAppID_js)
                if gameappid_match!=None:
                    gamelist.append([gamename,gameappid_match.group(1)])
            for gameitemIndex in range(len(gamelist)):
                print(str(gameitemIndex+1)+". "+gamelist[gameitemIndex][0])
            while True:
                syncGameSelect=input("请选择要同步的游戏(也可以直接输入AppID):")
                if syncGameSelect.isnumeric() and int(syncGameSelect)>0 and int(syncGameSelect)<=len(gamelist):
                    targetAppID=gamelist[int(syncGameSelect)-1][1]
                    break
                else:
                    if syncGameSelect.isnumeric():
                        targetAppID=int(syncGameSelect)
                    else:
                        print("输入有误，请重新输入.")
            break
else:
    targetAppID=int(syncGameSelect)

max_page=1
cur_page=1
screenshotDownloadList={}
args={"appid":targetAppID,"p":1,"privacy":privacy,"browsefilter":"myfiles","sort":"newestfirst","view":"grid"}
print("正在获取截图列表...",end='')
while True:
    try:
        firstPage = steamSession.request("post",screenshotsURL,args,verify=False,timeout=MAX_TIMEOUT) 
        #302会导致SSL验证出错，所以不检查证书
    except:
        continue
    if firstPage.ok==False:
        print("\n获取内容出错，正在重试...")
    else:
        break
firstPage=etree.HTML(firstPage.text)
pagenumbers = firstPage.xpath("//a[@class=\"pagingPageLink\"]")
for pagenumber in pagenumbers:
    if int(pagenumber.text)>max_page:
        max_page=int(pagenumber.text)
analyzePage(firstPage)
for i in range(2,max_page+1):
    args["p"]=i
    print("\r正在获取截图列表......["+str(i)+"/"+str(max_page)+ "]"+" "*20+"\b"*20,end='')
    while True:
        try:
            firstPage = steamSession.request("post",screenshotsURL,args,verify=False,timeout=MAX_TIMEOUT)
        except:
            continue
        if firstPage.ok==False:
            print("\n获取内容出错，正在重试...")
        else:
            break
    firstPage=etree.HTML(firstPage.text)
    analyzePage(firstPage)
threadlock=threading.Lock()
gethscreenshotRe=re.compile("ugc/(\\d*)/")
gettimeRe=re.compile("(\\d*)年(\\d*)月(\\d*)日(\\S*)午(\\d*):(\\d*)")
getresRe=re.compile("(\\d*)\\s*x\\s*(\\d*)")
cur_ss=0
max_ss=0
ThreadList=[]
for (appid,content) in screenshotDownloadList.items():
    max_ss+=len(content)
for i in range(MAX_THREAD):
    cur_thread=downloadWorkingThread()
    cur_thread.start()
    ThreadList.append(cur_thread)
animL=["-","\\","|","/"]
animI=0
while True:
    print("\r正在下载截图["+str(cur_ss)+"/"+str(max_ss)+"]......"+animL[animI]+" "*20+"\b"*20,end='')
    animI=(animI+1)%4
    time.sleep(0.25)
    isFinished=True
    for thread in ThreadList:
        if thread.is_alive():
            isFinished=False
            break
    if isFinished:
        print("\b同步完成，任意键退出"+" "*20+"\b"*20)
        os.system("pause>nul")
        exit()