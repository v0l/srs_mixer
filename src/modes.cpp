#include <ads-b/modes.hpp>

namespace ssr::ads_b::transport
{
    uint32_t ModeS::modesChecksum(unsigned char *msg, int bits)
    {
        uint32_t crc = 0;
        int offset = (bits == 112) ? 0 : (112 - 56);
        int j;

        for (j = 0; j < bits; j++)
        {
            int byte = j / 8;
            int bit = j % 8;
            int bitmask = 1 << (7 - bit);

            /* If bit is set, xor with corresponding table entry. */
            if (msg[byte] & bitmask)
                crc ^= modes_checksum_table[j + offset];
        }
        return crc; /* 24 bit checksum. */
    }

    int ModeS::modesMessageLenByType(int type)
    {
        if (type == 16 || type == 17 ||
            type == 19 || type == 20 ||
            type == 21)
            return MODES_LONG_MSG_BITS;
        else
            return MODES_SHORT_MSG_BITS;
    }

    int ModeS::fixSingleBitErrors(unsigned char *msg, int bits)
    {
        int j;
        unsigned char aux[MODES_LONG_MSG_BITS / 8];

        for (j = 0; j < bits; j++)
        {
            int byte = j / 8;
            int bitmask = 1 << (7 - (j % 8));
            uint32_t crc1, crc2;

            memcpy(aux, msg, bits / 8);
            aux[byte] ^= bitmask; /* Flip j-th bit. */

            crc1 = ((uint32_t)aux[(bits / 8) - 3] << 16) |
                   ((uint32_t)aux[(bits / 8) - 2] << 8) |
                   (uint32_t)aux[(bits / 8) - 1];
            crc2 = modesChecksum(aux, bits);

            if (crc1 == crc2)
            {
                /* The error is fixed. Overwrite the original buffer with
                    * the corrected sequence, and returns the error bit
                    * position. */
                memcpy(msg, aux, bits / 8);
                return j;
            }
        }
        return -1;
    }

    int ModeS::fixTwoBitsErrors(unsigned char *msg, int bits)
    {
        int j, i;
        unsigned char aux[MODES_LONG_MSG_BITS / 8];

        for (j = 0; j < bits; j++)
        {
            int byte1 = j / 8;
            int bitmask1 = 1 << (7 - (j % 8));

            /* Don't check the same pairs multiple times, so i starts from j+1 */
            for (i = j + 1; i < bits; i++)
            {
                int byte2 = i / 8;
                int bitmask2 = 1 << (7 - (i % 8));
                uint32_t crc1, crc2;

                memcpy(aux, msg, bits / 8);

                aux[byte1] ^= bitmask1; /* Flip j-th bit. */
                aux[byte2] ^= bitmask2; /* Flip i-th bit. */

                crc1 = ((uint32_t)aux[(bits / 8) - 3] << 16) |
                       ((uint32_t)aux[(bits / 8) - 2] << 8) |
                       (uint32_t)aux[(bits / 8) - 1];
                crc2 = modesChecksum(aux, bits);

                if (crc1 == crc2)
                {
                    /* The error is fixed. Overwrite the original buffer with
                        * the corrected sequence, and returns the error bit
                        * position. */
                    memcpy(msg, aux, bits / 8);
                    /* We return the two bits as a 16 bit integer by shifting
                        * 'i' on the left. This is possible since 'i' will always
                        * be non-zero because i starts from j+1. */
                    return j | (i << 8);
                }
            }
        }
        return -1;
    }

    uint32_t ModeS::ICAOCacheHashAddress(uint32_t a)
    {
        /* The following three rounds wil make sure that every bit affects
            * every output bit with ~ 50% of probability. */
        a = ((a >> 16) ^ a) * 0x45d9f3b;
        a = ((a >> 16) ^ a) * 0x45d9f3b;
        a = ((a >> 16) ^ a);
        return a & (MODES_ICAO_CACHE_LEN - 1);
    }

    void ModeS::addRecentlySeenICAOAddr(uint32_t addr)
    {
        uint32_t h = ICAOCacheHashAddress(addr);
        icao_cache[h * 2] = addr;
        icao_cache[h * 2 + 1] = (uint32_t)time(NULL);
    }

