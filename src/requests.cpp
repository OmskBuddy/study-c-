#include "requests.h"

#include <iostream>
#include <map>
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

std::string toBase(std::vector<unsigned char> & number, int base)
{
    const char * base_symbols = "0123456789ABCDEFGHIJKLMNOPQRASUVWXYZ";

    std::reverse(number.begin(), number.end());

    std::string result = "";
    long long num = 0;

    for (unsigned char el : number) {
        num = 256 * num + el;
    }

    while (num != 0) {
        result += base_symbols[num % base];
        num /= base;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

std::string getID(const std::vector<unsigned char> & message, int start, int len, int base = 10)
{
    std::vector<unsigned char> help(message.begin() + start, message.begin() + start + len);
    return toBase(help, base);
}

std::string trimZeroes(const std::string & str)
{
    std::string result = "";
    int i = 0;

    while (str[i] != '\0') {
        result += str[i++];
    }

    return result;
}

ExecutionDetails decode_order_execution(const std::vector<unsigned char> & message)
{
    std::vector<unsigned char> optional_fields = request_optional_fields_for_message(ResponseType::OrderExecution);
    ExecutionDetails exec_details;

#define char_ptr(x) reinterpret_cast<const char *>(&message[x])

    exec_details.cl_ord_id = trimZeroes(std::string{char_ptr(18), 20});
    exec_details.exec_id = trimZeroes(getID(message, 38, 8, 36));
    exec_details.filled_volume = atof(getID(message, 46, 4).c_str());
    exec_details.price = atof(getID(message, 50, 8).c_str()) / 10000;
    exec_details.active_volume = atof(getID(message, 58, 4).c_str());
    exec_details.liquidity_indicator = std::string{char_ptr(62), 1} == "A" ? LiquidityIndicator::Added : LiquidityIndicator::Removed;

    int num_of_bitfields = atoi(getID(message, 69, 1).c_str());
    std::vector<unsigned char> bitfields(message.begin() + 70, message.begin() + 70 + num_of_bitfields);

    exec_details.symbol = trimZeroes(std::string{char_ptr(70 + num_of_bitfields), 8});
    exec_details.last_mkt = trimZeroes(std::string{char_ptr(78 + num_of_bitfields), 4});
    exec_details.fee_code = trimZeroes(std::string{char_ptr(82 + num_of_bitfields), 2});

    return exec_details;

#undef char_ptr
}

std::map<std::string, RestatementReason> restReasons = {
        {"R", RestatementReason::Reroute},
        {"X", RestatementReason::LockedInCross},
        {"W", RestatementReason::Wash},
        {"L", RestatementReason::Reload},
        {"Q", RestatementReason::LiquidityUpdated}};

RestatementDetails decode_order_restatement(const std::vector<unsigned char> & message)
{
    std::vector<unsigned char> optional_fields = request_optional_fields_for_message(ResponseType::OrderRestatement);
    RestatementDetails restatement_details;

#define char_ptr(x) reinterpret_cast<const char *>(&message[x])

    restatement_details.cl_ord_id = trimZeroes(std::string{char_ptr(18), 20});
    restatement_details.reason = restReasons[std::string{char_ptr(46), 1}];

    int num_of_bitfields = atoi(getID(message, 48, 1).c_str());

    restatement_details.active_volume = atof(getID(message, 49 + num_of_bitfields, 4).c_str());
    restatement_details.secondary_order_id = trimZeroes(getID(message, 53 + num_of_bitfields, 8, 36).c_str());

    return restatement_details;

#undef char_ptr
}

std::vector<unsigned char> request_optional_fields_for_message(const ResponseType type)
{
    std::vector<unsigned char> result;

    switch (type) {
    case ResponseType::OrderExecution:
        result = {0, 1, 0, 0, 0, 0, 128, 1};
        break;
    case ResponseType::OrderRestatement:
        result = {0, 0, 0, 0, 2, 1};
        break;
    }

    return result;
}
