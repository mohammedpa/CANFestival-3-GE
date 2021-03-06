/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Jorge BERZOSA

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
* can4linux driver
*/

extern void (*stateChangeCallback)(const char* iface_name, int state);

/*
 * compatible with can_netlink.h -> enum can_state (used in CANopen::CANBusMonitor)
 */

#define CAN_STATE_ERROR_ACTIVE		( 0 )
#define CAN_STATE_ERROR_WARNING		( 1 )
#define CAN_STATE_ERROR_PASSIVE		( 2 )
#define CAN_STATE_BUS_OFF			( 3 )
#define CAN_STATE_STOPPED			( 4 )
#define CAN_STATE_SLEEPING			( 5 )
#define CAN_STATE_MAX				( 6 )

#define MAX_CAN_INTERFACES			( 16 )

char can_dev_name[MAX_CAN_INTERFACES][8] = {0x00};
int can_iface_fd[MAX_CAN_INTERFACES] = {-1};
int lastFreeIdx = 0;

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "can4linux.h"
#include "can_driver.h"

static int can_reset(int can_fd)
{
   int ret;
   volatile Command_par_t cmd;

   cmd.cmd = CMD_STOP;
   ret = ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);
   cmd.cmd = CMD_RESET;
   ret = ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

   return ret;
}

static int can_start(int can_fd)
{
   int ret;
   volatile Command_par_t cmd;

   cmd.cmd = CMD_CLEARBUFFERS;
   ret = ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);
   cmd.cmd = CMD_START;
   ret = ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

   return ret;
}

static void displayerror(canmsg_t *rx, int fd)
{
	// resolv fd->can_dev_name
	const char* dev_name = "unknown";
	int i = 0;
	for (i = 0; i < MAX_CAN_INTERFACES; i++)
	{
		if (can_iface_fd[i] == fd)
		{
			dev_name = can_dev_name[i];
			break;
		}
	}

	printf("Flags 0x%02x,", rx->flags);
	if( rx->flags & MSG_OVR)
	{
		printf(" CAN Controller Buffer Overflow,");
	}

	if( rx->flags & MSG_BOVR)
	{
		printf(" Driver Buffer Overflow,");
	}

	if( 0 == (rx->flags & MSG_ERR_MASK))
	{
		printf(" CAN Error Active");

		if (stateChangeCallback != NULL)
			stateChangeCallback(dev_name, CAN_STATE_ERROR_ACTIVE);
	}

	if( rx->flags & MSG_WARNING)
	{
		printf(" CAN Warning Level,");

		if (stateChangeCallback != NULL)
			stateChangeCallback(dev_name, CAN_STATE_ERROR_WARNING);
	}

	if( rx->flags & MSG_PASSIVE)
	{
		printf(" CAN Error Passive,");

		if (stateChangeCallback != NULL)
			stateChangeCallback(dev_name, CAN_STATE_ERROR_PASSIVE);
	}

	if( rx->flags & MSG_BUSOFF) {
		printf(" CAN Bus Off,");

		/* can_reset(); */
		/* sleep 100ms */
		/* usleep (100000); */
		/* can_start(); */

		if (stateChangeCallback != NULL)
			stateChangeCallback(dev_name, CAN_STATE_BUS_OFF);

		printf(" Restarting CAN interface hw,");
		can_reset(fd);
		usleep(100000);
		can_start(fd);

		printf(" Interface restarted");
	}
	printf("\n");
}

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
  int res,i;
  canmsg_t canmsg;
  //long int time_period;

	canmsg.flags = 0; 
	do{
   		res = read(fd0,&canmsg,1);
   		if((res<0)&&(errno == -EAGAIN)) res = 0;

   		if ((res > 0 ) && (CANDRIVERERROR == canmsg.id))
   		{
   			displayerror(&canmsg, fd0);
   		}

	}while(res==0);

	if(res !=1) // No new message
   	return 1;

	if(canmsg.flags&MSG_EXT){
   		/* There is no mark for extended messages in CanFestival */;
	}
  		
	m->cob_id = canmsg.id;
	m->len = canmsg.length;
	if(canmsg.flags&MSG_RTR){
 		m->rtr = 1;
	}else{
 		m->rtr = 0;
 		memcpy(m->data,canmsg.data,8);
	}
	
 	/*gettimeofday(&current_time,NULL);
 	time_period=(current_time.tv_sec - init_time.tv_sec)* 1000000 + current_time.tv_usec - init_time.tv_usec;
 	printf("%3ld.%3ld.%3ld - Receive ID: %lx ->",time_period/1000000,(time_period%1000000)/1000,time_period%1000,m->cob_id);
 	printf("Receive ID: %lx ->",m->cob_id);
 	for(i=0; i<canmsg.length;i++)printf("%x, ", m->data[i]);
 	printf("\n");*/
 
  return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message const *m)
{
  int res;
  canmsg_t canmsg;

  canmsg.flags = 0;
  canmsg.id = m->cob_id;
  canmsg.length = m->len;
  if(m->rtr){
    canmsg.flags |= MSG_RTR;
  }else{
    memcpy(canmsg.data,m->data,8);
  }
  
 /*printf("Send ID: %lx ->",canmsg.id); 
 for(i=0; i<canmsg.length;i++)printf("%x, ", canmsg.data[i]);
 printf("\n");*/

  if(canmsg.id >= 0x800){
    canmsg.flags |= MSG_EXT;
  }

  res = write(fd0,&canmsg,1);
  if(res!=1)
    return 1;

  return 0;
}

