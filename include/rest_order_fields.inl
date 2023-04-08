#if !defined(FIELD) || !defined(VAR_FIELD) || !defined(OPT_FIELD) || !defined(OPT_VAR_FIELD)
#  error You need to define FIELD, VAR_FIELD and OPT_FIELD macro
#else

VAR_FIELD(cl_ord_id, text, 18, 20)
FIELD(reason, reason, 46)
OPT_FIELD(active_volume, binary4)
OPT_FIELD(secondary_order_id, text36)

#endif