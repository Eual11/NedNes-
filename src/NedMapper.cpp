#include "../include/NedMapper.h"

namespace NedNes {
NedMapper::NedMapper(unsigned int npgr, unsigned int nchr) {
  nPGRBanks = npgr;
  nCHRBanks = nchr;
}
} // namespace NedNes
