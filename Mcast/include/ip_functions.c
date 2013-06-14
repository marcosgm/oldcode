int iproute_get(int argc, char **argv)
{
	struct rtnl_handle rth;
	struct {
		struct nlmsghdr 	n;
		struct rtmsg 		r;
		char   			buf[1024];
	} req;
	char  *idev = NULL;
	char  *odev = NULL;
//	int connected = 0;
//	int from_ok = 0;

	memset(&req, 0, sizeof(req));

	iproute_reset_filter();

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_GETROUTE;
	req.r.rtm_family = AF_INET6;
	req.r.rtm_table = 0;
	req.r.rtm_protocol = 0;
	req.r.rtm_scope = 0;
	req.r.rtm_type = 0;
	req.r.rtm_src_len = 0;
	req.r.rtm_dst_len = 0;
	req.r.rtm_tos = 0;

	inet_prefix addr;
	
	get_prefix(&addr, *argv, req.r.rtm_family);
		/*if (req.r.rtm_family == AF_UNSPEC)
			req.r.rtm_family = addr.family;*/
		if (addr.bytelen)
			addattr_l(&req.n, sizeof(req), RTA_DST, &addr.data, addr.bytelen);
		req.r.rtm_dst_len = addr.bitlen;
	
	if (rtnl_open(&rth, 0) < 0)
		return(1);

	ll_init_map(&rth);
	
	if (idev || odev)  {
		int idx;

		if (idev) {
			if ((idx = ll_name_to_index(idev)) == 0) {
				fprintf(stderr, "Cannot find device \"%s\"\n", idev);
				return -1;
			}
			addattr32(&req.n, sizeof(req), RTA_IIF, idx);
		}
		if (odev) {
			if ((idx = ll_name_to_index(odev)) == 0) {
				fprintf(stderr, "Cannot find device \"%s\"\n", odev);
				return -1;
			}
			addattr32(&req.n, sizeof(req), RTA_OIF, idx);
		}
	}

	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = AF_INET;

	if (rtnl_talk(&rth, &req.n, 0, 0, &req.n, NULL, NULL) < 0)
		return(2);

	if (1) {
		struct rtmsg *r = NLMSG_DATA(&req.n);
		int len = req.n.nlmsg_len;
		struct rtattr * tb[RTA_MAX+1];

		if (print_route(NULL, &req.n, (void*)stdout) < 0) {
			fprintf(stderr, "An error :-)\n");
			return(1);
		}

		if (req.n.nlmsg_type != RTM_NEWROUTE) {
			fprintf(stderr, "Not a route?\n");
			return -1;
		}
		len -= NLMSG_LENGTH(sizeof(*r));
		if (len < 0) {
			fprintf(stderr, "Wrong len %d\n", len);
			return -1;
		}

		memset(tb, 0, sizeof(tb));
		parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

		if (tb[RTA_PREFSRC]) {
			tb[RTA_PREFSRC]->rta_type = RTA_SRC;
			r->rtm_src_len = 8*RTA_PAYLOAD(tb[RTA_PREFSRC]);
		} else if (!tb[RTA_SRC]) {
			fprintf(stderr, "Failed to connect the route\n");
			return -1;
		}
		if (!odev && tb[RTA_OIF])
			tb[RTA_OIF]->rta_type = 0;
		if (tb[RTA_GATEWAY])
			tb[RTA_GATEWAY]->rta_type = 0;
		if (!idev && tb[RTA_IIF])
			tb[RTA_IIF]->rta_type = 0;
		req.n.nlmsg_flags = NLM_F_REQUEST;
		req.n.nlmsg_type = RTM_GETROUTE;

		if (rtnl_talk(&rth, &req.n, 0, 0, &req.n, NULL, NULL) < 0)
			return(2);
	}
/* //REPETIDO!
	if (print_route(NULL, &req.n, (void*)stdout) < 0) {
		fprintf(stderr, "An error :-)\n");
		return(1);
	}
*/
	return(0);
}

