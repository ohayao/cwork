#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <sys/sysinfo.h>
#include <regex.h>

#include "../../src/bridge/https_client/https.h"

typedef struct PRO_CPU_PACKED{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;

}PRO_CPU_OCCUPY;


typedef struct PRO_DISK_INFO{
    long total;
    long used;
    double used_rate;
}PRO_DISK_INFO;

typedef struct PRO_MEMORY_INFO{
    long total;
    long free;
    double used_rate;
}PRO_MEMORY_INFO;

typedef struct PRO_WIFI_INFO{
    char ssid[50];
    int signal;
}PRO_WIFI_INFO;

//获取本地IP地址
int Pro_GetLocalIP(char *local_ip);
//获取公网IP地址
int Pro_GetPublicIP(char *public_ip);
//获取本地mac地址
int Pro_GetMacAddr(char *mac_addr);
//获取本地mac地址不带":"号的
int Pro_GetMacAddrs(char *mac_addr);
//获取CPU使用率
double Pro_GetCpuRate();
//获取内存信息
PRO_MEMORY_INFO *Pro_GetMemoryInfo();
//获取磁盘信息
PRO_DISK_INFO *Pro_GetDiskInfo();
//获取系统启动时间戳
int Pro_GetInitedTime();
//获取wifi信息
PRO_WIFI_INFO *Pro_GetWifiInfo();

double cal_cpuoccupy(PRO_CPU_OCCUPY *o,PRO_CPU_OCCUPY *n);
void get_cpuoccupy(PRO_CPU_OCCUPY *cpust);


int Pro_GetLocalIP(char *local_ip){
    int sock_fd, intrface;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc))
        {
            intrface = ifc.ifc_len/sizeof(struct ifreq);
            while (intrface-- > 0)
            {
                if (!(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[intrface])))
                {
                    strcpy(local_ip,inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                    if(local_ip)
                    {
                        break;
                    }

                }
            }
        }
        close(sock_fd);
    }
    return 1;
}


int Pro_GetPublicIP(char *public_ip){
    HTTP_INFO hi;
    http_init(&hi,TRUE);
    char *url="http://ident.me";
    int ret=http_get(&hi,url,public_ip,1024);
    if(ret!=200) return 0;
    http_close(&hi);
    return 1;
}

int Pro_GetMacAddr(char *mac_addr){
    struct ifreq ifreq;
    int sock;
    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    strcpy(ifreq.ifr_name, "eth0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        printf("--->get eth0 info err\n");
    }else{
        printf("--->get eth0 info success\n");
    }
    if(ifreq.ifr_hwaddr.sa_data==NULL){
        strcpy(ifreq.ifr_name,"wlp3s0");
        if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0){
            printf("--->get wlp3s0 info err\n");
        }else{
            printf("--->get wlp3s0 info success\n");
        }
    }
    if(ifreq.ifr_hwaddr.sa_data==NULL){
        printf("---> get macaddr failed\n");
        return -1;
    }
    sprintf(mac_addr,"%X:%X:%X:%X:%X:%X", 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[0], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[1], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[3], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[4], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);

    return 1;
}
int Pro_GetMacAddrs(char *mac_addr){
    struct ifreq ifreq;
    int sock;
    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    strcpy(ifreq.ifr_name, "eth0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        printf("--->get eth0 info err\n");
    }else{
        printf("--->get eth0 info success\n");
    }
    if(ifreq.ifr_hwaddr.sa_data==NULL){
        strcpy(ifreq.ifr_name,"wlp3s0");
        if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0){
            printf("--->get wlp3s0 info err\n");
        }else{
            printf("--->get wlp3s0 info success\n");
        }
    }
    if(ifreq.ifr_hwaddr.sa_data==NULL){
        printf("---> get macaddr failed\n");
        return -1;
    }
    sprintf(mac_addr,"%X%X%X%X%X%X", 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[0], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[1], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[3], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[4], 
            (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);

    return 1;
}



