/*
 *  @(!--#) @(#) hs100110.c, version 003, 31-may-2018
 *
 *  TP-Link HS100 Smart WiFi Plug controller (ANSI C version)
 *
 *  Send encoded JSON requests via TCP/IP to turn a
 *  TP-Link HS100 and HS110 Smart WiFi plug on and off
 *
 *  Product details:
 *
 *      http://uk.tp-link.com/products/list-5258.html
 *
 *  Based on encoded JSON strings from:
 *
 *      https://community.openhab.org/t/tp-link-hs100-smart-plug-wifi/8591/6
 *
 *  This page has comprehensive and interesting reverse engineering details:
 *
 *      https://www.softscheck.com/en/reverse-engineering-tp-link-hs110/
 *
 */

/*****************************************************************************/

/*
 *  includes
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

/*****************************************************************************/

/*
 *  defines
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PACKET_SIZE 66000

#define JSON_ON     "{\"system\":{\"set_relay_state\":{\"state\":1}}}"
#define JSON_OFF    "{\"system\":{\"set_relay_state\":{\"state\":0}}}"
#define JSON_QUERY  "{\"system\":{\"get_sysinfo\":null}}"

/*****************************************************************************/

/*
 *  globals
 */

char *progname;

/*****************************************************************************/

void plain2cipher(buf, lenbuf)
  unsigned char   *buf;
  int              lenbuf;
{
  unsigned char    key;
  unsigned char    keyedbyte;
  int              i;

  key = 0xAB;  /* (10 * 16 = 160) + 11 = 171 | 0xAB */

  for (i = 0; i < lenbuf; i++) {
    keyedbyte = key ^ buf[i];
    key = keyedbyte;
    buf[i] = keyedbyte;
  }

  return;
}

/*****************************************************************************/

void cipher2plain(buf, lenbuf)
  unsigned char   *buf;
  int              lenbuf;
{
  unsigned char    key;
  unsigned char    keyedbyte;
  int              i;

  key = 0xAB;  /* (10 * 16 = 160) + 11 = 171 | 0xAB */

  for (i = 0; i < lenbuf; i++) {
    keyedbyte = key ^ buf[i];
    key = buf[i];
    buf[i] = keyedbyte;
  }

  return;
}

/*****************************************************************************/

void showpacket(title, packet, packetlen)
  char          *title;
  unsigned char *packet;
  int            packetlen;
{
  int            bytesperrow;
  int            i;

  bytesperrow = 10;

  printf("%s:\n", title);

  for (i = 0; i < packetlen; i++) {
    if ((i % bytesperrow) == 0) {
      printf("%04d:", i);
    }

    printf(" %02X", packet[i]);

    if (
         (i == (packetlen - 1))
         ||
         (((i + 1) % bytesperrow) == 0)
       ) {
      putchar('\n');
    }
  }

  return;
}

/*****************************************************************************/

/*
 *  Main
 */

