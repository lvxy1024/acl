#ifndef	__ACL_FIBER_MUTEX_INCLUDE_H__
#define	__ACL_FIBER_MUTEX_INCLUDE_H__

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ACL_FIBER_MUTEX ACL_FIBER_MUTEX;

#define	FIBER_MUTEX_F_LOCK_TRY	(1 << 0)

FIBER_API ACL_FIBER_MUTEX *acl_fiber_mutex_create(unsigned flag);
FIBER_API void acl_fiber_mutex_free(ACL_FIBER_MUTEX *mutex);
FIBER_API int acl_fiber_mutex_lock(ACL_FIBER_MUTEX *mutex);
FIBER_API int acl_fiber_mutex_trylock(ACL_FIBER_MUTEX *mutex);
FIBER_API int acl_fiber_mutex_unlock(ACL_FIBER_MUTEX *mutex);

#ifdef __cplusplus
}
#endif

#endif