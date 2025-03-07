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


/*
 * This plugin is a skeleton code for using matlabcoder 
 */

#include "mha_plugin.hh"
namespace matlab{
  #include "process.c"
  #include "process_emxAPI.c"
  #include "process_emxutil.c"
}

/** This C++ class is an interface between matlab coder 
 * generated C structs and MHA. 
 * This plugin is only meant for waveform domain.
 * It inherits from MHAPlugin::plugin_t for correct integration 
 * in the configuration language interface.  */
class matlabcoder_skeleton_t : public MHAPlugin::plugin_t<int> {
  matlab::emxArray_real_T *matlab_input = nullptr;
  matlab::emxArray_real_T *matlab_output = nullptr;
public:
  /** The constructor has to take these three
   * arguments, but it does not have to use them.
   * process_initialize from namespace matlab is called in the construtor*/
  matlabcoder_skeleton_t(algo_comm_t & ac,
            const std::string&,
            const std::string&)
      : MHAPlugin::plugin_t<int>("",ac)
  {matlab::process_initialize();}
  /** Destructor which calls process_terminate from namespace matlab  */
  ~matlabcoder_skeleton_t()
  {matlab::process_terminate();}

  void release(void)
  {
    matlab::emxDestroyArray_real_T(matlab_input); matlab_input = nullptr;
    matlab::emxDestroyArray_real_T(matlab_output); matlab_output = nullptr;
  }

  /** Plugin preparation. This checks that the input signal has the
   * waveform domain and contains at least one channel.
   * @param signal_info 
   *   Structure containing a description of the form of the signal (domain,
   *   number of channels, frames per block, sampling rate.
   */
  void prepare(mhaconfig_t & signal_info)
  {
    if (signal_info.domain != MHA_WAVEFORM)
      throw MHA_Error(__FILE__, __LINE__,
                      "This plugin can only process waveform signals.");
    if (signal_info.channels < 1)
      throw MHA_Error(__FILE__,__LINE__,
                      "This plugin requires at least one input channel.");
    matlab_input = matlab::emxCreate_real_T(signal_info.fragsize, signal_info.channels);
    matlab_output = matlab::emxCreate_real_T(signal_info.fragsize, signal_info.channels);
  }

  /** Signal processing performed by the plugin.  
   * switches left with right channels.
   * @param signal
   *   Pointer to the input signal structure.
   * @return
   *   Returns a pointer to the input signal structure,
   *   the signal modified in-place.
   */
  mha_wave_t * process(mha_wave_t * signal)
  {
    // copy input samples to autogenerated code
    for (unsigned sample = 0; sample < signal->num_frames; ++sample)
      for (unsigned ch = 0; ch < signal->num_channels; ++ch) {
        matlab_input->data[sample + ch * signal->num_frames] =
          static_cast<double>(value(signal,sample,ch));
      }
    matlab::process(matlab_input, matlab_output);

        // ensure the matlab code has not changed the matrix sizes
    if (signal->num_frames   != static_cast<unsigned>(matlab_input->size[0]) ||
        signal->num_channels != static_cast<unsigned>(matlab_input->size[1]) ||
        signal->num_frames   != static_cast<unsigned>(matlab_output->size[0]) ||
        signal->num_channels != static_cast<unsigned>(matlab_output->size[1]))
      throw MHA_Error(__FILE__, __LINE__, "Misbehaving code translated from "
                      "matlab changes matrix sizes during processing");
    
    // copy output samples from autogenerated code
    for (unsigned sample = 0; sample < signal->num_frames; ++sample)
      for (unsigned ch = 0; ch < signal->num_channels; ++ch)
        value(signal,sample,ch) = static_cast<float>
          (matlab_output->data[sample + ch * signal->num_frames]);

    // Algorithms may process data in-place and return the input signal
    // structure as their output signal:
    return signal;
  }

};

/*
 * This macro connects the matlabcoder_skeleton_t class with the \mha plugin C interface
 * The first argument is the class name, the other arguments define the 
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(matlabcoder_skeleton, matlabcoder_skeleton_t, wave, wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(matlabcoder_skeleton,
 "example audio-channels matlab",
 "Demo plugin for including signal processing code generated by Matlab"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