int main(argc, argv)
  int   argc;
  char *argv[];
{
  int                  verbose;
  char                *hostip;
  char                *portnum;
  char                *jsonstring;
  int                  lenjsonstring;
  int                  arg;
  struct addrinfo      hints, *res;
  int                  rc;
  struct addrinfo     *cres;
  int                  numres;
  int                  i;
  int                  s;
  unsigned char        sendpacket[MAX_PACKET_SIZE];
  unsigned char        recvpacket[MAX_PACKET_SIZE];
  int                  n;
  int                  responselength;

  progname = argv[0];

  verbose = 0;
  hostip = "-";
  portnum = "9999";
  jsonstring = JSON_QUERY;

  for ( arg = 1 ; arg < argc ; arg++ ) {
    if (strcmp(argv[arg], "-h") == 0) {
      arg++;

      if (! (arg < argc)) {
        fprintf(stderr, "%s: expected host IP after -h option\n", progname);
        exit(1);
      }
 
      hostip = argv[arg];
    } else if (strcmp(argv[arg], "-p") == 0) {
      arg++;

      if (! (arg < argc)) {
        fprintf(stderr, "%s: expected port number after -p option\n", progname);
        exit(1);
      }
 
      portnum = argv[arg];
    } else if (strcmp(argv[arg], "-j") == 0) {
      arg++;

      if (! (arg < argc)) {
        fprintf(stderr, "%s: expected JSON string after -j option\n", progname);
        exit(1);
      }
 
      jsonstring = argv[arg];
    } else if (strcmp(argv[arg], "-v") == 0) {
      verbose++;
    } else if (strcmp(argv[arg], "-on") == 0) {
      jsonstring = JSON_ON;
    } else if (strcmp(argv[arg], "-off") == 0) {
      jsonstring = JSON_OFF;
    } else if (strcmp(argv[arg], "-query") == 0) {
      jsonstring = JSON_QUERY;
    } else {
      fprintf(stderr, "%s: unrecognised command argument \"%s\"\n", progname, argv[arg]);
      exit(1);
    }
  }

  if (strcmp(hostip, "-") == 0) {
    fprintf(stderr, "%s: must specify a hostname with -h option on the command line (and it cannot be \"-\")\n", progname);
    exit(1);
  }

  if (verbose >= 1) {
    printf("Host IP.......: %s\n", hostip);
    printf("Port number...: %s\n", portnum);
    printf("JSON string...: %s\n", jsonstring);
  }

  lenjsonstring = strlen(jsonstring);

  if (lenjsonstring > (MAX_PACKET_SIZE - 4)) {
    fprintf(stderr, "%s: JSON string exceeded hardcoded length of %d\n", progname, MAX_PACKET_SIZE - 4);
    exit(1);
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  rc = getaddrinfo(hostip, portnum, &hints, &res);

  if (rc != 0) {
    fprintf(stderr, "%s: getaddrinfo error: %s\n", progname, gai_strerror(rc));
    exit(1);
  }

  if (verbose >= 1) {
    cres = res;
    numres = 0;

    while (cres) {
      numres++;
      cres = cres->ai_next;
    }

    printf("The getaddrinfo function returned %d matches\n", numres);
  }

  if (res->ai_next != NULL) {
    fprintf(stderr, "%s: multiple matches for host \"%s\"\n", progname, hostip);
    exit(1);
  }

  if (verbose >= 1) {
    printf("Making socket call\n");
  }

  s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (s == -1) {
    fprintf(stderr, "%s: socket creation error: %s\n", progname, strerror(errno));
    exit(1);
  }

  if (verbose >= 1) {
    printf("Making connect call\n");
  }

  rc = connect(s, res->ai_addr, res->ai_addrlen);

  if (rc != 0) {
    fprintf(stderr, "%s: connect error: %s\n", progname, strerror(errno));
    exit(1);
  }

  if (verbose >= 1) {
    printf("Sending JSON string\n");
  }

  sendpacket[0] = 0;
  sendpacket[1] = 0;
  sendpacket[2] = (lenjsonstring & 0x0000FF00) >> 8;
  sendpacket[3] =  lenjsonstring & 0x000000FF;

  strcpy(sendpacket + 4, jsonstring);

  if (verbose >= 1) {
    showpacket("JSON string", sendpacket, lenjsonstring + 4);
  }

  plain2cipher(sendpacket + 4, lenjsonstring);

  if (verbose >= 1) {
    showpacket("JSON string (plain2cipher'ed)", sendpacket, lenjsonstring + 4);
  }

  write(s, sendpacket, lenjsonstring + 4);

  n = read(s, recvpacket, 4);

  if (n < 0) {
    fprintf(stderr, "%s: read error getting response length: %s\n", progname, strerror(errno));
    exit(1);
  }

  if (n != 4) {
    fprintf(stderr, "%s: unable to read 4 bytes containing response length\n", progname);
    exit(1);
  }

  if ( (recvpacket[0] != 0) || (recvpacket[1] != 0) ) {
    fprintf(stderr, "%s: response length > 65535: high order bytes are 0x%02X and 0x%02X\n", progname, recvpacket[0], recvpacket[1]);
    exit(1);
  }

  responselength = (recvpacket[2] * 256) + recvpacket[3];

  if (responselength == 0) {
    fprintf(stderr, "%s: response length is 0 which makes no sense!\n", progname);
    exit(1);
  }

  n = read(s, recvpacket + 4, responselength);

  if (n < 0) {
    fprintf(stderr, "%s: read error getting response data: %s\n", progname, strerror(errno));
    exit(1);
  }

  if (n != responselength) {
    fprintf(stderr, "%s: unable to read response data - only got %d bytes\n", progname, n);
    exit(1);
  }

  if (verbose >= 1) {
    showpacket("Response string (plain2cipher)", recvpacket, responselength + 4);
  }

  cipher2plain(recvpacket + 4, responselength);

  if (verbose >= 1) {
    showpacket("Response string", recvpacket, responselength + 4);
  }

  for ( i = 4; i < (responselength + 4) ; i++ ) {
    putchar(recvpacket[i]);
  }
  putchar('\n');

  if (verbose >= 1) {
    printf("Making shutdown call on socket\n");
  }

  rc = shutdown(s, SHUT_RDWR);

  if (rc != 0) {
    fprintf(stderr, "%s: socket shutdown error: %s\n", progname, strerror(errno));
    exit(1);
  }

  return(0);
}