double cal_cpuoccupy (PRO_CPU_OCCUPY *o, PRO_CPU_OCCUPY *n)
{
    double od, nd;
    double id, sd;
    double cpu_use ;               
    od = (double) (o->user + o->nice + o->system +o->idle+o->softirq+o->iowait+o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system +n->idle+n->softirq+n->iowait+n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od                     
    id = (double) (n->idle);    //用户第一次和第二次的时间之差再赋给id
    sd = (double) (o->idle) ;    //系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
        cpu_use =100.0- ((id-sd))/(nd-od)*100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0;
    return cpu_use;                                        
}
void get_cpuoccupy (PRO_CPU_OCCUPY *cpust)
{
    FILE *fd;
    int n;
    char buff[256];
    PRO_CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;

    fd = fopen ("/proc/stat", "r");
    fgets (buff, sizeof(buff), fd);

    sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle ,&cpu_occupy->iowait,&cpu_occupy->irq,&cpu_occupy->softirq);

    fclose(fd);

}
double Pro_GetCpuRate()
{
    PRO_CPU_OCCUPY cpu_stat1;
    PRO_CPU_OCCUPY cpu_stat2;
    double cpu;
    get_cpuoccupy((PRO_CPU_OCCUPY *)&cpu_stat1);
    sleep(1);

    //第二次获取cpu使用情况
    get_cpuoccupy((PRO_CPU_OCCUPY *)&cpu_stat2);

    //计算cpu使用率
    cpu = cal_cpuoccupy ((PRO_CPU_OCCUPY *)&cpu_stat1, (PRO_CPU_OCCUPY *)&cpu_stat2);
    return cpu;                  
}

PRO_MEMORY_INFO *Pro_GetMemoryInfo()
{
    FILE *fd;
    int n;
    char buff[256];
    PRO_MEMORY_INFO *mi=(PRO_MEMORY_INFO *)malloc(sizeof(PRO_MEMORY_INFO));
    fd = fopen ("/proc/meminfo", "r");

    char name[20];
    char unit[20];
    long total;
    fgets (buff, sizeof(buff), fd);
    sscanf(buff,"%s %lu %s",&name,&total,&unit);
    long free;
    fgets(buff,sizeof(buff),fd);
    sscanf(buff,"%s %lu %s",&name,&free,&unit);

    mi->total=total;
    mi->free=free;
    mi->used_rate=((double)total-(double)free)/(double)total;
    fclose(fd);
    return mi;

}

PRO_DISK_INFO *Pro_GetDiskInfo()
{
    FILE * fp;
    fp=popen("df","r");
    PRO_DISK_INFO  *dev=(PRO_DISK_INFO *)malloc(sizeof(PRO_DISK_INFO));

    char buf[256], name[20],rpath[20],rate[20];
    fgets(buf,256,fp);
    fgets(buf,256,fp);
    long total=0,used=0,avab=0;
    sscanf(buf,"%s %d %d %d %s %s",&name,&total,&used,&avab,&rate,&rpath);
        
    dev->total=total;
    dev->used=used;
    dev->used_rate=((double)used)/(double)total;
    pclose(fp);
    return dev;
}

int Pro_GetInitedTime(){
    //setenv("TZ", "GMT-8", 1);
    struct sysinfo info;
    time_t cur_time = 0;
    time_t boot_time = 0;
    struct tm *ptm = NULL;
    sysinfo(&info);
    time(&cur_time);
    if (cur_time > info.uptime) {
        boot_time = cur_time - info.uptime;     
    }
    else {
        boot_time = info.uptime - cur_time;     
    }
    ptm = gmtime(&boot_time);
    int btime=(int)mktime(ptm)+28800;
    return btime;
}

