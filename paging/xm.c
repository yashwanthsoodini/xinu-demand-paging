/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  if((virtpage < 4096) || !valid_bsid(source) || npages <=0 || npages >= 128 ||  
     bsm_tab[source].bs_access == BS_PRIVATE)
    return SYSERR;

  return bsm_map(currpid, virtpage, source, npages);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  if(bsm_unmap(currpid, virtpage, 0) == OK)
  {
    write_cr3(currpid);
    return OK;
  }

  return SYSERR;
}
