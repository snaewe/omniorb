//                                Package   : threadtests
// diner.cc                       Created on: 23/1/95 sll
//
// Copyright (C) Olivetti Research Limited, 1994
//
// Last update Time-stamp: <95/01/24 17:49:01 sll>
//
// Description:
//     A solution to the infamous dining philosophers, implemented using
//     the omnithread package.
//     This program exercises the thread creation and destruction,
//     mutexes and condition variables.
//
//     Expected to see all the philosophers doing various things. None
//     of them is absent.
//       
//

/*
 $Log$
 Revision 1.1  1995/01/25 11:54:48  sll
 Initial revision

 */

#include <iostream.h>
#include <stdlib.h>
#include "omnithread.h"

#define N_DINERS  5

static omni_mutex  *chopsticks[N_DINERS]; // n philosophers sharing n chopsticks
static omni_thread *phillies[N_DINERS];

// At most n philosophers are allowed into the room, others would
// have to wait at the door. This restriction demonstrates the use
// of condition variables.

static omni_mutex     *room_mutex;
static omni_condition *room_cond;
static int             room_occupancy = 0;

static int random_l();

static void *
philosopher(void *arg)
{
  int id = (long)arg;
  int l = id;
  int r = l+1;
  if (r == N_DINERS)
    r = 0;
  if (l & 1)
    {
      int t = l;
      l = r;
      r = t;
    }
  cerr << "Philosopher #" << id << " has entered the room." << endl;
  { int count = random_l() % 10 + 1;
    while (count--)
      {
	chopsticks[l]->acquire();
	chopsticks[r]->acquire();
	cerr << "Philosopher #" << id << " is eating spaghetti now." << endl;
	omni_thread::sleep(random_l()%2,random_l()%1000000000);
	chopsticks[l]->release();
	chopsticks[r]->release();
	cerr << "Philosopher #" << id << " is pondering about life." << endl;
	omni_thread::sleep(random_l()%2,random_l()%1000000000);
      }
  }
  room_mutex->acquire();
  room_occupancy--;
  phillies[id] = NULL;
  room_mutex->release();
  room_cond->signal();
  cerr << "Philosophor #" << id << " has left the room." << endl;
  return NULL;
}

int
main(int argc, char ** argv)
{
  int i;
  for (i=0; i<N_DINERS; i++)
    if ((chopsticks[i] = omni_mutex::create()) == NULL)
      {
	cerr << "Cannot create chopstick #" << i << endl;
	exit(1);
      }
  room_mutex = omni_mutex::create();
  room_cond  = omni_condition::create(room_mutex);
  room_mutex->acquire();
  for (i=0; i<N_DINERS; i++)
    if ((phillies[i] = omni_thread::create(philosopher,1,(void *)i)) == NULL)
      {
	cerr << "Cannot create philosopher #" << i << endl;
	exit(1);
      }
  room_occupancy = N_DINERS;
  while (1)
    {
      while (room_occupancy == N_DINERS)
	room_cond->wait();
      // Hm.. someone has left the room.
      room_mutex->release();
      // Sleep for a while and then create a new philosopher
      omni_thread::sleep(0,random_l()%500000000);
      room_mutex->acquire();
      for (i=0; i<N_DINERS; i++)
	if (phillies[i] == NULL)
	  break;
      if (i == N_DINERS)
	{
	  cerr << "Contrary to what I was told, no one has left the room!!!!\n";
	  cerr << "I give up!!!" << endl;
	  exit(1);
	}
      if ((phillies[i] = omni_thread::create(philosopher,1,(void *)i)) == NULL)
	{
	  cerr << "Cannot create philosopher #" << i << endl;
	  exit(1);
	}
      room_occupancy++;
    }
  return(0);
}

static omni_mutex *rand_mutex = omni_mutex::create();

static
int random_l()
{
  rand_mutex->acquire();
  int i = rand();
  rand_mutex->release();
  return i;
}
