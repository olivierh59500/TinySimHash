#include <gtest/gtest.h>
#include "../src/simhash.hpp"

using namespace std;

class SimHashModule : public ::testing::Test{
protected:
  void SetUp(){ 
  }
  void TearDown(){
  }
  SimHash sh;
};


TEST_F(SimHashModule, split_string_case){
  string str = "123 456";
  vector<string> ret = sh.split_string(str, " ");
  EXPECT_EQ("123", ret[0]);
  EXPECT_EQ("456", ret[1]);
}

TEST_F(SimHashModule, split_string_case_empty_string){
  string str = " ";
  vector<string> ret = sh.split_string(str, " ");
  EXPECT_EQ(2, ret.size());
  EXPECT_EQ("", ret[0]);
  EXPECT_EQ("", ret[1]);
}

TEST_F(SimHashModule, split_string_case_no_delimiter){
  string str = "1234567890";
  vector<string> ret = sh.split_string(str, ":");
  EXPECT_EQ("1234567890", ret[0]);
}

TEST_F(SimHashModule, set_one_data_case){
  string str = "1 1:1 2:2 3:3";
  sh.set_one_data(str);
  EXPECT_EQ(1, sh.get_feature(1, 1));
  EXPECT_EQ(2, sh.get_feature(1, 2));
  EXPECT_EQ(3, sh.get_feature(1, 3));
}

TEST_F(SimHashModule, bit_shuffle){
  EXPECT_EQ(68, sh.bit_shuffle(21, 3, 5));
}


int main(int argc, char **argv){
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

