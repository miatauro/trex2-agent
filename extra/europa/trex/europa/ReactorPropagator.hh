/*********************************************************************
 * Software License Agreement (BSD License)
 * 
 *  Copyright (c) 2011, MBARI.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 * 
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TREX Project nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef H_trex_europa_ReactorPropagator 
# define H_trex_europa_ReactorPropagator

# include "config.hh"

// include plasma header as system files in order to disable warnings
# define TREX_PP_SYSTEM_FILE <Propagators.hh>
# include <trex/europa/bits/system_header.hh>

namespace TREX {
  namespace europa {

    class Assembly;
    class ReactorConstraint;

    /** @brief Europa propagator with access to T-REX
     *
     * This class provide a simple europa propagator that allow its clients to
     * easily access to the Assembly that created it and by doing so extract 
     * information from the reactor that executes this europa solver such as 
     * AgentTimelines' type in T-REX or current tick, timeline latencies and 
     * look-ahead, ...
     *
     * @ingroup europa
     * @author Frederic Py <fpy@mbari.org>
     */
    class ReactorPropagator :public EUROPA::DefaultPropagator {
    public:
      /** @brief Destructor */
      virtual ~ReactorPropagator() {}
      
    protected:
      /** @brief Constructor
       *
       * @param[in] assembly The assebmly that created this instance
       * @param[in] name The symbolic name associated to this instance
       * @param[in] cstrEngine The constraint engine 
       */
      ReactorPropagator(Assembly &assembly, EUROPA::LabelStr const &name, 
			EUROPA::ConstraintEngineId const &cstrEngine)
	:EUROPA::DefaultPropagator(name, cstrEngine), m_assembly(assembly) {}

      Assembly &m_assembly;

      friend class Assembly;
      friend class ReactorConstraint;
    }; // TREX::europa::ReactorPropagator

  } // TREX::europa
} // TREX

#endif // H_trex_europa_ReactorPropagator
