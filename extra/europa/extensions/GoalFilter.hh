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
#ifndef H_trex_europa_GoalFilter
# define H_trex_europa_GoalFilter

# include <trex/europa/config.hh>

// include plasma header as system files in order to disable warnings
# define TREX_PP_SYSTEM_FILE <FlawFilter.hh>
# include <trex/europa/bits/system_header.hh>

namespace TREX {
  namespace europa {
    
    class GoalFilter :public EUROPA::SOLVERS::FlawFilter {
    public:
      GoalFilter(EUROPA::TiXmlElement const &cfg);
      virtual ~GoalFilter() {}
      
    protected:
      virtual bool doTest(EUROPA::TokenId const &tok);
    private:
      bool test(EUROPA::EntityId const &entity);
    }; // TREX::europa::GoalFilter
    

    class NotGoalFilter :public GoalFilter {
    public:
      NotGoalFilter(EUROPA::TiXmlElement const &cfg)
        :GoalFilter(cfg) {}
      virtual ~NotGoalFilter() {}
      
    private:
      bool doTest(EUROPA::TokenId const &tok) {
        return !GoalFilter::doTest(tok);
      }
    }; // TREX::europa::GoalFilter

  
  } // TREX::europa
} // TREX 

#endif // H_trex_europa_GoalFilter
