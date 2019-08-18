#include "aml.h"

bool aml_get_pkt_length(uint8_t *buffer, uint64_t *pkg_length, uint64_t *offset)
{
    uint8_t lead_byte = *buffer;

    uint64_t length = 0;

    uint8_t pkt_length_bytes = lead_byte >> 6;
    switch(pkt_length_bytes)
    {
        case 0:
            length = lead_byte;
            break;
        case 1:
            length = *(buffer + 1);
            length = (length << 4) | (lead_byte & 0xf);
            break;
        case 2:
            length = *(buffer + 2);
            length = (length << 4) | *(buffer + 1);
            length = (length << 4) | (lead_byte & 0xf);
            break;
        case 3:
            length = *(buffer + 3);
            length = (length << 4) | *(buffer + 2);
            length = (length << 4) | *(buffer + 1);
            length = (length << 4) | (length & 0xf);
            break;
        default:
            return false;
            break;
    }

    *pkg_length = length;
    *offset = pkt_length_bytes + 1;

    return true;
}
