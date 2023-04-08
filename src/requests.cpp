#include "requests.h"

#include <vector>

namespace {

void encode_new_order_opt_fields(unsigned char * bitfield_start,
                                 const double price,
                                 const char ord_type,
                                 const char time_in_force,
                                 const unsigned max_floor,
                                 const std::string & symbol,
                                 const char capacity,
                                 const std::string & account)
{
    auto * p = bitfield_start + new_order_bitfield_num();
#define FIELD(name, bitfield_num, bit)                    \
    set_opt_field_bit(bitfield_start, bitfield_num, bit); \
    p = encode_field_##name(p, name);
#include "new_order_opt_fields.inl"
}

uint8_t encode_request_type(const RequestType type)
{
    switch (type) {
    case RequestType::New:
        return 0x38;
    }
    return 0;
}

unsigned char * add_request_header(unsigned char * start, unsigned length, const RequestType type, unsigned seq_no)
{
    *start++ = 0xBA;
    *start++ = 0xBA;
    start = encode(start, static_cast<uint16_t>(length));
    start = encode(start, encode_request_type(type));
    *start++ = 0;
    return encode(start, seq_no);
}

char convert_side(const Side side)
{
    switch (side) {
    case Side::Buy: return '1';
    case Side::Sell: return '2';
    }
    return 0;
}

char convert_ord_type(const OrdType ord_type)
{
    switch (ord_type) {
    case OrdType::Market: return '1';
    case OrdType::Limit: return '2';
    case OrdType::Pegged: return 'P';
    }
    return 0;
}

char convert_time_in_force(const TimeInForce time_in_force)
{
    switch (time_in_force) {
    case TimeInForce::Day: return '0';
    case TimeInForce::IOC: return '3';
    case TimeInForce::GTD: return '6';
    }
    return 0;
}

char convert_capacity(const Capacity capacity)
{
    switch (capacity) {
    case Capacity::Agency: return 'A';
    case Capacity::Principal: return 'P';
    case Capacity::RisklessPrincipal: return 'R';
    }
    return 0;
}

} // anonymous namespace

std::array<unsigned char, calculate_size(RequestType::New)> create_new_order_request(const unsigned seq_no,
                                                                                     const std::string & cl_ord_id,
                                                                                     const Side side,
                                                                                     const double volume,
                                                                                     const double price,
                                                                                     const OrdType ord_type,
                                                                                     const TimeInForce time_in_force,
                                                                                     const double max_floor,
                                                                                     const std::string & symbol,
                                                                                     const Capacity capacity,
                                                                                     const std::string & account)
{
    static_assert(calculate_size(RequestType::New) == 78, "Wrong New Order message size");

    std::array<unsigned char, calculate_size(RequestType::New)> msg;
    auto * p = add_request_header(&msg[0], msg.size() - 2, RequestType::New, seq_no);
    p = encode_text(p, cl_ord_id, 20);
    p = encode_char(p, convert_side(side));
    p = encode_binary4(p, static_cast<uint32_t>(volume));
    p = encode(p, static_cast<uint8_t>(new_order_bitfield_num()));
    encode_new_order_opt_fields(p,
                                price,
                                convert_ord_type(ord_type),
                                convert_time_in_force(time_in_force),
                                max_floor,
                                symbol,
                                convert_capacity(capacity),
                                account);
    return msg;
}

void decode_text(unsigned const char * start, const size_t size, std::string & str)
{
    decode(start, size, str);
}

std::string convert_to_base(int64_t value, int radix)
{
    const char * base_symbols = "0123456789ABCDEFGHIJKLMNOPQRASUVWXYZ";
    std::string result = "";

    while (value != 0) {
        result += base_symbols[value % radix];
        value /= radix;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

void decode_text36(unsigned const char * start, std::string & str)
{
    int64_t temp = 0;
    decode(start, temp);
    str = convert_to_base(temp, 36);
}

void decode_price(unsigned const char * start, double & value)
{
    int64_t temp;
    decode(start, temp);
    value = static_cast<double>(temp) / 10000;
}

void decode_binary4(unsigned const char * start, double & value)
{
    int32_t temp;
    decode(start, temp);
    value = static_cast<uint32_t>(temp);
}

void decode_liquidity_indicator(unsigned const char * start, LiquidityIndicator & value)
{
    value = convert_liquidity_indicator(*start);
}

void decode_reason(unsigned const char * start, RestatementReason & value)
{
    value = convert_restatement_reason(*start);
}

#define FIELD(name, type, offset) decode_##type(start + offset, ORDER.name);
#define VAR_FIELD(name, type, offset, size) decode_##type(start + offset, size, ORDER.name);
#define OPT_VAR_FIELD(name, type, size)                      \
    decode_##type(last_opt_field_offset, size, ORDER.name); \
    last_opt_field_offset += size;
#define OPT_FIELD(name, type)                          \
    decode_##type(last_opt_field_offset, ORDER.name); \
    last_opt_field_offset += type##_size;

ExecutionDetails decode_order_execution(const std::vector<unsigned char> & message)
{
#define BITFIELD_OFFSET 70
#define ORDER exec_details

    ExecutionDetails exec_details;
    unsigned const char * start = &message[0];
    unsigned const char * last_opt_field_offset = start + BITFIELD_OFFSET + exec_order_bitfield_num();

#include "exec_order_fields.inl"

    return exec_details;

#undef BITFIELD_OFFSET
#undef ORDER
}

RestatementDetails decode_order_restatement(const std::vector<unsigned char> & message)
{
#define BITFIELD_OFFSET 49
#define ORDER restatement_details

    RestatementDetails restatement_details;
    unsigned const char * start = &message[0];
    unsigned const char * last_opt_field_offset = start + BITFIELD_OFFSET + rest_order_bitfield_num();

#include "rest_order_fields.inl"

#undef FIELD
#undef VAR_FIELD
#undef OPT_FIELD
#undef OPT_VAR_FIELD

    return restatement_details;

#undef BITFIELD_OFFSET
#undef ORDER
}

std::vector<unsigned char> request_optional_fields_for_message(const ResponseType type)
{
    std::vector<unsigned char> result;

    switch (type) {
    case ResponseType::OrderExecution:
        result.resize(exec_order_bitfield_num());
#define FIELD(name, bitfield_num, bit) \
    set_opt_field_bit(&result[0], bitfield_num, bit);
#include "exec_order_opt_fields.inl"
        break;
    case ResponseType::OrderRestatement:
        result.resize(rest_order_bitfield_num());
#define FIELD(name, bitfield_num, bit) \
    set_opt_field_bit(&result[0], bitfield_num, bit);
#include "rest_order_opt_fields.inl"
        break;
    }
#undef FIELD
    return result;
}
