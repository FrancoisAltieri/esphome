#pragma once

#include "esphome/components/remote_base/remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

struct KakuData {
  unsigned int period;          // measured period (microseconds), may be 0 if unknown
  unsigned long address;        // 26-bit address
  bool group;                   // group bit
  enum SwitchType { off = 0, on = 1, dim = 2 } switchType;
  uint8_t unit;                 // 4-bit unit
  bool dimLevelPresent;         // whether dimLevel field is valid
  uint8_t dimLevel;             // 4-bit dim level

  bool operator==(const KakuData &rhs) const {
  return address == rhs.address &&
         group == rhs.group &&
         switchType == rhs.switchType &&
         unit == rhs.unit;
  }
};

class KakuProtocol : public RemoteProtocol<KakuData> {
 public:
  void encode(RemoteTransmitData *dst, const KakuData &data) override;
  optional<KakuData> decode(RemoteReceiveData src) override;
  void dump(const KakuData &data) override;
};

DECLARE_REMOTE_PROTOCOL(Kaku)

template<typename... Ts> class KakuAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(unsigned int, period)
  TEMPLATABLE_VALUE(unsigned long, address)
  TEMPLATABLE_VALUE(bool, group)
  TEMPLATABLE_VALUE(uint8_t, switch_type)
  TEMPLATABLE_VALUE(uint8_t, unit)
  TEMPLATABLE_VALUE(bool, dim_level_present)
  TEMPLATABLE_VALUE(uint8_t, dim_level)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    KakuData data{};
    data.period = this->period_.value_or(x..., 0);
    data.address = this->address_.value(x...);
    data.group = this->group_.value(x...);
    data.switchType = static_cast<KakuData::SwitchType>(this->switch_type_.value(x...));
    data.unit = this->unit_.value_or(x..., 0);
    data.dimLevelPresent = this->dim_level_present_.value_or(x..., false);
    data.dimLevel = this->dim_level_.value_or(x..., 0);
    KakuProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
