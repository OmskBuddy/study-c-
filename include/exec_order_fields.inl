#if !defined(FIELD) || !defined(VAR_FIELD) || !defined(OPT_FIELD) || !defined(OPT_VAR_FIELD)
#  error You need to define FIELD, VAR_FIELD and OPT_FIELD macro
#else

VAR_FIELD(cl_ord_id, text, 18, 20)
FIELD(exec_id, text36, 38)
FIELD(filled_volume, binary4, 46)
FIELD(price, price, 50)
FIELD(active_volume, binary4, 58)
FIELD(liquidity_indicator, liquidity_indicator, 62)
OPT_VAR_FIELD(symbol, text, 8)
OPT_VAR_FIELD(last_mkt, text, 4)
OPT_VAR_FIELD(fee_code, text, 2)

#endif
