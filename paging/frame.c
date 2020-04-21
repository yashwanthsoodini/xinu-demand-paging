/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];

void init_frm_entry(int);
int get_free_frm();

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  STATWORD ps;
  disable(ps);

  int i = 0;
  for(i = 0; i<NFRAMES; i++)
  {
    init_frm_entry(i);
  }

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
  disable(ps);

  int free_frm;
  if((free_frm = get_free_frm()) != -1)
  {
    *avail = free_frm;
    restore(ps);
    return OK;
  }

  int fr;
  if(grpolicy() == SC)
  {
    fr = frm_fifo_head;
    while(frm_tab[fr].fr_refbit == 1)
    {
      frm_tab[fr].fr_refbit = 0;
      fr = frm_tab[fr].fr_next;
    }
    frm_fifo_head = frm_tab[fr].next;
  } else if(grpolicy() == LFU)
  {
    fr = -1;
    int maxrefcnt = -1;
    int i;
    for(i=0; i<NFRAMES; i++)
    {
      if(frm_tab[i].fr_refcnt >= maxrefcnt)
      {
        maxrefcnt = frm_tab[i].fr_refcnt;
        fr = i;
      }
    }
  } else
  {
    restore(ps);
    return SYSERR;
  }

  if(free_frm(fr) != SYSERR)
  {
    *avail = fr;

    restore(ps);
    return OK;
  }

  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}


void init_frm_entry(int i)
{
  frm_tab[i].fr_status = FRM_UNMAPPED;
  frm_tab[i].fr_pid = -1;
  frm_tab[i].fr_vpno = -1;
  frm_tab[i].fr_refcnt = 0;
  frm_tab[i].fr_type = -1;
  frm_tab[i].fr_dirty = 0;
  frm_tab[i].fr_prev = -1;
  frm_tab[i].fr_next = -1;
  return;
}

int get_free_frm()
{
  int i = 0;
  for(i=0; i<NFRAMES; i++)
  {
    if(frm_tab[i] == FRM_UNMAPPED) return i;
  }

  return -1;
}