/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  STATWORD ps;
  disable(ps);

  unsigned long a = read_cr2();

  virt_addr_t vaddr;
  vaddr.pg_offset = (a & 0x00000fff);
	vaddr.pt_offset = ((a & 0x003ff000) >> 12);
	vaddr.pd_offset =	((a & 0xffc00000) >> 22);

  pd_t *pd = proctab[currpid].pdbr;

  bsd_t s;
  int o;
  if ((bsm_lookup(currpid, a, &s, &o) == SYSERR)) 
  {
		kill(currpid);
		restore(ps);
		return (SYSERR);
	}

  if((pd + sizeof(pd_t)*vaddr.pd_offset)->pd_pres == 0)
  {
    int pt_fr;
    if((pt_fr = create_pt(currpid)) == OK)
    {
      (pd + sizeof(pd_t)*vaddr.pd_offset)->pd_pres = 1;
      (pd + sizeof(pd_t)*vaddr.pd_offset)->pd_write = 1;
      (pd + sizeof(pd_t)*vaddr.pd_offset)->pd_base = FRAME0 + pt_fr;
      frm_tab[pt_fr].fr_refcnt++;
    }
    else 
    {
      restore(ps);
      return SYSERR;
    }
  }

  int f;
  if((f = get_frm()) == OK)
  {
    frm_tab[f].fr_status = FRM_MAPPED;
    frm_tab[f].fr_pid = currpid;
    frm_tab[f].fr_vpno = (a / NBPG) & 0x000fffff;
    frm_tab[f].fr_refcnt = 1;
    frm_tab[f].fr_refbit = 1;
    frm_tab[f].fr_type = FR_PAGE;
    frm_tab[f].fr_dirty = 0;

    read_bs((FRAME0 + f) * NBPG, s, o);

    insert_fifo(f);

    pt_t *pt = (pd + sizeof(pd_t)*vaddr.pd_offset)->pd_base * NBPG + 
               sizeof(pt_t) * vaddr.pt_offset;
    pt->pt_pres = 1;
    pt->pt_write = 1;
    pt->pt_base = FRAME0 + f;
  }
  else
  {
    restore(ps);
    return SYSERR;
  }

  write_cr3(currpid);
  restore(ps);
  return OK;
}


