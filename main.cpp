#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cJSON.h"
#include "cJSON.c"

// 使用全局替换，将/www/wwwroot/mirror.lmfty.com/magisk替换为你的网站路径再
// 将https://mirror.lmfty.com/magisk替换为你的网站URL即可
// 请使用计划任务自动运行，运行频率由你来定，编译命令：g++ main.cpp -O3，二进制文件名为a.out，你可以自己来定二进制文件的名字

using namespace std;

int start,final;
char Result[128];

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

int OverwriteFile(const char* FileName,const char* Content){
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
    sprintf(MagiskLink,"https://mirror.lmfty.com/magisk/Magisk-v%s.apk",cjson_magisk_version->valuestring);
    sprintf(MagiskNote,"https://mirror.lmfty.com/magisk/notes/%s.md",cjson_magisk_versionCode->valuestring);
    sprintf(StubLink,"https://mirror.lmfty.com/magisk/stub/%s/stub-release.apk",cjson_magisk_version->valuestring);
    cJSON_DetachItemFromObject(cjson_magisk, "link");
    cJSON_DetachItemFromObject(cjson_magisk, "note");
    cJSON_DetachItemFromObject(cjson_stub, "link");
    cJSON_AddItemToObject(cjson_magisk, "link", cJSON_CreateString(MagiskLink));
    cJSON_AddItemToObject(cjson_magisk, "note", cJSON_CreateString(MagiskNote));
    cJSON_AddItemToObject(cjson_stub, "link", cJSON_CreateString(StubLink));

    const char* Output = cJSON_Print(cjson_master);
    printf("\n%s", Output);
    OverwriteFile("/www/wwwroot/mirror.lmfty.com/magisk/update-stable.json",cJSON_Print(cjson_master));
    cJSON_Delete(cjson_master);
    return 0;
}

int main()
{
    remove("curltemp.txt");
    system("curl https://github.com/topjohnwu/Magisk/releases/latest/ > curltemp.txt");
    char* FileContent= ReadFile("curltemp.txt");
    remove("curltemp.txt");
    char *ptr = strstr(FileContent, "tag/");
    if(getPosition(ptr))
        exit(1);
    strncpy(Result, ptr+start, final);
    char DownloadAndOverwriteCommand[1024];
    char TargetFilePath[1024];
    char OverwriteLatestCommand[1024];
    sprintf(Result,"%s",Result);
    sprintf(TargetFilePath,"/www/wwwroot/mirror.lmfty.com/magisk/Magisk-v%s.apk",Result);
    if(FileExist(TargetFilePath))
        exit(0);
    else {
        sprintf(DownloadAndOverwriteCommand,
                "wget --no-check-certificate --content-disposition https://github.com/topjohnwu/Magisk/releases/download/v%s/Magisk-v%s.apk -O /www/wwwroot/mirror.lmfty.com/magisk/Magisk-v%s.apk",
                Result, Result, Result);
        sprintf(OverwriteLatestCommand,"cp -rf %s /www/wwwroot/mirror.lmfty.com/magisk/Magisk-latest.apk",TargetFilePath);
        printf("\nCommand:%s", DownloadAndOverwriteCommand);
        OverwriteFile("/www/wwwroot/mirror.lmfty.com/magisk/version.html",Result);
        system(DownloadAndOverwriteCommand);
        system(OverwriteLatestCommand);
        OrganizeJson();
    }
}