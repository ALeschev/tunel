#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/rtnetlink.h>

#define NL_BUFSIZE 8192

void parse_if_message (struct nlmsghdr *nl_msg)
{
  struct ifaddrmsg *if_msg;
  struct rtattr    *attrib;
  int len;

  char  address[100];
  char *label = NULL;

  if_msg = (struct ifaddrmsg *) NLMSG_DATA (nl_msg);

  if (if_msg->ifa_family != AF_INET)
    return;

  address[0] = '\0';

  len = IFA_PAYLOAD (nl_msg);
  for (attrib = IFA_RTA (if_msg);
       RTA_OK (attrib, len);
       attrib = RTA_NEXT (attrib, len))
    {
      switch (attrib->rta_type)
        {
          case IFA_LOCAL:
            inet_ntop (AF_INET, RTA_DATA (attrib), address, sizeof (address));
            break;

          case IFA_LABEL:
            label = (char *) RTA_DATA (attrib);
            break;

          default:
            /* ignore all other attributes */
            break;
        }
    }

  /* if we got both a label and an IP address */
  if (label && address[0])
    {
      switch (nl_msg->nlmsg_type)
        {
          case RTM_NEWADDR:
            fprintf (stderr, "adding %s to %s\n", address, label);
            break;
          case RTM_DELADDR:
            fprintf (stderr, "removing %s from %s\n", address, label);
            break;
          default:
            /* ignore all other messages */
            break;
        }
    }

  return;
}


int main (int argc, char *argv[])
{
  struct pollfd       pollfd;
  struct nlmsghdr    *nl_msg;
  struct sockaddr_nl  local;
  int fd, len;
  char buf[NL_BUFSIZE];

  /* Create Socket */
  if ((fd = socket (PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
      perror ("Socket Creation: ");
      return -1;
    }

  memset (&local, 0, sizeof (local));
  local.nl_family = AF_NETLINK;
  local.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

  if (bind (fd, (struct sockaddr*) &local, sizeof (local)) < 0)
    {
      perror("Cannot bind netlink socket");
      return -1;
    }

  if (fcntl (fd, F_SETFL, O_NONBLOCK))
    {
      perror ("fcntl O_NONBLOCK");
      return -1;
    }

  memset (buf, 0, NL_BUFSIZE);
  nl_msg = (struct nlmsghdr *) buf;

  /* For getting interface addresses */
  nl_msg->nlmsg_len   = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
  nl_msg->nlmsg_type  = RTM_GETADDR;
  nl_msg->nlmsg_flags = NLM_F_ROOT | NLM_F_REQUEST;
  nl_msg->nlmsg_pid   = getpid ();

  if (write (fd, nl_msg, nl_msg->nlmsg_len) < 0)
    {
      fprintf (stderr, "Write To Socket Failed...\n");
      return -1;
    }

  while (1)
    {
      pollfd.fd = fd;
      pollfd.events = POLLIN;
      pollfd.revents = 0;

      if (poll (&pollfd, 1, 20000))
        {
          if ((len = recv (fd, buf, NL_BUFSIZE, 0)) < 0)
            {
              fprintf (stderr, "Read From Socket Failed...\n");
              if (errno != EAGAIN)
                return -1;
            }

          for (nl_msg = (struct nlmsghdr *) buf;
               NLMSG_OK (nl_msg, len);
               nl_msg = NLMSG_NEXT (nl_msg, len))
            {
              switch (nl_msg->nlmsg_type)
                {
                  case RTM_NEWADDR:
                  case RTM_DELADDR:
                    parse_if_message (nl_msg);
                    break;
                  case RTM_NEWLINK:
                    {
                      struct ifinfomsg *ifi;
                      ifi = (struct ifinfomsg *) NLMSG_DATA (nl_msg);
                      if (ifi->ifi_flags & IFF_RUNNING)
                        fprintf (stderr, "Link active\n");
                      else
                        fprintf (stderr, "Link inactive\n");
                    }
                    break;
                  case RTM_DELLINK:
                    fprintf (stderr, "Link down\n");
                    break;
                  case NLMSG_DONE:
                    break;
                  default:
                    fprintf (stderr, "unhandled message (%d)\n",
                             nl_msg->nlmsg_type);
                    break;
                }
            }
        }
    }

  close (fd);
  return 0;
}