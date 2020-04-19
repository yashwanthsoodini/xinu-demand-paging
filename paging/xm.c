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
  if(npages <=0 || npages >= 128 || !valid_bsid(bs_id) || 
     bsm_tab[bs_id].bs_access == BS_PRIVATE)
    return SYSERR;

  return bsm_map(currpid, virtpage, source, npages);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  return bsm_unmap(currpid, virtpage, 0);
}