    int ModeS::ICAOAddressWasRecentlySeen(uint32_t addr)
    {
        uint32_t h = ICAOCacheHashAddress(addr);
        uint32_t a = icao_cache[h * 2];
        uint32_t t = icao_cache[h * 2 + 1];

        return a && a == addr && time(NULL) - t <= MODES_ICAO_CACHE_TTL;
    }

    int ModeS::bruteForceAP(unsigned char *msg, struct modesMessage *mm)
    {
        unsigned char aux[MODES_LONG_MSG_BYTES];
        int msgtype = mm->msgtype;
        int msgbits = mm->msgbits;

        if (msgtype == 0 ||  /* Short air surveillance */
            msgtype == 4 ||  /* Surveillance, altitude reply */
            msgtype == 5 ||  /* Surveillance, identity reply */
            msgtype == 16 || /* Long Air-Air survillance */
            msgtype == 20 || /* Comm-A, altitude request */
            msgtype == 21 || /* Comm-A, identity request */
            msgtype == 24)   /* Comm-C ELM */
        {
            uint32_t addr;
            uint32_t crc;
            int lastbyte = (msgbits / 8) - 1;

            /* Work on a copy. */
            memcpy(aux, msg, msgbits / 8);

            /* Compute the CRC of the message and XOR it with the AP field
                * so that we recover the address, because:
                *
                * (ADDR xor CRC) xor CRC = ADDR. */
            crc = modesChecksum(aux, msgbits);
            aux[lastbyte] ^= crc & 0xff;
            aux[lastbyte - 1] ^= (crc >> 8) & 0xff;
            aux[lastbyte - 2] ^= (crc >> 16) & 0xff;

            /* If the obtained address exists in our cache we consider
                * the message valid. */
            addr = aux[lastbyte] | (aux[lastbyte - 1] << 8) | (aux[lastbyte - 2] << 16);
            if (ICAOAddressWasRecentlySeen(addr))
            {
                mm->aa1 = aux[lastbyte - 2];
                mm->aa2 = aux[lastbyte - 1];
                mm->aa3 = aux[lastbyte];
                return 1;
            }
        }
        return 0;
    }

    int ModeS::decodeAC13Field(unsigned char *msg, int *unit)
    {
        int m_bit = msg[3] & (1 << 6);
        int q_bit = msg[3] & (1 << 4);

        if (!m_bit)
        {
            *unit = MODES_UNIT_FEET;
            if (q_bit)
            {
                /* N is the 11 bit integer resulting from the removal of bit
                    * Q and M */
                int n = ((msg[2] & 31) << 6) |
                        ((msg[3] & 0x80) >> 2) |
                        ((msg[3] & 0x20) >> 1) |
                        (msg[3] & 15);
                /* The final altitude is due to the resulting number multiplied
                    * by 25, minus 1000. */
                return n * 25 - 1000;
            }
            else
            {
                /* TODO: Implement altitude where Q=0 and M=0 */
            }
        }
        else
        {
            *unit = MODES_UNIT_METERS;
            /* TODO: Implement altitude when meter unit is selected. */
        }
        return 0;
    }

    int ModeS::decodeAC12Field(unsigned char *msg, int *unit)
    {
        int q_bit = msg[5] & 1;

        if (q_bit)
        {
            /* N is the 11 bit integer resulting from the removal of bit
                * Q */
            *unit = MODES_UNIT_FEET;
            int n = ((msg[5] >> 1) << 4) | ((msg[6] & 0xF0) >> 4);
            /* The final altitude is due to the resulting number multiplied
                * by 25, minus 1000. */
            return n * 25 - 1000;
        }
        else
        {
            return 0;
        }
    }