int print_route(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[RTA_MAX+1];
	char abuf[256];
	inet_prefix dst;
	inet_prefix src;
	inet_prefix prefsrc;
	inet_prefix via;
	int host_len = -1;
	SPRINT_BUF(b1);
	

	if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
		fprintf(stderr, "Not a route: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	}
	if (filter.flushb && n->nlmsg_type != RTM_NEWROUTE)
		return 0;
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (r->rtm_family == AF_INET6)
		host_len = 128;
	else if (r->rtm_family == AF_INET)
		host_len = 32;
	else if (r->rtm_family == AF_DECnet)
		host_len = 16;
	else if (r->rtm_family == AF_IPX)
		host_len = 80;

	if (r->rtm_family == AF_INET6) {
		if (filter.tb) {
			if (filter.tb < 0) {
				if (!(r->rtm_flags&RTM_F_CLONED))
					return 0;
			} else {
				if (r->rtm_flags&RTM_F_CLONED)
					return 0;
				if (filter.tb == RT_TABLE_LOCAL) {
					if (r->rtm_type != RTN_LOCAL)
						return 0;
				} else if (filter.tb == RT_TABLE_MAIN) {
					if (r->rtm_type == RTN_LOCAL)
						return 0;
				} else {
					return 0;
				}
			}
		}
	} else {
		if (filter.tb > 0 && filter.tb != r->rtm_table)
			return 0;
	}
	if ((filter.protocol^r->rtm_protocol)&filter.protocolmask)
		return 0;
	if ((filter.scope^r->rtm_scope)&filter.scopemask)
		return 0;
	if ((filter.type^r->rtm_type)&filter.typemask)
		return 0;
	if ((filter.tos^r->rtm_tos)&filter.tosmask)
		return 0;
	if (filter.rdst.family &&
	    (r->rtm_family != filter.rdst.family || filter.rdst.bitlen > r->rtm_dst_len))
		return 0;
	if (filter.mdst.family &&
	    (r->rtm_family != filter.mdst.family ||
	     (filter.mdst.bitlen >= 0 && filter.mdst.bitlen < r->rtm_dst_len)))
		return 0;
	if (filter.rsrc.family &&
	    (r->rtm_family != filter.rsrc.family || filter.rsrc.bitlen > r->rtm_src_len))
		return 0;
	if (filter.msrc.family &&
	    (r->rtm_family != filter.msrc.family ||
	     (filter.msrc.bitlen >= 0 && filter.msrc.bitlen < r->rtm_src_len)))
		return 0;
	if (filter.rvia.family && r->rtm_family != filter.rvia.family)
		return 0;
	if (filter.rprefsrc.family && r->rtm_family != filter.rprefsrc.family)
		return 0;


	memset(tb, 0, sizeof(tb));
	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

	memset(&dst, 0, sizeof(dst));
	dst.family = r->rtm_family;
	if (tb[RTA_DST])
		memcpy(&dst.data, RTA_DATA(tb[RTA_DST]), (r->rtm_dst_len+7)/8);
	if (filter.rsrc.family || filter.msrc.family) {
		memset(&src, 0, sizeof(src));
		src.family = r->rtm_family;
		if (tb[RTA_SRC])
			memcpy(&src.data, RTA_DATA(tb[RTA_SRC]), (r->rtm_src_len+7)/8);
	}
	if (filter.rvia.bitlen>0) {
		memset(&via, 0, sizeof(via));
		via.family = r->rtm_family;
		if (tb[RTA_GATEWAY])
			memcpy(&via.data, RTA_DATA(tb[RTA_GATEWAY]), host_len);
	}
	if (filter.rprefsrc.bitlen>0) {
		memset(&prefsrc, 0, sizeof(prefsrc));
		prefsrc.family = r->rtm_family;
		if (tb[RTA_PREFSRC])
			memcpy(&prefsrc.data, RTA_DATA(tb[RTA_PREFSRC]), host_len);
	}

	if (filter.rdst.family && inet_addr_match(&dst, &filter.rdst, filter.rdst.bitlen))
		return 0;
	if (filter.mdst.family && filter.mdst.bitlen >= 0 &&
	    inet_addr_match(&dst, &filter.mdst, r->rtm_dst_len))
		return 0;

	if (filter.rsrc.family && inet_addr_match(&src, &filter.rsrc, filter.rsrc.bitlen))
		return 0;
	if (filter.msrc.family && filter.msrc.bitlen >= 0 &&
	    inet_addr_match(&src, &filter.msrc, r->rtm_src_len))
		return 0;

	if (filter.rvia.family && inet_addr_match(&via, &filter.rvia, filter.rvia.bitlen))
		return 0;
	if (filter.rprefsrc.family && inet_addr_match(&prefsrc, &filter.rprefsrc, filter.rprefsrc.bitlen))
		return 0;
	if (filter.realmmask) {
		__u32 realms = 0;
		if (tb[RTA_FLOW])
			realms = *(__u32*)RTA_DATA(tb[RTA_FLOW]);
		if ((realms^filter.realm)&filter.realmmask)
			return 0;
	}
	if (filter.iifmask) {
		int iif = 0;
		if (tb[RTA_IIF])
			iif = *(int*)RTA_DATA(tb[RTA_IIF]);
		if ((iif^filter.iif)&filter.iifmask)
			return 0;
	}
	if (filter.oifmask) {
		int oif = 0;
		if (tb[RTA_OIF])
			oif = *(int*)RTA_DATA(tb[RTA_OIF]);
		if ((oif^filter.oif)&filter.oifmask)
			return 0;
	}
	if (filter.flushb && 
	    r->rtm_family == AF_INET6 &&
	    r->rtm_dst_len == 0 &&
	    r->rtm_type == RTN_UNREACHABLE &&
	    tb[RTA_PRIORITY] &&
	    *(int*)RTA_DATA(tb[RTA_PRIORITY]) == -1)
		return 0;

	if (filter.flushb) {
		struct nlmsghdr *fn;
		if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
			if (flush_update())
				return -1;
		}
		fn = (struct nlmsghdr*)(filter.flushb + NLMSG_ALIGN(filter.flushp));
		memcpy(fn, n, n->nlmsg_len);
		fn->nlmsg_type = RTM_DELROUTE;
		fn->nlmsg_flags = NLM_F_REQUEST;
		fn->nlmsg_seq = ++filter.rth->seq;
		filter.flushp = (((char*)fn) + n->nlmsg_len) - filter.flushb;
		filter.flushed++;
		if (show_stats < 2)
			return 0;
	}

	if (n->nlmsg_type == RTM_DELROUTE)
		fprintf(fp, "Deleted ");
	if (r->rtm_type != RTN_UNICAST && !filter.type)
		fprintf(fp, "%s ", rtnl_rtntype_n2a(r->rtm_type, b1, sizeof(b1)));

	if (tb[RTA_DST]) {
		if (r->rtm_dst_len != host_len) {
			fprintf(fp, "%s/%u ", rt_addr_n2a(r->rtm_family,
							 RTA_PAYLOAD(tb[RTA_DST]),
							 RTA_DATA(tb[RTA_DST]),
							 abuf, sizeof(abuf)),
				r->rtm_dst_len
				);
		} else {
			fprintf(fp, "%s ", format_host(r->rtm_family,
						       RTA_PAYLOAD(tb[RTA_DST]),
						       RTA_DATA(tb[RTA_DST]),
						       abuf, sizeof(abuf))
				);
		}
	} else if (r->rtm_dst_len) {
		fprintf(fp, "0/%d ", r->rtm_dst_len);
	} else {
		fprintf(fp, "default ");
	}
	if (tb[RTA_SRC]) {
		if (r->rtm_src_len != host_len) {
			fprintf(fp, "from %s/%u ", rt_addr_n2a(r->rtm_family,
							 RTA_PAYLOAD(tb[RTA_SRC]),
							 RTA_DATA(tb[RTA_SRC]),
							 abuf, sizeof(abuf)),
				r->rtm_src_len
				);
		} else {
			fprintf(fp, "from %s ", format_host(r->rtm_family,
						       RTA_PAYLOAD(tb[RTA_SRC]),
						       RTA_DATA(tb[RTA_SRC]),
						       abuf, sizeof(abuf))
				);
		}
	} else if (r->rtm_src_len) {
		fprintf(fp, "from 0/%u ", r->rtm_src_len);
	}
	if (r->rtm_tos && filter.tosmask != -1) {
		SPRINT_BUF(b1);
		fprintf(fp, "tos %s ", rtnl_dsfield_n2a(r->rtm_tos, b1, sizeof(b1)));
	}
	if (tb[RTA_GATEWAY] && filter.rvia.bitlen != host_len) {
		fprintf(fp, "via %s ", 
			format_host(r->rtm_family,
				    RTA_PAYLOAD(tb[RTA_GATEWAY]),
				    RTA_DATA(tb[RTA_GATEWAY]),
				    abuf, sizeof(abuf)));
	}
	if (tb[RTA_OIF] && filter.oifmask != -1)
		fprintf(fp, "dev %s ", ll_index_to_name(*(int*)RTA_DATA(tb[RTA_OIF])));

	if (!(r->rtm_flags&RTM_F_CLONED)) {
		if (r->rtm_table != RT_TABLE_MAIN && !filter.tb)
			fprintf(fp, " table %s ", rtnl_rttable_n2a(r->rtm_table, b1, sizeof(b1)));
		if (r->rtm_protocol != RTPROT_BOOT && filter.protocolmask != -1)
			fprintf(fp, " proto %s ", rtnl_rtprot_n2a(r->rtm_protocol, b1, sizeof(b1)));
		if (r->rtm_scope != RT_SCOPE_UNIVERSE && filter.scopemask != -1)
			fprintf(fp, " scope %s ", rtnl_rtscope_n2a(r->rtm_scope, b1, sizeof(b1)));
	}
	if (tb[RTA_PREFSRC] && filter.rprefsrc.bitlen != host_len) {
		/* Do not use format_host(). It is our local addr
		   and symbolic name will not be useful.
		 */
		fprintf(fp, " src %s ", 
			rt_addr_n2a(r->rtm_family,
				    RTA_PAYLOAD(tb[RTA_PREFSRC]),
				    RTA_DATA(tb[RTA_PREFSRC]),
				    abuf, sizeof(abuf)));
	}
	if (tb[RTA_PRIORITY])
		fprintf(fp, " metric %d ", *(__u32*)RTA_DATA(tb[RTA_PRIORITY]));
	if (r->rtm_flags & RTNH_F_DEAD)
		fprintf(fp, "dead ");
	if (r->rtm_flags & RTNH_F_ONLINK)
		fprintf(fp, "onlink ");
	if (r->rtm_flags & RTNH_F_PERVASIVE)
		fprintf(fp, "pervasive ");
	if (r->rtm_flags & RTM_F_EQUALIZE)
		fprintf(fp, "equalize ");
	if (r->rtm_flags & RTM_F_NOTIFY)
		fprintf(fp, "notify ");

	if (tb[RTA_FLOW] && filter.realmmask != ~0U) {
		__u32 to = *(__u32*)RTA_DATA(tb[RTA_FLOW]);
		__u32 from = to>>16;
		to &= 0xFFFF;
		fprintf(fp, "realm%s ", from ? "s" : "");
		if (from) {
			fprintf(fp, "%s/",
				rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
		}
		fprintf(fp, "%s ",
			rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
	}
	if ((r->rtm_flags&RTM_F_CLONED) && r->rtm_family == AF_INET) {
		__u32 flags = r->rtm_flags&~0xFFFF;
		int first = 1;

		//fprintf(fp, "%s    cache ", _SL_);

#define PRTFL(fl,flname) if (flags&RTCF_##fl) { \
  flags &= ~RTCF_##fl; \
  fprintf(fp, "%s" flname "%s", first ? "<" : "", flags ? "," : "> "); \
  first = 0; }
		PRTFL(LOCAL, "local");
		PRTFL(REJECT, "reject");
		PRTFL(MULTICAST, "mc");
		PRTFL(BROADCAST, "brd");
		PRTFL(DNAT, "dst-nat");
		PRTFL(SNAT, "src-nat");
		PRTFL(MASQ, "masq");
		PRTFL(DIRECTDST, "dst-direct");
		PRTFL(DIRECTSRC, "src-direct");
		PRTFL(REDIRECTED, "redirected");
		PRTFL(DOREDIRECT, "redirect");
		PRTFL(FAST, "fastroute");
		PRTFL(NOTIFY, "notify");
		PRTFL(TPROXY, "proxy");
#ifdef RTCF_EQUALIZE
		PRTFL(EQUALIZE, "equalize");
#endif
		if (flags)
			fprintf(fp, "%s%x> ", first ? "<" : "", flags);
		if (tb[RTA_CACHEINFO]) {
			struct rta_cacheinfo *ci = RTA_DATA(tb[RTA_CACHEINFO]);
			static int hz;
			if (!hz)
				hz = get_hz();
			if (ci->rta_expires != 0)
				fprintf(fp, " expires %dsec", ci->rta_expires/hz);
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
			if (show_stats) {
				if (ci->rta_clntref)
					fprintf(fp, " users %d", ci->rta_clntref);
				if (ci->rta_used != 0)
					fprintf(fp, " used %d", ci->rta_used);
				if (ci->rta_lastuse != 0)
					fprintf(fp, " age %dsec", ci->rta_lastuse/hz);
			}
#ifdef RTNETLINK_HAVE_PEERINFO
			if (ci->rta_id)
				fprintf(fp, " ipid 0x%04x", ci->rta_id);
			if (ci->rta_ts || ci->rta_tsage)
				fprintf(fp, " ts 0x%x tsage %dsec", ci->rta_ts, ci->rta_tsage);
#endif
		}
	} else if (r->rtm_family == AF_INET6) {
		struct rta_cacheinfo *ci = NULL;
		if (tb[RTA_CACHEINFO])
			ci = RTA_DATA(tb[RTA_CACHEINFO]);
		if ((r->rtm_flags & RTM_F_CLONED) || (ci && ci->rta_expires)) {
			static int hz;
			if (!hz)
				hz = get_hz();
			if (r->rtm_flags & RTM_F_CLONED)
				//fprintf(fp, "%s    cache ", _SL_);
				fprintf(fp, "    cache ");
			if (ci->rta_expires)
				fprintf(fp, " expires %dsec", ci->rta_expires/hz);
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
			if (show_stats) {
				if (ci->rta_clntref)
					fprintf(fp, " users %d", ci->rta_clntref);
				if (ci->rta_used != 0)
					fprintf(fp, " used %d", ci->rta_used);
				if (ci->rta_lastuse != 0)
					fprintf(fp, " age %dsec", ci->rta_lastuse/hz);
			}
		} else if (ci) {
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
		}
	}
	if (tb[RTA_METRICS]) {
		int i;
		unsigned mxlock = 0;
		struct rtattr *mxrta[RTAX_MAX+1];

		memset(mxrta, 0, sizeof(mxrta));

		parse_rtattr(mxrta, RTAX_MAX, RTA_DATA(tb[RTA_METRICS]),
			    RTA_PAYLOAD(tb[RTA_METRICS]));
		if (mxrta[RTAX_LOCK])
			mxlock = *(unsigned*)RTA_DATA(mxrta[RTAX_LOCK]);

		for (i=2; i<=RTAX_MAX; i++) {
			static char *mx_names[] = 
			{
				"mtu",
				"window",
				"rtt",
				"rttvar",
				"ssthresh",
				"cwnd",
				"advmss",
				"reordering",
			};
			static int hz;
			if (mxrta[i] == NULL)
				continue;
			if (!hz)
				hz = get_hz();
			if (i-2 < sizeof(mx_names)/sizeof(char*))
				fprintf(fp, " %s", mx_names[i-2]);
			else
				fprintf(fp, " metric%d", i);
			if (mxlock & (1<<i))
				fprintf(fp, " lock");

			if (i != RTAX_RTT && i != RTAX_RTTVAR)
				fprintf(fp, " %u", *(unsigned*)RTA_DATA(mxrta[i]));
			else {
				unsigned val = *(unsigned*)RTA_DATA(mxrta[i]);

				val *= 1000;
				if (i == RTAX_RTT)
					val /= 8;
				else
					val /= 4;
				if (val >= hz)
					fprintf(fp, " %ums", val/hz);
				else
					fprintf(fp, " %.2fms", (float)val/hz);
			}
		}
	}
	if (tb[RTA_IIF] && filter.iifmask != -1) {
		fprintf(fp, " iif %s", ll_index_to_name(*(int*)RTA_DATA(tb[RTA_IIF])));
	}
	if (tb[RTA_MULTIPATH]) {
		struct rtnexthop *nh = RTA_DATA(tb[RTA_MULTIPATH]);
		int first = 0;

		len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);

		for (;;) {
			if (len < sizeof(*nh))
				break;
			if (nh->rtnh_len > len)
				break;
			if (r->rtm_flags&RTM_F_CLONED && r->rtm_type == RTN_MULTICAST) {
				if (first)
					fprintf(fp, " Oifs:");
				else
					fprintf(fp, " ");
			} else
			//	fprintf(fp, "%s\tnexthop", _SL_);
				fprintf(fp, "\tnexthop");
			if (nh->rtnh_len > sizeof(*nh)) {
				memset(tb, 0, sizeof(tb));
				parse_rtattr(tb, RTA_MAX, RTNH_DATA(nh), nh->rtnh_len - sizeof(*nh));
				if (tb[RTA_GATEWAY]) {
					fprintf(fp, " via %s ", 
						format_host(r->rtm_family,
							    RTA_PAYLOAD(tb[RTA_GATEWAY]),
							    RTA_DATA(tb[RTA_GATEWAY]),
							    abuf, sizeof(abuf)));
				}
			}
			if (r->rtm_flags&RTM_F_CLONED && r->rtm_type == RTN_MULTICAST) {
				fprintf(fp, " %s", ll_index_to_name(nh->rtnh_ifindex));
				if (nh->rtnh_hops != 1)
					fprintf(fp, "(ttl>%d)", nh->rtnh_hops);
			} else {
				fprintf(fp, " dev %s", ll_index_to_name(nh->rtnh_ifindex));
				fprintf(fp, " weight %d", nh->rtnh_hops+1);
			}
			if (nh->rtnh_flags & RTNH_F_DEAD)
				fprintf(fp, " dead");
			if (nh->rtnh_flags & RTNH_F_ONLINK)
				fprintf(fp, " onlink");
			if (nh->rtnh_flags & RTNH_F_PERVASIVE)
				fprintf(fp, " pervasive");
			len -= NLMSG_ALIGN(nh->rtnh_len);
			nh = RTNH_NEXT(nh);
		}
	}
	fprintf(fp, "\n");
	fflush(fp);
	return 0;
}
char *rtnl_rtntype_n2a(int id, char *buf, int len)
{
	switch (id) {
	case RTN_UNSPEC:
		return "none";
	case RTN_UNICAST:
		return "unicast";
	case RTN_LOCAL:
		return "local";
	case RTN_BROADCAST:
		return "broadcast";
	case RTN_ANYCAST:
		return "anycast";
	case RTN_MULTICAST:
		return "multicast";
	case RTN_BLACKHOLE:
		return "blackhole";
	case RTN_UNREACHABLE:
		return "unreachable";
	case RTN_PROHIBIT:
		return "prohibit";
	case RTN_THROW:
		return "throw";
	case RTN_NAT:
		return "nat";
	case RTN_XRESOLVE:
		return "xresolve";
	default:
		snprintf(buf, len, "%d", id);
		return buf;
	}
}


