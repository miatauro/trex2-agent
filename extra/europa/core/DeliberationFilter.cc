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
#include "trex/europa/DeliberationFilter.hh"
#include "trex/europa/Assembly.hh"
#include "trex/europa/bits/europa_helpers.hh"

// include plasma header as system files in order to disable warnings
# define TREX_PP_SYSTEM_FILE <TokenVariable.hh>
# include <trex/europa/bits/system_header.hh>

using namespace TREX::europa;

/*
 * class TREX::europa::TokenFilter
 */
// structors

TokenFilter::TokenFilter(EUROPA::TiXmlElement const &cfg)
:EUROPA::SOLVERS::FlawFilter(cfg, true), m_assembly(NULL) {}

// observers

Assembly const &TokenFilter::assembly() const {
  if( !have_assembly() )
    throw EuropaException("TokenFilter not attached to a TREX assembly.");
  return *m_assembly;
}

// modifiers

void TokenFilter::set_assembly(EUROPA::EngineComponentId const &component) {
  if( !have_assembly() && component.isId() )
    m_assembly = &(details::assembly_of(component));
}

// manipulators

bool TokenFilter::tokenCheck(EUROPA::TokenId const &token) {
  return assembly().ignored(token) ||
  token->start()->lastDomain().getLowerBound() >= assembly().final_tick() ||
  token->end()->lastDomain().getUpperBound() <= assembly().initial_tick();
}

bool TokenFilter::test(EUROPA::EntityId const &entity) {
  EUROPA::TokenId token;
  
  if( EUROPA::TokenId::convertable(entity)  ) {
    token = EUROPA::TokenId(entity);
    set_assembly(token->getPlanDatabase());
  } else if( EUROPA::ConstrainedVariableId::convertable(entity) ) {
    EUROPA::ConstrainedVariableId var(entity);
    set_assembly(var->getConstraintEngine());
    
    token = details::parent_token(var);
  }
  
  return token.isNoId() || tokenCheck(token) || doTest(token);
}

/*
 * class TREX::europa::DeliberationScope
 */

bool DeliberationScope::doTest(EUROPA::TokenId const &tok) {
  EUROPA::IntervalIntDomain const &t_start(tok->start()->lastDomain());
  EUROPA::IntervalIntDomain const &t_end(tok->end()->lastDomain());
  EUROPA::IntervalIntDomain horizon = assembly().plan_scope();
  EUROPA::eint initial = assembly().initial_tick();
  
  if( t_start.getLowerBound() >= horizon.getUpperBound() ) {
    debugMsg("trex:filt:delib", "Exclude: token("<<tok<<") starts "<<t_start.toString()
             <<" after horizon "<<horizon.toString());
    return true;
  } else if( t_end.getUpperBound() < horizon.getLowerBound() ) {
    debugMsg("trex:filt:delib", "Exclude: token("<<tok<<") ends "<<t_end.toString()<<" before horizon "<<horizon.toString());
    return true;
  } else if( t_end.getUpperBound() <= initial ) {
    debugMsg("trex:filt:delib", "Exclude: token("<<tok<<") ends "<<t_end.toString()<<" before initial "<<initial);
    return true;
  } else
    return false;
}

/*
 * class TREX::europa::SynchronizationScope
 */

bool SynchronizationScope::doTest(EUROPA::TokenId const &tok) {
  EUROPA::eint cur = assembly().now(),
  initial = assembly().initial_tick();
  
  // debugMsg("trex:filt:synch", "Checking if "<<tok->toString()<<" overlaps "<<cur);
  
  if( tok->start()->lastDomain().getLowerBound() > cur ) {
    debugMsg("trex:filt:synch", tok->toString()<<".start="<<tok->start()->lastDomain().toString()
             <<" after "<<cur<<" => EXCLUDE");
    return true;
  }
  
  EUROPA::eint max_end = tok->end()->lastDomain().getUpperBound();
  
  if( max_end <= initial || max_end < (tok->isFact()?cur:(cur+1)) ) {
    debugMsg("trex:filt:synch", tok->toString()<<".end="
             <<tok->end()->lastDomain().toString()
             <<" before "<<cur<<" => EXCLUDE");
    return true;
  }
  //  debugMsg("trex:filt:synch", tok->toString()<<" overlaps "<<cur<<" => OK");
  return false;
}


