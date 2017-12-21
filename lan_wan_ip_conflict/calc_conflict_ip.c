
/*
 * 判断LAN WAN网段冲突并计算新的LNA IP地址
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define print_ip(ip) printf(#ip ": %s\n", inet_ntoa(*((struct in_addr *)&(ip))))

unsigned int lan_wan_ip_conflict(unsigned int lanip, unsigned int lan_mask, unsigned int wanip, unsigned int wan_mask)
{
    unsigned int min_mask = (lan_mask) > (wan_mask) ? wan_mask : lan_mask;
    unsigned int new_lan_ip = 0;
    unsigned int pool_start = 0, pool_end = 0;

    /* 打印 */
    print_ip(lanip);
    print_ip(lan_mask);
    print_ip(wanip);
    print_ip(wan_mask);
    print_ip(min_mask);

    /* 判断是否冲突 */
    if((lanip & min_mask) != (wanip & min_mask))
    {
        return new_lan_ip;
    }

    printf("Conflict!\n");

    /* 计算修改后的lanip, 需要转为本地字节序 */
    new_lan_ip = ntohl(lanip)+ ~ntohl(min_mask) + (1);
    new_lan_ip = htonl(new_lan_ip);
    print_ip(new_lan_ip);

    //计算新的地址池，使用lan_mask计算，避免地址池太大
    pool_start = (ntohl(new_lan_ip) & ntohl(lan_mask)) + 1;
    pool_end = (ntohl(new_lan_ip) & ntohl(lan_mask)) + ~ntohl(lan_mask) - 1;
    
    //解决255.255.255.254这种掩码计算出来的问题
    if(pool_start > pool_end)
        pool_end = pool_start;

    //按理说地址池里面包含自己br0 IP不会有问题，但这里实测有问题，udhcpd一直
    //分配br0的IP给客户端。所以这里处理一下。
    new_lan_ip = ntohl(new_lan_ip);
    if(new_lan_ip - pool_start > pool_end - new_lan_ip)
        pool_end = new_lan_ip - 1;
    else
        pool_start = new_lan_ip + 1;
    if(pool_start > pool_end)
    {
        printf("too complex! error\n");
        pool_start = pool_end;
    }

    pool_start = htonl(pool_start);
    pool_end = htonl(pool_end);

    printf("The new pool: \n");
    print_ip(pool_start);
    print_ip(pool_end);

    return new_lan_ip;
}

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        printf("help: %s [lanip] [lanmask] [wanip] [wanmask]\n", argv[0]);
        return;
    }

    in_addr_t lanip = inet_addr(argv[1]);
    in_addr_t lan_mask = inet_addr(argv[2]);
    in_addr_t wanip = inet_addr(argv[3]);
    in_addr_t wan_mask = inet_addr(argv[4]);

    lan_wan_ip_conflict(lanip, lan_mask, wanip, wan_mask);

    return 0;
}