int _get_wifi_ssid(PRO_WIFI_INFO *wf);
int _get_wifi_signal(PRO_WIFI_INFO *wf);
int _get_wifi_ssid(PRO_WIFI_INFO *wf){
    FILE *fp;
    fp=popen("iwconfig wlan0","r");
    regex_t reg;
    regmatch_t rmatch[20];
    if(regcomp(&reg,"^wlan0.*?ESSID:\"(.*)\".*?\n",REG_EXTENDED)<0){
        printf("reg comp error");
        return -1;
    }
    char buff[1024];
    memset(buff,0,sizeof(buff));
    while(fgets(buff,1024,fp)!=NULL){
        if(regexec(&reg,buff,20,rmatch,0)==0){
            char match[50];
            memset(match,0,sizeof(match));
            int len=rmatch[1].rm_eo-rmatch[1].rm_so;
            memcpy(match,buff+rmatch[1].rm_so,len);
            strcpy(wf->ssid,match);
            break;
        }
        memset(buff,0,sizeof(buff));
    }
    regfree(&reg);
    pclose(fp);
    return 0;
}
int _get_wifi_signal(PRO_WIFI_INFO *wf){
    FILE *fp;
    fp=popen("iwconfig wlan0","r");
    regex_t reg;
    regmatch_t rmatch[20];
    if(regcomp(&reg,"^\\s+.*?Quality=([0-9]{1,}).*?\n",REG_EXTENDED)<0){
        printf("reg comp error");
        return -1;
    }
    char buff[1024];
    memset(buff,0,sizeof(buff));
    while(fgets(buff,1024,fp)!=NULL){
        if(regexec(&reg,buff,20,rmatch,0)==0){
            char match[50];
            memset(match,0,sizeof(match));
            int len=rmatch[1].rm_eo-rmatch[1].rm_so;
            memcpy(match,buff+rmatch[1].rm_so,len);
            wf->signal=atoi(match);
            break;
        }
        memset(buff,0,sizeof(buff));
    }
    regfree(&reg);
    pclose(fp);
    return 0;
}

int _get_wifi_ssid_other(PRO_WIFI_INFO *wf);
int _get_wifi_signal_other(PRO_WIFI_INFO *wf);
int _get_wifi_ssid_other(PRO_WIFI_INFO *wf){
    FILE *fp;
    fp=popen("iwconfig wlp3s0","r");
    regex_t reg;
    regmatch_t rmatch[20];
    if(regcomp(&reg,"^wlp3s0.*?ESSID:\"(.*)\".*?\n",REG_EXTENDED)<0){
        printf("reg comp error");
        return -1;
    }
    char buff[1024];
    memset(buff,0,sizeof(buff));
    while(fgets(buff,1024,fp)!=NULL){
        if(regexec(&reg,buff,20,rmatch,0)==0){
            char match[50];
            memset(match,0,sizeof(match));
            int len=rmatch[1].rm_eo-rmatch[1].rm_so;
            memcpy(match,buff+rmatch[1].rm_so,len);
            strcpy(wf->ssid,match);
            break;
        }
        memset(buff,0,sizeof(buff));
    }
    regfree(&reg);
    pclose(fp);
    return 0;
}
int _get_wifi_signal_other(PRO_WIFI_INFO *wf){
    FILE *fp;
    fp=popen("iwconfig wlp3s0","r");
    regex_t reg;
    regmatch_t rmatch[20];
    if(regcomp(&reg,"^\\s+.*?Quality=([0-9]{1,}).*?\n",REG_EXTENDED)<0){
        printf("reg comp error");
        return -1;
    }
    char buff[1024];
    memset(buff,0,sizeof(buff));
    while(fgets(buff,1024,fp)!=NULL){
        if(regexec(&reg,buff,20,rmatch,0)==0){
            char match[50];
            memset(match,0,sizeof(match));
            int len=rmatch[1].rm_eo-rmatch[1].rm_so;
            memcpy(match,buff+rmatch[1].rm_so,len);
            wf->signal=atoi(match);
            break;
        }
        memset(buff,0,sizeof(buff));
    }
    regfree(&reg);
    pclose(fp);
    return 0;
}


PRO_WIFI_INFO *Pro_GetWifiInfo(){
    PRO_WIFI_INFO *wf=(PRO_WIFI_INFO *)malloc(sizeof(PRO_WIFI_INFO));
    if(_get_wifi_ssid(wf)!=0){
        _get_wifi_ssid_other(wf);
    }
    if(_get_wifi_signal(wf)!=0){
        _get_wifi_ssid_other(wf);
    }
    return wf;
}

#endif
