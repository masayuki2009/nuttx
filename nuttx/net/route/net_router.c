/****************************************************************************
 * net/route/net_router.c
 *
 *   Copyright (C) 2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <nuttx/net/ip.h>

#include "route/route.h"

#if defined(CONFIG_NET) && defined(CONFIG_NET_ROUTE)

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct route_match_s
{
  net_ipaddr_t target;   /* The target IP address on an external network to match */
  net_ipaddr_t router;   /* The IP address of the router on one of our networks*/
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Function: net_match
 *
 * Description:
 *   Return 1 if the route is available
 *
 * Parameters:
 *   route - The next route to examine
 *   arg   - The match values (cast to void*)
 *
 * Returned Value:
 *   0 if the entry is not a match; 1 if the entry matched and was cleared.
 *
 ****************************************************************************/

static int net_match(FAR struct net_route_s *route, FAR void *arg)
{
  FAR struct route_match_s *match = (FAR struct route_match_s *)arg;

  /* To match, the masked target addresses must be the same.  In the event
   * of multiple matches, only the first is returned.  There not (yet) any
   * concept for the precedence of networks.
   */

  if (net_ipaddr_maskcmp(route->target, match->target, route->netmask))
    {
      /* They match.. Copy the router address */

      net_ipaddr_copy(match->router, route->router);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function: net_router
 *
 * Description:
 *   Given an IP address on a external network, return the address of the
 *   router on a local network that can forward to the external network.
 *
 * Parameters:
 *   target - An IP address on a remote network to use in the lookup.
 *   router - The address of router on a local network that can forward our
 *     packets to the target.
 *
 *   NOTE:  For IPv6, router will be an array, for IPv4 it will be a scalar
 *   value.  Hence, the change in the function signature.
 *
 * Returned Value:
 *   OK on success; Negated errno on failure.
 *
 ****************************************************************************/

#ifdef CONFIG_NET_IPv6
int net_router(net_ipaddr_t target, net_ipaddr_t router)
#else
int net_router(net_ipaddr_t target, FAR net_ipaddr_t *router)
#endif
{
  struct route_match_s match;
  int ret;

  /* Set up the comparison structure */

  memset(&match, 0, sizeof(struct route_match_s));
  net_ipaddr_copy(match.target, target);

  /* Find an router entry with the routing table that can forward to this
   * address
   */

  ret = net_foreachroute(net_match, &match);
  if (ret > 0)
    {
      /* We found a route.  Return the router address. */

#ifdef CONFIG_NET_IPv6
      net_ipaddr_copy(router, match.target);
#else
      net_ipaddr_copy(*router, match.target);
#endif
      ret = OK;
    }
  else
    {
      /* There is no route for this address */

      ret = -ENOENT;
    }

  return ret;
}

#endif /* CONFIG_NET && CONFIG_NET_ROUTE */
