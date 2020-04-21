/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];

void init_frm_entry(int);
int get_free_frm();
void init_pt_t(pt_t *pt_t_i);
void init_pt_fr_t(int i);
void init_pd_fr_t(int i);

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

int create_pt(int pid)
{
  int avail_fr;
  if(get_frm(&avail_fr) == OK)
  {
    init_pt_fr_t(avail_fr);

    pt_t *pt_i = (pt_t *)((FRAME0 + avail_fr) * NBPG);
    int i;
    for(i=0; i<1024; i++)
    {
      pt_i->pt_pres = 0;
      pt_i->pt_write = 1;
      pt_i->pt_user = 0;
      pt_i->pt_pwt = 0;
      pt_i->pt_pcd = 0;
      pt_i->pt_acc = 0;
      pt_i->pt_dirty = 0;
      pt_i->pt_mbz = 0;
      pt_i->pt_global = 0;
      pt_i->pt_avail = 0;
      pt_i->pt_base = -1;
      pt_i++;
    }

    return avail_fr;
  }

  return SYSERR;
}

void init_pt_fr_t(int i)
{
  frm_tab[i].fr_status = FRM_UNMAPPED;
  frm_tab[i].fr_pid = pid;
  frm_tab[i].fr_vpno = -1;
  frm_tab[i].fr_refcnt = 0;
  frm_tab[i].fr_refbit = 0;
  frm_tab[i].fr_type = FR_TBL;
  frm_tab[i].fr_dirty = 0;
  insert_fifo(i);
  return;
}

int create_pd(int pid)
{
  int avail_fr;
  if(get_frm(&avail_fr) == OK)
  {
    init_pd_fr_t(avail_fr);

    proctab[pid].pdbr = (FRAME0 + avail_fr) * NBPG;

    pd_t *pd_t_i = (pd_t *)((FRAME0 + avail_fr) * NBPG);
    int i;
    for(i=0; i<1024; i++)
    {
      if(i<4)
      {
        pd_t_i->pd_pres = 1;
        pd_t_i->pd_base = glb_pt_fr[i];
      } else
      {
        pd_t_i->pd_pres = 0;
        pd_t_i->pd_base = 0;
      }
      pd_t_i->pt_write = 1;
      pd_t_i->pd_user = 0;
      pd_t_i->pd_pwt = 0;
      pd_t_i->pd_pcd = 0;
      pd_t_i->pd_acc = 0;
      pd_t_i->pd_mbz = 0;
      pd_t_i->pd_fmb = 0;
      pd_t_i->pd_global = 0;
      pd_t_i->pd_avail = 0;
      pd_t_i++;
    }

    return avail_fr;
  }

  return SYSERR;
}

void init_pd_fr_t(int i)
{
	frm_tab[i].fr_status = FRM_MAPPED;
	frm_tab[i].fr_pid = pid;
	frm_tab[i].fr_vpno = -1;
	frm_tab[i].fr_refcnt = 0;
  frm_tab[i].fr_refbit = 0;
	frm_tab[i].fr_type = FR_DIR;
  frm_tab[i].fr_dirty = 0;
	insert_fifo(i);
  return;
}