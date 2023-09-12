#ifndef HITS_H
#define HITS_H

#include <stdint.h>

struct pixelhit {
    pixelhit(uint64_t h):hitdata(h){};
    uint64_t hitdata;
    uint32_t chipid(){return hitdata >> 48;}
    uint32_t pixelid(){return hitdata >> 32;}
    uint8_t col(){return (hitdata >> 40) & 0xFF;}
    uint8_t row(){return (hitdata >> 32) & 0xFF;}
    uint8_t tot(){return (hitdata >> 27) & 0x1F;}
    uint32_t time(){return hitdata & 0x3FFFFFF;}
};

struct fibrehit {
    fibrehit(uint64_t h):hitdata(h){};
    uint64_t hitdata;
    uint16_t channelid(){return hitdata >> 48;}
    uint16_t asic(){return (hitdata >> 53) & 0xFF;}
    uint8_t channel(){return (hitdata >>48) &0x1F;}
    bool eflag(){return (hitdata >> 47) & 0x1;}
    uint16_t eminust(){return (hitdata >> 32) & 0x7FF;}
    uint32_t time(){return hitdata & 0xFFFFFFFF;}
    uint32_t finetime(){return hitdata & 0x1F;}
};

struct tilehit {
    tilehit(uint64_t h):hitdata(h){};
    uint64_t hitdata;
    uint16_t channelid(){return hitdata >> 48;}
    uint16_t asic(){return (hitdata >> 53) & 0xFF;}
    uint8_t channel(){return (hitdata >>48) &0x1F;}
    bool eflag(){return (hitdata >> 47) & 0x1;}
    uint16_t eminust(){return (hitdata >> 32) & 0x7FF;}
    uint32_t time(){return hitdata & 0xFFFFFFFF;}
    uint32_t finetime(){return hitdata & 0x1F;}
};

#endif