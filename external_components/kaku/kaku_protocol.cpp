#include "kaku_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.kaku";

// Helper functions used by encode/decode --------------------------------------------------

static void send_bit(RemoteTransmitData *dst, uint32_t period, bool bit) {
  // bit '1' -> high T, low 5T, high T, low T
  // bit '0' -> high T, low T, high T, low 5T
  if (bit) {
    dst->item(period, period * 5);
    dst->item(period, period);
  } else {
    dst->item(period, period);
    dst->item(period, period * 5);
  }
}

static void send_dim_pattern(RemoteTransmitData *dst, uint32_t period) {
  // "dim" pattern: two short pulses
  dst->item(period, period);
  dst->item(period, period);
}

// Encode implementation ------------------------------------------------------------------

void KakuProtocol::encode(RemoteTransmitData *dst, const KakuData &data) {
    ESP_LOGI(TAG, "KakuProtocol::encode: addr=0x%08lX group=%d type=%d unit=%d dimpresent=%d dim=%d period=%u", 
           data.address, data.group ? 1 : 0, data.switchType, data.unit, data.dimLevelPresent ? 1 : 0,
           data.dimLevel, data.period);
  uint32_t period = data.period == 0 ? 260u : data.period;
  dst->set_carrier_frequency(0);

  // start pulse
  dst->item(period, period * 10 + period / 2);

  // address (26 bits, MSB first)
  for (int32_t i = 25; i >= 0; i--) {
    bool bit = (data.address >> i) & 0x1UL;
    send_bit(dst, period, bit);
  }

  // group bit
  send_bit(dst, period, data.group);

  // switch type / dim marker
  if (data.switchType == KakuData::dim) {
    send_dim_pattern(dst, period);
  } else {
    send_bit(dst, period, data.switchType == KakuData::on);
  }

  // unit (4 bits, MSB first)
  for (int32_t i = 3; i >= 0; i--) {
    bool bit = (data.unit >> i) & 0x1u;
    send_bit(dst, period, bit);
  }

  // optional dim level
  if (data.switchType == KakuData::dim || data.dimLevelPresent) {
    for (int32_t i = 3; i >= 0; i--) {
      bool bit = (data.dimLevel >> i) & 0x1u;
      send_bit(dst, period, bit);
    }
  }

  // stop pulse
  dst->item(period, period * 40);
}

// Decode implementation ------------------------------------------------------------------

optional<KakuData> KakuProtocol::decode(RemoteReceiveData src) {
  KakuData out{};
  out.period = 0;
  out.address = 0;
  out.group = false;
  out.switchType = KakuData::off;
  out.unit = 0;
  out.dimLevelPresent = false;
  out.dimLevel = 0;

  // we'll use a fixed nominal period; tolerance will take care of small variations
  const uint32_t T = 260u;

  // helper that looks ahead for a '1'/'0' bit and advances if found
  auto read_bit = [&](bool &bit) -> bool {
    if (!src.is_valid(3))
      return false;
    // look for '1' pattern
    if (src.peek_mark(T, 0) && src.peek_space(T * 5, 1) && src.peek_mark(T, 2) && src.peek_space(T, 3)) {
      bit = true;
      src.advance(4);
      return true;
    }
    // look for '0' pattern
    if (src.peek_mark(T, 0) && src.peek_space(T, 1) && src.peek_mark(T, 2) && src.peek_space(T * 5, 3)) {
      bit = false;
      src.advance(4);
      return true;
    }
    return false;
  };

  // header: start pulse
  if (!src.expect_pulse_with_gap(T, T * 10 + T / 2))
    return {};

  // compute period from first space if possible
  if (src.get_index() >= 1) {
    // header low should be ~10.5T
    int32_t header_space = std::abs(src.get_raw_data()[src.get_index() - 1]);
    if (header_space > 0)
      out.period = uint32_t(header_space / 10.5);
  }

  // address bits
  for (int i = 0; i < 26; i++) {
    bool b;
    if (!read_bit(b))
      return {};
    out.address = (out.address << 1) | (b ? 1UL : 0UL);
  }

  // group bit
  {
    bool b;
    if (!read_bit(b))
      return {};
    out.group = b;
  }

  // check for dim pattern (two short pulses)
  if (src.peek_mark(T, 0) && src.peek_space(T, 1) && src.peek_mark(T, 2) && src.peek_space(T, 3)) {
    out.switchType = KakuData::dim;
    out.dimLevelPresent = true;
    src.advance(4);
  } else {
    bool b;
    if (!read_bit(b))
      return {};
    out.switchType = b ? KakuData::on : KakuData::off;
  }

  // unit bits
  for (int i = 0; i < 4; i++) {
    bool b;
    if (!read_bit(b))
      return {};
    out.unit = (out.unit << 1) | (b ? 1u : 0u);
  }

  // optional dim level bits (only if we recognized dim or space remains)
  if (out.switchType == KakuData::dim || src.is_valid(3)) {
    for (int i = 0; i < 4 && src.is_valid(3); i++) {
      bool b;
      if (!read_bit(b))
        break;
      out.dimLevel = (out.dimLevel << 1) | (b ? 1u : 0u);
      out.dimLevelPresent = true;
    }
  }

  return out;
}

// Dump implementation --------------------------------------------------------------------

void KakuProtocol::dump(const KakuData &data) {
  ESP_LOGI(TAG, "Received KAKU: addr=0x%08lX group=%d type=%d unit=%d dimpresent=%d dim=%d period=%u", 
           data.address, data.group ? 1 : 0, data.switchType, data.unit, data.dimLevelPresent ? 1 : 0,
           data.dimLevel, data.period);
}

}  // namespace remote_base
}  // namespace esphome