    void ModeS::decodeModesMessage(struct modesMessage *mm, unsigned char *msg)
    {
        uint32_t crc2; /* Computed CRC, used to verify the message CRC. */
        const char *ais_charset = "?ABCDEFGHIJKLMNOPQRSTUVWXYZ????? ???????????????0123456789??????";

        /* Work on our local copy */
        memcpy(mm->msg, msg, MODES_LONG_MSG_BYTES);
        msg = mm->msg;

        /* Get the message type ASAP as other operations depend on this */
        mm->msgtype = msg[0] >> 3; /* Downlink Format */
        mm->msgbits = modesMessageLenByType(mm->msgtype);

        /* CRC is always the last three bytes. */
        mm->crc = ((uint32_t)msg[(mm->msgbits / 8) - 3] << 16) |
                  ((uint32_t)msg[(mm->msgbits / 8) - 2] << 8) |
                  (uint32_t)msg[(mm->msgbits / 8) - 1];
        crc2 = modesChecksum(msg, mm->msgbits);

        /* Check CRC and fix single bit errors using the CRC when
            * possible (DF 11 and 17). */
        mm->errorbit = -1; /* No error */
        mm->crcok = (mm->crc == crc2);

        if (!mm->crcok && FIX_1_BIT_ERRORS &&
            (mm->msgtype == 11 || mm->msgtype == 17))
        {
            if ((mm->errorbit = fixSingleBitErrors(msg, mm->msgbits)) != -1)
            {
                mm->crc = modesChecksum(msg, mm->msgbits);
                mm->crcok = 1;
            }
            else if (FIX_2_BIT_ERRORS && mm->msgtype == 17 &&
                     (mm->errorbit = fixTwoBitsErrors(msg, mm->msgbits)) != -1)
            {
                mm->crc = modesChecksum(msg, mm->msgbits);
                mm->crcok = 1;
            }
        }

        /* Note that most of the other computation happens *after* we fix
            * the single bit errors, otherwise we would need to recompute the
            * fields again. */
        mm->ca = msg[0] & 7; /* Responder capabilities. */

        /* ICAO address */
        mm->aa1 = msg[1];
        mm->aa2 = msg[2];
        mm->aa3 = msg[3];

        /* DF 17 type (assuming this is a DF17, otherwise not used) */
        mm->metype = msg[4] >> 3; /* Extended squitter message type. */
        mm->mesub = msg[4] & 7;   /* Extended squitter message subtype. */

        /* Fields for DF4,5,20,21 */
        mm->fs = msg[0] & 7;           /* Flight status for DF4,5,20,21 */
        mm->dr = msg[1] >> 3 & 31;     /* Request extraction of downlink request. */
        mm->um = ((msg[1] & 7) << 3) | /* Request extraction of downlink request. */
                 msg[2] >> 5;

        /* In the squawk (identity) field bits are interleaved like that
            * (message bit 20 to bit 32):
            *
            * C1-A1-C2-A2-C4-A4-ZERO-B1-D1-B2-D2-B4-D4
            *
            * So every group of three bits A, B, C, D represent an integer
            * from 0 to 7.
            *
            * The actual meaning is just 4 octal numbers, but we convert it
            * into a base ten number tha happens to represent the four
            * octal numbers.
            *
            * For more info: http://en.wikipedia.org/wiki/Gillham_code */
        {
            int a, b, c, d;

            a = ((msg[3] & 0x80) >> 5) |
                ((msg[2] & 0x02) >> 0) |
                ((msg[2] & 0x08) >> 3);
            b = ((msg[3] & 0x02) << 1) |
                ((msg[3] & 0x08) >> 2) |
                ((msg[3] & 0x20) >> 5);
            c = ((msg[2] & 0x01) << 2) |
                ((msg[2] & 0x04) >> 1) |
                ((msg[2] & 0x10) >> 4);
            d = ((msg[3] & 0x01) << 2) |
                ((msg[3] & 0x04) >> 1) |
                ((msg[3] & 0x10) >> 4);
            mm->identity = a * 1000 + b * 100 + c * 10 + d;
        }

        /* DF 11 & 17: try to populate our ICAO addresses whitelist.
            * DFs with an AP field (xored addr and crc), try to decode it. */
        if (mm->msgtype != 11 && mm->msgtype != 17)
        {
            /* Check if we can check the checksum for the Downlink Formats where
                * the checksum is xored with the aircraft ICAO address. We try to
                * brute force it using a list of recently seen aircraft addresses. */
            if (bruteForceAP(msg, mm))
            {
                /* We recovered the message, mark the checksum as valid. */
                mm->crcok = 1;
            }
            else
            {
                mm->crcok = 0;
            }
        }
        else
        {
            /* If this is DF 11 or DF 17 and the checksum was ok,
                * we can add this address to the list of recently seen
                * addresses. */
            if (mm->crcok && mm->errorbit == -1)
            {
                uint32_t addr = (mm->aa1 << 16) | (mm->aa2 << 8) | mm->aa3;
                addRecentlySeenICAOAddr(addr);
            }
        }

        /* Decode 13 bit altitude for DF0, DF4, DF16, DF20 */
        if (mm->msgtype == 0 || mm->msgtype == 4 ||
            mm->msgtype == 16 || mm->msgtype == 20)
        {
            mm->altitude = decodeAC13Field(msg, &mm->unit);
        }

        /* Decode extended squitter specific stuff. */
        if (mm->msgtype == 17)
        {
            /* Decode the extended squitter message. */

            if (mm->metype >= 1 && mm->metype <= 4)
            {
                /* Aircraft Identification and Category */
                mm->aircraft_type = mm->metype - 1;
                mm->flight[0] = ais_charset[msg[5] >> 2];
                mm->flight[1] = ais_charset[((msg[5] & 3) << 4) | (msg[6] >> 4)];
                mm->flight[2] = ais_charset[((msg[6] & 15) << 2) | (msg[7] >> 6)];
                mm->flight[3] = ais_charset[msg[7] & 63];
                mm->flight[4] = ais_charset[msg[8] >> 2];
                mm->flight[5] = ais_charset[((msg[8] & 3) << 4) | (msg[9] >> 4)];
                mm->flight[6] = ais_charset[((msg[9] & 15) << 2) | (msg[10] >> 6)];
                mm->flight[7] = ais_charset[msg[10] & 63];
                mm->flight[8] = '\0';
            }
            else if (mm->metype >= 9 && mm->metype <= 18)
            {
                /* Airborne position Message */
                mm->fflag = msg[6] & (1 << 2);
                mm->tflag = msg[6] & (1 << 3);
                mm->altitude = decodeAC12Field(msg, &mm->unit);
                mm->raw_latitude = ((msg[6] & 3) << 15) |
                                   (msg[7] << 7) |
                                   (msg[8] >> 1);
                mm->raw_longitude = ((msg[8] & 1) << 16) |
                                    (msg[9] << 8) |
                                    msg[10];
            }
            else if (mm->metype == 19 && mm->mesub >= 1 && mm->mesub <= 4)
            {
                /* Airborne Velocity Message */
                if (mm->mesub == 1 || mm->mesub == 2)
                {
                    mm->ew_dir = (msg[5] & 4) >> 2;
                    mm->ew_velocity = ((msg[5] & 3) << 8) | msg[6];
                    mm->ns_dir = (msg[7] & 0x80) >> 7;
                    mm->ns_velocity = ((msg[7] & 0x7f) << 3) | ((msg[8] & 0xe0) >> 5);
                    mm->vert_rate_source = (msg[8] & 0x10) >> 4;
                    mm->vert_rate_sign = (msg[8] & 0x8) >> 3;
                    mm->vert_rate = ((msg[8] & 7) << 6) | ((msg[9] & 0xfc) >> 2);
                    /* Compute velocity and angle from the two speed
                        * components. */
                    mm->velocity = sqrt(mm->ns_velocity * mm->ns_velocity +
                                        mm->ew_velocity * mm->ew_velocity);
                    if (mm->velocity)
                    {
                        int ewv = mm->ew_velocity;
                        int nsv = mm->ns_velocity;
                        double heading;

                        if (mm->ew_dir)
                            ewv *= -1;
                        if (mm->ns_dir)
                            nsv *= -1;
                        heading = atan2(ewv, nsv);

                        /* Convert to degrees. */
                        mm->heading = heading * 360 / (M_PI * 2);
                        /* We don't want negative values but a 0-360 scale. */
                        if (mm->heading < 0)
                            mm->heading += 360;
                    }
                    else
                    {
                        mm->heading = 0;
                    }
                }
                else if (mm->mesub == 3 || mm->mesub == 4)
                {
                    mm->heading_is_valid = msg[5] & (1 << 2);
                    mm->heading = (360.0 / 128) * (((msg[5] & 3) << 5) |
                                                   (msg[6] >> 3));
                }
            }
        }
        mm->phase_corrected = 0; /* Set to 1 by the caller if needed. */
    }

    int ModeS::hexDigitVal(int c)
    {
        c = tolower(c);
        if (c >= '0' && c <= '9')
            return c - '0';
        else if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        else
            return -1;
    }

} // namespace ssr::decoder