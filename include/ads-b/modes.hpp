/* Mode1090, a Mode S messages decoder
 *
 * Copyright (C) 2012 by Salvatore Sanfilippo <antirez@gmail.com>
 * Copyright (C) 2020 by Kieran Harkin <kieran+git@harkin.me>
 * 
 * All rights reserved.
 * 
 */

#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <memory.h>
#include <cmath>

#include <ads-b/system.hpp>

#define FIX_1_BIT_ERRORS true
#define FIX_2_BIT_ERRORS true

#define MODES_SHORT_MSG_BITS 56
#define MODES_LONG_MSG_BITS 112
#define MODES_LONG_MSG_BYTES (MODES_LONG_MSG_BITS / 8)

#define MODES_ICAO_CACHE_LEN 1024 /* Power of two required. */
#define MODES_ICAO_CACHE_TTL 60   /* Time to live of cached addresses. */
#define MODES_UNIT_FEET 0
#define MODES_UNIT_METERS 1

namespace ssr::ads_b::transport
{
    enum class DownlinkMsgType
    {
        ShortAirSurveillance = 0,    /* Short air surveillance */
        Surveillance_AltReply = 4,   /* Surveillance, altitude reply */
        Surveillance_IdentReply = 5, /* Surveillance, identity reply */
        AllCallReplyAddress = 11,    /* All-Call reply containing aircraft address */
        LongAirAirSurveillance = 16, /* Long Air-Air survillance */
        ExtendedSquitter = 17,       /* Extended Squitter */
        TIS_B = 18,                  /* TIS-B */
        MilExtendedSquitter = 19,    /* Military Extended Squitter */
        CommB_AltReply = 20,         /* Comm-B, altitude reply */
        CommB_IdentReply = 21,       /* Comm-B, identity reply */
        CommD_ELM = 24               /* Comm-D ELM */
    };

    struct __attribute__((__packed__)) ModeSNew {
        uint32_t parity : 24;
        uint64_t data : 56;
        uint32_t icao_address : 24;
        uint8_t capability : 3;
        uint8_t downlink_format : 5;
    };
    static_assert(sizeof(ModeSNew) == 14);

    /* The struct we use to store information about a decoded message. */
    struct modesMessage
    {
        /* Generic fields */
        unsigned char msg[MODES_LONG_MSG_BYTES]; /* Binary message. */
        int msgbits;                             /* Number of bits in message */
        int msgtype;                             /* Downlink format # */
        int crcok;                               /* True if CRC was valid */
        uint32_t crc;                            /* Message CRC */
        int errorbit;                            /* Bit corrected. -1 if no bit corrected. */
        int aa1, aa2, aa3;                       /* ICAO Address bytes 1 2 and 3 */
        int phase_corrected;                     /* True if phase correction was applied. */

        /* DF 11 */
        int ca; /* Responder capabilities. */

        /* DF 17 */
        int metype; /* Extended squitter message type. */
        int mesub;  /* Extended squitter message subtype. */
        int heading_is_valid;
        int heading;
        int aircraft_type;
        int fflag;            /* 1 = Odd, 0 = Even CPR message. */
        int tflag;            /* UTC synchronized? */
        int raw_latitude;     /* Non decoded latitude */
        int raw_longitude;    /* Non decoded longitude */
        char flight[9];       /* 8 chars flight number. */
        int ew_dir;           /* 0 = East, 1 = West. */
        int ew_velocity;      /* E/W velocity. */
        int ns_dir;           /* 0 = North, 1 = South. */
        int ns_velocity;      /* N/S velocity. */
        int vert_rate_source; /* Vertical rate source. */
        int vert_rate_sign;   /* Vertical rate sign. */
        int vert_rate;        /* Vertical rate. */
        int velocity;         /* Computed from EW and NS velocity. */

        /* DF4, DF5, DF20, DF21 */
        int fs;       /* Flight status for DF4,5,20,21 */
        int dr;       /* Request extraction of downlink request. */
        int um;       /* Request extraction of downlink request. */
        int identity; /* 13 bits identity (Squawk). */

        /* Fields used by multiple message types. */
        int altitude, unit;
    };

    class ModeS
    {
    private:
        uint32_t icao_cache[MODES_ICAO_CACHE_LEN * 2];
        ssr::ads_b::System _sys;
        