/***************************************************************************/
int TranslateBaudRate(char* optarg){
	if(!strcmp( optarg, "1M")) return (int)1000;
	if(!strcmp( optarg, "500K")) return (int)500;
	if(!strcmp( optarg, "250K")) return (int)250;
	if(!strcmp( optarg, "125K")) return (int)125;
	if(!strcmp( optarg, "100K")) return (int)100;
	if(!strcmp( optarg, "50K")) return (int)50;
	if(!strcmp( optarg, "20K")) return (int)20;
	if(!strcmp( optarg, "10K")) return (int)10;
	if(!strcmp( optarg, "5K")) return (int)5;
	return 0;
}

UNS8 _canChangeBaudRate( CAN_HANDLE fd, int baud)
{
    Config_par_t  cfg;	
    volatile Command_par_t cmd;
    
    cmd.cmd = CMD_STOP;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

	cfg.target = CONF_TIMING; 
	cfg.val1  = baud;

	ioctl(fd, CAN_IOCTL_CONFIG, &cfg);

	cmd.cmd = CMD_START;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);
    
    return 0;
}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
    int temp=TranslateBaudRate(baud);

    if(temp==0)return 1;
    _canChangeBaudRate(fd, temp);

    printf("Baudrate changed to %s\n", baud);
    return 0;
}

/***************************************************************************/
//static const char lnx_can_dev_prefix[] = "/dev/can";
static const char lnx_can_dev_prefix[] = "/dev/";

CAN_HANDLE canOpen_driver(s_BOARD *board)
{
  int name_len = strlen(board->busname);
  int prefix_len = strlen(lnx_can_dev_prefix);
  char dev_name[prefix_len+name_len+1];
  int o_flags = 0;
  //int baud = TranslateBaudeRate(board->baudrate);
  int fd0;
  int res;
  
  /*o_flags = O_NONBLOCK;*/

  memcpy(dev_name,lnx_can_dev_prefix,prefix_len);
  memcpy(dev_name+prefix_len,board->busname,name_len);
  dev_name[prefix_len+name_len] = 0;

  fd0 = open(dev_name, O_RDWR|o_flags);
  if(fd0 == -1){
    fprintf(stderr,"!!! %s is unknown. See can4linux.c\n", dev_name);
    goto error_ret;
  }

  res=TranslateBaudRate(board->baudrate);
  if(res == 0){
    fprintf(stderr,"!!! %s baudrate not supported. See can4linux.c\n", board->baudrate);
    goto error_ret;
  }

  /* map fd to device name */
  can_iface_fd[lastFreeIdx] = fd0;

  if (name_len <= sizeof(can_dev_name[lastFreeIdx]))
  {
	  memcpy(can_dev_name[lastFreeIdx], board->busname, name_len);
  }
  else
  {
	  can_dev_name[lastFreeIdx][0] = 'u';
	  can_dev_name[lastFreeIdx][1] = 'n';
	  can_dev_name[lastFreeIdx][2] = 'k';
	  can_dev_name[lastFreeIdx][3] = 'n';
	  can_dev_name[lastFreeIdx][4] = 'o';
	  can_dev_name[lastFreeIdx][5] = 'w';
	  can_dev_name[lastFreeIdx][6] = 'n';
  }
  lastFreeIdx++;
	
  _canChangeBaudRate( (CAN_HANDLE)fd0, res);

  printf("CAN device /dev/%s opened. Baudrate %s\n",board->busname, board->baudrate);

  return (CAN_HANDLE)fd0;

 error_ret:
  return NULL;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
	if((int)fd0 != -1) {
		return close((int)fd0);
	}

	return -1;
}

int canfd_driver(CAN_HANDLE fd0)
{
	return ((int)fd0);
}
