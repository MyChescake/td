//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/files/FileBitmask.h"

#include "td/utils/common.h"
#include "td/utils/misc.h"

namespace td {

Bitmask::Bitmask(Decode, Slice data) : data_(zero_one_decode(data)) {
}

Bitmask::Bitmask(Ones, int64 count) : data_(narrow_cast<size_t>((count + 7) / 8), '\0') {
  for (int64 i = 0; i < count; i++) {
    set(i);
  }
}

std::string Bitmask::encode() const {
  // remove zeroes in the end to make encoding deterministic
  td::Slice data(data_);
  while (!data.empty() && data.back() == '\0') {
    data.remove_suffix(1);
  }
  return zero_one_encode(data);
}

int64 Bitmask::get_ready_prefix_size(int64 offset, int64 part_size, int64 file_size) const {
  if (offset < 0) {
    return 0;
  }
  auto offset_part = offset / part_size;
  auto ones = get_ready_parts(offset_part);
  if (ones == 0) {
    return 0;
  }
  auto ready_parts_end = (offset_part + ones) * part_size;
  if (file_size != 0 && ready_parts_end > file_size) {
    ready_parts_end = file_size;
    if (offset > file_size) {
      offset = file_size;
    }
  }
  auto res = ready_parts_end - offset;
  CHECK(res >= 0);
  return res;
}

int64 Bitmask::get_total_size(int64 part_size) const {
  int64 res = 0;
  for (int64 i = 0; i < size(); i++) {
    res += static_cast<int64>(get(i));
  }
  return res * part_size;
}

bool Bitmask::get(int64 offset_part) const {
  if (offset_part < 0) {
    return 0;
  }
  auto index = narrow_cast<size_t>(offset_part / 8);
  if (index >= data_.size()) {
    return 0;
  }
  return (static_cast<uint8>(data_[index]) & (1 << static_cast<int>(offset_part % 8))) != 0;
}

int64 Bitmask::get_ready_parts(int64 offset_part) const {
  int64 res = 0;
  while (get(offset_part + res)) {
    res++;
  }
  return res;
}

std::vector<int32> Bitmask::as_vector() const {
  std::vector<int32> res;
  auto size = narrow_cast<int32>(data_.size() * 8);
  for (int32 i = 0; i < size; i++) {
    if (get(i)) {
      res.push_back(i);
    }
  }
  return res;
}

void Bitmask::set(int64 offset_part) {
  CHECK(offset_part >= 0);
  auto need_size = narrow_cast<size_t>(offset_part / 8 + 1);
  if (need_size > data_.size()) {
    data_.resize(need_size, '\0');
  }
  data_[need_size - 1] |= (1 << (offset_part % 8));
}

int64 Bitmask::size() const {
  return static_cast<int64>(data_.size() * 8);
}

}  // namespace td
