#include "rqsvr.h"


/*
 * admin_command()
 * Accept & response for admin command
 */
int admin_command()
{
	struct	sockaddr_in sockaddr_in;
	struct	sockaddr_in sockadm;
	int	options;
	struct	pollfd pollfd;
	int	lsock, csock, check;
	socklen_t socklen;
	struct	{
		int	type;
		int	rqid;
		in_addr_t addr;
	} peer4chck;
	int	port;
	int	ii;

	port = RTD_PORT + 1;
	while (1)
	{
		sleep(1);
		memset(&sockaddr_in, 0, sizeof(sockaddr_in));
		sockaddr_in.sin_family = AF_INET;
		sockaddr_in.sin_port = htons(port);
		sockaddr_in.sin_addr.s_addr = INADDR_ANY;
		lsock = socket(AF_INET, SOCK_STREAM, 0);
		if (lsock < 0)
			continue;
		options = 1;
		setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
		if (bind(lsock, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) != 0)
		{
			close(lsock);
			continue;
		}
		listen(lsock, 20);
	
		while (1)
		{
			socklen = sizeof(sockaddr_in);
			csock = accept(lsock, (struct sockaddr *)&sockadm, &socklen);
			if (csock < 0)
				continue;
		
			pollfd.fd = csock;
			pollfd.events = POLLIN | POLLHUP;
			pollfd.revents = 0;
			check = poll(&pollfd, 1, 5000);
			if (check <= 0)
			{
				close(csock);
				continue;
			}
			if (read(csock, &peer4chck, sizeof(peer4chck)) != sizeof(peer4chck))
			{
				close(csock);
				continue;
			}
			switch (peer4chck.type)
			{
			case 0: // by rqid
				if (peer4chck.rqid <= 0 || peer4chck.rqid >= no_of_linked || 
				    peer4chck.rqid >= no_of_clients ||
			    	    rq_client[peer4chck.rqid].sock != peer4chck.rqid ||
				     rq_client[peer4chck.rqid].mask == NULL)
					break;
				write(csock, rq_client[peer4chck.rqid].mask, sizeof(mask_t)*no_of_symbols);
				break;
			case 1: // by pid
				if (peer4chck.rqid <= 0)
					break;
				for (ii = 1; ii < no_of_clients; ii++)
				{
					if (rq_client[ii].pid == peer4chck.rqid)
						break;
				}
				if (ii >= no_of_clients || rq_client[ii].sock != ii || rq_client[ii].mask == NULL)
					break;
				write(csock, rq_client[ii].mask, sizeof(mask_t)*no_of_symbols);
				break;
			case 2: // by net address
				if (peer4chck.addr <= 0)
					break;
				for (ii = 1; ii < no_of_clients; ii++)
				{
					if (rq_client[ii].addr == peer4chck.addr)
						break;
				}
				if (ii >= no_of_clients || rq_client[ii].sock != ii || rq_client[ii].mask == NULL)
					break;
				write(csock, rq_client[ii].mask, sizeof(mask_t)*no_of_symbols);
				break;
			default:
				break;
			}
			close(csock);
		}
	}
}