        /* Parity table for MODE S Messages.
        * The table contains 112 elements, every element corresponds to a bit set
        * in the message, starting from the first bit of actual data after the
        * preamble.
        *
        * For messages of 112 bit, the whole table is used.
        * For messages of 56 bits only the last 56 elements are used.
        *
        * The algorithm is as simple as xoring all the elements in this table
        * for which the corresponding bit on the message is set to 1.
        *
        * The latest 24 elements in this table are set to 0 as the checksum at the
        * end of the message should not affect the computation.
        *
        * Note: this function can be used with DF11 and DF17, other modes have
        * the CRC xored with the sender address as they are reply to interrogations,
        * but a casual listener can't split the address from the checksum.
        */
        uint32_t modes_checksum_table[112] = {
            0x3935ea, 0x1c9af5, 0xf1b77e, 0x78dbbf, 0xc397db, 0x9e31e9, 0xb0e2f0, 0x587178,
            0x2c38bc, 0x161c5e, 0x0b0e2f, 0xfa7d13, 0x82c48d, 0xbe9842, 0x5f4c21, 0xd05c14,
            0x682e0a, 0x341705, 0xe5f186, 0x72f8c3, 0xc68665, 0x9cb936, 0x4e5c9b, 0xd8d449,
            0x939020, 0x49c810, 0x24e408, 0x127204, 0x093902, 0x049c81, 0xfdb444, 0x7eda22,
            0x3f6d11, 0xe04c8c, 0x702646, 0x381323, 0xe3f395, 0x8e03ce, 0x4701e7, 0xdc7af7,
            0x91c77f, 0xb719bb, 0xa476d9, 0xadc168, 0x56e0b4, 0x2b705a, 0x15b82d, 0xf52612,
            0x7a9309, 0xc2b380, 0x6159c0, 0x30ace0, 0x185670, 0x0c2b38, 0x06159c, 0x030ace,
            0x018567, 0xff38b7, 0x80665f, 0xbfc92b, 0xa01e91, 0xaff54c, 0x57faa6, 0x2bfd53,
            0xea04ad, 0x8af852, 0x457c29, 0xdd4410, 0x6ea208, 0x375104, 0x1ba882, 0x0dd441,
            0xf91024, 0x7c8812, 0x3e4409, 0xe0d800, 0x706c00, 0x383600, 0x1c1b00, 0x0e0d80,
            0x0706c0, 0x038360, 0x01c1b0, 0x00e0d8, 0x00706c, 0x003836, 0x001c1b, 0xfff409,
            0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
            0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
            0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000};

        uint32_t modesChecksum(unsigned char *msg, int bits);

        /* Given the Downlink Format (DF) of the message, return the message length
        * in bits. */
        int modesMessageLenByType(int type);

        /* Try to fix single bit errors using the checksum. On success modifies
        * the original buffer with the fixed version, and returns the position
        * of the error bit. Otherwise if fixing failed -1 is returned. */
        int fixSingleBitErrors(unsigned char *msg, int bits);

        /* Similar to fixSingleBitErrors() but try every possible two bit combination.
        * This is very slow and should be tried only against DF17 messages that
        * don't pass the checksum, and only in Aggressive Mode. */
        int fixTwoBitsErrors(unsigned char *msg, int bits);

        /* Hash the ICAO address to index our cache of MODES_ICAO_CACHE_LEN
        * elements, that is assumed to be a power of two. */
        uint32_t ICAOCacheHashAddress(uint32_t a);

        /* Add the specified entry to the cache of recently seen ICAO addresses.
        * Note that we also add a timestamp so that we can make sure that the
        * entry is only valid for MODES_ICAO_CACHE_TTL seconds. */
        void addRecentlySeenICAOAddr(uint32_t addr);

        /* Returns 1 if the specified ICAO address was seen in a DF format with
        * proper checksum (not xored with address) no more than * MODES_ICAO_CACHE_TTL
        * seconds ago. Otherwise returns 0. */
        int ICAOAddressWasRecentlySeen(uint32_t addr);

        /* If the message type has the checksum xored with the ICAO address, try to
        * brute force it using a list of recently seen ICAO addresses.
        *
        * Do this in a brute-force fashion by xoring the predicted CRC with
        * the address XOR checksum field in the message. This will recover the
        * address: if we found it in our cache, we can assume the message is ok.
        *
        * This function expects mm->msgtype and mm->msgbits to be correctly
        * populated by the caller.
        *
        * On success the correct ICAO address is stored in the modesMessage
        * structure in the aa3, aa2, and aa1 fiedls.
        *
        * If the function successfully recovers a message with a correct checksum
        * it returns 1. Otherwise 0 is returned. */
        int bruteForceAP(unsigned char *msg, struct modesMessage *mm);

        /* Decode the 13 bit AC altitude field (in DF 20 and others).
        * Returns the altitude, and set 'unit' to either MODES_UNIT_METERS
        * or MDOES_UNIT_FEETS. */
        int decodeAC13Field(unsigned char *msg, int *unit);

        /* Decode the 12 bit AC altitude field (in DF 17 and others).
        * Returns the altitude or 0 if it can't be decoded. */
        int decodeAC12Field(unsigned char *msg, int *unit);

        /* Decode a raw Mode S message demodulated as a stream of bytes by
        * detectModeS(), and split it into fields populating a modesMessage
        * structure. */
        void decodeModesMessage(struct modesMessage *mm, unsigned char *msg);

        /* Turn an hex digit into its 4 bit decimal value.
        * Returns -1 if the digit is not in the 0-F range. */
        int hexDigitVal(int c);

    public:
        /* This function decodes a string representing a Mode S message in
        * raw hex format like: *8D4B969699155600E87406F5B69F;
        * The string is supposed to be at the start of the client buffer
        * and null-terminated.
        */
        std::unique_ptr<struct modesMessage> decodeHexMessage(const std::string &line)
        {
            const char *hex = line.data();
            int l = line.length(), j;
            unsigned char msg[MODES_LONG_MSG_BYTES];
            struct modesMessage mm;

            /* Turn the message into binary. */
            if (l < 2 || hex[0] != '*' || hex[l - 1] != ';')
                return 0;
            hex++;
            l -= 2; /* Skip * and ; */
            if (l > MODES_LONG_MSG_BYTES * 2)
                return 0; /* Too long message... broken. */
            for (j = 0; j < l; j += 2)
            {
                int high = hexDigitVal(hex[j]);
                int low = hexDigitVal(hex[j + 1]);

                if (high == -1 || low == -1)
                    return 0;
                msg[j / 2] = (high << 4) | low;
            }

            decodeModesMessage(&mm, msg);

            return std::make_unique<struct modesMessage>(mm);
        }
    };
} // namespace ssr::decoder