#include "gtest/gtest.h"

#include "utils.hpp"


namespace keva {

class UtilsTest : public ::testing::Test {};

TEST_F(UtilsTest, ReadFileValue) {
  const FileValue uint8_chars = {-1};
  uint8_t uint8_expected = 255u;
  EXPECT_EQ(read_file_value<uint8_t>(uint8_chars), uint8_expected);

  const FileValue int8_chars = {127};
  int8_t int8_expected = 127;
  EXPECT_EQ(read_file_value<int8_t>(int8_chars), int8_expected);

  const FileValue uint16_chars = {-1, -1};
  uint16_t uint16_expected = 65535u;
  EXPECT_EQ(read_file_value<uint16_t>(uint16_chars), uint16_expected);

  const FileValue int16_chars = {-1, 127};
  int16_t int16_expected = 32767;
  EXPECT_EQ(read_file_value<int16_t>(int16_chars), int16_expected);

  const FileValue uint32_chars = {-1, -1, -1, -1};
  uint32_t uint32_expected = 4294967295u;
  EXPECT_EQ(read_file_value<uint32_t>(uint32_chars), uint32_expected);

  const FileValue int32_chars = {-1, -1, -1, 127};
  int32_t int32_expected = 2147483647;
  EXPECT_EQ(read_file_value<int32_t>(int32_chars), int32_expected);

  const FileValue uint64_chars = {-1, -1, -1, -1, -1, -1, -1, -1};
  uint64_t uint64_expected = 18446744073709551615u;
  EXPECT_EQ(read_file_value<uint64_t>(uint64_chars), uint64_expected);

  const FileValue int64_chars = {-1, -1, -1, -1, -1, -1, -1, 127};
  int64_t int64_expected = 9223372036854775807;
  EXPECT_EQ(read_file_value<int64_t>(int64_chars), int64_expected);

  const FileValue string_chars = {'a', 'b', 'c', 'd'};
  std::string string_expected = "abcd";
  EXPECT_EQ(read_file_value<std::string>(string_chars), string_expected);
}

TEST_F(UtilsTest, WriteFileValue) {
  uint8_t uint8_value = 255u;
  const FileValue uint8_chars_expected = {-1};
  EXPECT_EQ(write_file_value(uint8_value), uint8_chars_expected);

  int8_t int8_value = 127;
  const FileValue int8_chars_expected = {127};
  EXPECT_EQ(write_file_value(int8_value), int8_chars_expected);

  uint16_t uint16_value = 65535u;
  const FileValue uint16_chars_expected = {-1, -1};
  EXPECT_EQ(write_file_value(uint16_value), uint16_chars_expected);

  int16_t int16_value = 32767;
  const FileValue int16_chars_expected = {-1, 127};
  EXPECT_EQ(write_file_value(int16_value), int16_chars_expected);

  uint32_t uint32_value = 4294967295u;
  const FileValue uint32_chars_expected = {-1, -1, -1, -1};
  EXPECT_EQ(write_file_value(uint32_value), uint32_chars_expected);

  int32_t int32_value = 2147483647;
  const FileValue int32_chars_expected = {-1, -1, -1, 127};
  EXPECT_EQ(write_file_value(int32_value), int32_chars_expected);

  uint64_t uint64_value = 18446744073709551615u;
  const FileValue uint64_chars_expected = {-1, -1, -1, -1, -1, -1, -1, -1};
  EXPECT_EQ(write_file_value(uint64_value), uint64_chars_expected);

  int64_t int64_value = 9223372036854775807;
  const FileValue int64_chars_expected = {-1, -1, -1, -1, -1, -1, -1, 127};
  EXPECT_EQ(write_file_value(int64_value), int64_chars_expected);

  std::string string_value = "abcd";
  const FileValue string_chars_expected = {4, 0, 0, 0, 'a', 'b', 'c', 'd'}; // contains length
  EXPECT_EQ(write_file_value(string_value), string_chars_expected);
}


}  // namespace keva
