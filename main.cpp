#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cJSON.h"
#include "cJSON.c"

// 首先，你需要将下面SitePath和MirrorURL两个变量替换为你的网站
// 当然，这还没完，你需要反向代理https://raw.rat.dev/topjohnwu/magisk-files/到你网站的/stub目录下
// 还需要反向代理https://topjohnwu.github.io/Magisk/releases/到你网站的/notes目录下
// 这两个文件由于实在太小，我没有设置镜像，而是简单的反向代理，谅解一下
// 你可以使用宝塔面板，可以很容易的配置完成
// 请使用计划任务自动运行，运行频率由你来定，编译命令：g++ main.cpp -O3，二进制文件名为a.out，你可以自己来定二进制文件的名字

using namespace std;

int start,final;
char VerData[128];
const char* SitePath = "/www/wwwroot/mirror.lmfty.com/magisk";
const char* MirrorURL = "https://mirror.lmfty.com/magisk";

char* ReadFile(char* filename)
{
    char* text;
    FILE *pf = fopen(filename,"r");
    fseek(pf,0,SEEK_END);
    long lSize = ftell(pf);
    text=(char*)malloc(lSize+1);
    rewind(pf);
    fread(text,sizeof(char),lSize,pf);
    text[lSize] = '\0';
    return text;
}

int OverwriteFile(char* FileName,const char* Content){
    printf("\nOutputFile:");
    printf(FileName);
    FILE* fp;
    fp = fopen(FileName,"w+");
    if (fp == NULL){
        printf("\nFailed");
        exit(1);
    }
    fprintf(fp,Content);
    fclose(fp);
    printf("\nSuccess");
    return 0;
}

// Return true if exists.
int FileExist(const char* FileName){
    FILE *fp;
    fp=fopen(FileName , "r");
    if (fp == NULL){
        return 0;
    }
    else{
        fclose(fp);
        return 1;
    }
}

int getPosition(const char* InputString){
    int i[2] = {0,0};
    while(true){
        if (InputString[i[0]] == '\"') break;
        i[0]++;
        if(i[0] >= 50){
            return 1;
        }
    }
    while(true){
        if (InputString[i[1]] == '/') break;
        i[1]++;
        if(i[1] >= 50){
            return 1;
        }
    }
    i[1]++;
    i[0] = i[0] - i[1];
    start = i[1]+1;
    final = i[0]-1;
    printf("\nStart:%d\nEnd:%d",i[0],i[1]);
    return 0;
}

int OrganizeJson(){
    //Get Json File
    remove("jsontemp.json");
    system("curl https://magisk.topjohnwu.com/stable.json > jsontemp.json");
    char* JsonContent = ReadFile("jsontemp.json");
    remove("jsontemp.json");

    //Start Parse Json File

    // Init
    cJSON* cjson_master = NULL;

    cJSON* cjson_magisk = NULL;
    cJSON* cjson_magisk_version = NULL;
    cJSON* cjson_magisk_versionCode = NULL;
    cJSON* cjson_magisk_link = NULL;
    cJSON* cjson_magisk_note = NULL;

    cJSON* cjson_stub = NULL;
    cJSON* cjson_stub_versionCode = NULL;
    cJSON* cjson_stub_link = NULL;
    // Parse
    cjson_master = cJSON_Parse(JsonContent);
    if(cjson_master == NULL){
        printf("\nCannot parse the json.");
        exit(1);
    }
    // Get items
    // Magisk
    cjson_magisk = cJSON_GetObjectItem(cjson_master,"magisk");
    cjson_magisk_version = cJSON_GetObjectItem(cjson_magisk,"version");
    cjson_magisk_versionCode = cJSON_GetObjectItem(cjson_magisk,"versionCode");
    // Stub
    cjson_stub = cJSON_GetObjectItem(cjson_master,"stub");

    // Organize new strings
    char MagiskLink[1024];
    char MagiskNote[1024];
    char StubLink[1024];
    sprintf(MagiskLink,"%s/Magisk-v%s.apk",MirrorURL,cjson_magisk_version->valuestring);
    sprintf(MagiskNote,"%s/notes/%s.md",MirrorURL,cjson_magisk_versionCode->valuestring);
    sprintf(StubLink,"%s/stub/%s/stub-release.apk",MirrorURL,cjson_magisk_version->valuestring);
    cJSON_DetachItemFromObject(cjson_magisk, "link");
    cJSON_DetachItemFromObject(cjson_magisk, "note");
    cJSON_DetachItemFromObject(cjson_stub, "link");
    cJSON_AddItemToObject(cjson_magisk, "link", cJSON_CreateString(MagiskLink));
    cJSON_AddItemToObject(cjson_magisk, "note", cJSON_CreateString(MagiskNote));
    cJSON_AddItemToObject(cjson_stub, "link", cJSON_CreateString(StubLink));

    const char* Output = cJSON_Print(cjson_master);
    char WriteFilepath[1024];
    printf("\n%s", Output);
    sprintf(WriteFilepath,"%s/update-stable.json",SitePath);
    OverwriteFile(WriteFilepath,cJSON_Print(cjson_master));
    cJSON_Delete(cjson_master);
    return 0;
}

void getVersion(){
    remove("curltemp.txt");
    system("curl https://github.com/topjohnwu/Magisk/releases/latest/ > curltemp.txt");
    char* FileContent= ReadFile("curltemp.txt");
    remove("curltemp.txt");
    char *ptr = strstr(FileContent, "tag/");
    if(getPosition(ptr))
        exit(1);
    strncpy(VerData, ptr+start, final);
    sprintf(VerData,"%s",VerData);
}

void EnableMaintenance(){
    
}

void DisableMaintenance(){

}

void Update(const char* TargetFilePath){
    char DownloadAndOverwriteCommand[1024];
    char OverwriteLatestCommand[1024];
    char VersionAPIPath[1024];
    sprintf(DownloadAndOverwriteCommand,
            "wget --no-check-certificate --content-disposition https://github.com/topjohnwu/Magisk/releases/download/v%s/Magisk-v%s.apk -O %s",
            VerData, VerData, TargetFilePath);
    sprintf(OverwriteLatestCommand,"cp -rf %s %s/Magisk-latest.apk",TargetFilePath,SitePath);
    sprintf(VersionAPIPath,"%s/version.html",SitePath);
    printf("\nCommand:%s", DownloadAndOverwriteCommand);
    OverwriteFile(VersionAPIPath,VerData);
    system(DownloadAndOverwriteCommand);
    system(OverwriteLatestCommand);
    OrganizeJson();
}

int main()
{
    getVersion();
    char TargetFilePath[1024];
    sprintf(TargetFilePath,"%s/Magisk-v%s.apk",SitePath,VerData);
    if(FileExist(TargetFilePath))
        exit(0);
    else {
        Update(TargetFilePath);
    }
}