int rtnl_rtntype_a2n(int *id, char *arg)
{
	char *end;
	unsigned long res;

	if (strcmp(arg, "local") == 0)
		res = RTN_LOCAL;
	else if (strcmp(arg, "nat") == 0)
		res = RTN_NAT;
	else if (matches(arg, "broadcast") == 0 ||
		 strcmp(arg, "brd") == 0)
		res = RTN_BROADCAST;
	else if (matches(arg, "anycast") == 0)
		res = RTN_ANYCAST;
	else if (matches(arg, "multicast") == 0)
		res = RTN_MULTICAST;
	else if (matches(arg, "prohibit") == 0)
		res = RTN_PROHIBIT;
	else if (matches(arg, "unreachable") == 0)
		res = RTN_UNREACHABLE;
	else if (matches(arg, "blackhole") == 0)
		res = RTN_BLACKHOLE;
	else if (matches(arg, "xresolve") == 0)
		res = RTN_XRESOLVE;
	else if (matches(arg, "unicast") == 0)
		res = RTN_UNICAST;
	else if (strcmp(arg, "throw") == 0)
		res = RTN_THROW;
	else {
		res = strtoul(arg, &end, 0);
		if (!end || end == arg || *end || res > 255)
			return -1;
	}
	*id = res;
	return 0;
}
