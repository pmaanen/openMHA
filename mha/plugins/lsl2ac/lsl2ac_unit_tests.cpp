// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.


#include "lsl2ac.hh"
#include <gtest/gtest.h>
#include "lsl_cpp.h"
#include "mha_algo_comm.hh"

#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <random>
using namespace std::chrono_literals;

constexpr int NCHUNKS=12;
constexpr int NCHANNELS=12;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;

namespace{
  std::string to_string(lsl2ac::overrun_behavior ob_){
    if(ob_==lsl2ac::overrun_behavior::Discard)
      return "Discard";
    else if(ob_==lsl2ac::overrun_behavior::Ignore)
      return "Ignore";
    else
      throw std::invalid_argument("Unknown overrun behavior!");
  }
  std::string random_string(){
    std::string s(7,'a');
    std::random_device r;
    std::mt19937 g(r());
    std::uniform_int_distribution<> d('a', 'z'); // Generate random integers from int('a') to int('z')
    for (char & c : s) c = static_cast<char>(d(g));
    return s;
  }
}
class Test_save_var_t : public TestWithParam<::std::tuple<lsl2ac::overrun_behavior,int,int> > {
protected:
  Test_save_var_t():
    /*  We need to use a unique name for the stream as sometimes the lsl background process does not close the
     steam timely even if our stream_outlet is already destroyed. Append random characters to ensure uniqueness
     among concurrently running tests.
    */
    name([](){auto test_info=testing::UnitTest::GetInstance()->current_test_info();
        return std::string(test_info->test_suite_name())+"."+std::string(test_info->name())+"_"+random_string();}()),
    info(name,"Audio",NCHANNELS,lsl::IRREGULAR_RATE,lsl::cf_float32,name),
    acspace(),
    ac(acspace.get_c_handle())
  {
    expected.resize(NCHUNKS);
    int i=0;
    for(auto& vec : expected){
      vec.resize(NCHANNELS);
      std::generate(vec.begin(), vec.end(), [&i](){ return i++;});
    }
    outlet_thread=std::thread([this](){
                                {
                                  lsl::stream_outlet outlet(info);
                                  outlet_ready=true;
                                  outlet.wait_for_consumers(2/*s*/);
                                  for(const auto& vec : expected)
                                    outlet.push_sample(vec);
                                  // Short sleep needed for the lsl buffers to sort themselves out
                                  std::this_thread::sleep_for(0.01s);
                                  outlet_done=true;
                                  while(!stop_request){
                                    std::this_thread::sleep_for(0.01s);
                                  }
                                }
                              });
    while(!outlet_ready)
      std::this_thread::sleep_for(0.01s);
    lsl2ac::overrun_behavior ob;
    int nchannels;
    int nsamples;
    std::tie(ob,nchannels,nsamples)=GetParam();
    // Resolving and using that info is much faster than reusing the info from the outlet construction
    auto infos=lsl::resolve_stream("name",name);
    // Lambda trickery needed b/c fatal ASSERTions can only be placed in
    // void-returning functions. C'tors and D'tors do not count as void-returning. See also
    // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#assertion-placement
    [&infos]() { ASSERT_FALSE(infos.empty());
    }();
    info=infos[0];
    var =  std::make_unique<lsl2ac::save_var_t>(info, ac, ob, 1, 1, nchannels,
                                             nsamples);
  }
  ~Test_save_var_t(){
    var.reset();
    stop_request=true;
    outlet_thread.join();
  }
  virtual void SetUp() override {
    while(!outlet_done)
      std::this_thread::sleep_for(0.01s);
  }
  // Creates an outlet stream. Sends the samples as fast as possible once a
  // consumer connects. Synchronization through the atomic bools.
  void open_stream();
  void TearDown() override {}
  std::atomic<bool> outlet_ready{false};
  std::atomic<bool> outlet_done{false};
  std::atomic<bool> stop_request{false};
  std::thread outlet_thread;
  std::string name;
  lsl::stream_info info;
  MHAKernel::algo_comm_class_t acspace;
  algo_comm_t ac;
  std::vector<std::vector<float>> expected;
  std::unique_ptr<lsl2ac::save_var_t> var;
};

using Test_save_var_t_discard=Test_save_var_t;
TEST_P(Test_save_var_t_discard,OverrunBehavior){
  var->receive_frame();
  auto buf=MHA_AC::get_var_waveform(ac,name).buf;
  std::vector<float> actual(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);
}

using Test_save_var_t_ignore=Test_save_var_t;
TEST_P(Test_save_var_t_ignore,OverrunBehavior){
  var->receive_frame();
  auto buf=MHA_AC::get_var_waveform(ac,name).buf;
  std::vector<float> actual(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.front(),actual);
}

TEST_P(Test_save_var_t,ResizeVariable){
  // Empty buffer
  var->receive_frame();
  // Try to pull next, we should see empty
  var->receive_frame();
  auto ac_var=MHA_AC::get_var_waveform(ac,name);
  EXPECT_EQ(0U,(unsigned)ac_var.num_frames);
}

using Test_save_var_t_fixed_size=Test_save_var_t;
TEST_P(Test_save_var_t_fixed_size,ThrowOnWrongChannelNo){
  lsl2ac::overrun_behavior ob;
  int nchannels;
  std::tie(ob, nchannels, std::ignore) = GetParam();
  /// Try to construct new save_var_t with wrong number of channels
  EXPECT_THROW(var.reset(new lsl2ac::save_var_t(info,ac,ob,1,1,nchannels+1,0)),MHA_Error);
}

INSTANTIATE_TEST_SUITE_P(ThrowOnResize,Test_save_var_t_fixed_size,
                         Combine(Values(lsl2ac::overrun_behavior::Discard),
                                 Values(12),
                                 Values(1)),
                         [](const testing::TestParamInfo<Test_save_var_t::ParamType>& info) {
                           std::string name = to_string(std::get<0>(info.param));
                           return name;
                         });

INSTANTIATE_TEST_SUITE_P(ThrowOnWrongChannelNo,Test_save_var_t,
                         Combine(Values(lsl2ac::overrun_behavior::Discard),
                                 Values(12),
                                 Values(0)),
                         [](const testing::TestParamInfo<Test_save_var_t::ParamType>& info) {
                           std::string name = to_string(std::get<0>(info.param));
                           return name;
                         });

INSTANTIATE_TEST_SUITE_P(DiscardOverrun,Test_save_var_t_discard,
                         Combine(Values(lsl2ac::overrun_behavior::Discard),
                                 Values(12),Values(0)),
                         [](const testing::TestParamInfo<Test_save_var_t::ParamType>& info) {
                           std::string name = to_string(std::get<0>(info.param));
                           return name;
                         });

INSTANTIATE_TEST_SUITE_P(IgnoreOverrun,Test_save_var_t_ignore,
                         Combine(Values(lsl2ac::overrun_behavior::Ignore),
                                 Values(12),
                                 Values(0)),
                         [](const testing::TestParamInfo<Test_save_var_t::ParamType>& info) {
                           std::string name = to_string(std::get<0>(info.param));
                           return name;
                         });
