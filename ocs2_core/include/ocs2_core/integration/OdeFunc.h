/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <functional>

#include "OdeBase.h"

namespace ocs2 {

template <int STATE_DIM>
class OdeFunc final : public OdeBase<STATE_DIM> {
 public:
  using BASE = OdeBase<STATE_DIM>;
  using typename BASE::scalar_t;
  using typename BASE::state_vector_t;

  explicit OdeFunc(std::function<void(const scalar_t& t, const state_vector_t& x, state_vector_t& dxdt)> flowMap)
      : flowMap_(std::move(flowMap)){};

  void computeFlowMap(const scalar_t& t, const state_vector_t& x, state_vector_t& dxdt) override { flowMap_(t, x, dxdt); };

  void setFlowMap(std::function<void(const scalar_t& t, const state_vector_t& x, state_vector_t& dxdt)>&& flowMap) { flowMap_ = flowMap; };

 private:
  std::function<void(const scalar_t& t, const state_vector_t& x, state_vector_t& dxdt)> flowMap_;
};
}  // namespace ocs2
